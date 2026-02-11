# 带 DLSS 的贪吃蛇：可落地实现方案

你问“能不能做带 DLSS 的贪吃蛇，怎么实现？”——**能做**，但要分清两种路径：

1. **浏览器版（当前仓库）**：基本不能直接接入 NVIDIA DLSS（因为 DLSS/NGX/Streamline 主要是原生图形 API 路线，目标是 DX12/Vulkan 等）。
2. **原生桌面版（推荐）**：用 C++ + DX12/Vulkan + Streamline 接入 DLSS，贪吃蛇逻辑照搬当前项目核心状态机即可。

---

## 1) 复用当前仓库哪些内容

当前仓库里，贪吃蛇的规则逻辑已经是独立可复用的：

- 方向、移动、碰撞、增长、食物生成：`src/game.ts`
- 渲染与输入在 `src/main.ts`，可以视为“壳层”替换目标。

迁移思路：

- **保留规则层**（状态机思路不变）；
- **替换渲染层**（从 PixiJS/Canvas 换成原生图形 API）；
- 在渲染管线里插入 Streamline/DLSS 的初始化与每帧评估。

---

## 2) 推荐原生架构（DLSS 版）

建议拆 4 层：

1. `SnakeCore`（纯逻辑）
   - `Init/Step/SetDirection/Restart`
   - 输出蛇身格子、食物格子、分数、状态
2. `Renderer`（DX12 或 Vulkan）
   - 低分辨率渲染目标（Internal Render Resolution）
   - 输出 color + depth + motion vectors（DLSS 所需）
3. `Upscaler`（Streamline + DLSS）
   - `slInit`
   - `slIsFeatureSupported(kFeatureDLSS)`
   - `slEvaluateFeature(...)`
4. `Platform`（窗口、输入、present）

这样你可以在不开 DLSS 时直接走原始渲染，开 DLSS 时走 upscale 分支。

---

## 3) 每帧时序（核心）

```text
Input -> SnakeCore::Step()
      -> Render Internal Resolution (比如 720p)
      -> Tag/Prepare DLSS Inputs (color/depth/mvec/exposure)
      -> slEvaluateFeature(DLSS)
      -> Present Upscaled Frame (比如 1440p)
```

关键点：

- 贪吃蛇画面简单，motion vector 也相对简单；
- 但为了“真 DLSS”，依然要满足 Streamline/DLSS 对资源格式与时序的要求；
- UI/HUD 通常在 upscale 后再叠加，避免文字边缘被处理得不稳定。

---

## 4) 与当前网页版本关系

- 你现在这个仓库是 Vite + TS + PixiJS，适合网页演示与逻辑验证；
- DLSS 目标是原生图形栈，不建议强行在这个前端项目里“硬塞”。

**实际工程做法**：

1. 继续把这里当规则原型（快速迭代玩法）；
2. 新建 native 子项目（例如 `native/`）实现渲染与 DLSS；
3. 两边共享同一份规则测试用例（保证行为一致）。

---

## 5) 最小可行里程碑（MVP）

1. 先做 native 非 DLSS 贪吃蛇（固定分辨率）
2. 接入 Streamline，完成 feature 探测与开关 UI
3. 跑通 DLSS Quality 模式
4. 增加日志与调试视图（开/关 DLSS 对比）

---

## 6) 现实提醒

- DLSS 依赖 NVIDIA RTX 设备与驱动环境；
- 还要看目标平台和发布许可流程；
- 如果只是“让小游戏看起来更清晰”，可以先做 TAAU/FSR 路线验证接口设计，再切 DLSS。

