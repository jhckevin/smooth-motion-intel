# R996 完整包验证 / Full-bundle validation

日期：2026-10-11。客户端新增首次正常 GUI 启动的模型使用前声明；原生推理引擎、模型数学逻辑和权重未改。

- 远程 Windows 上重新编译客户端，通过预设 54 项、诊断检查及宿主 20 场景/43 项 CPU 检查。
- 模型声明 12 项 CPU 检查通过：取消/缺声明拦截、只读确认、重复确认、内容变化及损坏确认记录；未显示真实弹框，未启动 GPU。
- 打包前校验 227 个图/模型文件，以及 2 个外部 SPIR-V 的大小与 SHA256；均与当前 assets988.json 匹配。
- 导入器通过 16 项回归，并通过 Windows PowerShell 命令行 WhatIf 校验完整 229 文件；修复了参数默认路径与 WhatIf 下的文件哈希兼容问题。\n- 包内 model/ 直接符合当前 EngineHost 的查找目录，不需要 NVIDIA 原装 DLL 或额外提取步骤。
- 启动验证输入和参考结果为合成白块图，未打包游戏/桌面截图、用户日志、配置、诊断包或调试符号。
- 当前完整包仍需设备能力通过。B390 推荐；其他 Intel GPU 未实机验证。
- 本轮不是新的游戏帧率、功耗、画质或长时间稳定性验收。
- 模型使用声明的确认只是阅读记录，不授予第三方权利，详见 MODEL-NOTICE.md。

The client now requires the model notice at normal first GUI launch. Native inference and weights are unchanged. All 229 asset files are integrity-checked; synthetic validation images contain no captured desktop/game content. Hardware capability checks still apply. This packaging validation is not a new game performance, energy, image-quality or sustained-stability result.
