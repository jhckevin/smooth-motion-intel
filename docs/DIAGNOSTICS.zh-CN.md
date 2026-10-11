# 诊断与故障报告（R995，记录格式 994）

## 玩家怎么用

记录默认启用，不需要管理员权限。运行时只写本地；停止插帧后，进入“设置 → 本地数据 → 导出诊断包”。压缩包保存到程序目录的 UserData/diagnostics。导出不会上传文件，也不会开始新的捕获。

出现花屏、卡顿或闪退时，记下大致时间、现象和操作。先停止插帧，保留程序目录，再导出。请保留当时使用的程序版本，不要把旧故障日志与新版本可执行文件混在一起。正常停止也可以导出，用于区分速度不足、取消任务、迟到与显示丢失。

如果界面自身已退出，可重开同一版本后尝试导出已有会话。界面和引擎同时异常退出、尚未产生不可变归档时，按钮会明确报缺少归档，绝不拿旧会话冒充。此时保留 UserData/runtime-work 中的 app-flight994.bin 和其他 app-* 日志、UserData/desktop-events994.jsonl、UserData/sessions。下一次启动前的恢复步骤会在引擎锁允许时保全遗留日志。不要为了拿日志反复启动或压力测试故障 GPU。

## 文件导航

| 文件 | 用途与边界 |
|---|---|
| diagnosis-summary994.json | 证据索引：缺失项、原生失败、异常、未完成调用、生成核算、输入输出配置；不是自动根因判决 |
| bundle-manifest994.json | 导出文件 SHA256/大小，每项复制、排除、截断或失败状态；会话归档身份 |
| launch.json | 该引擎进程启动时的版本、程序/关键模型资源哈希、配置及环境；重复 Start 的生效状态结合 app-actual-config653.json / app-requested-effective988.json |
| archive-result.json / engine-exit.json | 捕获会话序号、进程身份、归档完整性、退出码、是否主动终止 |
| app-flight994.bin | 固定 4 MiB 的最近 API 事件环，内含独立异常槽；仍有原始证据时解码失败不删除它 |
| app-flight994.decoded.jsonl | 可逐行检索的调用时间线，错误原码、线程、会话、帧、任务、资源及 fence 关联 |
| desktop-events994.jsonl | 界面异常、IPC 开始/结束/失败、超时清理及导出事件；UTC 与 QPC |
| app-device994.json / app-vulkan-queues994.json / app-d3d-device994.json / app-bridge-adapter994.json / app-model994.json | 原生记录器写出的设备、接口及能力元数据；包含 Vulkan 能力、队列、D3D、adapter 对应和模型形状 |
| app-processing992.json / app-output993.json | 捕获、处理、最终窗口与内容矩形，是否采样缩小及是否受工作区约束 |
| app-commit700.json / app-lifecycle988.json | 生成完成、提交、取消/暂停/停止等不同去向；不能直接等同于上屏 |
| app-present.csv / app-display988.csv / app-hud988.jsonl | 展示与监测依据；需按原有字段有效性判断 OS 统计与应用提交 |
| app-telemetry-shutdown993.json / app-node-map830.csv | 遥测关闭状态和指标域映射；不把 GPU 节点利用率写成 XMX 占用率 |

离线解码：在同版本目录运行 diag-export994.exe “输入.bin” “输出.jsonl”。它只读取 CPU 文件，不创建 GPU 设备。不得让输出覆盖输入。导出器、记录格式和匹配符号随版本保存。

## 调查顺序

1. 确认 launch、archive、engine_run_id、PID 和 session_sequence。引擎跨 Stop/Start 复用时，同一 PID 不是同一次捕获；每条原生事件带会话序号，BEGIN 后跨会话返回的 END 仍归原会话。序号 0 明确表示未知。
2. 看完整性：缺文件、导出失败、CRC/半写、环覆盖、丢记录、缺 BEGIN/END 均需要写进结论。没有错误文本不等于没有故障。
3. 按 QPC 对齐捕获到帧、共享纹理交接、模型提交/等待、显示访问/复制、Present 与暂停/停止。frame/job 只在正确会话及阶段中解释；无法获取的字段为 0，不补猜。
4. 先看 FAIL/ABORT 的原始 HRESULT、VkResult 或 Win32/异常码，保留结果域。VK_NOT_READY / 超时等正值也不能只凭 END 判成功。未配对 BEGIN 只说明没有保留完成记录，可能是尚未返回、异常、覆盖或进程终止。
5. CPU 调用历时含等待/调度，不是 GPU 独占执行时间；GPU 时间戳跨度也可能含竞争。Present 返回、OS 报告显示、屏幕实际扫描输出是三个不同证据边界。
6. 对比配置/资源哈希和最后状态后再提出根因假设。需要证实画面像素错误时另行录制；本版默认不保存画面，不能凭文本证明白点或重影已修复。

