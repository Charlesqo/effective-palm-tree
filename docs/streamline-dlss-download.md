# Streamline / DLSS 下载验证

你问“能不能装/下载 DLSS 的 Streamline”，我做了实际验证：**可以下载**。

## 我做了什么

1. 验证 Streamline 仓库可访问：
   - `git ls-remote https://github.com/NVIDIAGameWorks/Streamline.git`
2. 读取最新 release 元数据：
   - `https://api.github.com/repos/NVIDIA-RTX/Streamline/releases/latest`
3. 下载官方 SDK zip 并校验：
   - `streamline-sdk-v2.10.3.zip`
   - SHA256: `8564646ca6dd7c3960370d05b06054549563f499bfb93d160b46b7494a47bb3c`
4. 抽样查看 zip 内容，确认包含 DLSS/FrameGen 相关二进制：
   - `nvngx_dlss.dll`
   - `nvngx_dlssg.dll`
   - `sl.dlss.dll`
   - `sl.dlss_g.dll`

## 结论

- **Streamline SDK 可以直接从 GitHub Releases 下载**。
- 你如果是想先“拿到包、落地集成”，这条路是可行的。

## 仓库内提供的复现脚本

```bash
# 仅查看最新版本元数据
scripts/fetch-streamline-sdk.sh

# 下载到 /tmp/streamline-sdk 并打印 sha256 与压缩包内容预览
scripts/fetch-streamline-sdk.sh /tmp/streamline-sdk --download
```

