# Source publication provenance boundary

Audit date: 2026-10-11. This is a bounded source/provenance review, not a claim that every historical contribution was independently authored or that proprietary interoperability has been legally cleared.

## Project license scope

Apache License 2.0 may be applied to the project's own desktop, capture, presentation, synchronization, telemetry, and diagnostic implementation. Existing third-party notices and licenses remain applicable. The project license does not grant rights to NVIDIA or Lossless Scaling weights, recovered graphs, private shaders, driver DLLs, trademarks, or third-party patents.

The reviewed source archive does not contain standalone model weights, SPIR-V files, or vendor DLLs. The runtime still requires a separately obtained compatible model package. Open source for this runtime does not make that model package open source.

The three embedded DXBC headers reviewed are ordinary capture/resize shaders. Corresponding HLSL and build commands were found for `capture_gpu_shader.h`, `window_normalize795_vs.h`, and `window_normalize795_ps.h`. They perform texture loading/sampling and geometry setup, not neural inference. Include that HLSL and portable rebuild instructions alongside the generated headers. The audit did not recompile them; matching source records and generated-file hashes are provenance evidence, not a reproducible-build claim.

The application icon is composed of six PNG members matching the project's software-rendered icon assets. The renderer uses `VectorIcons989.cs` geometry, and the icon manifest records that source hash. No third-party logo was identified in this chain. This is a source/generation-chain finding, not a guarantee of originality of every simple geometric shape.

## Required exclusions

`EXCLUDE-ASSETS.json` identifies every runtime model asset that must stay outside the public source and releases under the current plan: 227 files (194 SPIR-V, 32 BIN, one RGBA file), plus the two separately loaded native969 SPIR-V files. This includes `graph922.bin`, weights, precomputed parameters, captured/test image data, and compiled neural shaders. The JSON is an exclusion/identity list, not a rights grant.

Keep vendor DLLs, driver packages, extraction/rehosting tools, private runtime data, screenshots, logs and diagnostic dumps out of the source repository. `assets988.json` may remain as metadata; its existence and hashes do not establish rights to the referenced files.

Do not bundle the inspected LSFG-VK source tree or the separately inspected Smooth-Motion-for-RTX30 source snapshot under this project's Apache license. The latter snapshot has a license badge and disclaimer but no complete LICENSE/COPYING/NOTICE text; no sufficient standalone grant was established by this audit.

## Third-party notices

| Component | Evidence | Publication handling |
|---|---|---|
| Intel Level Zero `ze_api.h`, `zes_api.h` | Headers identify v1.19-r1.19.12, Intel copyright, SPDX MIT | Keep notices and include the complete MIT text/origin/version when vendoring. Do not replace their license with Apache. |
| Khronos Vulkan-Headers | Build dependency; reviewed SDK 1.4.341.0. Header licensing is per file (Apache-2.0 or MIT), and the reviewed `vulkan_core.h` is Apache-2.0 | If distributed, keep per-file notices and corresponding full texts. If external, document the pinned dependency. |
| Windows SDK, WPF/.NET, MSVC | External build/runtime dependencies | Do not redistribute their toolchains or headers as project-authored source. Applicable binary redistribution terms remain separate. |
| LSFG-VK references | The inspected 2.0.0 snapshot has CC BY-NC-ND 4.0; three key files and LICENSE match that official tag | General engineering reference is not a license to distribute adapted source. No LSFG-VK source is included in the publication boundary. |

## LSFG-VK version-specific license finding

The official author describes MIT for v1, GPLv3 during the v2 rewrite, and a later change to CC BY-NC-ND 4.0. The inspected local 2.0.0 reference is the later CC release, not an assumed earlier GPL version. The separately accessible archived `v2.0.0-dev` license is GPLv3. Previously granted GPL rights are governed by that license and do not disappear merely because later revisions use another license; using that history requires identifying the actual earlier code/version and complying with its terms.

CC BY-NC-ND 4.0 permits noncommercial sharing of unadapted licensed material and producing/reproducing adaptations, but does not license sharing adapted material. Adding attribution alone does not resolve that restriction. If copied protected source is later identified, obtain the required permission or replace it before distributing the affected component. This audit does not infer copying solely from implementing similar ideas or APIs.

Sources: [LSFG-VK license change announcement](https://lsfg-vk.dev/blog/important-changes-to-lsfg-vk/), [official 2.0.0 license](https://git.lsfg-vk.dev/lsfg-vk/plain/LICENSE.txt?h=2.0.0), [archived v2.0.0-dev GPL license](https://git.lsfg-vk.dev/lsfg-vk-archive/plain/LICENSE.md?h=v2.0.0-dev), [CC BY-NC-ND legal code](https://creativecommons.org/licenses/by-nc-nd/4.0/legalcode.en), [GPLv3 text](https://www.gnu.org/licenses/gpl-3.0.html).

## Review limitations

Exact normalized-text comparisons found no matching substantial 8-line windows or 3-line windows of at least 180 characters between the reviewed native project sources and the inspected LSFG-VK/upstream reference sources. This does not detect renamed, translated, or structurally rewritten copying, and is not a clean-room certification. The runtime's project-specific graph bindings still reflect an interoperability target; the audit does not decide copyright, contract, patent, or anti-circumvention questions for every jurisdiction.

No GPU tests, model extraction, recompilation, publication, or upload was performed for this review. Final public artifacts should be checked against the exclusion list, preserve third-party notices, and avoid describing the whole NVIDIA technology or model as open source.
