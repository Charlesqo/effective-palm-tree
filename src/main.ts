import "./styles.css";
import { Application, Container, Graphics } from "pixi.js";
import { createInitialState, setDirection, step, type Direction, type GameState } from "./game";

const GRID_SIZE = 20;
const TICK_MS = 140;
const CELL_SIZE = 24;
const BOARD_SIZE = GRID_SIZE * CELL_SIZE;

const boardEl = document.getElementById("board") as HTMLDivElement;
const scoreEl = document.getElementById("score") as HTMLElement;
const statusEl = document.getElementById("status") as HTMLElement;
const pauseBtn = document.getElementById("pause") as HTMLButtonElement;
const restartBtn = document.getElementById("restart") as HTMLButtonElement;

let gameState: GameState = createInitialState(GRID_SIZE);
let timerId: number | null = null;

const bootstrap = async () => {
  const app = new Application();
  await app.init({
    width: BOARD_SIZE,
    height: BOARD_SIZE,
    background: "#fefefe",
    antialias: true,
  });
  boardEl.appendChild(app.canvas);

  const layer = new Container();
  app.stage.addChild(layer);

  const drawGrid = () => {
    const grid = new Graphics();
    grid.rect(0, 0, BOARD_SIZE, BOARD_SIZE).fill({ color: 0xfefefe });

    for (let i = 0; i <= GRID_SIZE; i += 1) {
      const pos = i * CELL_SIZE;
      grid
        .moveTo(pos, 0)
        .lineTo(pos, BOARD_SIZE)
        .stroke({ color: 0x000000, alpha: 0.06, width: 1 });
      grid
        .moveTo(0, pos)
        .lineTo(BOARD_SIZE, pos)
        .stroke({ color: 0x000000, alpha: 0.06, width: 1 });
    }
    layer.addChild(grid);
  };

  const drawSnake = (snake: GameState["snake"]) => {
    snake.forEach((segment, index) => {
      const cell = new Graphics();
      cell
        .rect(segment.x * CELL_SIZE + 1, segment.y * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2)
        .fill({ color: index === 0 ? 0x1f6f30 : 0x2b8a3e });
      layer.addChild(cell);
    });
  };

  const drawFood = (food: GameState["food"]) => {
    if (!food) return;
    const item = new Graphics();
    item
      .circle(food.x * CELL_SIZE + CELL_SIZE / 2, food.y * CELL_SIZE + CELL_SIZE / 2, CELL_SIZE / 2.6)
      .fill({ color: 0xe03131 });
    layer.addChild(item);
  };

  const render = () => {
    layer.removeChildren();
    drawGrid();
    drawSnake(gameState.snake);
    drawFood(gameState.food);
    scoreEl.textContent = String(gameState.score);

    if (gameState.isGameOver) {
      statusEl.textContent = "游戏结束！点击重新开始。";
    } else if (gameState.isPaused) {
      statusEl.textContent = "已暂停。";
    } else {
      statusEl.textContent = "";
    }
  };

  const stopLoop = () => {
    if (timerId === null) return;
    clearInterval(timerId);
    timerId = null;
  };

  const tick = () => {
    gameState = step(gameState);
    render();
    if (gameState.isGameOver) {
      stopLoop();
    }
  };

  const startLoop = () => {
    if (timerId !== null) return;
    timerId = window.setInterval(tick, TICK_MS);
  };

  const restart = () => {
    gameState = createInitialState(GRID_SIZE);
    pauseBtn.textContent = "暂停";
    render();
    startLoop();
  };

  const togglePause = () => {
    gameState = { ...gameState, isPaused: !gameState.isPaused };
    pauseBtn.textContent = gameState.isPaused ? "继续" : "暂停";
    if (gameState.isPaused) {
      stopLoop();
    } else {
      startLoop();
    }
    render();
  };

  const setDirectionFromKey = (key: string) => {
    const mapping: Record<string, Direction> = {
      arrowup: "up",
      w: "up",
      arrowdown: "down",
      s: "down",
      arrowleft: "left",
      a: "left",
      arrowright: "right",
      d: "right",
    };
    const dir = mapping[key];
    if (dir) {
      gameState = setDirection(gameState, dir);
    }
  };

  window.addEventListener("keydown", (event) => {
    const key = event.key.toLowerCase();
    if (key === " " || key === "p") {
      togglePause();
      return;
    }
    setDirectionFromKey(key);
  });

  document.querySelectorAll<HTMLButtonElement>("[data-dir]").forEach((button) => {
    button.addEventListener("click", () => {
      gameState = setDirection(gameState, button.dataset.dir as Direction);
    });
  });

  pauseBtn.addEventListener("click", togglePause);
  restartBtn.addEventListener("click", restart);

  render();
  startLoop();
};

bootstrap().catch((error) => {
  statusEl.textContent = "初始化失败，请刷新重试。";
  console.error(error);
});
