export type Direction = "up" | "down" | "left" | "right";

export type Point = {
  x: number;
  y: number;
};

export type GameState = {
  gridSize: number;
  snake: Point[];
  direction: Direction;
  nextDirection: Direction;
  food: Point | null;
  score: number;
  isGameOver: boolean;
  isPaused: boolean;
};

const DIRS: Record<Direction, Point> = {
  up: { x: 0, y: -1 },
  down: { x: 0, y: 1 },
  left: { x: -1, y: 0 },
  right: { x: 1, y: 0 },
};

const OPPOSITE: Record<Direction, Direction> = {
  up: "down",
  down: "up",
  left: "right",
  right: "left",
};

export const createInitialState = (gridSize: number, rng = Math.random): GameState => {
  const center = Math.floor(gridSize / 2);
  const snake = [
    { x: center, y: center },
    { x: center - 1, y: center },
    { x: center - 2, y: center },
  ];
  return {
    gridSize,
    snake,
    direction: "right",
    nextDirection: "right",
    food: spawnFood(gridSize, snake, rng),
    score: 0,
    isGameOver: false,
    isPaused: false,
  };
};

export const spawnFood = (
  gridSize: number,
  snake: Point[],
  rng = Math.random
): Point | null => {
  const occupied = new Set(snake.map((part) => `${part.x},${part.y}`));
  const available: Point[] = [];
  for (let y = 0; y < gridSize; y += 1) {
    for (let x = 0; x < gridSize; x += 1) {
      if (!occupied.has(`${x},${y}`)) {
        available.push({ x, y });
      }
    }
  }
  if (available.length === 0) {
    return null;
  }
  const index = Math.floor(rng() * available.length);
  return available[index];
};

export const step = (state: GameState, rng = Math.random): GameState => {
  if (state.isGameOver || state.isPaused) {
    return state;
  }

  const direction = state.nextDirection;
  const move = DIRS[direction];
  const head = state.snake[0];
  const nextHead = { x: head.x + move.x, y: head.y + move.y };

  if (
    nextHead.x < 0 ||
    nextHead.y < 0 ||
    nextHead.x >= state.gridSize ||
    nextHead.y >= state.gridSize
  ) {
    return { ...state, isGameOver: true };
  }

  const bodySet = new Set(state.snake.map((segment) => `${segment.x},${segment.y}`));
  if (bodySet.has(`${nextHead.x},${nextHead.y}`)) {
    return { ...state, isGameOver: true };
  }

  const nextSnake = [nextHead, ...state.snake];
  let nextFood = state.food;
  let nextScore = state.score;

  if (state.food && nextHead.x === state.food.x && nextHead.y === state.food.y) {
    nextScore += 1;
    nextFood = spawnFood(state.gridSize, nextSnake, rng);
  } else {
    nextSnake.pop();
  }

  return {
    ...state,
    snake: nextSnake,
    direction,
    food: nextFood,
    score: nextScore,
  };
};

export const setDirection = (state: GameState, dir: Direction): GameState => {
  if (OPPOSITE[dir] === state.direction) {
    return state;
  }
  return { ...state, nextDirection: dir };
};
