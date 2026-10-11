# Local model assets

[English](MODEL-ASSETS.en.md) · [简体中文](MODEL-ASSETS.zh-CN.md)

Neither the public repository nor the release ZIP includes NVIDIA/LSFG weights, recovered graphs, proprietary neural shader binaries or vendor DLLs. assets988.json contains file paths, sizes and SHA256 hashes, not the model or a license to it.

## Import into the portable Windows release

If you have a matching model package and the right to use it with this project, open PowerShell in the extracted application directory:

~~~powershell
.\tools\import-model.ps1 -Source "D:\MySmoothMotionModel" -Destination "." -WhatIf
.\tools\import-model.ps1 -Source "D:\MySmoothMotionModel" -Destination "."
~~~

The first command checks without copying. Destination must be the directory containing SmoothMotion.exe. If working from a source checkout whose compiled application is in dist, use -Destination ".\dist" instead.

Source can point to a previous client root with UserData/runtime-work and UserData/runtime-shaders, or to a matching model root with graph922.bin, assets, shaders and the files in the manifest.

The importer verifies size and SHA256 before copying to Destination/model. It rejects traversal, reparse paths, reserved receipt paths and duplicate destinations. It does not download, extract or run vendor DLLs, and does not change the registry, drivers or global environment. Missing or mismatched assets cause an explicit failure.

Integrity checks are not proof of rights. Without a compatible model package, the public client cannot perform frame generation. This project does not provide a model mirror or automatic driver extractor.

## Shader and device requirements

A model package's compiled shaders have specific requirements. Some shaders in the current FP16 pipeline declare scalar float8 conversion. If the GPU/driver does not support them, validated compatible shaders are needed; bypassing the capability check is not a solution.
