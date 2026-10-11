# Start here / 从这里开始

[English](README.md) · [简体中文](README.zh-CN.md)

## English

This is the portable Windows **x86_64 / x64 / AMD64** application package. Extract all files before starting. No build tools or Python environment are required.

- Launch **SmoothMotion.exe** for the desktop client.
- **SmoothMotion-Engine.exe** is the native inference/capture/presentation process, normally managed by the client.
- **diag-export994.exe** decodes the local diagnostic flight recorder.
- **tools/import-model.ps1** imports a separately held compatible model package.
- Documentation, the large SVG logo and license notices are included.

**Model assets are not bundled.** Frame generation cannot start until a compatible package that you are entitled to use has been imported. From this extracted directory:

~~~powershell
.\tools\import-model.ps1 -Source "D:\MySmoothMotionModel" -Destination "." -WhatIf
.\tools\import-model.ps1 -Source "D:\MySmoothMotionModel" -Destination "."
~~~

Keep the whole directory together. See README.md and docs/MODEL-ASSETS.en.md. The current application UI is Chinese.

## 简体中文

这是 Windows **x86_64 / x64 / AMD64** 便携应用包。请完整解压后运行，无须编译工具或 Python 环境。

- 双击 **SmoothMotion.exe** 打开客户端。
- **SmoothMotion-Engine.exe** 为客户端管理的推理、捕获和展示引擎。
- **diag-export994.exe** 用于离线解码诊断记录。
- **tools/import-model.ps1** 用于导入你持有的兼容模型包。
- 附中英文说明、大尺寸 SVG logo 和许可声明。

**包内不含模型资产，导入有权使用的兼容模型后才能插帧。** 在解压目录执行上面的两条命令，第一条只检查，第二条验证后复制。将示例 Source 路径改为自己的模型目录或此前客户端根目录。

推荐先使用 2×、WGC、跟随游戏叠层。Ctrl+Alt+S 开始/停止，Ctrl+Alt+Esc 紧急停止。详细说明见 README.zh-CN.md；诊断包通过“设置 → 本地数据 → 导出诊断包”导出。
