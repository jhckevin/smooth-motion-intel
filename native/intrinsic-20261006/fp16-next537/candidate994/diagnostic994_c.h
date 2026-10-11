#pragma once
#include <stdint.h>
#include <stddef.h>
#ifndef __cplusplus
#include <stdbool.h>
#define SMX_DIAG_NOEXCEPT994
#define SMX_DIAG_DEFAULT994(x)
#else
#define SMX_DIAG_NOEXCEPT994 noexcept
#define SMX_DIAG_DEFAULT994(x) =x
#endif
// Strict C ABI. Initialize fields with {0} in C or {} in C++.
// sourceQpc is raw QPC ticks; stage-specific value0/value1 document other domains.
enum SmxDiagDomain994 {SMX_DIAG_NONE994=0,SMX_DIAG_WIN32994=1,SMX_DIAG_HRESULT994=2,SMX_DIAG_VKRESULT994=3,SMX_DIAG_SEMANTIC994=4,SMX_DIAG_EXCEPTION994=5};
enum SmxDiagAction994 {SMX_DIAG_BEGIN994=1,SMX_DIAG_END994=2,SMX_DIAG_FAIL994=3,SMX_DIAG_ABORT994=4,SMX_DIAG_INSTANT994=5};
enum SmxDiagStage994 {
 SMX_DIAG_ENGINE994=1,SMX_DIAG_SESSION994=2,SMX_DIAG_MODEL_INIT994=3,SMX_DIAG_FOREGROUND_WAIT994=4,
 SMX_DIAG_CAPTURE_ACQUIRE994=10,SMX_DIAG_CAPTURE_COPY994=11,SMX_DIAG_CAPTURE_SIGNAL994=12,SMX_DIAG_CAPTURE_RELEASE994=13,
 SMX_DIAG_MODEL_SUBMIT994=20,SMX_DIAG_MODEL_FENCE994=21,SMX_DIAG_MODEL_COMPLETE994=22,
 SMX_DIAG_DISPLAY_CREDIT994=30,SMX_DIAG_DISPLAY_ACCESS994=31,SMX_DIAG_DISPLAY_COPY994=32,SMX_DIAG_DISPLAY_SIGNAL994=33,
 SMX_DIAG_DISPLAY_FLUSH994=34,SMX_DIAG_PRESENT994=35,SMX_DIAG_DISPLAY_STATS994=36,SMX_DIAG_DISPLAY_CLOSE994=37,
 SMX_DIAG_DEVICE_REASON994=40,SMX_DIAG_TARGET_CLOSED994=41,SMX_DIAG_PAUSE994=42,SMX_DIAG_RESUME994=43,
 SMX_DIAG_STOP994=44,SMX_DIAG_FATAL994=45,SMX_DIAG_UNHANDLED994=46,SMX_DIAG_TELEMETRY994=47,
 SMX_DIAG_MODULES994=48,SMX_DIAG_END_PROCESS994=49,SMX_DIAG_CAPTURE_FLUSH994=1000,SMX_DIAG_POOL_CLOSE994=1001,SMX_DIAG_GEOMETRY994=1002,
 SMX_DIAG_D3D_WAIT994=1003,SMX_DIAG_KEYED_ACQUIRE994=1004,SMX_DIAG_KEYED_RELEASE994=1005,
 SMX_DIAG_MODEL_QUERY994=1006,SMX_DIAG_MODEL_IDLE994=1007,SMX_DIAG_WGC_TRYGET994=1008,
 SMX_DIAG_EXCEPTION_CAUGHT994=1009,SMX_DIAG_CUSTOM_BASE994=1100
};
typedef struct SmxDiagFields994 {
 uint64_t frame,job,resource,fence;
 int64_t sourceQpc,value0,value1;
 uint64_t captureSession; // 0=current; UINT64_MAX=explicit unknown. Scopes latch at BEGIN.
} SmxDiagFields994;

#ifdef __cplusplus
extern "C" {
#endif
bool smx_diag_open994(const wchar_t*directory,const char*sessionId,const char*buildId) SMX_DIAG_NOEXCEPT994;
uint32_t smx_diag_open_error994(void) SMX_DIAG_NOEXCEPT994;
void smx_diag_session994(uint64_t sessionSequence) SMX_DIAG_NOEXCEPT994;
uint64_t smx_diag_current_session994(void) SMX_DIAG_NOEXCEPT994;
uint64_t smx_diag_begin994(uint32_t stage,const SmxDiagFields994*fields SMX_DIAG_DEFAULT994(NULL)) SMX_DIAG_NOEXCEPT994;
void smx_diag_event994(uint32_t stage,uint16_t action,uint64_t callId,uint16_t domain,int64_t result,const SmxDiagFields994*fields SMX_DIAG_DEFAULT994(NULL)) SMX_DIAG_NOEXCEPT994;
void smx_diag_refresh_modules994() SMX_DIAG_NOEXCEPT994; // Only from ordinary safe setup, never a handler.
bool smx_diag_install_handlers994() SMX_DIAG_NOEXCEPT994;
void smx_diag_close994() SMX_DIAG_NOEXCEPT994;
// Offline exporter: returns Win32-like status (0 success); never queries a GPU.
uint32_t smx_diag_export994(const wchar_t*input,const wchar_t*output) SMX_DIAG_NOEXCEPT994;
const char*smx_diag_stage_name994(uint32_t stage) SMX_DIAG_NOEXCEPT994;
uint32_t smx_diag_selftest994(const wchar_t*testDirectory) SMX_DIAG_NOEXCEPT994;

#ifdef __cplusplus
}
#endif
