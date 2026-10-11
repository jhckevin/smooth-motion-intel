# Third-party notices

The root Apache-2.0 license applies to project-authored code. It does not relicense third-party files or grant rights to proprietary model assets.

| Component | License and treatment |
|---|---|
| Intel Level Zero headers, v1.19.12 | MIT. Copyright notices are preserved in the vendored headers; the complete license is in native/deps/level-zero/LICENSE. |
| Vulkan-Headers, vulkan-sdk-1.4.341.0 | External build dependency, pinned to b5c8f996196ba4aa6d8f97e52b5d3b6e70f7e4e2. Per-file Apache-2.0 / MIT licensing applies. Not included in this source archive. |
| Microsoft Windows SDK, MSVC, .NET Framework/WPF | External dependencies. Their licenses remain applicable; no development toolchain is bundled. |
| Converted NVIDIA Smooth Motion model assets | Included only in the Full binary release under MODEL-NOTICE.md; not relicensed by the project. The original driver DLL is not included. Third-party rights and applicable terms remain separate. |
| LSFG / LSFG-VK | Engineering references only. No reference source tree, weights or vendor DLLs are bundled. The inspected LSFG-VK 2.0.0 reference uses CC BY-NC-ND 4.0; older releases have different terms. |

The embedded capture/resize DXBC headers have corresponding project HLSL and a rebuild script. They contain ordinary texture/crop/render operations, not the excluded neural model.

See [provenance review](docs/PROVENANCE.md) for evidence and limitations. This is not a clean-room certification or a grant from the vendors. A disclaimer does not supply missing redistribution permission.
