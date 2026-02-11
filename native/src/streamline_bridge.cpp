#include "snake/streamline_bridge.hpp"

#include <algorithm>
#include <cmath>

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

int alignEven(int value) {
  return std::max(2, value - (value % 2));
}
}  // namespace

bool StreamlineBridge::initialize(const StreamlineConfig& config) {
  initialized_ = false;
  available_ = false;
  reason_.clear();
  backend_ = UpscalerBackend::None;

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

RenderResolution StreamlineBridge::recommendedInternalResolution() const {
  if (!initialized_ || !available_ || mode_ == DLSSMode::Off) {
    return outputRes_;
  }

  const float scale = presetScale(preset_);
  return {
      alignEven(static_cast<int>(std::round(static_cast<float>(outputRes_.width) * scale))),
      alignEven(static_cast<int>(std::round(static_cast<float>(outputRes_.height) * scale))),
  };
}

std::optional<std::string> StreamlineBridge::validateFrameInputs(
    const StreamlineFrameInputs& inputs) const {
  if (!initialized_) {
    return "StreamlineBridge not initialized";
  }

  if (mode_ == DLSSMode::Off || !available_) {
    return std::nullopt;
  }

  if (!inputs.hasColor) return "Missing color input";
  if (!inputs.hasDepth) return "Missing depth input";
  if (!inputs.hasMotionVectors) return "Missing motion vectors input";
  if (!inputs.hasExposure) return "Missing exposure input";

  return std::nullopt;
}

FrameInfo StreamlineBridge::evaluate(double frameTimeMs, const StreamlineFrameInputs& inputs) const {
  FrameInfo info{};
  info.frameTimeMs = frameTimeMs;
  info.outputResolution = outputRes_;
  info.internalResolution = outputRes_;

  if (!initialized_) {
    info.note = "bridge not initialized";
    return info;
  }

  const auto validationError = validateFrameInputs(inputs);
  const bool canRunDLSS = available_ && mode_ != DLSSMode::Off && !validationError.has_value();

  info.dlssActive = canRunDLSS;
  info.frameGenActive = canRunDLSS && frameGenEnabled_ && mode_ == DLSSMode::On;
  info.internalResolution = canRunDLSS ? recommendedInternalResolution() : outputRes_;

  if (validationError.has_value()) {
    info.note = *validationError;
  } else if (!available_) {
    info.note = reason_;
  } else if (mode_ == DLSSMode::Off) {
    info.note = "DLSS mode off";
  } else {
    info.note = "DLSS path ready";
  }

  return info;
}

}  // namespace snake
