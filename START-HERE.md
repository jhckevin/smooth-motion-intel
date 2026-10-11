# Start here / 从这里开始

[English](README.md) · [简体中文](README.zh-CN.md)

## 简体中文

本文件属于 Windows x86_64 / x64 / AMD64 **Full 完整应用包**。完整解压后双击 **SmoothMotion.exe**；无需编译、Python/conda、原装 NVIDIA DLL 或手工导入模型。

model/ 已包含与当前引擎匹配的转换模型：权重与预计算参数、导出图、Intel Vulkan/XMX SPIR-V 和合成启动验证数据。请不要删除或单独移动此目录。

首次正常启动会显示模型使用前声明；确认表示已阅读，不会授予第三方权利。阅读 [MODEL-NOTICE.md](MODEL-NOTICE.md)。维护者免费提供；非盈利和免责声明不能替代厂商授权。

推荐先用 2×、WGC 和跟随游戏叠层。Ctrl+Alt+S 开始/停止，Ctrl+Alt+Esc 紧急停止。B390 推荐；其他 Intel GPU 仍需通过实际能力检查，不保证支持。

tools/import-model.ps1 仅用于另行替换匹配的已转换模型包；它不具备从 NVIDIA DLL 提取或转换的功能。

## English

This is the Windows x86_64 / x64 / AMD64 **Full application bundle**. Extract everything and launch **SmoothMotion.exe**. No compiler, Python/conda environment, original NVIDIA DLL or manual model import is required.

The model/ directory already contains the matched converted model: weights/precomputed parameters, exported graph, Intel Vulkan/XMX SPIR-V and synthetic validation data. Keep it with the application.

Read [MODEL-NOTICE.md](MODEL-NOTICE.md) and acknowledge it at normal first launch. Acknowledgement grants no third-party rights. The maintainer provides the project free of charge; non-profit distribution and disclaimers do not establish vendor permission.

Start with 2×, WGC and the game-window overlay. Ctrl+Alt+S starts/stops; Ctrl+Alt+Esc is emergency stop. B390 is recommended; other Intel GPUs still require actual capability checks.

The optional importer only verifies/copies a matching converted package. It cannot extract or convert the original NVIDIA DLL. The current application UI is Chinese.
