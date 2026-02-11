#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <vector>

namespace snake {

enum class Direction { Up, Down, Left, Right };

struct Cell {
  int x{};
  int y{};

  bool operator==(const Cell& other) const { return x == other.x && y == other.y; }
};

struct StepResult {
  bool moved{};
  bool ateFood{};
  bool gameOver{};
};

struct GameState {
  int gridSize{};
  std::vector<Cell> snake;
  Direction direction{Direction::Right};
  Direction nextDirection{Direction::Right};
  std::optional<Cell> food;
  int score{};
  bool gameOver{};
  bool paused{};
};

class SnakeCore {
 public:
  explicit SnakeCore(int gridSize = 20, uint32_t seed = 12345);

  const GameState& state() const { return state_; }
  void restart();
  void setPaused(bool paused);
  void setDirection(Direction direction);
  StepResult step();

  std::string debugBoard() const;

 private:
  GameState state_;
  std::mt19937 rng_;

  std::optional<Cell> spawnFood(const std::vector<Cell>& snake);
  static bool isOpposite(Direction a, Direction b);
};

}  // namespace snake
