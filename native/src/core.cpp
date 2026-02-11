#include "snake/core.hpp"

#include <algorithm>
#include <sstream>

namespace snake {
namespace {
Cell directionDelta(Direction d) {
  switch (d) {
    case Direction::Up:
      return {0, -1};
    case Direction::Down:
      return {0, 1};
    case Direction::Left:
      return {-1, 0};
    case Direction::Right:
      return {1, 0};
  }
  return {0, 0};
}

bool containsCell(const std::vector<Cell>& cells, const Cell& c) {
  return std::find(cells.begin(), cells.end(), c) != cells.end();
}
}  // namespace

SnakeCore::SnakeCore(int gridSize, uint32_t seed) : rng_(seed) {
  state_.gridSize = gridSize;
  restart();
}

void SnakeCore::restart() {
  const int center = state_.gridSize / 2;
  state_.snake = {{center, center}, {center - 1, center}, {center - 2, center}};
  state_.direction = Direction::Right;
  state_.nextDirection = Direction::Right;
  state_.score = 0;
  state_.gameOver = false;
  state_.paused = false;
  state_.food = spawnFood(state_.snake);
}

void SnakeCore::setPaused(bool paused) { state_.paused = paused; }

bool SnakeCore::isOpposite(Direction a, Direction b) {
  return (a == Direction::Up && b == Direction::Down) ||
         (a == Direction::Down && b == Direction::Up) ||
         (a == Direction::Left && b == Direction::Right) ||
         (a == Direction::Right && b == Direction::Left);
}

void SnakeCore::setDirection(Direction direction) {
  if (!isOpposite(direction, state_.direction)) {
    state_.nextDirection = direction;
  }
}

std::optional<Cell> SnakeCore::spawnFood(const std::vector<Cell>& snake) {
  std::vector<Cell> available;
  for (int y = 0; y < state_.gridSize; ++y) {
    for (int x = 0; x < state_.gridSize; ++x) {
      Cell c{x, y};
      if (!containsCell(snake, c)) {
        available.push_back(c);
      }
    }
  }
  if (available.empty()) {
    return std::nullopt;
  }

  std::uniform_int_distribution<int> dist(0, static_cast<int>(available.size()) - 1);
  return available[dist(rng_)];
}

StepResult SnakeCore::step() {
  if (state_.gameOver || state_.paused) {
    return {.moved = false, .ateFood = false, .gameOver = state_.gameOver};
  }

  const auto delta = directionDelta(state_.nextDirection);
  const auto head = state_.snake.front();
  const Cell nextHead{head.x + delta.x, head.y + delta.y};

  if (nextHead.x < 0 || nextHead.y < 0 || nextHead.x >= state_.gridSize ||
      nextHead.y >= state_.gridSize) {
    state_.gameOver = true;
    return {.moved = false, .ateFood = false, .gameOver = true};
  }

  if (containsCell(state_.snake, nextHead)) {
    state_.gameOver = true;
    return {.moved = false, .ateFood = false, .gameOver = true};
  }

  state_.snake.insert(state_.snake.begin(), nextHead);
  state_.direction = state_.nextDirection;

  bool ate = false;
  if (state_.food && nextHead == *state_.food) {
    ate = true;
    state_.score += 1;
    state_.food = spawnFood(state_.snake);
  } else {
    state_.snake.pop_back();
  }

  return {.moved = true, .ateFood = ate, .gameOver = false};
}

std::string SnakeCore::debugBoard() const {
  std::vector<std::string> grid(state_.gridSize, std::string(state_.gridSize, '.'));
  if (state_.food) {
    grid[state_.food->y][state_.food->x] = 'F';
  }
  for (size_t i = 0; i < state_.snake.size(); ++i) {
    const auto& s = state_.snake[i];
    grid[s.y][s.x] = i == 0 ? 'H' : 'S';
  }

  std::ostringstream oss;
  for (const auto& row : grid) {
    oss << row << '\n';
  }
  return oss.str();
}

}  // namespace snake
