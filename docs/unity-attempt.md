# Unity 安装尝试记录

按要求已在当前环境实际尝试安装 Unity Hub，并验证能否在无桌面环境下运行。

## 已执行

1. 添加 Unity Hub 官方 Linux APT 源并安装 `unityhub`。
2. 安装 Unity Hub 运行所需依赖（如 `libasound2t64`）。
3. 安装 `xvfb`，以便在无图形桌面的环境里尝试 headless 命令。
4. 使用 headless CLI 做连通性验证。

## 结果

- `unityhub` 已安装到 `/usr/bin/unityhub`。
- 通过 `xvfb-run -a unityhub -- --headless help` 可以拿到 CLI 帮助，说明 Hub 可启动到 headless 模式。
- `xvfb-run -a unityhub -- --headless editors -r -j` 返回 `[]`，并伴随多条 SSL handshake 错误（`net_error -202`），当前环境无法正常拉取 Unity 发布列表。

## 结论

- **能装**：Unity Hub 已安装成功。
- **当前环境下不能完整拉取/安装 Editor**：因为 headless 请求到 Unity 服务端时出现 TLS/网络握手失败。

## 复现实验

运行：

```bash
scripts/test-unity-headless.sh
```

