# Converted model assets

[English](MODEL-ASSETS.en.md) · [简体中文](MODEL-ASSETS.zh-CN.md)

The **Full Windows release** includes a converted Smooth Motion package matched to this engine. The source checkout and automatic GitHub Source code archives do not contain the model.

Included assets: weights/precomputed parameters, an exported computation graph, Intel Vulkan/XMX SPIR-V inference shaders, and synthetic validation inputs/reference results. The runtime reads these files, not the original NVIDIA Smooth Motion DLL.

No manual import is needed for the Full bundle. The original DLL, even with an RTX GPU and driver, is not a usable input to this importer.

## Optional import for developers/replacement packages

The importer validates sizes and SHA256 against assets988.json, then copies a **previously converted, matching package** to Destination/model. It does not download, extract, convert or execute a vendor DLL. It changes no driver, registry or global environment.

~~~powershell
.\tools\import-model.ps1 -Source "D:\MyConvertedSmoothMotionModel" -Destination "." -WhatIf
.\tools\import-model.ps1 -Source "D:\MyConvertedSmoothMotionModel" -Destination "."
~~~

Source may be a previous client root containing UserData/runtime-work and UserData/runtime-shaders, or a converted model root containing the manifest's graph922.bin, assets and shaders. For a source checkout's build output, use -Destination ".\dist".

Integrity checks do not establish rights. Read [the model notice](../MODEL-NOTICE.md). Third-party model assets are not licensed under the project's Apache-2.0 code license.

## Capability contract

Some current FP16-pipeline shaders require scalar float8 conversion. Other GPU/driver requirements remain applicable; bundling a model does not provide unsupported hardware features. Missing support requires validated compatible shaders, not bypassing checks.
