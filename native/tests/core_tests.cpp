#include "snake/core.hpp"

#include <cassert>
#include <iostream>

int main() {
  {
    snake::SnakeCore core(10, 1);
    const auto head = core.state().snake.front();
    auto r = core.step();
    assert(r.moved);
    assert(!r.gameOver);
    assert(core.state().snake.front().x == head.x + 1);
  }

  {
    snake::SnakeCore core(3, 2);
    core.setDirection(snake::Direction::Right);
    core.step();
    core.step();
    auto r = core.step();
    assert(r.gameOver);
  }

  {
    snake::SnakeCore core(10, 3);
    const auto dirBefore = core.state().nextDirection;
    core.setDirection(snake::Direction::Left);
    assert(core.state().nextDirection == dirBefore);
  }

  std::cout << "core tests passed\n";
  return 0;
}
