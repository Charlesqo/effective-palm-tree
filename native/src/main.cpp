#include "snake/core.hpp"
#include "snake/streamline_bridge.hpp"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
  snake::SnakeCore game(20, 2026);

  snake::StreamlineBridge streamline;
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

  snake::AutoPolicy autoPolicy{
      .offThresholdMs = 16.5,
      .onThresholdMs = 19.5,
      .smoothingWindowFrames = 5,
      .dynamicPreset = true,
  };

  streamline.setAutoPolicy(autoPolicy);
  const bool ok = streamline.initialize(cfg);
  std::cout << "[Streamline] available=" << (ok ? "yes" : "no")
            << " reason=" << (ok ? "none" : streamline.reason()) << "\n";

  for (int tick = 0; tick < 40; ++tick) {
    if (tick == 6) game.setDirection(snake::Direction::Down);
    if (tick == 12) game.setDirection(snake::Direction::Left);
    if (tick == 18) game.setDirection(snake::Direction::Up);
    if (tick == 24) streamline.setMode(snake::DLSSMode::On);
    if (tick == 30) streamline.setMode(snake::DLSSMode::Auto);

    const auto result = game.step();

    snake::StreamlineFrameInputs inputs{
        .hasColor = true,
        .hasDepth = true,
        .hasMotionVectors = tick % 11 != 0,
        .hasExposure = true,
        .jitterX = 0.5f,
        .jitterY = -0.5f,
    };

    const double simulatedFrameTime =
        tick < 10 ? 14.0 : (tick < 20 ? 23.0 : (tick < 30 ? 28.0 : 15.0));
    const auto frameInfo = streamline.evaluate(simulatedFrameTime, inputs);

    std::cout << "tick=" << tick << " moved=" << result.moved << " ate=" << result.ateFood
              << " over=" << result.gameOver << " score=" << game.state().score
              << " dlss=" << frameInfo.dlssActive
              << " fg=" << frameInfo.frameGenActive
              << " mode=" << snake::toString(frameInfo.effectiveMode)
              << " preset=" << snake::toString(frameInfo.effectivePreset)
              << " ft=" << frameInfo.frameTimeMs
              << " smooth=" << frameInfo.smoothedFrameTimeMs
              << " internal=" << frameInfo.internalResolution.width << "x"
              << frameInfo.internalResolution.height
              << " out=" << frameInfo.outputResolution.width << "x"
              << frameInfo.outputResolution.height << " ratio=" << frameInfo.upscaleRatio
              << " note=" << frameInfo.note << "\n";

    if (result.gameOver) {
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  const auto stats = streamline.stats();
  std::cout << "[stats] frames=" << stats.totalFrames << " dlssFrames=" << stats.dlssFrames
            << " fgFrames=" << stats.frameGenFrames
            << " autoSwitches=" << stats.autoModeSwitches
            << " validationFailures=" << stats.validationFailures
            << " avgFrameMs=" << stats.averageFrameTimeMs
            << " lastMode=" << snake::toString(stats.lastEffectiveMode)
            << " lastNote=" << stats.lastNote << "\n";

  std::cout << "final-score=" << game.state().score << "\n";
  return 0;
}
