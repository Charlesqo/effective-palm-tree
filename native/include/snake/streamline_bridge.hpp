#pragma once

#include <optional>
#include <string>

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

struct FrameInfo {
  double frameTimeMs{};
  bool dlssActive{};
  bool frameGenActive{};
  RenderResolution internalResolution{};
  RenderResolution outputResolution{};
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

  RenderResolution recommendedInternalResolution() const;
  std::optional<std::string> validateFrameInputs(const StreamlineFrameInputs& inputs) const;
  FrameInfo evaluate(double frameTimeMs, const StreamlineFrameInputs& inputs) const;

 private:
  bool initialized_{false};
  bool available_{false};
  std::string reason_;

  UpscalerBackend backend_{UpscalerBackend::None};
  DLSSMode mode_{DLSSMode::Off};
  DLSSPreset preset_{DLSSPreset::Quality};

  RenderResolution renderRes_{};
  RenderResolution outputRes_{};
  bool frameGenEnabled_{false};
};

}  // namespace snake
