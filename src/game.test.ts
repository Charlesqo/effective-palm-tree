import { createInitialState, setDirection, spawnFood, step } from "./game";

const createRng = (values: number[]) => {
  let index = 0;
  return () => {
    const value = values[index % values.length];
    index += 1;
    return value;
  };
};

describe("snake game logic", () => {
  it("moves the snake forward", () => {
    const rng = createRng([0]);
    let state = createInitialState(10, rng);
    const head = state.snake[0];
    state = step(state, rng);
    expect(state.snake[0]).toEqual({ x: head.x + 1, y: head.y });
  });

  it("grows and scores when eating food", () => {
    const rng = createRng([0]);
    let state = createInitialState(10, rng);
    const head = state.snake[0];
    state = {
      ...state,
      food: { x: head.x + 1, y: head.y },
    };
    state = step(state, rng);
    expect(state.score).toBe(1);
    expect(state.snake.length).toBe(4);
  });

  it("ends game on wall collision", () => {
    const rng = createRng([0]);
    let state = createInitialState(3, rng);
    state = { ...state, snake: [{ x: 2, y: 1 }], direction: "right", nextDirection: "right" };
    state = step(state, rng);
    expect(state.isGameOver).toBe(true);
  });

  it("prevents reversing direction", () => {
    const rng = createRng([0]);
    let state = createInitialState(10, rng);
    state = setDirection(state, "left");
    expect(state.nextDirection).toBe("right");
  });

  it("spawns food in free cell", () => {
    const rng = createRng([0]);
    const food = spawnFood(2, [
      { x: 0, y: 0 },
      { x: 1, y: 0 },
      { x: 0, y: 1 },
    ], rng);
    expect(food).toEqual({ x: 1, y: 1 });
  });
});
