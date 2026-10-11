# Capability-gated Intel native runtime

This source tree is derived from the frozen R994 runtime. No model weights, model graph binaries or SPIR-V files are distributed here. Supply a compatible, legally obtained model package through the desktop importer. Historical numeric suffixes name existing interfaces, not GPU model IDs.

## Build
Run `build.cmd <Vulkan-Headers include directory> [OUT_DIR]` in an x64 Visual Studio Developer Command Prompt with the Windows SDK installed. `VULKAN_HEADERS` and `OUT_DIR` environment variables are optional alternatives. The default output is `../dist`. Tested compiler: MSVC 19.44, Windows SDK 10.0.26100; Vulkan-Headers 1.4.341. The build does not install tools or alter system configuration.

Outputs: `SmoothMotion-Engine.exe`, `diag-export994.exe`, `capability995-test.exe`. PDB files are maintenance symbols, not required at runtime; CodeView refers only to the PDB basename. `rebuild-shader-headers.cmd [optional destination root]` regenerates the three first-party embedded DXBC headers using Windows SDK fxc. Their capture/normalizer HLSL sources are included. Exact shader payload reproduction was verified remotely; no GPU test was used for this check.

## Device contract
No B390 or B580 device-ID allowlist is used. Choose the first Intel adapter passing actual Vulkan capability checks; D3D must resolve the identical actual adapter LUID. B390 is the previously tested/recommended platform. B580 and other Intel GPUs remain untested; names alone do not establish support, throughput, power, or visual correctness.

Vulkan 1.3 is required. Required device extensions: `VK_KHR_cooperative_matrix`, `VK_EXT_shader_float8`, `VK_KHR_external_memory_win32`, `VK_KHR_win32_keyed_mutex`, `VK_KHR_external_semaphore_win32`, and `VK_KHR_swapchain`. The current shader package declares scalar float8 conversion even though the matrix work is FP16. Thus shaderFloat8 is required unconditionally for this package; shaderFloat8CooperativeMatrix is explicitly disabled. Removing this requirement needs validated replacement shaders, not a weaker check.

Required features include shaderInt64, shaderInt8, shaderFloat16, storageBuffer16BitAccess, vulkanMemoryModel, timelineSemaphore, maintenance4, subgroupSizeControl, computeFullSubgroups, cooperativeMatrix, and BGRA storage writes without a format. The queried matrix shape must be subgroup-scope 8x16x16 with FP16 A/B/C/result and non-saturating accumulation. Required compute subgroups are 16 and 32, with basic and shuffle operations. The current shader package requires FP16 denorm/signed-zero/Inf/NaN preservation and RTE, 256-thread workgroups, and 64-byte push constants. The actual BGRA shared-image import and timeline D3D12-fence handle import capabilities are queried separately.

Actual Vulkan image/dispatch limits, overflow-safe image estimates, model allocation bytes, and matching DXGI local-memory budget continue to gate allocation. Passing a capability probe is not allocation or rendering validation.

Compute queues are selected dynamically, preferring a timestamp-capable dedicated compute family. No family number is required. Optional Vulkan WSI uses a different available timestamp-capable family and is rejected if none exists; default DXGI/DComp does not require this extra queue. It never aliases two independently locked users onto the same Vulkan queue. Default queue priority is ordinary. Explicit low/medium/high is process-local; when the global-priority extension is absent or creation returns VK_ERROR_NOT_PERMITTED, the request is logged and ordinary priority is used. Other errors are not retried. Realtime is prohibited.

Optional extensions: VK_EXT_global_priority (only explicit priority requests), VK_KHR_pipeline_executable_properties (only optional statistics), VK_EXT_calibrated_timestamps (required only when the selected diagnostics configuration requests calibrated timing). Device-fault extensions are reported if enumerated and remain disabled. D3D11 DRED is unavailable. No new fault driver queries or GPU synchronization were added.

Telemetry resolves Level Zero LUID to core UUID and then Sysman UUID, with no model-name or device-ID-only fallback. If exact identity is unavailable, GPU telemetry stays unavailable rather than reporting another adapter. CPU package power keeps its existing distinct domain attribution.

## Validation and diagnostics
`SmoothMotion-Engine.exe --device-capabilities995` writes `app-capabilities995.jsonl` in the current directory. It creates only a Vulkan instance and reads capabilities; no device, model, window or GPU work is submitted. Exit 0 means at least one eligible adapter; 3 means unavailable/unsupported, 2 means report-file failure. Memory budget, allocation, D3D device creation and actual performance are deliberately untested by this command.

`capability995-test.exe` is a separate CPU-only executable, not an engine command-line switch. It covers B390/B580 capability mocks, arbitrary future device IDs, missing capabilities, same-vendor/device multi-GPU LUID identity, dynamic queue families and restricted priority retry. See `tests/cpu995-results.json` for the nine retained engine CPU selftest flags, plus this policy test. No public995 GPU or B580 physical test has been performed.

Only the new capability-probe report filename is added. Existing R994 metadata filenames and decoder schema are unchanged. `diag-export994.exe input.bin output.jsonl` remains the offline flight-recorder decoder. Full module PDBs should be retained privately for maintenance.

## Publication list
`public-source-files995.json` is the exact source whitelist. It excludes private remote build helpers, build/dist outputs, old R994 regression logs and manifests. No model `.bin`, `.spv`, weights or third-party frame-generation reference code is included. Level Zero header copyright/SPDX notices and the complete upstream MIT license are retained in `deps/level-zero`.