## 驱动故障能查到什么

记录 API 返回原码、故障前调用、关联帧/资源/fence、队列及已获得的 adapter/driver 身份；原生未处理异常保留异常码、指令地址和已记录模块偏移。匹配 PDB 供维护者将自有模块偏移映射到源码。

这不是完整栈或进程转储；不会把最后调用或异常所在模块自动当成根因。记录器本身不调用 GPU。R994 没有新增高频驱动查询，也没有启用调试层、验证层、DRED 或改变 Vulkan device 的功能链。Vulkan device-fault 扩展是否被枚举、是否启用、是否实际取得故障数据必须分开看；未启用时不能声称有 GPU fault dump。当前 D3D11 路径不能伪称拥有 D3D12 DRED。

应用 API 证据无法保证定位闭源驱动内部缺陷、内核故障、掉电或硬件错误。CPU 未处理异常回调不保证覆盖 fail-fast、强制杀进程及系统崩溃；固定环通常还能保留最近事件，但掉电持久性没有保证。这些情况明确标为未知，必要时再征求用户授权使用厂商/系统诊断。

官方边界参考：
- [D3D11 GetDeviceRemovedReason 返回范围](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-getdeviceremovedreason)
- [Vulkan device fault 扩展](https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_device_fault.html)

## 性能、容量与隐私

热路径写固定记录，不逐帧刷盘、不分配日志字符串、不在异常处理器中查询 GPU。核心并发写入遇到槽竞争会丢弃并计数，避免为了日志拖住渲染。记录所有模型内部节点会改变性能，所以默认只加粗粒度边界；它不是完整 GPU profiler。

原生单环 4 MiB、32,640 条记录；最多保留一个前环，核心文件合计最多 8 MiB。保留时长取决于事件速率，不承诺完整记录 2–4 小时每一帧。界面事件约 2 MiB 后轮换一个旧文件。旧 CSV/遥测继续使用已有日志预算，关键边界环持续记录最近事件；长期历史细节可能已覆盖或预算截断。

一次导出最多选 256 个源文件、单文件 20 MiB、源快照总量 128 MiB、扫描最多 15 秒，离线解码最多 10 秒。生成的 JSON/索引另计，因此最终解压大小以清单 total_bytes 为准。未收录文件明确标注，不静默宣称完整。历史 sessions、导出 ZIP 和维护符号包累计占用不受单环限制；请保留有价值故障后自行清理旧包，本版不自动删除未分析故障。

导出仅允许已登记诊断文件，排除图片和未知二进制。默认没有截图、键鼠/手柄输入、完整内存或自动上传。程序目录和用户目录路径替换，配置名称与匹配过滤词移除；游戏名称、模块名、其他路径或错误消息仍可能出现，不承诺任意文本完全匿名，分享前请查看清单。

诊断格式沿用 R994；R995 另有适配器、队列与互操作能力选择变更，不能把 R994 的 GPU 验证直接当作所有设备的验证。远程 CPU 故障注入及本机离线验收不等于真实游戏开销或 2–4 小时稳定性验证；需要后续实测。

## 建议的开源 Issue 内容

复制同目录 ISSUE-TEMPLATE.zh-CN.md，填写可复现操作、预期/实际现象、大致时间、游戏与模式、输入/处理/输出尺寸、倍率、捕获 API、显示设置及是否改过功耗/限帧。附诊断包和 release 清单标识。不要只截一行报错，也不要先删原始日志。

维护者应保留与发行二进制匹配的源码、schema、PDB 及 CPU 故障用例报告。源码与不含模型的客户端可以公开发布；用户诊断日志不会自动上传。
