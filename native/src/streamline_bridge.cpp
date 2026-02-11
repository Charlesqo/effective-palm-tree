#include "snake/streamline_bridge.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace snake {
namespace {
float presetScale(DLSSPreset preset) {
  switch (preset) {
    case DLSSPreset::Quality:
      return 1.0f / 1.5f;
    case DLSSPreset::Balanced:
      return 1.0f / 1.7f;
    case DLSSPreset::Performance:
      return 1.0f / 2.0f;
    case DLSSPreset::UltraPerformance:
      return 1.0f / 3.0f;
  }
  return 1.0f;
}

float clampRatio(float value) {
  return std::clamp(value, 0.1f, 1.0f);
}

int alignEven(int value) {
  return std::max(2, value - (value % 2));
}

int clampDimension(int value, int min, int max) {
  return std::clamp(value, std::max(2, min), std::max(2, max));
}

DLSSPreset dynamicPresetForFrameTime(double smoothedFrameTimeMs) {
  if (smoothedFrameTimeMs >= 32.0) {
    return DLSSPreset::UltraPerformance;
  }
  if (smoothedFrameTimeMs >= 24.0) {
    return DLSSPreset::Performance;
  }
  if (smoothedFrameTimeMs >= 18.0) {
    return DLSSPreset::Balanced;
  }
  return DLSSPreset::Quality;
}
}  // namespace

const char* toString(DLSSMode mode) {
  switch (mode) {
    case DLSSMode::Off:
      return "off";
    case DLSSMode::Auto:
      return "auto";
    case DLSSMode::On:
      return "on";
  }
  return "unknown";
}

const char* toString(DLSSPreset preset) {
  switch (preset) {
    case DLSSPreset::Quality:
      return "quality";
    case DLSSPreset::Balanced:
      return "balanced";
    case DLSSPreset::Performance:
      return "performance";
    case DLSSPreset::UltraPerformance:
      return "ultra-performance";
  }
  return "unknown";
}

bool StreamlineBridge::initialize(const StreamlineConfig& config) {
  initialized_ = false;
  available_ = false;
  reason_.clear();
  backend_ = UpscalerBackend::None;
  resetStats();

  if (config.outputWidth <= 0 || config.outputHeight <= 0 || config.renderWidth <= 0 ||
      config.renderHeight <= 0) {
    reason_ = "Invalid render/output resolution";
    return false;
  }

  outputRes_ = {config.outputWidth, config.outputHeight};
  renderRes_ = {config.renderWidth, config.renderHeight};
  mode_ = config.mode;
  preset_ = config.preset;
  frameGenEnabled_ = config.enableFrameGeneration;

  if (!config.enableDLSS && !config.enableFrameGeneration) {
    reason_ = "DLSS/FG disabled by config";
    initialized_ = true;
    return true;
  }

#ifdef _WIN32
  available_ = config.enableDLSS;
  backend_ = available_ ? UpscalerBackend::DLSS : UpscalerBackend::None;
  reason_ = available_ ? "" : "DLSS requested but unavailable on this adapter/runtime";
#else
  available_ = false;
  backend_ = UpscalerBackend::None;
  reason_ = "Streamline DLSS runtime requires Windows DX12/Vulkan pipeline";
#endif

  initialized_ = true;
  return available_;
}

void StreamlineBridge::setMode(DLSSMode mode) { mode_ = mode; }

void StreamlineBridge::setPreset(DLSSPreset preset) { preset_ = preset; }

void StreamlineBridge::setAutoPolicy(const AutoPolicy& policy) {
  autoPolicy_ = policy;
  autoPolicy_.offThresholdMs = std::max(1.0, autoPolicy_.offThresholdMs);
  autoPolicy_.onThresholdMs = std::max(autoPolicy_.offThresholdMs, autoPolicy_.onThresholdMs);
  autoPolicy_.smoothingWindowFrames = std::max(1, autoPolicy_.smoothingWindowFrames);

  if (static_cast<int>(frameTimes_.size()) > autoPolicy_.smoothingWindowFrames) {
    frameTimes_.erase(frameTimes_.begin(),
                      frameTimes_.begin() + (frameTimes_.size() - autoPolicy_.smoothingWindowFrames));
  }
}

double StreamlineBridge::pushFrameTime(double frameTimeMs) {
  const double clamped = std::max(0.1, frameTimeMs);
  frameTimes_.push_back(clamped);
  while (static_cast<int>(frameTimes_.size()) > autoPolicy_.smoothingWindowFrames) {
    frameTimes_.pop_front();
  }

  const double sum = std::accumulate(frameTimes_.begin(), frameTimes_.end(), 0.0);
  return sum / static_cast<double>(std::max<std::size_t>(1, frameTimes_.size()));
}

DLSSMode StreamlineBridge::resolveEffectiveMode(double smoothedFrameTimeMs) {
  if (mode_ != DLSSMode::Auto) {
    return mode_;
  }

  const DLSSMode previous = stats_.lastEffectiveMode;
  if (smoothedFrameTimeMs >= autoPolicy_.onThresholdMs) {
    return DLSSMode::On;
  }
  if (smoothedFrameTimeMs <= autoPolicy_.offThresholdMs) {
    return DLSSMode::Off;
  }

  if (previous == DLSSMode::On || previous == DLSSMode::Off) {
    return previous;
  }
  return DLSSMode::Off;
}

