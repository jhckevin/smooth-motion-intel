# 本地模型资产

公开仓库与 Release 不包含 NVIDIA/LSFG 权重、提取图、第三方私有 shader 二进制或厂商 DLL。assets988.json 是文件路径、长度和 SHA256 清单，不是模型本身，也不是权重使用授权。

## 导入已有模型包
若你已合法取得并有权用于本项目的匹配模型包，或者有先前的本地项目目录，可在 PowerShell 执行：

~~~powershell
.\tools\import-model.ps1 -Source "D:\MySmoothMotionModel" -Destination ".\dist"
~~~

Release 内同样附带 tools/import-model.ps1；Destination 指向 SmoothMotion.exe 所在目录。Source 可指向旧客户端根目录（含 UserData/runtime-work 与 UserData/runtime-shaders），或模型根目录（含 graph922.bin、assets、shaders 等清单文件）。

导入器仅按当前 manifest 逐文件验证并复制到 Destination/model，不下载、提取或执行厂商 DLL，不改注册表、驱动或全局环境；失败明确报告缺失或哈希不符。使用 -WhatIf 先核查，不写入资产。完整性校验不表示你已取得权利许可。

没有现成模型包时，此公开客户端暂时无法进行帧生成。这里不提供权重镜像、自动抓取脚本或“免责声明即可使用”的说法。具体 NVIDIA 版本适用的许可需要单独确认。

## 资产与设备能力
同一模型包包含固定编译的 shader 需求。当前 FP16 主计算路径中的部分 shader 声明 scalar float8 转换能力；设备/驱动不满足时，必须提供经过校验的兼容 shader 包，不能删除检查后继续运行。
