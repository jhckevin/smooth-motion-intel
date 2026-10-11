# 转换模型包使用前声明 / Converted model package notice

## 简体中文

本应用包附带与当前引擎匹配的、已经导出和转换的 Smooth Motion 模型资产：

- 模型权重与预计算参数；
- 导出的计算图；
- 适配 Intel Vulkan/XMX 的 SPIR-V 推理着色器；
- 启动验证用的合成输入及参考结果。

运行时读取这些文件，不直接加载 NVIDIA 原装 Smooth Motion DLL。它也不具备从原装 DLL 自动提取、转换模型的功能。仅有 RTX 显卡、驱动或原装 DLL 不足以生成此模型包。

这是第三方移植研究项目，维护者免费提供，未向 NVIDIA、Intel、Lossless Scaling 或游戏厂商申请或取得官方认证。本项目没有隶属或赞助关系。

模型所涉及的第三方权利仍属于相应权利人。本项目的 Apache-2.0 许可仅适用于项目原创代码，**不将第三方模型权重、计算图或相关资产重新许可为 Apache-2.0**。转换、优化、非盈利发布和免责声明都不构成权利人授权。用户应自行确认适用许可；不得将此声明理解为 NVIDIA 已允许这套转换模型在 Intel 上使用或再分发。

[NVIDIA 官方驱动许可](https://www.nvidia.com/en-us/drivers/nvidia-license/)对第三方二进制的修改、分发及适用硬件设有限制。它是否完整适用于具体版本和组件，应按随来源版本附带的实际条款核实。本声明不宣称已获得额外转换或再分发许可，不提供法律结论。

软件与模型按原样提供。可能产生画面瑕疵、延迟、设备错误或程序退出；无性能、画质、兼容性或持续稳定性的保证。请保留原软件及重要数据，出现持续花屏、device lost 或严重响应问题时停止并保存诊断记录。

首次启动的确认仅表示已阅读此声明，**不会授予缺失的第三方权利或免除适用义务**。不愿接受这些限制与风险时，请退出并删除此应用包。

## English

This application bundle contains a previously exported and converted Smooth Motion model package matched to the current runtime: weights and precomputed parameters, an exported computation graph, Intel Vulkan/XMX SPIR-V inference shaders, and synthetic startup-validation inputs/reference results.

The runtime reads those converted files. It neither loads the original NVIDIA Smooth Motion DLL nor automatically extracts/converts a model from it. Owning an RTX GPU, a driver or the original DLL does not itself produce this package.

This is an independent porting/research project provided without charge by its maintainer, without affiliation, sponsorship or certification by NVIDIA, Intel, Lossless Scaling or game publishers.

Third-party rights remain with their respective owners. Apache-2.0 covers project-authored code; **it does not relicense third-party weights, graphs or model assets**. Conversion, optimization, non-profit distribution and disclaimers do not constitute permission from the rights holder. This notice must not be read as NVIDIA authorizing Intel use or redistribution of this converted model.

Consult the terms actually accompanying the source version/component, including the [official NVIDIA driver license](https://www.nvidia.com/en-us/drivers/nvidia-license/). This project does not claim additional conversion or redistribution permission and gives no legal determination.

The software/model are provided as-is, with no guarantee of performance, quality, compatibility or sustained stability. Keep backups. Stop and preserve diagnostics after persistent corruption, device loss or severe responsiveness problems.

The first-launch acknowledgement records that you read this notice; **it grants no missing third-party rights and waives no applicable obligations**. If you do not wish to proceed under these limitations and risks, exit and remove the bundle.
