# UE 包来源排查（"去别的地方找包" 的实际结果）

你提到“能不能去别的地方找包”，这里做了真实排查，结果如下。

## 已尝试来源

1. **APT 仓库**
   - 命令：`apt-cache search unreal` / `apt-cache search ue5`
   - 结果：没有 Unreal Engine 官方包。

2. **Snap 商店**
   - 结果：当前环境没有安装 `snap` 客户端，无法直接用 `snap find`。

3. **Flatpak 仓库**
   - 结果：当前环境没有安装 `flatpak` 客户端，无法直接用 `flatpak search`。

4. **Docker 方案（如 ue4-docker）**
   - 结果：当前环境没有 `docker`，无法走容器化安装链路。

5. **Epic 官方 GitHub 源码仓库**
   - 仓库：`https://github.com/EpicGames/UnrealEngine.git`
   - 结果：访问需要 Epic 账号关联 GitHub 并认证；当前无交互 token，直接 `git ls-remote` 失败。

## 结论

- 不是“不能联网”，而是 **UE 的分发方式本身就带权限和环境门槛**：
  - 官方 apt 包基本不可用；
  - 源码仓库需要账号授权；
  - 构建又吃磁盘/CPU；
  - 还需要更完整的图形/工具链环境。

## 我已经做的可复现脚本

可执行：

```bash
scripts/check-ue-sources.sh
```

这个脚本会一次性检查 APT、snap、flatpak、docker、Epic GitHub 访问情况。

