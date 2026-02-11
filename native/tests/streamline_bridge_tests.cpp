#include "snake/streamline_bridge.hpp"

#include <cassert>
#include <iostream>

int main() {
  {
    snake::StreamlineBridge bridge;
    snake::StreamlineConfig cfg{
        .renderWidth = 1280,
        .renderHeight = 720,
        .outputWidth = 1920,
        .outputHeight = 1080,
        .enableDLSS = false,
        .enableFrameGeneration = false,
        .preset = snake::DLSSPreset::Quality,
        .mode = snake::DLSSMode::Off,
    };

    const bool ok = bridge.initialize(cfg);
    assert(ok);
    assert(!bridge.available());

    snake::StreamlineFrameInputs inputs{true, true, true, true, 0.0f, 0.0f};
    const auto info = bridge.evaluate(16.6, inputs);
    assert(!info.dlssActive);
    assert(info.internalResolution.width == 1920);
    assert(info.outputResolution.height == 1080);
  }

  {
    snake::StreamlineBridge bridge;
    snake::StreamlineConfig cfg{
        .renderWidth = 1280,
        .renderHeight = 720,
        .outputWidth = 1920,
        .outputHeight = 1080,
        .enableDLSS = true,
        .enableFrameGeneration = true,
        .preset = snake::DLSSPreset::Performance,
        .mode = snake::DLSSMode::On,
    };
    bridge.initialize(cfg);

    snake::StreamlineFrameInputs bad{true, true, false, true, 0.0f, 0.0f};
    const auto err = bridge.validateFrameInputs(bad);
#ifndef _WIN32
    // On non-Windows we expect validation to be bypassed because DLSS runtime is unavailable.
    assert(!err.has_value());
#else
    assert(err.has_value());
#endif
  }

  {
    snake::StreamlineBridge bridge;
    snake::StreamlineConfig badCfg{
        .renderWidth = 0,
        .renderHeight = 720,
        .outputWidth = 1920,
        .outputHeight = 1080,
        .enableDLSS = true,
        .enableFrameGeneration = false,
    };
    const bool ok = bridge.initialize(badCfg);
    assert(!ok);
    assert(!bridge.reason().empty());
  }

  std::cout << "streamline bridge tests passed\n";
  return 0;
}
