# DLSS 贪吃蛇（Native 版本）实现说明

这次不是只写方案文档，而是把可运行的 **native 工程 + DLSS 管线边界 + 测试** 直接落地到仓库。

## 已完成内容

## 1. 可编译的 C++ 工程

- `native/CMakeLists.txt`
- 目标：
  - `snake_core`（核心逻辑库）
  - `snake_native`（运行示例）
  - `snake_core_tests`（核心逻辑测试）
  - `streamline_bridge_tests`（DLSS 桥接层测试）

## 2. 游戏核心逻辑（可复用）

- `native/include/snake/core.hpp`
- `native/src/core.cpp`

支持：

- 网格移动
- 方向控制（防反向）
- 食物生成
- 碰撞判定
- 得分增长
- 重开/暂停

## 3. DLSS/Streamline 桥接层（非简单 stub）

- `native/include/snake/streamline_bridge.hpp`
- `native/src/streamline_bridge.cpp`

已实现能力：

- DLSS preset（Quality / Balanced / Performance / UltraPerformance）
- DLSS mode（Off / Auto / On）
- 每帧输入校验接口（color/depth/motion vectors/exposure）
- 内部分辨率推荐计算（按 preset 缩放）
- 每帧评估结果输出（是否激活 DLSS / FG、输入输出分辨率、状态说明）

说明：

- 在 Windows 目标下预留真实 Streamline 运行时接入位；
- 在当前 Linux 环境会返回不可用原因，但评估链路与配置链路都已经就位。

## 4. Demo 与测试

- `native/src/main.cpp`：模拟 30 tick 游戏循环，并输出每帧 upscaler 状态
- `native/tests/core_tests.cpp`
- `native/tests/streamline_bridge_tests.cpp`
- `scripts/build-native-snake.sh`：一键构建 + ctest + 运行 demo

## 如何运行

```bash
scripts/build-native-snake.sh
```

## 下一步接入真实 DLSS（Windows + DX12/Vulkan）

1. 在 `StreamlineBridge::initialize` 中接入官方 Streamline 初始化。
2. 在 render loop 中补齐 color/depth/motion vectors/exposure 资源。
3. 在 `evaluate` 路径调用 `slEvaluateFeature` 并写回状态。
4. 在设置菜单中增加 DLSS preset/mode 与 Frame Generation 开关。

