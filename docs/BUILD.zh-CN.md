# 构建与维护

仅在你的开发机/构建服务器上执行。玩家使用便携 Release 不需要开发工具。构建不下载或使用私有模型文件，不需要 GPU。

## 依赖

- Windows x64，Visual Studio 2022 C++ Build Tools（x64、Windows SDK、C++/WinRT 头）。
- 系统 .NET Framework 4.x 的 C# 编译器与 WPF 引用。当前项目沿用原桌面端，不需要 NuGet 或修改全局 Python/conda。
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) 固定版本 vulkan-sdk-1.4.341.0，提交 b5c8f996196ba4aa6d8f97e52b5d3b6e70f7e4e2。也可使用兼容 SDK 的 Include；必须包含所用的 float8/cooperative matrix 扩展定义。

## 构建

在 x64 Native Tools Command Prompt 中：

~~~cmd
set "VULKAN_HEADERS=D:\deps\Vulkan-Headers\include"
build.cmd
tools\test-desktop.cmd
powershell -NoProfile -File tools\test-import-model.ps1
powershell -NoProfile -File tools\test-native.ps1
~~~

此处 set 只作用于当前命令进程，不写系统 PATH。build.cmd 可尝试通过 vswhere 找到既有 VS 工具；不会安装工具。

dist 输出桌面客户端、原生引擎、离线诊断解码器。运行客户端前还需要匹配的本地模型包，见 [模型资产说明](MODEL-ASSETS.zh-CN.md)。

## 布局

- desktop/src：WPF 客户端、配置、宿主生命周期、诊断导出。
- native：R994 基线上的能力选择与互操作实现；保留历史文件编号以利定位回归。
- tools：相对目录构建、CPU 检查和本地资产导入。
- docs：玩家操作、诊断格式、来源及兼容边界。
- assets988.json：模型完整性清单；不包含任何模型字节。
- art/logo.svg：项目 SVG 标志；不是厂商商标。

## 验证边界

CPU 测试覆盖适配器策略、多卡匹配、生命周期与诊断，不创建推理负载，不证明某款显卡实测成功。只读设备能力探测同样不代表真实游戏通过。新增显卡支持需要记录驱动/能力、尺寸、实际展示、画质、延迟和长测；请不要把 mock 中的 B580 条目当成 B580 成绩。

默认 CI 仅构建和 CPU 测试，不获取模型、不跑 GPU、不自动发布 Release。匹配符号需要随自己的构建留存；公开二进制的源码版本必须可追溯。
