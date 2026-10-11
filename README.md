<p align="center"><img src="art/logo.svg" width="900" alt="Smooth Motion Intel — Vulkan / XMX frame generation"></p>

<p align="center"><strong>English</strong> · <a href="README.zh-CN.md">简体中文</a></p>
<p align="center">Windows desktop frame generation · Optimized for Intel XMX · Arc B390 recommended</p>

# Smooth Motion Intel

An unofficial Smooth Motion port and Windows desktop client for Intel GPUs. It runs custom inference through Vulkan, captures with Windows Graphics Capture or DXGI Desktop Duplication, and presents through D3D/DXGI. It does not load NVIDIA models through the XeSS SDK and is not an official NVIDIA, Intel, or Lossless Scaling product.

**[Download for Windows x86_64](https://github.com/jhckevin/smooth-motion-intel/releases/tag/v0.1.0-preview.2)** · [Quick start](START-HERE.md) · [中文说明](README.zh-CN.md)

The portable ZIP includes the desktop application, native inference runtime, offline diagnostic decoder, local model importer, documentation and license notices. **NVIDIA weights, recovered model graphs, proprietary neural shaders and vendor DLLs are not included. A compatible model package that you are entitled to use must be imported before frame generation can run.** This is a compiled application package, not just source code; it is not a ready-to-run model bundle.

## Author's introduction and experience

The following translates the author's project description, preserved verbatim on the [Chinese page](README.zh-CN.md). Quality comparisons are personal observations on the author's hardware, not independent measurements or guarantees for other devices.

> This is an Intel-optimized version of NVIDIA Smooth Motion, developed and optimized on the B390 integrated GPU. Other Intel GPUs have not been tested, though I expect GPUs with XMX to be good candidates. I tested 2× frame generation in Forza Horizon 6 and found it generally usable. To my eyes, quality is nearly unchanged from the NVIDIA version and comparable to LSFG 3.1.
>
> It is more broadly applicable than engine-integrated XeSS MFG and can also interpolate video; I recommend making the player fullscreen first. I expect it to be less efficient and effective than XeSS MFG when that has access to engine motion vectors. Intended uses include video, older games without XeSS support, and Minecraft.
>
> The port was originally made for Panther Lake and adapted to the B390 iGPU. My reported test driver is 32.0.101.8274. Other Intel GPUs may be able to run it, but I have not tried them. Model inference uses Vulkan.
>
> Reproducible, useful reports are welcome; I may improve the project. You can export a diagnostic bundle to help investigate compatibility issues.

Technical qualifications:

- **XMX refers to hardware matrix units; XeSS 3 is a software technology/SDK, not a type of GPU core.** XeSS support or the presence of XMX alone does not establish compatibility with these Vulkan shaders.
- The author's reported driver version is not mandatory. Diagnose a session using the actual device and driver identity in its logs. The application does not install or switch drivers.
- “Nearly unchanged” and “comparable to LSFG 3.1” are subjective. Fast motion, occlusions, thin lines, HUDs, recursive interpolation and sustained power limits may expose differences. No universal quality score or superiority claim has been established.
- Desktop capture normally lacks engine-provided motion vectors. Cost and quality comparisons with XeSS MFG require controlled measurements on the same hardware and scene.

## Hardware support

| Device | Status |
|---|---|
| Intel Arc B390 / Panther Lake | Recommended; used for development and earlier Forza Horizon 6 2× testing |
| Intel Arc B580 / B570 discrete GPUs | No model-ID block; actual capabilities are checked. Not physically tested |
| Other Intel XMX GPUs | May be attempted if the runtime and shader requirements are satisfied; untested |
| AMD, NVIDIA, or devices lacking the required matrix features | Not current optimization or validation targets; support is not promised |

R995 removes the B390-only device-ID check and fixed queue-family assumptions. It selects an eligible Intel Vulkan adapter and binds D3D to the same actual LUID. Requirements and rejection reasons are logged.

The current model package requires **scalar FP8 conversion support**, although its main matrix calculations are FP16. This is not FP8 matrix inference. Missing extensions require validated replacement shaders; changing the GPU name does not provide compatibility.

See the [native capability contract](native/README-native995.md) and [Chinese compatibility guide](docs/COMPATIBILITY.zh-CN.md). A successful capability probe is not proof of successful allocation, inference or gaming performance.

## Quick start

1. Use Windows x86_64 (x64/AMD64), with an existing Intel graphics driver exposing the required Vulkan features. Windows 10/11 are the intended desktop platforms; individual capture/presentation features have additional requirements. The optional scheduled backend needs newer Windows APIs and rejects unsupported systems.
2. Download **SmoothMotion-Intel-R995-Windows-x86_64.zip** from [Releases](https://github.com/jhckevin/smooth-motion-intel/releases/tag/v0.1.0-preview.2). Extract the entire ZIP to a writable directory. No compiler, Python or conda installation is needed to use this package.
3. Import a compatible model package you are entitled to use. From the extracted directory, run:

~~~powershell
.\tools\import-model.ps1 -Source "D:\MySmoothMotionModel" -Destination "."
~~~

   Add -WhatIf to validate without copying. The source can be a previous client directory containing UserData/runtime-work and UserData/runtime-shaders, or a matching model directory. The importer checks file sizes and SHA256 against assets988.json; it does not download models or extract/execute vendor DLLs. See [model import details](docs/MODEL-ASSETS.en.md).
4. Open your game, preferably windowed or borderless, then launch **SmoothMotion.exe**. Select the target window and begin with **2×**, **WGC** and the game-window overlay. The current client interface is Chinese; these bilingual pages do not change the application language.
5. **Ctrl+Alt+S** starts/stops; **Ctrl+Alt+Esc** is the emergency stop. Switching away, minimizing or closing the target pauses/stops the relevant capture work.
6. Stop and preserve logs if you see persistent corruption, device loss or severe responsiveness problems. Do not disable system protection or repeatedly stress a failing GPU.

**Video:** select a player window and preferably make it fullscreen first. DRM-protected content, minimized windows and some composition paths may not be capturable. This is not an offline video transcoder.

## Client features

- Recommended **2×** generation; experimental **4×/8×** use recursive generation and add inference work, power use and accumulated error.
- WGC window capture and experimental DXGI Desktop Duplication, subject to foreground/monitor constraints.
- Game-window overlay, borderless fullscreen preview, processing-size/1080p/small-window preview options.
- Separate capture, processing and output dimensions. Keep native processing or explicitly select proportional 1080p/1440p limits.
- Input pass-through, game profiles, shortcuts, tray integration and continuous operation. Detached previews do not offer complete absolute-mouse remapping across positions.
- Native/generated/displayed-frame monitoring, pacing and latency metrics. Package power and the GPU driver power domain are labeled separately; unverified sensors show unavailable.
- Correlated local diagnostics and an exportable ZIP after capture stops safely.

Detailed controls and limitations are in the [Chinese user guide](docs/USER-GUIDE.zh-CN.md). This release's validation scope is recorded in [R995 validation](docs/VALIDATION-R995.md).

## Resolution, latency and performance

Native processing does not intentionally downscale the outer input/output. It does not mean every internal layer uses full resolution: the current model includes a fixed 768×432 multiscale grid. Larger inputs still increase capture, full-resolution warp, texture and composition costs.

Successful submission is not the same as actual display. Evaluate 30→60 or 60→120 using effective content, displayed-frame pacing, base-frame performance, latency and power together. Stable doubling under every device or 20 W/PL1 condition is not promised; the 3 W and 6 W goals are not claimed as achieved. Longer timeline buffering may reduce lateness at the expense of input latency.

Continuous operation is supported, but the absence of a fixed test timeout is not a 2–4-hour stability certification for every device.

## Diagnostics and feedback

[Open an issue](https://github.com/jhckevin/smooth-motion-intel/issues) with the game/player, reproduction steps, approximate time, hardware/driver, capture/processing/output dimensions and power/frame-limit conditions.

After safely stopping, select **设置 → 本地数据 → 导出诊断包** (Settings → Local data → Export diagnostic bundle). Records include raw errors, call boundaries, session/frame/task/resource identifiers, capabilities and file hashes. Export does not require administrator rights and does not upload anything. Screenshots and keyboard/mouse input are not collected by default. Review game titles and paths before sharing a bundle in a public issue.

[Diagnostic guide (Chinese)](docs/DIAGNOSTICS.zh-CN.md) · [Record schema](docs/DIAGNOSTIC994-SCHEMA.md) · [Issue template](https://github.com/jhckevin/smooth-motion-intel/issues/new?template=bug_report.yml)

Logs narrow the investigation; they cannot guarantee a root cause inside a closed-source driver, hardware or sudden power failure. Missing records, ring overwrites and untested conditions must remain visible in the analysis. The last recorded call is not automatically the cause. Response times and fixes are not guaranteed.

## Source builds

See [build instructions (Chinese)](docs/BUILD.zh-CN.md). Development requires Windows x64, Visual Studio C++ tools, the Windows SDK, .NET Framework/WPF and the pinned Vulkan-Headers dependency. Builds and CI do not fetch or bundle proprietary models.

The Windows release contains already-compiled executables. The separate source ZIP and GitHub's Source code downloads are intended for development, not normal installation.

## License and disclaimer

Project-authored code is licensed under **Apache License 2.0**. See [LICENSE](LICENSE), [NOTICE](NOTICE), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and the [provenance review](docs/PROVENANCE.md).

**That license does not grant rights to NVIDIA or other parties' models, weights, graphs, private shaders, DLLs, trademarks or patents.** Porting and interoperability work do not transfer ownership. A disclaimer is not redistribution permission; owning hardware or software does not automatically permit porting or redistributing all its contents.

This project is not affiliated with, sponsored by or certified by NVIDIA, Intel, Lossless Scaling or game publishers. Names identify compatibility targets and references, not endorsement. The project logo is not a vendor's official mark.

Software is provided as-is, without warranties of merchantability, fitness, compatibility, stability or non-infringement, subject to the license and applicable law. It may cause visual artifacts, latency, application exits or data loss. Keep backups of your original application and important data. Compatibility with game anti-cheat rules is not guaranteed.

**The open-source release covers the implementation the project can license, not all NVIDIA Smooth Motion technology or assets.** It is not an official NVIDIA open-source release or a legally certified clean-room implementation.
