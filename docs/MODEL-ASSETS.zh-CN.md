# 已转换的模型资产

[English](MODEL-ASSETS.en.md) · 简体中文

**Full 完整 Windows Release**已附带与当前引擎匹配的转换模型包。源码仓库与 GitHub 自动 Source code 下载不包含模型。

模型包含：权重和预计算参数、导出的计算图、适配 Intel Vulkan/XMX 的 SPIR-V 推理 shader，以及合成验证输入和参考结果。引擎读取这些文件，**不直接加载 NVIDIA 原装 Smooth Motion DLL**。

完整包无需手动导入。即使安装 RTX 显卡和驱动，也不能将原装 DLL 放入客户端直接运行。

## 开发者/更换模型时的可选导入

导入器只按 assets988.json 校验大小及 SHA256，再将**已转换且匹配的包**复制到 Destination/model。它没有 DLL 自动提取、转换功能，也不下载或执行厂商 DLL，不修改驱动、注册表或全局环境。

~~~powershell
.\tools\import-model.ps1 -Source "D:\MyConvertedSmoothMotionModel" -Destination "." -WhatIf
.\tools\import-model.ps1 -Source "D:\MyConvertedSmoothMotionModel" -Destination "."
~~~

Source 可指向此前客户端根目录（含 UserData/runtime-work、UserData/runtime-shaders）或匹配的转换模型目录（含 graph922.bin、assets、shaders）。从源码仓库向构建输出导入时，使用 -Destination ".\dist"。

完整性校验不表示取得权利许可。见 [模型使用前声明](../MODEL-NOTICE.md)；第三方模型资产不适用项目原创代码的 Apache-2.0 许可。

## 设备要求

当前 FP16 管线中部分 shader 要求 scalar float8 转换。附带模型不等于提供硬件缺少的功能；设备/驱动仍须通过能力检查，不支持时需要经过验证的兼容 shader。
