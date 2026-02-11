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

  const bool ok = streamline.initialize(cfg);
  std::cout << "[Streamline] available=" << (ok ? "yes" : "no")
            << " reason=" << (ok ? "none" : streamline.reason()) << "\n";

  for (int tick = 0; tick < 30; ++tick) {
    if (tick == 6) game.setDirection(snake::Direction::Down);
    if (tick == 12) game.setDirection(snake::Direction::Left);
    if (tick == 18) game.setDirection(snake::Direction::Up);
    if (tick == 10) streamline.setPreset(snake::DLSSPreset::Performance);
    if (tick == 20) streamline.setMode(snake::DLSSMode::On);

    const auto result = game.step();

    snake::StreamlineFrameInputs inputs{
        .hasColor = true,
        .hasDepth = true,
        .hasMotionVectors = true,
        .hasExposure = true,
        .jitterX = 0.5f,
        .jitterY = -0.5f,
    };
    const auto frameInfo = streamline.evaluate(16.6, inputs);

    std::cout << "tick=" << tick << " moved=" << result.moved << " ate=" << result.ateFood
              << " over=" << result.gameOver << " score=" << game.state().score
              << " dlss=" << frameInfo.dlssActive
              << " fg=" << frameInfo.frameGenActive
              << " internal=" << frameInfo.internalResolution.width << "x"
              << frameInfo.internalResolution.height
              << " out=" << frameInfo.outputResolution.width << "x"
              << frameInfo.outputResolution.height << " note=" << frameInfo.note << "\n";

    if (result.gameOver) {
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  std::cout << "final-score=" << game.state().score << "\n";
  return 0;
}
