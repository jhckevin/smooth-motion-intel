# R994 flight recorder schema 1

This is a bounded CPU/API flight recorder, not a GPU profiler, full-session trace, or proof of a closed-source kernel root cause. No GPU counters, debug layer, driver commands, pixels, input content, full memory, or module directory paths are added by the core.

## Integration and storage

Link `diagnostic994.obj`; include `diagnostic994.hpp` for C++ or strict-C `diagnostic994_c.h`. Initialize and close from one lifecycle coordinator; event calls are concurrent and noexcept. Open once per engine process using `SMX_DIAGNOSTIC_DIR994` and `SMX_DIAGNOSTIC_SESSION994`. Do not reopen at retained-model Stop/Start. `smx_diag_session994(sequence)` records the host capture session identity at START. Sequence zero or overflow beyond UINT32_MAX is unknown, not inferred from a neighboring event.

The active file is `app-flight994.bin`, exactly 4,194,304 bytes: 16,384-byte header plus 32,640 fixed 128-byte slots. Open rotates at most one `app-flight994.previous.bin`; the current pair is at most 8 MiB. This core does not limit separate legacy logs, exported JSON, or user-requested diagnostic archives. The header contains a bounded 96-entry module catalog and an independent crash slot. Module names are basenames only.

A record uses a single nonblocking slot claim, negative writing sequence, positive matching begin/end committed sequences, payload CRC32 and generation checking. Busy slots are skipped; old preempted writers cannot overwrite newer generations. No event waits on a writer or performs file flush. Normal close disables admission and flushes once. If a producer is still in a write, the fixed mapping remains alive until process teardown rather than unmapping underneath it. Late isolated telemetry ENDs after close are ignored safely. Normal event APIs preserve GetLastError and errno; use smx_diag_open_error994() for an open failure.

A process crash or abrupt termination normally leaves the mapped data readable, as CPU tests demonstrate. Last-cache persistence across OS crash or power loss is NOT guaranteed. The unhandled exception filter uses preallocated memory, atomics, QPC and Win32 IDs only; no allocation, lock, driver query, stack walk, dump, or flush is issued there. It does not intercept every TerminateProcess, fail-fast, or kernel failure. Open calls are still useful when the handler cannot execute.

## Correlation and timing

Each event has a global `sequence` and a capture `session_sequence`; frame/job identifiers are only comparable in their session and stage. The engine-run ID and PID are in the header and included in export. The header's current_session_sequence and session_begin_global_sequence_hint are convenience metadata, not the basis for assigning historical events.

C++ Scope latches session identity at BEGIN, retains it through update and a delayed END/FAIL/ABORT, even after another session starts. C callers with long-lived BEGIN/END pairs must latch `fields.captureSession=smx_diag_current_session994()` themselves (UINT64_MAX explicitly means unknown; zero fields use current session). Scope destruction without end/fail produces ABORT, not success. END means the API returned; positive timeout/not-ready result codes must still be interpreted using their domain. BEGIN/END duration measures CPU-call elapsed time, not exclusive GPU execution or XMX use.

All event qpc and source_qpc values are raw QPC ticks. Divide by summary.qpc_frequency for seconds. UTC is startup FILETIME plus QPC delta; later wall-clock adjustments are not tracked. source_qpc zero means unavailable. stage-specific value0/value1 meanings are supplied by native instrumentation; generic core never fabricates source stamps. Physical display, GPU completion, and API return are different boundaries.

## Export

`diag-export994.exe input.bin output.jsonl`

Returns 0 only if format validation/export/flush succeeds. Parent should allow at most 10 seconds. Output may never be the input or a hardlink to it. Exporter does no GPU/driver work. It supports a copied live mapping but marks live_or_abnormal_snapshot=true; a concurrent copy can still omit/torn records. JSONL contains, in order:

- **summary**: schema, build_id, engine_run_id, pid, current_session_sequence, session_begin_global_sequence_hint, file_bytes, record_bytes, capacity, qpc_frequency, start_qpc, utc_filetime_base, utc_derivation, normal_close, live_or_abnormal_snapshot, requested_sequence, committed_total, overwritten_total, dropped_contention, dropped_stale_writer, valid_records, empty_slots, torn_slots, crc_invalid_slots, future_slots, retained_sequence_gaps, orphan_ends, open_begin_count, crash_present, crash_slot_invalid, module_catalog_stable, module_catalog_truncated, truncated_or_missing, scope.
- **module**: base, size, basename. A setup-time snapshot, not a continually refreshed loader trace.
- **event**: sequence, qpc, utc, pid, session_sequence, tid, stage, stage_name, action, call_id, domain, result, result_hex, frame, job, resource, fence, source_qpc, value0, value1, module, module_base, module_offset.
- **crash**: same event fields, from the dedicated crash slot. `result` is the exception code; resource is instruction address, module/base/offset locate it when the bounded catalog has the module. No stack or memory dump is implied.
- **open_begin**: same event fields; BEGIN has no matching retained END/FAIL/ABORT with its call_id.

`result` keeps signed HRESULT/VkResult/semantic interpretation; `result_hex` is its full 64-bit bit pattern, so negative 32-bit HRESULTs have ffffffff sign extension. HRESULT/Win32/exception numbers must not be conflated. Stages are named by the dictionary in diagnostic994_c.h and diagnostic994.cpp. Unknown stages export stage_name=custom with original numeric stage intact.

A retained open_begin can be an in-flight call, lost terminal record, overwrite, exception, or process termination. It does NOT by itself establish a stuck driver. orphan_ends means BEGIN is absent from the retained ring. retained_sequence_gaps is a span-of-retained-records statistic, not an independent count of dropped events; a failed overwrite may leave an older slot and widen that span. Nonzero overwritten_total means earlier history is intentionally unavailable. truncated_or_missing is true for abnormal/live snapshots, overwrite, contention/stale drops, or torn/CRC/future records. This is explicitly not a complete session log.

## CPU validation and limits

build-diagnostic994.cmd uses the already installed remote MSVC and verifies strict C header compilation. run-diagnostic994-tests.py runs only on the authorized remote Windows/WSL host. Eight cases pass: 80,000 events from four writers with ring wrap; error-state preservation; abrupt own-process termination; actual own-process access violation; concurrent close and delayed events; CRC corruption; torn commit; evidence self-overwrite rejection. Session7 BEGIN ending after session8 begins retains session7. The internal recorder selftest includes 17 assertions for slot contention, generation refusal, CRC/torn detection, and synthetic crash data. The actual exception test recovered C0000005 and the exporter module offset. No GPU or window is used. These results do not validate real-game performance overhead or explain the previous game exit.