DLSSPreset StreamlineBridge::resolveEffectivePreset(double smoothedFrameTimeMs,
                                                    DLSSMode effectiveMode) const {
  if (effectiveMode == DLSSMode::Off) {
    return DLSSPreset::Quality;
  }

  if (!autoPolicy_.dynamicPreset) {
    return preset_;
  }
  return dynamicPresetForFrameTime(smoothedFrameTimeMs);
}

RenderResolution StreamlineBridge::recommendedInternalResolutionFor(DLSSMode effectiveMode,
                                                                    DLSSPreset effectivePreset) const {
  if (!initialized_ || !available_ || effectiveMode == DLSSMode::Off) {
    return outputRes_;
  }

  const float scale = presetScale(effectivePreset);
  const int scaledWidth = alignEven(static_cast<int>(std::round(static_cast<float>(outputRes_.width) * scale)));
  const int scaledHeight = alignEven(static_cast<int>(std::round(static_cast<float>(outputRes_.height) * scale)));

  return {
      clampDimension(scaledWidth, renderRes_.width, outputRes_.width),
      clampDimension(scaledHeight, renderRes_.height, outputRes_.height),
  };
}

RenderResolution StreamlineBridge::recommendedInternalResolution() const {
  return recommendedInternalResolutionFor(mode_, preset_);
}

std::optional<std::string> StreamlineBridge::validateFrameInputsForMode(
    const StreamlineFrameInputs& inputs, DLSSMode effectiveMode) const {
  if (!initialized_) {
    return "StreamlineBridge not initialized";
  }

  if (effectiveMode == DLSSMode::Off || !available_) {
    return std::nullopt;
  }

  if (!inputs.hasColor) return "Missing color input";
  if (!inputs.hasDepth) return "Missing depth input";
  if (!inputs.hasMotionVectors) return "Missing motion vectors input";
  if (!inputs.hasExposure) return "Missing exposure input";

  return std::nullopt;
}

std::optional<std::string> StreamlineBridge::validateFrameInputs(
    const StreamlineFrameInputs& inputs) const {
  return validateFrameInputsForMode(inputs, mode_);
}

FrameInfo StreamlineBridge::evaluate(double frameTimeMs, const StreamlineFrameInputs& inputs) {
  FrameInfo info{};
  info.frameTimeMs = frameTimeMs;
  info.outputResolution = outputRes_;
  info.internalResolution = outputRes_;
  info.effectiveMode = DLSSMode::Off;
  info.effectivePreset = DLSSPreset::Quality;
  info.upscaleRatio = 1.0f;

  if (!initialized_) {
    info.note = "bridge not initialized";
    return info;
  }

  info.smoothedFrameTimeMs = pushFrameTime(frameTimeMs);
  const DLSSMode effectiveMode = resolveEffectiveMode(info.smoothedFrameTimeMs);
  const DLSSPreset effectivePreset = resolveEffectivePreset(info.smoothedFrameTimeMs, effectiveMode);
  info.effectiveMode = effectiveMode;
  info.effectivePreset = effectivePreset;

  const auto validationError = validateFrameInputsForMode(inputs, effectiveMode);
  const bool canRunDLSS = available_ && effectiveMode != DLSSMode::Off && !validationError.has_value();

  info.dlssActive = canRunDLSS;
  info.frameGenActive = canRunDLSS && frameGenEnabled_ && effectiveMode == DLSSMode::On;
  info.internalResolution =
      canRunDLSS ? recommendedInternalResolutionFor(effectiveMode, effectivePreset) : outputRes_;

  info.upscaleRatio = clampRatio(static_cast<float>(info.internalResolution.width) /
                                 static_cast<float>(std::max(1, info.outputResolution.width)));

  if (validationError.has_value()) {
    info.note = *validationError;
  } else if (!available_) {
    info.note = reason_;
  } else if (effectiveMode == DLSSMode::Off) {
    info.note = "DLSS mode off";
  } else if (mode_ == DLSSMode::Auto) {
    info.note = "DLSS auto mode selected";
  } else {
    info.note = "DLSS path ready";
  }

  stats_.totalFrames += 1;
  if (info.dlssActive) {
    stats_.dlssFrames += 1;
  }
  if (info.frameGenActive) {
    stats_.frameGenFrames += 1;
  }
  if (validationError.has_value()) {
    stats_.validationFailures += 1;
  }
  if (mode_ == DLSSMode::Auto && stats_.lastEffectiveMode != effectiveMode) {
    stats_.autoModeSwitches += 1;
  }
  stats_.lastEffectiveMode = effectiveMode;
  stats_.averageFrameTimeMs =
      ((stats_.averageFrameTimeMs * static_cast<double>(stats_.totalFrames - 1)) + info.frameTimeMs) /
      static_cast<double>(stats_.totalFrames);
  stats_.lastNote = info.note;

  return info;
}

RuntimeStats StreamlineBridge::stats() const { return stats_; }

void StreamlineBridge::resetStats() {
  stats_ = RuntimeStats{};
  frameTimes_.clear();
}

std::vector<double> StreamlineBridge::recentFrameTimes() const {
  return {frameTimes_.begin(), frameTimes_.end()};
}

}  // namespace snake
