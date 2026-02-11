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
    assert(info.effectiveMode == snake::DLSSMode::Off);
    assert(info.effectivePreset == snake::DLSSPreset::Quality);
    assert(info.upscaleRatio == 1.0f);

    const auto stats = bridge.stats();
    assert(stats.totalFrames == 1);
    assert(stats.dlssFrames == 0);
    assert(stats.lastEffectiveMode == snake::DLSSMode::Off);
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
    snake::AutoPolicy policy{
        .offThresholdMs = 12.0,
        .onThresholdMs = 20.0,
        .smoothingWindowFrames = 3,
        .dynamicPreset = true,
    };
    bridge.setAutoPolicy(policy);

    snake::StreamlineConfig cfg{
        .renderWidth = 1280,
        .renderHeight = 720,
        .outputWidth = 1920,
        .outputHeight = 1080,
        .enableDLSS = true,
        .enableFrameGeneration = true,
        .preset = snake::DLSSPreset::Quality,
        .mode = snake::DLSSMode::Auto,
    };
    bridge.initialize(cfg);

    snake::StreamlineFrameInputs complete{true, true, true, true, 0.0f, 0.0f};

    // Start fast -> Auto should settle on Off.
    const auto f0 = bridge.evaluate(10.0, complete);
    const auto f1 = bridge.evaluate(11.0, complete);
    assert(f0.effectiveMode == snake::DLSSMode::Off);
    assert(f1.effectiveMode == snake::DLSSMode::Off);

    // Mid frame-time keeps previous mode due to hysteresis.
    const auto mid = bridge.evaluate(15.0, complete);
    assert(mid.effectiveMode == snake::DLSSMode::Off);

    // Slow frames push average above onThreshold -> switch to On.
    const auto s0 = bridge.evaluate(30.0, complete);
    const auto s1 = bridge.evaluate(30.0, complete);
    assert(s0.effectiveMode == snake::DLSSMode::On || s1.effectiveMode == snake::DLSSMode::On);

    const auto stats = bridge.stats();
    assert(stats.totalFrames == 5);
    assert(stats.autoModeSwitches >= 1);
    assert(stats.averageFrameTimeMs > 0.0);

    const auto times = bridge.recentFrameTimes();
    assert(times.size() <= 3);
    assert(!times.empty());
  }

  {
    snake::StreamlineBridge bridge;
    snake::AutoPolicy policy;
    policy.dynamicPreset = true;
    bridge.setAutoPolicy(policy);

    snake::StreamlineConfig cfg{
        .renderWidth = 1280,
        .renderHeight = 720,
        .outputWidth = 1920,
        .outputHeight = 1080,
        .enableDLSS = true,
        .enableFrameGeneration = true,
        .preset = snake::DLSSPreset::Quality,
        .mode = snake::DLSSMode::Auto,
    };
    bridge.initialize(cfg);

    snake::StreamlineFrameInputs inputs{true, true, true, true, 0.0f, 0.0f};

    // Prime auto mode to On using slow frame and verify dynamic preset tiering.
    const auto slow0 = bridge.evaluate(35.0, inputs);
    const auto slow1 = bridge.evaluate(35.0, inputs);
    assert(slow0.effectivePreset == snake::DLSSPreset::UltraPerformance ||
           slow1.effectivePreset == snake::DLSSPreset::UltraPerformance);

    const auto medium = bridge.evaluate(24.0, inputs);
    assert(medium.effectivePreset == snake::DLSSPreset::Performance ||
           medium.effectivePreset == snake::DLSSPreset::UltraPerformance);

    bridge.resetStats();
    const auto resetStats = bridge.stats();
    assert(resetStats.totalFrames == 0);
    assert(resetStats.autoModeSwitches == 0);
    assert(bridge.recentFrameTimes().empty());
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

  {
    assert(std::string(snake::toString(snake::DLSSMode::Auto)) == "auto");
    assert(std::string(snake::toString(snake::DLSSPreset::Balanced)) == "balanced");
  }

  std::cout << "streamline bridge tests passed\n";
  return 0;
}
