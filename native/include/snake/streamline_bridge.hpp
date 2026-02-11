#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace snake {

enum class UpscalerBackend {
  None,
  DLSS,
};

enum class DLSSPreset {
  Quality,
  Balanced,
  Performance,
  UltraPerformance,
};

enum class DLSSMode {
  Off,
  Auto,
  On,
};

const char* toString(DLSSMode mode);
const char* toString(DLSSPreset preset);

struct StreamlineConfig {
  int renderWidth{};
  int renderHeight{};
  int outputWidth{};
  int outputHeight{};
  bool enableDLSS{};
  bool enableFrameGeneration{};
  DLSSPreset preset{DLSSPreset::Quality};
  DLSSMode mode{DLSSMode::Auto};
};

struct StreamlineFrameInputs {
  bool hasColor{};
  bool hasDepth{};
  bool hasMotionVectors{};
  bool hasExposure{};
  float jitterX{};
  float jitterY{};
};

struct RenderResolution {
  int width{};
  int height{};
};

struct AutoPolicy {
  // Use lower/higher thresholds to avoid mode thrashing.
  double offThresholdMs{16.5};
  double onThresholdMs{19.5};
  int smoothingWindowFrames{6};
  bool dynamicPreset{true};
};

struct RuntimeStats {
  std::size_t totalFrames{};
  std::size_t dlssFrames{};
  std::size_t frameGenFrames{};
  std::size_t autoModeSwitches{};
  std::size_t validationFailures{};
  double averageFrameTimeMs{};
  DLSSMode lastEffectiveMode{DLSSMode::Off};
  std::string lastNote;
};

struct FrameInfo {
  double frameTimeMs{};
  double smoothedFrameTimeMs{};
  bool dlssActive{};
  bool frameGenActive{};
  DLSSMode effectiveMode{DLSSMode::Off};
  DLSSPreset effectivePreset{DLSSPreset::Quality};
  RenderResolution internalResolution{};
  RenderResolution outputResolution{};
  float upscaleRatio{};
  std::string note;
};

class StreamlineBridge {
 public:
  bool initialize(const StreamlineConfig& config);

  bool available() const { return available_; }
  const std::string& reason() const { return reason_; }
  UpscalerBackend backend() const { return backend_; }

  void setMode(DLSSMode mode);
  void setPreset(DLSSPreset preset);
  DLSSMode mode() const { return mode_; }
  DLSSPreset preset() const { return preset_; }

  void setAutoPolicy(const AutoPolicy& policy);
  const AutoPolicy& autoPolicy() const { return autoPolicy_; }

  RenderResolution recommendedInternalResolution() const;
  std::optional<std::string> validateFrameInputs(const StreamlineFrameInputs& inputs) const;

  FrameInfo evaluate(double frameTimeMs, const StreamlineFrameInputs& inputs);
  RuntimeStats stats() const;
  void resetStats();
  std::vector<double> recentFrameTimes() const;

 private:
  DLSSMode resolveEffectiveMode(double smoothedFrameTimeMs);
  DLSSPreset resolveEffectivePreset(double smoothedFrameTimeMs, DLSSMode effectiveMode) const;
  double pushFrameTime(double frameTimeMs);

  RenderResolution recommendedInternalResolutionFor(DLSSMode effectiveMode,
                                                    DLSSPreset effectivePreset) const;
  std::optional<std::string> validateFrameInputsForMode(const StreamlineFrameInputs& inputs,
                                                        DLSSMode effectiveMode) const;

  bool initialized_{false};
  bool available_{false};
  std::string reason_;

  UpscalerBackend backend_{UpscalerBackend::None};
  DLSSMode mode_{DLSSMode::Off};
  DLSSPreset preset_{DLSSPreset::Quality};

  RenderResolution renderRes_{};
  RenderResolution outputRes_{};
  bool frameGenEnabled_{false};

  AutoPolicy autoPolicy_{};
  std::deque<double> frameTimes_;
  RuntimeStats stats_{};
};

}  // namespace snake
