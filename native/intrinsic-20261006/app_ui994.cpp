#include "../d3d_adapter995.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "fp16-next537/candidate994/bounded_stdio989.h"
#include "fp16-next537/candidate994/capture_config989.hpp"
#include "fp16-next537/candidate994/window_state990.hpp"
#include "fp16-next537/candidate994/foreground_wait992.hpp"
#include "fp16-next537/candidate994/resolution_budget989.hpp"
#include "fp16-next537/candidate994/host_control988.hpp"
#include "fp16-next537/candidate994/display_shutdown988.hpp"
#include "fp16-next537/candidate994/session_archive988.hpp"
#include <d3d11.h>
#include <share.h>
#include <io.h>

#include <dwmapi.h>
#include <d3d11_4.h>
#include <dxgi1_3.h>
#include <dcomp.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <deque>
#include <vector>
#include <string>
#include <chrono>
#include <cstdio>
#include <cerrno>
#include <cstdint>
#include <algorithm>
#include "gpu_bridge994.hpp"
#include "fp16-next537/candidate994/integration994_selftest.hpp"
#include "fp16-next537/candidate994/hud_monitor988.hpp"
#include "fp16-next537/candidate994/hud_render988.hpp"
#include "fp16-next537/candidate994/scaling992/processing_size992.hpp"
#include "fp16-next537/candidate994/scaling992/window_normalize992.hpp"
#include "fp16-next537/candidate994/output_geometry993.hpp"
#include "fp16-next537/candidate994/output_geometry993_selftest.hpp"
#include "fp16-next537/candidate994/output_background993.hpp"
extern "C" int smx_graph_extent830(uint32_t,uint32_t);
static std::atomic<bool> nativeRestart830{false};
static std::atomic<uint64_t> diagnosticCaptureEpoch994{0};
#include "deadline_guard930.hpp"
#include "reserved_timeline989.hpp"
#include "fp16-next537/candidate994/natural_end988.hpp"
#include "fp16-next537/candidate994/multiphase989.hpp"
#include "fp16-next537/candidate994/display_fence_probe988.hpp"
#include "frame_budget_r6.h"
#include "frame_output_balance_r9.h"
#include "frame_resource_r30.hpp"
#include <functional>
#include "display_rates.hpp"
#include "capture_display_session.hpp"
#include <array>
#include <memory>
#include "fp16-next537/candidate876/scheduled_present876.hpp"
static void dumpLiveProof(ID3D11Device*d,ID3D11DeviceContext*c,ID3D11Texture2D*t,const char*path,bool bgra){
 D3D11_TEXTURE2D_DESC desc{};t->GetDesc(&desc);if(desc.Width!=1920||desc.Height!=1080)throw std::runtime_error("Live proof requires native 1080p");
 desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.MiscFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
 Microsoft::WRL::ComPtr<ID3D11Texture2D>staging;smxdiag994::checkedHr((d->CreateTexture2D(&desc,nullptr,&staging)),__LINE__);ComPtr<IDXGIKeyedMutex>m;std::unique_ptr<BridgeLock>lock;if(SUCCEEDED(t->QueryInterface(IID_PPV_ARGS(&m))))lock=std::make_unique<BridgeLock>(m.Get());c->CopyResource(staging.Get(),t);
 D3D11_MAPPED_SUBRESOURCE map{};smxdiag994::checkedHr((c->Map(staging.Get(),0,D3D11_MAP_READ,0,&map)),__LINE__);
 std::vector<unsigned char>bytes(size_t(desc.Width)*desc.Height*4);
 for(unsigned y=0;y<desc.Height;y++){auto*src=(unsigned char*)map.pData+size_t(y)*map.RowPitch;auto*dst=bytes.data()+size_t(y)*desc.Width*4;memcpy(dst,src,desc.Width*4);if(bgra)for(unsigned x=0;x<desc.Width;x++)std::swap(dst[x*4],dst[x*4+2]);}
 c->Unmap(staging.Get(),0);FILE*f=nullptr;smx_log_fopen_s989(&f,path,"wb");if(!f)throw std::runtime_error("Live proof file failed");bool ok=fwrite(bytes.data(),1,bytes.size(),f)==bytes.size();smx_log_fclose989(f);if(!ok)throw std::runtime_error("Live proof write failed");
}
#include "fp16-next537/candidate994/pixel_probe989.hpp"
using namespace winrt;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
extern "C" int smx_graph_main(int,char**);
static host988::Config hostConfig988;
static std::unique_ptr<host988::Server> hostServer988;
static std::atomic<bool> hostCaptureStarted988{false};
static uint64_t hostSession988=0;
static bool hostError988=false,hostQuitting988=false,hostEmergencyRegistered988=false;
static std::wstring hostMessage988=L"Ready";
enum class PreparationStage992 { Idle, ModelInitializing, CaptureSetup, WaitForeground, Capturing };
static std::atomic<PreparationStage992> preparationStage992{PreparationStage992::Idle};
static std::atomic<bool> modelReady992{false};
static std::atomic<double> preparationStageBegin992{0},modelPreparationSeconds992{0},captureSetupSeconds992{0},foregroundWaitSeconds992{0};
static std::mutex preparationLogMutex992;
static const char* preparationStageName992(PreparationStage992 stage){switch(stage){case PreparationStage992::ModelInitializing:return "model_initializing";case PreparationStage992::CaptureSetup:return "capture_setup";case PreparationStage992::WaitForeground:return "wait_foreground";case PreparationStage992::Capturing:return "capturing";default:return "idle";}}
static void setPreparationStage992(PreparationStage992 stage){
 const double now=smx988::qpcNow988();const auto previous=preparationStage992.exchange(stage);const double began=preparationStageBegin992.exchange(now),elapsed=began>0?std::max(0.0,now-began):0;
 if(previous==PreparationStage992::ModelInitializing)modelPreparationSeconds992=elapsed;
 if(previous==PreparationStage992::CaptureSetup)captureSetupSeconds992=elapsed;
 if(previous==PreparationStage992::WaitForeground)foregroundWaitSeconds992=elapsed;
 std::lock_guard lock(preparationLogMutex992);
 const std::string path=std::string(getenv("SMX_MODEL_DIR")?getenv("SMX_MODEL_DIR"):".")+"\\app-preparation-stages992.jsonl";
 if(FILE*f=smx_log_fopen989(path.c_str(),"ab")){smx_log_fprintf989(f,"{\"qpc_seconds\":%.9f,\"stage\":\"%s\",\"previous_stage\":\"%s\",\"previous_stage_seconds\":%.6f,\"model_ready\":%s,\"scope\":\"wall-clock stage boundaries, not pure GPU or shader compilation time\"}\n",now,preparationStageName992(stage),preparationStageName992(previous),elapsed,modelReady992?"true":"false");smx_log_fclose989(f);}
}
static std::string preparationFields992(){
 const auto stage=preparationStage992.load();const double began=preparationStageBegin992.load(),elapsed=began>0?std::max(0.0,smx988::qpcNow988()-began):0;
 return std::string("\"modelReady992\":")+(modelReady992?"true":"false")+",\"preparationStage992\":"+host988::quote(preparationStageName992(stage))+",\"foregroundWaitSeconds992\":"+std::to_string(stage==PreparationStage992::WaitForeground?elapsed:foregroundWaitSeconds992.load())+",\"modelPreparationSeconds992\":"+std::to_string(stage==PreparationStage992::ModelInitializing?elapsed:modelPreparationSeconds992.load())+",\"captureSetupSeconds992\":"+std::to_string(stage==PreparationStage992::CaptureSetup?elapsed:captureSetupSeconds992.load());
}

static HWND refreshModeCheck;
static bool useSession120=false;
static HWND control,output,sourceTest,combo,statusText,startButton,stopButton,sizeCombo,fullButton;
static std::vector<HWND> windows;
static std::atomic<bool> stopFlag{false},running{false};
static std::atomic<unsigned> generated{0},presented{0},originalPresented{0},generatedPresented{0},capturedOriginal{0},observedOriginal{0},observedGenerated{0},unknownObserved{0};
static HWND selected;
static HWND hud964=nullptr;
static std::mutex hudSessionMutex988;
static std::shared_ptr<smx988::HudSession988> hudSessionGlobal988;
static std::weak_ptr<smx988::HudSession988> proofSession988;
static bool proofDone988=false,hudDetails988=false;
static std::weak_ptr<smx988::HudSession988> paintedSession988;
static std::shared_ptr<smx988::HudSession988> currentHud988(){std::lock_guard<std::mutex>lock(hudSessionMutex988);return hudSessionGlobal988;}
static int hudPowerProbe988(){
 FILE*file=nullptr;smx_log_fopen_s989(&file,"hud-power-probe988.jsonl","wb");if(!file)return 2;
 HANDLE done=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!done){smx_log_fclose989(file);return 3;}
 std::thread watchdog([done]{if(WaitForSingleObject(done,5000)!=WAIT_OBJECT_0)TerminateProcess(GetCurrentProcess(),86);});int result=0;
 try{smx988::PowerSampler988 sampler;smx988::HudSession988 state;state.backend="power-probe-no-compute";
  for(unsigned i=0;i<3;++i){state.power=sampler.sample();state.json(file,smx988::qpcNow988());WaitForSingleObject(done,1000);}
 }catch(...){result=1;}
 SetEvent(done);watchdog.join();CloseHandle(done);smx_log_fclose989(file);return result;
}
static int hudSelftest988(){
 std::string error;bool metric=smx988::HudMetrics::selfTest(&error),power=smx988::PowerSampler988::selfTest();
 auto session=std::make_shared<smx988::HudSession988>();session->backend="cpu-selftest";const double now=smx988::qpcNow988();session->recordSubmitted(7,true,now-.1,now,now-.01);DXGI_FRAME_STATISTICS stats{};stats.PresentCount=7;stats.SyncQPCTime.QuadPart=10000000;
 session->raw.push_back({now,stats,S_OK,0,0,0,10000000});FILE*raw=smx_log_fopen989("hud-monitor-cpu988.csv","wb");if(!raw)return 3;smx_log_fprintf989(raw,"query_qpc,epoch,result,present_count,present_refresh_count,sync_refresh_count,sync_qpc,sync_gpu,gap_ids,mapped,kind,source_qpc,due_qpc,present_cpu_qpc,id_epoch,qpc_frequency\n");session->flushRaw(raw);
 // A count rollback must never reuse the previous ID epoch's metadata.
 session->recordSubmitted(1,false,now,now+.1,now+.01);session->raw.push_back({now+.02,stats,S_OK,0,0,0,10000000});stats.PresentCount=1;session->raw.push_back({now+.02,stats,S_OK,1,0,1,10000000});session->flushRaw(raw);smx_log_fclose989(raw);
 FILE*json=smx_log_fopen989("hud-monitor-cpu988.jsonl","wb");if(!json)return 4;session->json(json,now+.02);session->pollJson(json,now+.02);smx_log_fclose989(json);
 FILE*f=smx_log_fopen989("hud-selftest988.json","wb");if(!f)return 2;smx_log_fprintf989(f,"{\"pass\":%s,\"metrics\":%s,\"power_counter_math\":%s,\"gpu_used\":false,\"windows_created\":false,\"sysman_loaded\":false}\n",metric&&power?"true":"false",metric?"true":"false",power?"true":"false");smx_log_fclose989(f);if(!metric)smx_log_fprintf989(stderr,"%s\n",error.c_str());return metric&&power?0:1;
}

static HFONT font964=nullptr;
static HBRUSH panel964=nullptr;
static bool hudEnabled964=true,controlMinimized964=false,uiAuto964=false,visualProof964=false;
// R988 changes only our own output window. Opaque is a diagnostic alternative;
// WS_EX_TRANSPARENT/HTTRANSPARENT do not certify cross-process mouse forwarding.
static bool opaqueWindow976=false;
static std::mutex windowLogMutex976;
static bool parseWindowMode976(const char*value,bool&opaque){if(!value||!*value||!_stricmp(value,"layered")){opaque=false;return true;}if(!_stricmp(value,"opaque")){opaque=true;return true;}return false;}
static DWORD outputExStyle976(bool overlay,bool noActivate){return overlay?((opaqueWindow976?0:WS_EX_LAYERED)|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW):(WS_EX_LAYERED|(noActivate?WS_EX_NOACTIVATE:0));}
static bool exactOne976(const char*value){return value&&strcmp(value,"1")==0;}
static DXGI_SWAP_CHAIN_DESC1 compositionDesc976(unsigned width,unsigned height,unsigned /*maxLatency*/,bool opaqueContent=false){
 DXGI_SWAP_CHAIN_DESC1 desc{};desc.Width=width;desc.Height=height;desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;desc.SampleDesc.Count=1;desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;desc.BufferCount=4; // Keep resource capacity independent of the DXGI queued-frame limit.
 // CreateSwapChainForComposition specifically requires FLIP_SEQUENTIAL + STRETCH.
 desc.SwapEffect=DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;desc.Scaling=DXGI_SCALING_STRETCH;desc.AlphaMode=opaqueContent?DXGI_ALPHA_MODE_IGNORE:DXGI_ALPHA_MODE_PREMULTIPLIED;desc.Flags=DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;return desc;
}

static bool opaqueContentAllowed988(bool dxgi,bool fullCoverage,bool layered){return dxgi&&fullCoverage&&layered;}
static void contentAlphaLog988(bool requested,HRESULT createHr,HRESULT descHr,const DXGI_SWAP_CHAIN_DESC1&desc){
 FILE*f=smx_log_fopen989("app-opaque-content988.json","wb");if(!f)throw std::runtime_error("Cannot write own content-alpha configuration record");
 smx_log_fprintf989(f,"{\"requested\":%s,\"requested_alpha_mode\":%u,\"create_hr\":%ld,\"get_desc1_hr\":%ld,\"actual_valid\":%s,\"actual_alpha_mode\":%u,\"requested_mode_confirmed\":%s,\"scope\":\"Own full-coverage Layered DComp content only. IGNORE ignores per-pixel alpha; shader bytes and window styles unchanged. Historical R969 full-frame snapshots contain nonopaque border pixels; current R986 full-frame alpha is not certified. Border transparency/premultiplied appearance may differ. Not MPO/independent-flip or throughput proof. No fallback.\"}\n",requested?"true":"false",unsigned(requested?DXGI_ALPHA_MODE_IGNORE:DXGI_ALPHA_MODE_PREMULTIPLIED),long(createHr),long(descHr),SUCCEEDED(createHr)&&SUCCEEDED(descHr)?"true":"false",unsigned(desc.AlphaMode),SUCCEEDED(createHr)&&SUCCEEDED(descHr)&&desc.AlphaMode==(requested?DXGI_ALPHA_MODE_IGNORE:DXGI_ALPHA_MODE_PREMULTIPLIED)?"true":"false");smx_log_fclose989(f);
}
static bool visualRequested976(const char*proof,const char*marker){return exactOne976(proof)||exactOne976(marker);}
static void recordWindow976(HWND h,const char*role,const char*event,DWORD requested=0xffffffffu,int setResult=-1,DWORD setError=0){
 if(!h)return;
 std::lock_guard<std::mutex> logLock976(windowLogMutex976);
 DWORD actual=0xffffffffu;SetLastError(0);BOOL affinityOk=GetWindowDisplayAffinity(h,&actual);DWORD affinityError=affinityOk?0:GetLastError();if(!affinityOk)actual=0xffffffffu;
 const DWORD style=DWORD(GetWindowLongPtrW(h,GWL_STYLE)),exstyle=DWORD(GetWindowLongPtrW(h,GWL_EXSTYLE));
 BOOL composing=FALSE;HRESULT compositionResult=DwmIsCompositionEnabled(&composing);
 COLORREF key=0;BYTE alpha=0;DWORD layeredFlags=0;SetLastError(0);BOOL alphaOk=(exstyle&WS_EX_LAYERED)?GetLayeredWindowAttributes(h,&key,&alpha,&layeredFlags):FALSE;DWORD alphaError=alphaOk?0:((exstyle&WS_EX_LAYERED)?GetLastError():ERROR_NOT_SUPPORTED);
 DWORD pid=0;GetWindowThreadProcessId(h,&pid);LARGE_INTEGER q{},freq{};QueryPerformanceCounter(&q);QueryPerformanceFrequency(&freq);
 std::string path=getenv("SMX_BENCH_DIR")?std::string(getenv("SMX_BENCH_DIR"))+"\\app-window976.jsonl":"app-window976.jsonl";
 FILE*trace=nullptr;smx_log_fopen_s989(&trace,path.c_str(),"ab");if(!trace){smx_log_fprintf989(stderr,"WINDOW976 cannot open diagnostic log error=%d\n",errno);return;}
 smx_log_fprintf989(trace,"{\"event\":\"%s\",\"role\":\"%s\",\"hwnd\":%llu,\"pid\":%lu,\"qpc_seconds\":%.9f,\"requested_window_mode\":\"%s\",\"visual_proof\":%s,\"style\":%lu,\"ex_style\":%lu,\"layered\":%s,\"transparent_style\":%s,\"noactivate_style\":%s,\"owner_hwnd\":%llu,\"parent_hwnd\":%llu,\"requested_affinity\":%lu,\"set_affinity_result\":%d,\"set_affinity_error\":%lu,\"get_affinity_ok\":%s,\"affinity_if_available\":%lu,\"get_affinity_error\":%lu,\"layered_alpha_ok\":%s,\"layered_alpha\":%u,\"layered_flags\":%lu,\"layered_alpha_error\":%lu,\"dwm_composing\":%s,\"dwm_query_hr\":%ld,\"scope\":\"Own-window diagnostic. GetWindowDisplayAffinity is only documented to succeed for layered windows under DWM; unavailable is not NONE. Styles do not certify input forwarding, MPO or scanout.\"}\n",event,role,(unsigned long long)(uintptr_t)h,pid,double(q.QuadPart)/freq.QuadPart,opaqueWindow976?"opaque":"layered",visualProof964?"true":"false",style,exstyle,(exstyle&WS_EX_LAYERED)?"true":"false",(exstyle&WS_EX_TRANSPARENT)?"true":"false",(exstyle&WS_EX_NOACTIVATE)?"true":"false",(unsigned long long)(uintptr_t)GetWindow(h,GW_OWNER),(unsigned long long)(uintptr_t)GetParent(h),requested,setResult,setError,affinityOk?"true":"false",actual,affinityError,alphaOk?"true":"false",unsigned(alpha),layeredFlags,alphaError,SUCCEEDED(compositionResult)&&composing?"true":"false",long(compositionResult));
 smx_log_fclose989(trace);
}
static bool setAffinity976(HWND h,const char*role,DWORD requested){
 SetLastError(0);BOOL ok=SetWindowDisplayAffinity(h,requested);DWORD error=ok?0:GetLastError();
 recordWindow976(h,role,"affinity_configured",requested,ok?1:0,error);
 return ok!=FALSE;
}
static int windowConfigSelftest976(){
 bool o=false;unsigned cases=0;
 if(!parseWindowMode976(nullptr,o)||o)return 1;++cases;
 if(!parseWindowMode976("opaque",o)||!o)return 2;++cases;
 if(parseWindowMode976("typo",o))return 3;++cases;
 opaqueWindow976=false;DWORD layered=outputExStyle976(true,false);
 opaqueWindow976=true;DWORD opaque=outputExStyle976(true,false);
 if((layered^opaque)!=WS_EX_LAYERED||(opaque&WS_EX_LAYERED)||!(opaque&WS_EX_NOACTIVATE)||!(opaque&WS_EX_TRANSPARENT))return 4;++cases;
 if(!(outputExStyle976(false,false)&WS_EX_LAYERED))return 5;++cases;
 if(visualRequested976(nullptr,"0")||visualRequested976("0",nullptr)||!visualRequested976(nullptr,"1")||!visualRequested976("1",nullptr))return 6;++cases;
 for(unsigned depth:{1u,2u,3u}){auto desc=compositionDesc976(1600,900,depth);if(desc.SwapEffect!=DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL||desc.Scaling!=DXGI_SCALING_STRETCH||desc.BufferCount!=4||desc.AlphaMode!=DXGI_ALPHA_MODE_PREMULTIPLIED||desc.Flags!=DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)return 8;++cases;}
 for(unsigned depth:{1u,2u,3u}){auto old=compositionDesc976(1600,900,depth);auto opaqueDesc=compositionDesc976(1600,900,depth,true);if(opaqueDesc.AlphaMode!=DXGI_ALPHA_MODE_IGNORE)return 9;opaqueDesc.AlphaMode=old.AlphaMode;if(memcmp(&old,&opaqueDesc,sizeof(old)))return 10;++cases;}
 for(unsigned bits=0;bits<8;++bits){if(opaqueContentAllowed988(bits&1,bits&2,bits&4)!=(bits==7))return 11;++cases;}
 opaqueWindow976=false;
 FILE*test=smx_log_fopen989("window-config976-selftest.json","wb");if(!test)return 7;smx_log_fprintf989(test,"{\"pass\":true,\"cases\":%u,\"gpu_used\":false,\"windows_created\":false,\"system_settings_changed\":false}\n",cases);smx_log_fclose989(test);return 0;
}

static ULONGLONG start964=0,lastHud964=0;
static unsigned oldCapture964=0,oldGen964=0,oldPresent964=0;
static double hzCapture964=0,hzGen964=0,hzPresent964=0;
static std::wstring hudText964=L"正在准备插帧";
static void updateHud964();
static void hideHud964(){if(hud964)ShowWindow(hud964,SW_HIDE);}
static LRESULT CALLBACK hudProc964(HWND h,UINT m,WPARAM w,LPARAM l){
 if(m==WM_NCHITTEST)return HTTRANSPARENT;
 if(m==WM_MOUSEACTIVATE)return MA_NOACTIVATE;
 if(m==WM_ERASEBKGND)return 1;
 if(m==WM_DESTROY){smx988::releaseHud();return 0;}
 if(m==WM_PAINT){PAINTSTRUCT ps{};HDC dc=BeginPaint(h,&ps);RECT r{};GetClientRect(h,&r);auto session=currentHud988();smx988::HudMetrics::Snapshot snapshot;smx988::HudExtra extra;extra.details=hudDetails988;
  if(session){double now=smx988::qpcNow988();snapshot=session->displaySnapshot993(now);auto p=session->powerSnapshot(now);extra.nativeWidth=session->width;extra.nativeHeight=session->height;extra.backendName=session->backend=="dxgi"?L"DXGI · DComp":L"Vulkan · 显示观测 N/A";extra.status=session->observerFailed?L"监测不可用":snapshot.stalled?L"等待显示事件":L"只读监测 · 系统显示事件";extra.countersValid=true;extra.captureCount=session->captured.load();extra.inferCount=snapshot.generationCompleteCount;extra.submitCount=snapshot.submittedCount;extra.lateCompleted=snapshot.lateCompleted;extra.lateSubmitted=snapshot.lateSubmitted;extra.inflight=snapshot.inflight;
   extra.cpuPackageValid=p.cpuPackageValid;extra.cpuPackageWatts=p.cpuPackageWatts;extra.powerValid=p.powerValid;extra.powerWatts=p.watts;extra.powerAgeSeconds=p.sampleQpc?now-p.sampleQpc:-1;extra.powerDomain=smx988::powerDomainName988(p.domain);extra.frequencyValid=p.frequencyValid;extra.gpuMHz=p.gpuMHz;extra.throttleFlagsValid=p.throttleValid;extra.throttleFlags=p.throttleFlags;extra.gpuTimingValid=extra.serviceTimingValid=snapshot.modelTimingValid;extra.queueTimingValid=snapshot.queueWaitValid;extra.gpuMs=snapshot.modelGpuMs;extra.serviceMs=snapshot.serviceMs;extra.queueWaitMs=snapshot.queueWaitMs;
  }else{extra.powerDomain=L"计数域未确认";extra.status=L"等待捕获会话";}
  if(paintedSession988.lock()!=session){smx988::releaseHud();paintedSession988=session;}
  smx988::drawHud(dc,r,snapshot,extra);
  if(session&&exactOne976(getenv("SMX_HUD_PROOF988"))){if(proofSession988.lock()!=session){proofSession988=session;proofDone988=false;}if(!proofDone988&&smx988::qpcNow988()-session->started>=12){proofDone988=true;std::string base=getenv("SMX_BENCH_DIR")?getenv("SMX_BENCH_DIR"):".";auto path=winrt::to_hstring(base+"\\app-hud988.bmp");bool ok=smx988::dumpHudBackingBmp(path.c_str());FILE*f=smx_log_fopen989((base+"\\app-hud-proof988.json").c_str(),"wb");if(f){smx_log_fprintf989(f,"{\"success\":%s,\"own_backing_dib_only\":true,\"desktop_captured\":false,\"capture_affinity_changed\":false,\"diagnostic_not_performance\":true}\n",ok?"true":"false");smx_log_fclose989(f);}}}
  EndPaint(h,&ps);return 0;}
 return DefWindowProcW(h,m,w,l);
}
static std::wstring baseDir;
static std::thread worker;
static std::atomic<int> engineExit989{0};
static bool fullscreen=false,selftest=false,demo=false,benchmarkSource=false,windowTest795=false;
static std::atomic<bool> closePending{false};
static std::mutex engineMutex783;static std::condition_variable engineCv783;static bool engineIdle783=false,restart783=false,quit783=false;static unsigned session783=0;static FILETIME sessionBegin852{};static std::atomic<unsigned long> generation783{0};
static RECT oldRect;
static unsigned testTick=0;static std::atomic<unsigned> fullscreenEntries{0},fullscreenExits{0},passthroughChecks{0};
static int selectedSize=0;static bool verifyOnly=false,verifyGpuOnly=false;
static std::wstring lastCaptureError;
static std::atomic<bool> performanceWarning775{false};
static bool recoveryTest809=false,queueTest809=false;
static HWND overlayCheck;
static bool sourceOverlay=false,overlayVisible=false;
static std::atomic<bool> overlayPaused{false},overlayReady{false},overlayBypass{false};
static std::atomic<bool> captureBackendPaused989{false};
static bool capturePaused989();
static std::atomic<bool> targetClosed990{false};
static smx992::ProcessingState processingState992;
static std::string processingFields992(){return processingState992.fields();}
static std::atomic<DWORD> targetPid990{0};
static std::array<HWINEVENTHOOK,3> windowHooks990{};
static std::string windowStatePath990,lastWindowState990;
static std::atomic<unsigned> bypassCaptured{0},activeCaptured{0};
static std::atomic<unsigned long long> overlayEpoch{0};
static unsigned long long seenOverlayEpoch=0;
static std::atomic<unsigned> overlayEntries{0},overlayHides{0},overlayHitPasses{0},overlayHitFailures{0};
static RECT overlayRect{};
static FILE*overlayEvents=nullptr;
static void status(std::wstring);
#include "fp16-next537/candidate994/output_functions993.inc"
#include "fp16-next537/candidate994/target_process_evidence993.hpp"
static TargetProcessEvidence993 targetProcessEvidence993;
static void overlayEvent(const char*name,const RECT&r){
 if(!overlayEvents)return;
 LARGE_INTEGER q{},f{};QueryPerformanceCounter(&q);QueryPerformanceFrequency(&f);
 smx_log_fprintf989(overlayEvents,"%.9f,%s,%lld,%d,%ld,%ld,%ld,%ld\n",double(q.QuadPart)/f.QuadPart,name,
 (long long)(INT_PTR)GetForegroundWindow(),IsIconic(selected)?1:0,r.left,r.top,r.right,r.bottom);
 fflush(overlayEvents);
}
static bool validExtent795(long w,long h){return w>0&&h>0&&w<=16384&&h<=16384;}
static uint64_t packExtent795(unsigned w,unsigned h){return uint64_t(w)<<32|h;}
// R990 only observes the target. It never changes target styles, placement, display mode or focus.
static bool targetIdentity990(){DWORD pid=0;if(!selected||!IsWindow(selected))return false;GetWindowThreadProcessId(selected,&pid);return pid&&pid==targetPid990.load();}
static bool targetForeground990(){HWND fg=GetForegroundWindow();return fg&&GetAncestor(fg,GA_ROOT)==selected;}
static void targetClosedEvent990(){smxdiag994::instant(SMX_DIAG_TARGET_CLOSED994,smxdiag994::fields(0,0,(uintptr_t)selected));if(!targetClosed990.exchange(true))targetProcessEvidence993.record("window_or_identity_closed");stopFlag=true;if(control)PostMessageW(control,WM_APP+10,0,0);}
static bool capturePaused989(){
 if(sourceOverlay&&running&&overlayReady){
  bool exists=targetIdentity990();
  bool keep=benchmarkSource&&getenv("SMX_BENCH_KEEP_CAPTURE")&&strcmp(getenv("SMX_BENCH_KEEP_CAPTURE"),"1")==0;
  if(smx990::blocked(exists,exists,IsIconic(selected)!=FALSE,IsWindowVisible(selected)!=FALSE,keep||targetForeground990())){
   if(!overlayPaused.exchange(true)){++overlayEpoch;if(control)PostMessageW(control,WM_APP+10,0,0);}
   if(!exists)targetClosedEvent990();
   return true;
  }
 }
 return overlayPaused.load()||captureBackendPaused989.load();
}
static smx990::WindowInfo readWindowInfo990(){
 smx990::WindowInfo unknown;if(!targetIdentity990())return unknown;
 RECT client{};POINT origin{};MONITORINFO monitor{sizeof(monitor)};
 if(!GetClientRect(selected,&client)||!ClientToScreen(selected,&origin)||!GetMonitorInfoW(MonitorFromWindow(selected,MONITOR_DEFAULTTONEAREST),&monitor))return unknown;
 LONG_PTR style=GetWindowLongPtrW(selected,GWL_STYLE);
 return smx990::geometry({origin.x,origin.y,int64_t(origin.x)+client.right,int64_t(origin.y)+client.bottom},{monitor.rcMonitor.left,monitor.rcMonitor.top,monitor.rcMonitor.right,monitor.rcMonitor.bottom},(style&(WS_CAPTION|WS_THICKFRAME))==0,targetForeground990());
}
static const char* pauseReason990(){
 if(!selected)return "preparing";
 return smx990::reason(targetClosed990.load()||!targetIdentity990(),IsIconic(selected)!=FALSE,IsWindowVisible(selected)!=FALSE,targetForeground990(),captureBackendPaused989.load(),overlayReady.load());
}
static std::string windowFields990(){auto v=readWindowInfo990();
 return std::string("\"targetInfoValid990\":")+(v.valid?"true":"false")+",\"targetFullscreen990\":"+(v.fullscreen?"true":"false")+",\"coverage990\":"+std::to_string(v.coverage)+",\"pauseReason990\":"+host988::quote(pauseReason990())+",\"targetForeground990\":"+(v.foreground?"true":"false")+",\"targetClientWidth990\":"+std::to_string(v.clientWidth)+",\"targetClientHeight990\":"+std::to_string(v.clientHeight)+",\"targetMonitorWidth990\":"+std::to_string(v.monitorWidth)+",\"targetMonitorHeight990\":"+std::to_string(v.monitorHeight);
}
static void recordWindowState990(){
 if(windowStatePath990.empty())return;auto v=readWindowInfo990();
 std::string state=windowFields990()+",\"running\":"+(running?"true":"false")+",\"stopRequested\":"+(stopFlag?"true":"false")+",\"overlayPaused\":"+(overlayPaused?"true":"false")+",\"overlayEpoch\":"+std::to_string(overlayEpoch.load())+",\"clientRect\":["+std::to_string(v.client.left)+","+std::to_string(v.client.top)+","+std::to_string(v.client.right)+","+std::to_string(v.client.bottom)+"],\"monitorRect\":["+std::to_string(v.monitor.left)+","+std::to_string(v.monitor.top)+","+std::to_string(v.monitor.right)+","+std::to_string(v.monitor.bottom)+"],\"foregroundHwnd\":"+std::to_string(uint64_t(uintptr_t(GetForegroundWindow())));
 if(state==lastWindowState990)return;lastWindowState990=state;
 if(FILE*f=smx_log_fopen989(windowStatePath990.c_str(),"ab")){smx_log_fprintf989(f,"{\"qpc_seconds\":%.9f,%s,\"scope\":\"read-only client/monitor geometry; not exclusive fullscreen or scanout proof\"}\n",smx988::qpcNow988(),state.c_str());smx_log_fclose989(f);}
}
static void stopWindowHooks990(){for(auto&h:windowHooks990){if(h)UnhookWinEvent(h);h=nullptr;}}
static void CALLBACK windowEvent990(HWINEVENTHOOK,DWORD event,HWND hwnd,LONG object,LONG,DWORD,DWORD){
 if(!running||!sourceOverlay)return;
 if(event==EVENT_OBJECT_DESTROY){if(hwnd!=selected||object!=OBJID_WINDOW)return;targetClosedEvent990();}
 else if(event!=EVENT_SYSTEM_FOREGROUND&&hwnd!=selected)return;
 // WinEvent runs out of context on the UI thread. Latch a transition even if focus returns before the timer.
 if(event==EVENT_SYSTEM_FOREGROUND&&GetAncestor(hwnd,GA_ROOT)!=selected){if(!overlayPaused.exchange(true))++overlayEpoch;}
 if(event==EVENT_SYSTEM_MINIMIZESTART){if(!overlayPaused.exchange(true))++overlayEpoch;}
 PostMessageW(control,WM_APP+10,0,0);
}
static void startWindowHooks990(){stopWindowHooks990();
 windowHooks990[0]=SetWinEventHook(EVENT_SYSTEM_FOREGROUND,EVENT_SYSTEM_FOREGROUND,nullptr,windowEvent990,0,0,WINEVENT_OUTOFCONTEXT);
 windowHooks990[1]=SetWinEventHook(EVENT_SYSTEM_MINIMIZESTART,EVENT_SYSTEM_MINIMIZEEND,nullptr,windowEvent990,0,0,WINEVENT_OUTOFCONTEXT);
 windowHooks990[2]=SetWinEventHook(EVENT_OBJECT_DESTROY,EVENT_OBJECT_DESTROY,nullptr,windowEvent990,targetPid990.load(),0,WINEVENT_OUTOFCONTEXT);
 smx_log_fprintf989(stderr,"WINDOW990 hooks foreground=%u minimize=%u destroy=%u; submission foreground recheck and 100ms UI timer retained; target styles unchanged\n",windowHooks990[0]!=nullptr,windowHooks990[1]!=nullptr,windowHooks990[2]!=nullptr);
}
static void updateSourceOverlay(){
 recordWindowState990();
 if(!sourceOverlay||!running||!output){outputState993.hide();hideHud964();return;}
 if(!targetIdentity990()){targetClosed990=true;stopFlag=true;if(!overlayPaused.exchange(true))++overlayEpoch;ShowWindow(output,SW_HIDE);overlayVisible=false;outputState993.hide();hideHud964();recordWindowState990();return;}
 HWND fg=GetForegroundWindow();
 bool active=overlayReady.load()&&!stopFlag&&!IsIconic(selected)&&IsWindowVisible(selected)&&
  ((benchmarkSource&&getenv("SMX_BENCH_KEEP_CAPTURE")&&strcmp(getenv("SMX_BENCH_KEEP_CAPTURE"),"1")==0)||(fg&&GetAncestor(fg,GA_ROOT)==selected));
 if(!active){hideHud964();outputState993.hide();lastHud964=0;hzCapture964=hzGen964=hzPresent964=0;
  if(!overlayPaused.exchange(true)){overlayEpoch++;overlayEvent("pause_foreground_or_visibility",overlayRect);}
  if(overlayVisible){ShowWindow(output,SW_HIDE);overlayVisible=false;overlayHides++;overlayEvent("hide",overlayRect);}return;
 }
 RECT source{};POINT origin{};
 if(!GetClientRect(selected,&source)||!ClientToScreen(selected,&origin)||!validExtent795(source.right,source.bottom)){
  if(!overlayPaused.exchange(true))overlayEpoch++;ShowWindow(output,SW_HIDE);overlayVisible=false;outputState993.hide();return;
 }
 if(captureBackendPaused989){hideHud964();outputState993.hide();if(overlayVisible){ShowWindow(output,SW_HIDE);overlayVisible=false;}return;}
 if(overlayBypass){hideHud964();outputState993.hide();overlayPaused=false;if(overlayVisible){ShowWindow(output,SW_HIDE);overlayVisible=false;overlayHides++;overlayEvent("passthrough",overlayRect);}return;}
 OffsetRect(&source,origin.x,origin.y);
 try{
 auto plan=plannedOutput993(source);RECT desiredWindow=outputWindowsRect993(plan.window),actualWindow{};
 bool show=presented.load()!=0;GetWindowRect(output,&actualWindow);bool moved=!EqualRect(&desiredWindow,&actualWindow);
 if(moved||(!overlayVisible&&show)){
  if(!SetWindowPos(output,HWND_TOPMOST,desiredWindow.left,desiredWindow.top,desiredWindow.right-desiredWindow.left,desiredWindow.bottom-desiredWindow.top,
    SWP_NOACTIVATE|SWP_FRAMECHANGED|(show?SWP_SHOWWINDOW:0)))throw std::runtime_error("Cannot position own output window");
 }
 // Read back the actual physical client rectangle, including DPI-aware nonclient frame.
 RECT actualClient{};POINT actualOrigin{};if(!GetClientRect(output,&actualClient)||!ClientToScreen(output,&actualOrigin)||actualClient.right<=0||actualClient.bottom<=0)throw std::runtime_error("Cannot read output client rectangle");
 OffsetRect(&actualClient,actualOrigin.x,actualOrigin.y);GetWindowRect(output,&actualWindow);
 plan.window=outputRectValue993(actualWindow);plan.client=outputRectValue993(actualClient);
 plan.content=outputPreview993()?smx993::fitContent(plan.client,unsigned(source.right-source.left),unsigned(source.bottom-source.top)):plan.client;
 outputState993.set(plan,show);overlayRect=actualClient;
 if(!show){overlayPaused=false;hideHud964();return;}
 bool entering=!overlayVisible;overlayVisible=true;if(entering){overlayEntries++;overlayEvent("show",actualClient);}else if(moved)overlayEvent("move",actualClient);
 if((entering||moved)&&!outputPreview993()){
  unsigned pass=0;for(int y=1;y<=3;y++)for(int x=1;x<=3;x++){POINT pt{actualClient.left+(actualClient.right-actualClient.left)*x/4,actualClient.top+(actualClient.bottom-actualClient.top)*y/4};if(GetAncestor(WindowFromPoint(pt),GA_ROOT)==selected)pass++;}
  overlayHitPasses+=pass;overlayHitFailures+=9-pass;overlayEvent(pass==9?"hit_test_pass":"hit_test_failure",actualClient);
 }
 overlayPaused=false;
 if(!hostConfig988.hosted&&!benchmarkSource&&!controlMinimized964){ShowWindow(control,SW_SHOWMINNOACTIVE);controlMinimized964=true;}
 updateHud964();
 }catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));lastCaptureError=L"Output geometry failed: "+std::wstring(winrt::to_hstring(e.what()).c_str());outputGeometryFailed993=true;stopFlag=true;ShowWindow(output,SW_HIDE);overlayVisible=false;outputState993.hide();hideHud964();}
}
static void updateHud964(){
 if(!hudEnabled964||!overlayVisible||!running||stopFlag){hideHud964();return;}
 if(!hud964){hud964=CreateWindowExW(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,L"SMXHud964",L"Smooth Motion 真实显示监测",WS_POPUP,0,0,438,364,output,nullptr,GetModuleHandleW(nullptr),nullptr);if(!hud964)return;SetLayeredWindowAttributes(hud964,0,210,LWA_ALPHA);if(!setAffinity976(hud964,"hud",visualProof964?WDA_NONE:WDA_EXCLUDEFROMCAPTURE)){recordWindow976(hud964,"hud","affinity_failure_hidden");DestroyWindow(hud964);hud964=nullptr;return;}}
 RECT target=smx988::preferredHudRect(overlayRect,hudDetails988),actual{};int hw=int(target.right-target.left),hh=int(target.bottom-target.top);GetWindowRect(hud964,&actual);
 if(int(actual.right-actual.left)!=hw||int(actual.bottom-actual.top)!=hh){int radius=std::max(2,int(20.*hw/438.));HRGN region=CreateRoundRectRgn(0,0,hw+1,hh+1,radius,radius);if(region&&!SetWindowRgn(hud964,region,TRUE))DeleteObject(region);}
 if(!IsWindowVisible(hud964)||!EqualRect(&target,&actual))SetWindowPos(hud964,HWND_TOPMOST,target.left,target.top,hw,hh,SWP_NOACTIVATE|SWP_SHOWWINDOW);
 ULONGLONG now=GetTickCount64();if(!lastHud964||now-lastHud964>=250){lastHud964=now;InvalidateRect(hud964,nullptr,FALSE);}
}
static void status(std::wstring s){auto p=new std::wstring(std::move(s));PostMessageW(control,WM_APP+1,0,(LPARAM)p);}
static void preparationMarker862(const char* state, bool captureStarted=false){
 if(hostConfig988.hosted&&captureStarted)hostCaptureStarted988=true;
 const char* dir=getenv("SMX_BENCH_DIR");if(!dir)return;
 std::string name=std::string(dir)+"\\app-preparation862.json";
 LARGE_INTEGER q{},freq{};QueryPerformanceCounter(&q);QueryPerformanceFrequency(&freq);
 DWORD sourcePid=0;GetWindowThreadProcessId(selected,&sourcePid);RECT r{};GetClientRect(selected,&r);
 FILE* file=smx_log_fopen989(name.c_str(),"wb");if(!file)throw std::runtime_error("Cannot write preparation state in run folder");
 smx_log_fprintf989(file,"{\"state\":\"%s\",\"capture_started\":%s,\"pid\":%lu,\"source_pid\":%lu,\"source_hwnd\":%llu,\"foreground_hwnd\":%llu,\"width\":%ld,\"height\":%ld,\"qpc\":%lld,\"qpc_frequency\":%lld}\n",state,captureStarted?"true":"false",GetCurrentProcessId(),sourcePid,(unsigned long long)(UINT_PTR)selected,(unsigned long long)(UINT_PTR)GetForegroundWindow(),r.right,r.bottom,q.QuadPart,freq.QuadPart);smx_log_fclose989(file);
}

static void writeFile(const char* path,const void* data,size_t n){FILE*f=nullptr;smx_log_fopen_s989(&f,path,"wb");if(!f)throw std::runtime_error("Cannot write verification output");bool ok=fwrite(data,1,n,f)==n;smx_log_fclose989(f);if(!ok)throw std::runtime_error("Incomplete verification output");}
static void toggleFullscreen(){
 if(!output)return;
 if(sourceOverlay){status(outputPreview993()?L"输出预览尺寸由开始前选项决定。点击预览或 Ctrl+Alt+Esc 停止。":L"Overlay follows the game client. Ctrl+Alt+Esc stops capture.");return;}
 fullscreen=!fullscreen;
 if(fullscreen){GetWindowRect(output,&oldRect);MONITORINFO mi{sizeof(mi)};GetMonitorInfoW(MonitorFromWindow(output,MONITOR_DEFAULTTONEAREST),&mi);SetWindowLongPtrW(output,GWL_STYLE,WS_POPUP|WS_VISIBLE);SetWindowLongPtrW(output,GWL_EXSTYLE,WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW);SetLayeredWindowAttributes(output,0,255,LWA_ALPHA);SetWindowPos(output,HWND_TOPMOST,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,SWP_FRAMECHANGED|SWP_NOACTIVATE);SetForegroundWindow(selected);fullscreenEntries++;
  RECT sr{};GetWindowRect(selected,&sr);POINT center{(sr.left+sr.right)/2,(sr.top+sr.bottom)/2};HWND hit=GetAncestor(WindowFromPoint(center),GA_ROOT);if(hit==selected)passthroughChecks++;
  smx_log_fprintf989(stderr,"Fullscreen entered; target hit-test=%s; output rect=%ld,%ld,%ld,%ld\n",hit==selected?"source":"other",mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right,mi.rcMonitor.bottom);
 }
 else{SetWindowLongPtrW(output,GWL_EXSTYLE,WS_EX_LAYERED);SetLayeredWindowAttributes(output,0,255,LWA_ALPHA);SetWindowLongPtrW(output,GWL_STYLE,WS_OVERLAPPEDWINDOW|WS_VISIBLE);SetWindowPos(output,HWND_NOTOPMOST,oldRect.left,oldRect.top,oldRect.right-oldRect.left,oldRect.bottom-oldRect.top,SWP_FRAMECHANGED);fullscreenExits++;smx_log_fprintf989(stderr,"Fullscreen exited\n");}
}
static LRESULT CALLBACK outputProc(HWND h,UINT m,WPARAM w,LPARAM l){
 // The swapchain exclusively draws this HWND. GDI must not erase/paint it.
 if(m==WM_ERASEBKGND)return 1;
 if(m==WM_PAINT){PAINTSTRUCT ps{};BeginPaint(h,&ps);EndPaint(h,&ps);return 0;}
 if(outputPreview993()&&sourceOverlay&&m==WM_NCHITTEST)return HTCLIENT;
 if(outputPreview993()&&sourceOverlay&&m==WM_MOUSEACTIVATE){stopPreviewFromClick993(h);return MA_NOACTIVATEANDEAT;}
 if(outputPreview993()&&sourceOverlay&&(m==WM_LBUTTONDOWN||m==WM_RBUTTONDOWN||m==WM_MBUTTONDOWN||m==WM_XBUTTONDOWN||m==WM_NCLBUTTONDOWN)){stopPreviewFromClick993(h);return 0;}
 if((sourceOverlay||fullscreen)&&m==WM_NCHITTEST)return HTTRANSPARENT;
 if((sourceOverlay||fullscreen)&&m==WM_MOUSEACTIVATE)return MA_NOACTIVATE;
 if(m==WM_KEYDOWN&&w==VK_F11){toggleFullscreen();return 0;}
 if(m==WM_KEYDOWN&&w==VK_ESCAPE){if(fullscreen)toggleFullscreen();else stopFlag=true;return 0;}
 if(m==WM_CLOSE){stopFlag=true;ShowWindow(h,SW_HIDE);return 0;}
 return DefWindowProcW(h,m,w,l);
}
static LRESULT CALLBACK testProc(HWND h,UINT m,WPARAM w,LPARAM l){
 if(m==WM_TIMER){testTick++;InvalidateRect(h,nullptr,FALSE);return 0;}
 if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);int cw=std::max(1L,r.right),ch=std::max(1L,r.bottom);HBRUSH background=CreateSolidBrush(RGB(24,40,64));FillRect(dc,&r,background);DeleteObject(background);for(int i=0;i<12;i++){int x=int((uint64_t(testTick)*5+uint64_t(i)*cw/12)%cw);RECT band{0,i*ch/12,cw,(i+1)*ch/12};HBRUSH dim=CreateSolidBrush(RGB(16+i*7,24+i*9,32+i*11));FillRect(dc,&band,dim);DeleteObject(dim);RECT q{x,band.top,std::min(cw,x+std::max(4,cw/12)),band.bottom};HBRUSH b=CreateSolidBrush(RGB((i*71+80)%256,(i*97+64)%256,(i*31+160)%256));FillRect(dc,&q,b);DeleteObject(b);}EndPaint(h,&ps);return 0;}
 return DefWindowProcW(h,m,w,l);
}

struct CaptureFrame949 {
 struct Owner {
  Direct3D11CaptureFrame frame{nullptr};winrt::com_ptr<ID3D11Texture2D> desktopTexture;
  winrt::Windows::Graphics::SizeInt32 size{};winrt::Windows::Foundation::TimeSpan stamp{};
  std::mutex releaseMutex;std::function<void()> releaseDesktop;bool desktopGpuRead=false;
  explicit Owner(Direct3D11CaptureFrame f):frame(std::move(f)){size=frame.ContentSize();stamp=frame.SystemRelativeTime();}
  Owner(winrt::com_ptr<ID3D11Texture2D> t,winrt::Windows::Graphics::SizeInt32 s,winrt::Windows::Foundation::TimeSpan q,std::function<void()> r):desktopTexture(std::move(t)),size(s),stamp(q),releaseDesktop(std::move(r)){}
  void releaseCopyOwner(bool fenceComplete=false){std::lock_guard lock(releaseMutex);if(releaseDesktop){if(desktopGpuRead&&!fenceComplete)smxBridgeFailure555("DXGI capture lease lost before copy fence; preserve GPU ownership",ERROR_INVALID_STATE);auto release=std::move(releaseDesktop);release();desktopTexture=nullptr;desktopGpuRead=false;}}
  ~Owner(){releaseCopyOwner();if(frame){try{SMX_DIAG_COMMAND994(SMX_DIAG_CAPTURE_RELEASE994,smxdiag994::fields(0,0,(uintptr_t)this,0,stamp.count()*1e-7),frame.Close());}catch(const hresult_error&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,int32_t(e.code()),SMX_DIAG_HRESULT994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));smxdiag994::error(SMX_DIAG_CAPTURE_RELEASE994,int32_t(e.code()),SMX_DIAG_HRESULT994);smxBridgeFailure555("R949 WGC close",E_FAIL);}catch(...){smxBridgeFailure555("R949 WGC close",E_FAIL);}}}
 };
 std::shared_ptr<Owner> owner;
 CaptureFrame949()=default;CaptureFrame949(std::nullptr_t){}
 CaptureFrame949(Direct3D11CaptureFrame f){if(f)owner=std::make_shared<Owner>(std::move(f));}
 CaptureFrame949(winrt::com_ptr<ID3D11Texture2D> t,winrt::Windows::Graphics::SizeInt32 s,winrt::Windows::Foundation::TimeSpan q,std::function<void()> r){owner=std::make_shared<Owner>(std::move(t),s,q,std::move(r));}
 explicit operator bool()const{return bool(owner);}
 void Close(){owner.reset();}
 auto ContentSize()const{return owner->size;}
 auto SystemRelativeTime()const{return owner->stamp;}
 void BeginCopy989(){if(owner){std::lock_guard lock(owner->releaseMutex);if(owner->releaseDesktop)owner->desktopGpuRead=true;}}
 void ReleaseAfterCopy989(){if(owner)owner->releaseCopyOwner(true);} // DDA only; WGC lifetime stays unchanged.
 winrt::com_ptr<ID3D11Texture2D> Texture989()const{if(owner->desktopTexture)return owner->desktopTexture;auto access=owner->frame.Surface().as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();winrt::com_ptr<ID3D11Texture2D> t;smxdiag994::checkedHr((access->GetInterface(__uuidof(ID3D11Texture2D),t.put_void())),__LINE__);return t;}
};
#include "fp16-next537/candidate994/desktop_duplication989.hpp"

static int captureContractSelftest989(){try{unsigned checks=0;auto check=[&](bool value){++checks;if(!value)throw std::runtime_error("capture contract CPU check");};
 check(smx989::sessionSeconds(nullptr)==0);check(smx989::sessionSeconds("0")==0);check(smx989::sessionSeconds("10")==10);check(smx989::sessionSeconds("14400")==14400);
 for(const char*value:{"-1","9","14401","30junk"}){bool rejected=false;try{smx989::sessionSeconds(value);}catch(...){rejected=true;}check(rejected);}
 check(!smx989::desktopCapture(nullptr));check(!smx989::desktopCapture("wgc"));check(smx989::desktopCapture("dxgi"));bool rejected=false;try{smx989::desktopCapture("unknown");}catch(...){rejected=true;}check(rejected);
 RECT monitor{-1920,0,0,1080};check(smx989::clientInsideMonitor(monitor,POINT{-1920,0},1920,1080));check(!smx989::clientInsideMonitor(monitor,POINT{-1921,0},1920,1080));check(!smx989::clientInsideMonitor(monitor,POINT{-1920,0},1921,1080));
 check(smxPixelProofEligible989(320,180));check(smxPixelProofEligible989(1024,1024));check(!smxPixelProofEligible989(1920,1080));check(!smxPixelProofEligible989(0,1080));check(!smxPixelProofEligible989(UINT32_MAX,UINT32_MAX));
 unsigned releases=0;{CaptureFrame949 frame(winrt::com_ptr<ID3D11Texture2D>{},{320,180},winrt::Windows::Foundation::TimeSpan{12345},[&]{++releases;});CaptureFrame949 copy=frame;frame.BeginCopy989();frame.ReleaseAfterCopy989();check(releases==1);frame.Close();check(copy.ContentSize().Width==320&&copy.SystemRelativeTime().count()==12345);copy.ReleaseAfterCopy989();check(releases==1);}check(releases==1);
 {CaptureFrame949 unused(winrt::com_ptr<ID3D11Texture2D>{},{1,1},winrt::Windows::Foundation::TimeSpan{1},[&]{++releases;});unused.Close();}check(releases==2);
 FILE*f=smx_log_fopen989("capture-contract-selftest989.json","wb");if(!f)return 2;smx_log_fprintf989(f,"{\"pass\":true,\"checks\":%u,\"gpu_used\":false,\"actual_duplication_api_tested\":false,\"cached_metadata_after_copy_release\":true}\n",checks);smx_log_fclose989(f);return 0;
 }catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));smx_log_fprintf989(stderr,"%s\n",e.what());return 1;}}


struct GpuFrames {ComPtr<ID3D11Texture2D> first,middle;ComPtr<ID3D11Query> captureReady809,copyStart937,copyEnd937,copyRelease938,copyDomain937;UINT64 readyValue912=0;BridgeHandle readyEvent912;bool readyObserved809=false;double captureStamp809=0,readyStamp809=0;};
struct Pair {std::shared_ptr<GpuFrames> textures;double interval,sourceTime;};
// Diagnostic observer only: does not gate producer or inference, and never resets events.
// Retained lease prevents slot/event reuse until its corresponding observation is complete.
class CaptureReadyMonitor933 {
 struct Item {CaptureFrame949 frame;std::shared_ptr<GpuFrames> lease;unsigned sequence;UINT64 value;double source,publish;double callbackEntry=0,inboxAcquired=0,frameGetBegin=0,frameGetEnd=0,copyBegin=0;};
 std::mutex mutex;std::condition_variable cv;std::deque<Item> items;std::thread thread;
 bool stopping=false;std::atomic<DWORD> failure{0};FILE*file=nullptr;ID3D11Fence*fence;ID3D11DeviceContext*context;HANDLE wake;
 static double clock(){LARGE_INTEGER q,f;QueryPerformanceCounter(&q);QueryPerformanceFrequency(&f);return double(q.QuadPart)/f.QuadPart;}
public:
 CaptureReadyMonitor933(ID3D11Fence*f,ID3D11DeviceContext*c,HANDLE w):fence(f),context(c),wake(w){
  file=smx_log_fopen989("app-producer933.csv","wb");if(!file)throw std::runtime_error("Producer observation file unavailable");
  smx_log_fprintf989(file,"sequence,ready_fence,source_qpc,publish_qpc,wait_begin_qpc,ready_observed_qpc,wait_result,completed_value,query_result,disjoint,copy_gpu_ms,query_frequency,post_copy_release_gpu_ms,copy_begin_to_release_gpu_ms,observer_resources_released_qpc,log_begin_qpc,callback_entry_qpc,inbox_lock_acquired_qpc,frame_get_begin_qpc,frame_get_end_qpc,copy_begin_qpc\n");
  thread=std::thread([this]{
   for(;;){Item it;
    {std::unique_lock lock(mutex);cv.wait(lock,[&]{return stopping||!items.empty();});if(stopping&&items.empty())return;it=std::move(items.front());items.pop_front();}
    SmxDiagFields994 producerFields994=smxdiag994::fields(it.sequence,0,(uintptr_t)fence,it.value,it.source);
    smxdiag994::Scope producerWait994(SMX_DIAG_CAPTURE_RELEASE994,producerFields994);double begin=clock();DWORD result;
    for(;;){
     result=WaitForSingleObject(it.lease->readyEvent912.value,50);
     if(result!=WAIT_TIMEOUT)break;
     // Drain submitted copies before releasing their WGC frame owners, including Stop.
     if(clock()-begin>=5.0)break;
    }
    double observed=clock();auto completed=SMX_DIAG_CALL994(SMX_DIAG_CAPTURE_RELEASE994,producerFields994,SMX_DIAG_SEMANTIC994,fence->GetCompletedValue());producerFields994.value0=int64_t(completed);producerWait994.update(producerFields994);producerWait994.end(SMX_DIAG_WIN32994,result);
     if(result!=WAIT_OBJECT_0||completed==UINT64_MAX||completed<it.value)smxBridgeFailure555("R949 producer fence timeout/device failure",long(result));
    UINT64 ts937=0,te937=0,tr938=0;D3D11_QUERY_DATA_TIMESTAMP_DISJOINT dq937{};
    HRESULT qa937=SMX_DIAG_CALL994(SMX_DIAG_MODEL_QUERY994,producerFields994,SMX_DIAG_HRESULT994,context->GetData(it.lease->copyStart937.Get(),&ts937,sizeof(ts937),D3D11_ASYNC_GETDATA_DONOTFLUSH));
    HRESULT qb937=SMX_DIAG_CALL994(SMX_DIAG_MODEL_QUERY994,producerFields994,SMX_DIAG_HRESULT994,context->GetData(it.lease->copyEnd937.Get(),&te937,sizeof(te937),D3D11_ASYNC_GETDATA_DONOTFLUSH));
    HRESULT qc938=SMX_DIAG_CALL994(SMX_DIAG_MODEL_QUERY994,producerFields994,SMX_DIAG_HRESULT994,context->GetData(it.lease->copyRelease938.Get(),&tr938,sizeof(tr938),D3D11_ASYNC_GETDATA_DONOTFLUSH));
    HRESULT qd937=SMX_DIAG_CALL994(SMX_DIAG_MODEL_QUERY994,producerFields994,SMX_DIAG_HRESULT994,context->GetData(it.lease->copyDomain937.Get(),&dq937,sizeof(dq937),D3D11_ASYNC_GETDATA_DONOTFLUSH));
    HRESULT qr937=qa937!=S_OK?qa937:qb937!=S_OK?qb937:qc938!=S_OK?qc938:qd937;
    double copy937=qr937==S_OK&&!dq937.Disjoint&&dq937.Frequency?1000.0*double(te937-ts937)/dq937.Frequency:-1.0;
    if(FAILED(qr937))smxBridgeFailure555("R949 producer query",long(qr937));
     // R988: release only this observer's ownership after the copy fence and all
     // necessary query reads. Logging may block without pinning a WGC/pool slot.
     // Other users retain their shared ownership; no in-flight GPU resource is destroyed.
     it.frame.ReleaseAfterCopy989();it.frame.Close();it.lease.reset();const double resourcesReleased988=clock();
     const double producerLogBegin988=clock();
     smx_log_fprintf989(file,"%u,%llu,%.9f,%.9f,%.9f,%.9f,%lu,%llu,%ld,%u,%.9f,%llu,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f\n",it.sequence,(unsigned long long)it.value,it.source,it.publish,begin,observed,result,(unsigned long long)completed,long(qr937),unsigned(dq937.Disjoint),copy937,(unsigned long long)dq937.Frequency,copy937>=0?1000.0*double(tr938-te937)/dq937.Frequency:-1.0,copy937>=0?1000.0*double(tr938-ts937)/dq937.Frequency:-1.0,resourcesReleased988,producerLogBegin988,it.callbackEntry,it.inboxAcquired,it.frameGetBegin,it.frameGetEnd,it.copyBegin);
   }
  });
 }
 void enqueue(CaptureFrame949 frame,const std::shared_ptr<GpuFrames>&p,unsigned seq,double publish,double callbackEntry=0,double inboxAcquired=0,double frameGetBegin=0,double frameGetEnd=0,double copyBegin=0){
  check();{std::lock_guard lock(mutex);if(stopping)throw std::runtime_error("Producer observer stopped");if(items.size()>=16)throw std::runtime_error("Producer observer bounded queue full");items.push_back({std::move(frame),p,seq,p->readyValue912,p->captureStamp809,publish,callbackEntry,inboxAcquired,frameGetBegin,frameGetEnd,copyBegin});}cv.notify_one();
 }
 void check()const{if(failure.load())throw std::runtime_error("Producer GPU completion observation failed; stop, no retry");}
 ~CaptureReadyMonitor933(){{std::lock_guard lock(mutex);stopping=true;}cv.notify_all();if(thread.joinable())thread.join();if(file)smx_log_fclose989(file);}
};

extern "C" int smx_app_wait_restart783(int code){
 ++session783;fflush(stdout);fflush(stderr);if(verifyGpuOnly){FILE*sink783=nullptr;smx_log_freopen_s989(&sink783,"NUL","w",stdout);}
 const char*bd=getenv("SMX_BENCH_DIR");
 if(bd){
  std::string archivePrefix988;DWORD identityError988=ERROR_SUCCESS;
  if(!smx988::sessionArchivePrefix988(session783,archivePrefix988,identityError988))smx_log_fprintf989(stderr,"REUSE783 session archive identity failed error=%lu\n",identityError988);
  else for(const char*name:{"app-gpu-verification.json","app-session.json","app-commit700.json","app-startup775.json","app-observed-output.json","app-window795.json"}){
   std::string source=verifyGpuOnly?std::string(bd)+"\\"+name:std::string(name);
   WIN32_FILE_ATTRIBUTE_DATA meta852{};
   if(GetFileAttributesExA(source.c_str(),GetFileExInfoStandard,&meta852)&&CompareFileTime(&meta852.ftLastWriteTime,&sessionBegin852)>=0){
    auto archived988=smx988::copySessionArchive988(source,bd,archivePrefix988,name);
    if(!archived988.copied)smx_log_fprintf989(stderr,"REUSE783 session log archive failed error=%lu collisions=%u destination=%s\n",archived988.error,archived988.collisions,archived988.destination.c_str());
   }
  }
 }
 if(code||closePending)return 0;
 if(nativeRestart830.exchange(false)){stopFlag=false;smx_log_fprintf989(stderr,"NATIVE830 resize restart: model pipelines retained; re-evaluate selected processing mode\n");return 1;}
 if(verifyGpuOnly){int cycles=getenv("SMX_VERIFY_REUSE")?std::clamp(atoi(getenv("SMX_VERIFY_REUSE")),1,3):1;return session783<unsigned(cycles);}
 if(verifyOnly||selftest||demo||benchmarkSource||!getenv("SMX_RETAIN_ENGINE"))return 0;
 std::unique_lock lock(engineMutex783);if(quit783)return 0;restart783=false;engineIdle783=true;
 PostMessageW(control,WM_APP+8,0,(LPARAM)generation783.load());
 smx_log_fprintf989(stderr,"REUSE783 idle: model retained, capture/display released, no GPU work submitted; maximum 120 seconds\n");
 bool signaled=engineCv783.wait_for(lock,std::chrono::seconds(120),[]{return restart783||quit783;});
 bool resume=signaled&&restart783&&!quit783;engineIdle783=false;restart783=false;
 smx_log_fprintf989(stderr,"REUSE783 resume=%d reason=%s\n",resume,resume?"Start":quit783?"Close":"idle-timeout");return resume?1:0;
}
extern "C" int smx_app_session(void* frames,const void* result,uint32_t width,uint32_t height,void* graph,double(*run)(void*),const char* referencePath){
 smxdiag994::instant(SMX_DIAG_SESSION994,smxdiag994::fields(0,++diagnosticCaptureEpoch994,0,0,0,0,1));
 GetSystemTimeAsFileTime(&sessionBegin852);hostCaptureStarted988=false;processingState992.clear();outputProcessingWidth993=0;outputProcessingHeight993=0;outputState993.clear(outputMode993);modelReady992=true;setPreparationStage992(PreparationStage992::CaptureSetup);
 if(verifyGpuOnly){static std::vector<unsigned char> fixture783;if(fixture783.empty())fixture783.assign((unsigned char*)frames,(unsigned char*)frames+size_t(width)*height*8);memcpy(frames,fixture783.data(),fixture783.size());run(graph);FILE*report=nullptr;std::string reportPath=getenv("SMX_BENCH_DIR")?std::string(getenv("SMX_BENCH_DIR"))+"\\app-gpu-verification.json":"app-gpu-verification.json";smx_log_freopen_s989(&report,reportPath.c_str(),"w",stdout);try{return verifyGpuPipelined(frames,result,width,height,graph,run);}catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));smx_log_fprintf989(stderr,"%s\n",e.what());return 5;}}
 if(verifyOnly){
  int repetitions=getenv("SMX_VERIFY_REPEATS")?std::clamp(atoi(getenv("SMX_VERIFY_REPEATS")),10,100000):10;
  double runSeconds=getenv("SMX_VERIFY_SECONDS")?atof(getenv("SMX_VERIFY_SECONDS")):0;
  double hz=getenv("SMX_VERIFY_HZ")?atof(getenv("SMX_VERIFY_HZ")):0;
  double requestedDuty=getenv("SMX_MODEL_DUTY")?atof(getenv("SMX_MODEL_DUTY")):0;
  SmxFrameBudget budget;smx_budget_init(&budget,requestedDuty?requestedDuty:1);
  const char*benchDir=getenv("SMX_BENCH_DIR");std::string prefix=benchDir?std::string(benchDir)+"\\":"";
  FILE*timing=nullptr;if(benchDir)smx_log_fopen_s989(&timing,(prefix+"model-times.csv").c_str(),"wb");
  if(timing)smx_log_fprintf989(timing,"index,start_qpc_seconds,complete_qpc_seconds,gpu_ms,wall_ms,duty,deadline_ms,on_time\n");
  BridgeHandle timer;timer.value=CreateWaitableTimerExW(nullptr,nullptr,2,TIMER_ALL_ACCESS);
  if(!timer.value)return 5;
  using Clock=std::chrono::steady_clock;
  auto epoch=Clock::now();auto next=epoch;double total=0;unsigned measured=0;
  LARGE_INTEGER frequency{};QueryPerformanceFrequency(&frequency);
  for(int i=0;i<repetitions;i++){if(GetFileAttributesW((baseDir+L"\\STOP").c_str())!=INVALID_FILE_ATTRIBUTES)break;
   auto current=Clock::now();
   if(runSeconds>0&&std::chrono::duration<double>(current-epoch).count()>=runSeconds)break;
   if(current<next){
    LARGE_INTEGER due;due.QuadPart=-std::max<LONGLONG>(1,std::chrono::duration_cast<std::chrono::nanoseconds>(next-current).count()/100);
    DWORD timeout=DWORD(std::clamp(std::ceil(std::chrono::duration<double,std::milli>(next-current).count())+1000.0,1000.0,30000.0));if(!SetWaitableTimer(timer.value,&due,0,nullptr,nullptr,FALSE)||WaitForSingleObject(timer.value,timeout)!=WAIT_OBJECT_0)return 5;
   }
   auto start=Clock::now();LARGE_INTEGER q0{},q1{};QueryPerformanceCounter(&q0);
   double ms=run(graph);auto end=Clock::now();QueryPerformanceCounter(&q1);
   double wall=std::chrono::duration<double,std::milli>(end-start).count();
   double deadline=80-(hz?500/hz:0);bool onTime=wall<deadline;
   if(requestedDuty)smx_budget_complete(&budget,std::chrono::duration<double,std::milli>(end-epoch).count(),wall,onTime);
   total+=ms;measured++;
   if(timing){smx_log_fprintf989(timing,"%d,%.9f,%.9f,%.6f,%.6f,%.6f,%.6f,%d\n",i,double(q0.QuadPart)/frequency.QuadPart,double(q1.QuadPart)/frequency.QuadPart,ms,wall,budget.duty,deadline,onTime?1:0);fflush(timing);}
   double interval=hz?1000/hz:0;
   if(requestedDuty)interval=fmax(interval,wall+(budget.nextMs-std::chrono::duration<double,std::milli>(end-epoch).count()));
   next=start+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double,std::milli>(interval));
  }
  if(timing)smx_log_fclose989(timing);
  repetitions=measured;if(!measured)return 5;
  size_t bytes=size_t(width)*height*4;std::vector<unsigned char> reference(bytes);
  FILE*f=nullptr;smx_log_fopen_s989(&f,referencePath,"rb");if(!f)return 5;
  bool complete=fread(reference.data(),1,bytes,f)==bytes&&fgetc(f)==EOF;smx_log_fclose989(f);
  size_t mismatches=0,alpha=0;double mae=0,mse=0;
  for(size_t i=0;i<bytes;i++){int delta=int(reference[i])-int(((const unsigned char*)result)[i]);mismatches+=delta!=0;if((i&3)==3)alpha+=delta!=0;else{mae+=abs(delta);mse+=double(delta)*delta;}}
  mae/=double(width)*height*3;mse/=double(width)*height*3;
  bool qualityPass=mae<=0.02&&mse<=0.065025&&alpha==0;
  bool passed=complete&&(mismatches==0||(getenv("SMX_ALLOW_NUMERIC_DIFF")&&qualityPass));
  smx_log_fopen_s989(&f,(prefix+"app-backend-verification.json").c_str(),"wb");if(f){smx_log_fprintf989(f,"{\"width\":%u,\"height\":%u,\"runs\":%u,\"mean_gpu_ms\":%.6f,\"bytes\":%llu,\"mismatches\":%llu,\"byte_exact\":%s,\"rgb_mae_255\":%.9f,\"rgb_mse_255\":%.9f,\"alpha_mismatches\":%llu,\"quality_gate\":\"MAE <= 0.02 and MSE <= 0.065025 and alpha exact\",\"pass\":%s}\n",width,height,repetitions,total/repetitions,(unsigned long long)bytes,(unsigned long long)mismatches,mismatches==0?"true":"false",mae,mse,(unsigned long long)alpha,passed?"true":"false");smx_log_fclose989(f);}
  return passed?0:5;
 }
 const char*capturePhase989="init_apartment";
 try{
 init_apartment(apartment_type::multi_threaded);
 struct ApartmentRelease783{~ApartmentRelease783(){uninit_apartment();}} apartmentRelease783;
 const bool desktopCapture989=smx989::desktopCapture(getenv("SMX_CAPTURE_BACKEND989"));
 smx_log_fprintf989(stderr,"Capture backend=%s no_fallback=1\n",desktopCapture989?"dxgi_desktop_duplication":"wgc");if(!desktopCapture989){capturePhase989="WGC.GraphicsCaptureSession.IsSupported";if(!GraphicsCaptureSession::IsSupported())throw std::runtime_error("Windows Graphics Capture is unavailable");}capturePhase989="native_extent";
 if(!IsWindow(selected))throw std::runtime_error("Source window has closed");
 if(stopFlag)return 0;
 if(stopFlag)return 0;
  RECT nativeRect830{};if(!GetClientRect(selected,&nativeRect830)||nativeRect830.right<=0||nativeRect830.bottom<=0)throw std::runtime_error("Source has no drawable client");
  const unsigned sourceWidth992=unsigned(nativeRect830.right),sourceHeight992=unsigned(nativeRect830.bottom);
  const auto processingExtent992=smx992::processingExtent(sourceWidth992,sourceHeight992,getenv("SMX_PROCESSING_SIZE992"));
  width=processingExtent992.width;height=processingExtent992.height;outputProcessingWidth993=width;outputProcessingHeight993=height;smxdiag994::instant(SMX_DIAG_GEOMETRY994,smxdiag994::fields(width,height,sourceWidth992,sourceHeight992,0,2));
  if(processingExtent992.resampled&&getenv("SMX_PRESENT_BACKEND")&&strcmp(getenv("SMX_PRESENT_BACKEND"),"dxgi")!=0)throw std::runtime_error("Explicit processing-size resampling currently requires the DXGI/DComp presentation backend");
  if(smx_graph_extent830(width,height))throw std::runtime_error("Selected processing extent exceeds actual device texture/dispatch limits");
  smx_log_fprintf989(stderr,"PROCESSING992 mode=%s source=%ux%u processing=%ux%u display_client=%ux%u external_resample=%u internal_grid=768x432 original_multiscale_retained=1 native_copy_fastpath=%u\n",smx992::processingName(processingExtent992.mode),sourceWidth992,sourceHeight992,width,height,sourceWidth992,sourceHeight992,processingExtent992.resampled?1:0,processingExtent992.resampled?0:1);
  CaptureDisplaySession displaySession(useSession120,baseDir);
 capturePhase989="D3D11CreateDevice";smx_log_fprintf989(stderr,"Create D3D11 device\n");com_ptr<ID3D11Device> device;com_ptr<ID3D11DeviceContext> context;D3D_FEATURE_LEVEL level;
 smxdiag994::checkedHr((D3D11CreateDevice(smxD3DAdapter995().Get(),D3D_DRIVER_TYPE_UNKNOWN,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,nullptr,0,D3D11_SDK_VERSION,device.put(),&level,context.put())),__LINE__);
 com_ptr<ID3D11Multithread> multi=context.as<ID3D11Multithread>();multi->SetMultithreadProtected(TRUE);
 auto dxgi=device.as<IDXGIDevice>();com_ptr<IDXGIAdapter> adapter;smxdiag994::checkedHr((dxgi->GetAdapter(adapter.put())),__LINE__);
 if(FILE*md=smx_log_fopen989("app-d3d-device994.json","wb")){smx_log_fprintf989(md,"{\"api\":\"D3D11\",\"feature_level\":%u,\"capture_device\":%llu,\"dred_available\":false,\"dred_unavailable_reason\":\"D3D11 not D3D12\",\"added_device_removed_query\":false,\"adapter_evidence\":\"app-bridge-adapter994.json reuses existing adapter validation\",\"driver_evidence\":\"app-device994.json reuses existing Vulkan physical properties; no additional driver version query\",\"scope\":\"existing capture device; display adapter selected from same IDXGIAdapter\"}\n",unsigned(level),(unsigned long long)(uintptr_t)device.get());smx_log_fclose989(md);}
 smx_preflight_dxgi989(adapter.get(),width,height,smx989::frameMultiplier(),true,"capture_before_pools");
 com_ptr<IInspectable> inspect;smxdiag994::checkedHr((CreateDirect3D11DeviceFromDXGIDevice(dxgi.get(),inspect.put())),__LINE__);
 auto captureDevice=inspect.as<IDirect3DDevice>();
 FrameScheduling liveScheduling;
 recordWindow976(selected,"game_source_read_only","capture_item_target");
 GraphicsCaptureItem item{nullptr};Direct3D11CaptureFramePool pool{nullptr};GraphicsCaptureSession session{nullptr};std::unique_ptr<DesktopDuplication989> desktopSource989;
 winrt::Windows::Graphics::SizeInt32 size{};
 if(desktopCapture989){
  if(!sourceOverlay)throw std::runtime_error("DXGI desktop capture requires the own overlay path");
  if(getenv("SMX_CAPTURE_CURSOR"))throw std::runtime_error("DXGI cursor synthesis is not implemented; disable capture cursor for this backend");
  capturePhase989="DXGI.DuplicateOutput";desktopSource989=std::make_unique<DesktopDuplication989>(device.get(),selected,output,hud964);size=desktopSource989->size();
  smx992::preflightScaling(adapter.get(),processingExtent992,unsigned(size.Width),unsigned(size.Height),0,smx989::frameMultiplier());
 }else{
  capturePhase989="WGC.CreateForWindow";auto factory=get_activation_factory<GraphicsCaptureItem,IGraphicsCaptureItemInterop>();smxdiag994::checkedHr((factory->CreateForWindow(selected,guid_of<GraphicsCaptureItem>(),put_abi(item))),__LINE__);size=item.Size();
  smx992::preflightScaling(adapter.get(),processingExtent992,unsigned(size.Width),unsigned(size.Height),6,smx989::frameMultiplier());
  capturePhase989="WGC.FramePool.CreateFreeThreaded";pool=Direct3D11CaptureFramePool::CreateFreeThreaded(captureDevice,DirectXPixelFormat::B8G8R8A8UIntNormalized,6,size);capturePhase989="WGC.CreateCaptureSession";session=pool.CreateCaptureSession(item);capturePhase989="WGC.CursorSetting";session.IsCursorCaptureEnabled(getenv("SMX_CAPTURE_CURSOR")!=nullptr);
  try{long long us=getenv("SMX_CAPTURE_INTERVAL_US")?_atoi64(getenv("SMX_CAPTURE_INTERVAL_US")):0;session.MinUpdateInterval(winrt::Windows::Foundation::TimeSpan{us*10});smx_log_fprintf989(stderr,"WGC selected minimum update interval: %.6f ms\n",session.MinUpdateInterval().count()*0.0001);}catch(const hresult_error&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,int32_t(e.code()),SMX_DIAG_HRESULT994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));smx_log_fprintf989(stderr,"WGC update interval unavailable: 0x%08x\n",unsigned(e.code()));}
 }
 capturePhase989="capture_and_display_setup";
 BridgeHandle captureEvent;captureEvent.value=CreateEventW(nullptr,FALSE,FALSE,nullptr);HANDLE event=captureEvent.value;
 if(!event)throw std::runtime_error("Capture event creation failed");
 struct Inbox {std::mutex mutex;struct Entry{CaptureFrame949 frame;std::shared_ptr<GpuFrames> original;unsigned sequence;unsigned geometryEpoch;};std::deque<Entry> frames;unsigned arrived=0,dropped=0;HRESULT error=S_OK;std::string errorText;bool active=true;std::vector<int64_t> stamps;};
 auto inbox=std::make_shared<Inbox>();
 std::atomic<uint64_t>poolExtent795{packExtent795(unsigned(size.Width),unsigned(size.Height))},pendingExtent795{0};std::atomic<bool>recreating795{false};
 unsigned geometryEpoch795=0,lastClientW795=0,lastClientH795=0,nativeFrames795=0,scaledFrames795=0,geometrySkipped795=0,poolRecreates795=0;
 winrt::event_token closed{};if(item)closed=item.Closed([](auto const&,auto const&){targetClosedEvent990();});
 bool wsi958=getenv("SMX_PRESENT_BACKEND")&&strcmp(getenv("SMX_PRESENT_BACKEND"),"vulkan")==0;
 bool scheduled871=getenv("SMX_PRESENT_BACKEND")&&strcmp(getenv("SMX_PRESENT_BACKEND"),"scheduled")==0;
 bool splitDisplay700=!wsi958&&(scheduled871||(getenv("SMX_SPLIT_DISPLAY")&&strcmp(getenv("SMX_SPLIT_DISPLAY"),"1")==0));
 com_ptr<ID3D11Device> displayDevice700=device;com_ptr<ID3D11DeviceContext> displayContext700=context;
 if(splitDisplay700){
  if(!smxDirectEnabled()||!smxDirectOutputEnabled())throw std::runtime_error("Split display requires direct retained input/output");
  displayDevice700=nullptr;displayContext700=nullptr;D3D_FEATURE_LEVEL dl;
  smxdiag994::checkedHr((D3D11CreateDevice(adapter.get(),D3D_DRIVER_TYPE_UNKNOWN,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT|(scheduled871?D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS:0),nullptr,0,D3D11_SDK_VERSION,displayDevice700.put(),&dl,displayContext700.put())),__LINE__);
  auto dm=displayContext700.as<ID3D11Multithread>();dm->SetMultithreadProtected(TRUE);
 }
 auto displayDxgi700=displayDevice700.as<IDXGIDevice>();
  const bool displayBoost988=exactOne976(getenv("SMX_DISPLAY_BOOST988"));
  if(displayBoost988&&(!splitDisplay700||wsi958||scheduled871))throw std::runtime_error("DisplayBoost requires a separate DComp display device; no shared/capture/game priority changes");
  INT priorityBefore988=0,priorityAfter988=0;HRESULT priorityBeforeHr988=displayDxgi700->GetGPUThreadPriority(&priorityBefore988),prioritySetHr988=S_FALSE;
  if(displayBoost988)prioritySetHr988=displayDxgi700->SetGPUThreadPriority(3);
  HRESULT priorityAfterHr988=displayDxgi700->GetGPUThreadPriority(&priorityAfter988);
  FILE*priorityLog988=smx_log_fopen989("app-display-priority988.json","wb");if(priorityLog988){smx_log_fprintf989(priorityLog988,R"({"requested":%s,"separate_display_device":%s,"set_called":%s,"requested_relative_priority":3,"get_before_hr":%ld,"before":%d,"set_hr":%ld,"get_after_hr":%ld,"after":%d,"effective":%s,"scope":"Only own separate DComp D3D display device. Relative mode +3; no absolute/realtime priority; capture, model, game and global settings unchanged. No performance claim."})",displayBoost988?"true":"false",splitDisplay700?"true":"false",displayBoost988?"true":"false",long(priorityBeforeHr988),priorityBefore988,long(prioritySetHr988),long(priorityAfterHr988),priorityAfter988,(displayBoost988&&SUCCEEDED(prioritySetHr988)&&SUCCEEDED(priorityAfterHr988)&&priorityAfter988==3)?"true":"false");smx_log_fclose989(priorityLog988);}
  if(displayBoost988&&(FAILED(prioritySetHr988)||FAILED(priorityAfterHr988)||priorityAfter988!=3))throw std::runtime_error("Requested relative display priority did not apply; stopping this candidate");
  smx_log_fprintf989(stderr,"DISPLAY_DEVICE700 separate=%u same_adapter=1 shared_resource_views_only=1\n",unsigned(splitDisplay700));com_ptr<IDXGIFactory2> dxgiFactory;smxdiag994::checkedHr((adapter->GetParent(__uuidof(IDXGIFactory2),dxgiFactory.put_void())),__LINE__);
 unsigned presentDepth=getenv("SMX_PRESENT_DEPTH")?unsigned(atoi(getenv("SMX_PRESENT_DEPTH"))):(wsi958||scheduled871?2u:3u);if(presentDepth<1||presentDepth>4)throw std::runtime_error("Presentation queue depth must be 1..4");unsigned prefill=getenv("SMX_PRESENT_PREFILL")?unsigned(atoi(getenv("SMX_PRESENT_PREFILL"))):4u;prefill=std::clamp(prefill,1u,4u);
 const bool opaqueContent988=exactOne976(getenv("SMX_OPAQUE_CONTENT988"));
 if(opaqueContent988&&!opaqueContentAllowed988(!wsi958&&!scheduled871,sourceOverlay,!opaqueWindow976))throw std::runtime_error("OpaqueContent requires full-coverage Layered DXGI DComp overlay; no fallback");
 DXGI_SWAP_CHAIN_DESC1 desc=compositionDesc976(width,height,presentDepth,opaqueContent988);
 com_ptr<IDXGISwapChain1> chain;BridgeHandle outputReady;std::unique_ptr<ScheduledPresenter871> scheduledPresenter871;
 if(scheduled871){scheduledPresenter871=std::make_unique<ScheduledPresenter871>(displayDevice700.get(),width,height,presentDepth+2);}
 else if(!wsi958){HRESULT createHr988=dxgiFactory->CreateSwapChainForComposition(displayDevice700.get(),&desc,nullptr,chain.put());
  DXGI_SWAP_CHAIN_DESC1 actual976{};if(FAILED(createHr988)){contentAlphaLog988(opaqueContent988,createHr988,E_FAIL,actual976);smxdiag994::checkedHr((createHr988),__LINE__);}
  HRESULT descHr988=chain->GetDesc1(&actual976);contentAlphaLog988(opaqueContent988,createHr988,descHr988,actual976);
  if(opaqueContent988&&(FAILED(descHr988)||actual976.AlphaMode!=DXGI_ALPHA_MODE_IGNORE))throw std::runtime_error("OpaqueContent actual AlphaMode could not be confirmed as IGNORE; no fallback");
  auto chain2=chain.as<IDXGISwapChain2>();const HRESULT setLatencyHr988=chain2->SetMaximumFrameLatency(presentDepth);UINT actualLatency988=0;const HRESULT getLatencyHr988=chain2->GetMaximumFrameLatency(&actualLatency988);
  FILE*dxgi988=smx_log_fopen989("app-dxgi-config988.json","wb");if(dxgi988){smx_log_fprintf989(dxgi988,"{\"create_succeeded\":true,\"get_desc1_hr\":%ld,\"get_desc1_succeeded\":%s,\"width\":%u,\"height\":%u,\"format\":%u,\"requested_buffer_count\":4,\"buffer_count\":%u,\"buffer_usage\":%u,\"sample_count\":%u,\"swap_effect\":%u,\"alpha_mode\":%u,\"scaling\":%u,\"flags\":%u,\"requested_maximum_frame_latency\":%u,\"set_maximum_frame_latency_hr\":%ld,\"get_maximum_frame_latency_hr\":%ld,\"actual_maximum_frame_latency_valid\":%s,\"actual_maximum_frame_latency\":%u,\"scope\":\"R988 own DXGI swapchain only: buffer capacity fixed at four, queued-frame latency independent. Present(1) and ordered timeline unchanged. This is not an OS-display-latency bound or proof of display.\"}\n",long(descHr988),SUCCEEDED(descHr988)?"true":"false",actual976.Width,actual976.Height,unsigned(actual976.Format),actual976.BufferCount,actual976.BufferUsage,actual976.SampleDesc.Count,unsigned(actual976.SwapEffect),unsigned(actual976.AlphaMode),unsigned(actual976.Scaling),actual976.Flags,presentDepth,long(setLatencyHr988),long(getLatencyHr988),SUCCEEDED(getLatencyHr988)?"true":"false",actualLatency988);smx_log_fclose989(dxgi988);}
  smx_log_fprintf989(stderr,"DXGI988 set_max_latency_hr=%ld get_max_latency_hr=%ld requested=%u actual=%u buffers=%u desc_hr=%ld\n",long(setLatencyHr988),long(getLatencyHr988),presentDepth,actualLatency988,actual976.BufferCount,long(descHr988));
  smxdiag994::checkedHr((descHr988),__LINE__);smxdiag994::checkedHr((setLatencyHr988),__LINE__);smxdiag994::checkedHr((getLatencyHr988),__LINE__);if(actualLatency988!=presentDepth)throw std::runtime_error("DXGI maximum frame latency readback mismatch");
 outputReady.value=chain2->GetFrameLatencyWaitableObject();if(!outputReady.value)throw std::runtime_error("Display waitable object unavailable");}
 BridgeHandle displayTimer;displayTimer.value=CreateWaitableTimerExW(nullptr,nullptr,2,TIMER_ALL_ACCESS);if(!displayTimer.value)throw std::runtime_error("Display timer unavailable");
 logDisplayRates();
 unsigned presentSync=getenv("SMX_PRESENT_SYNC")?unsigned(atoi(getenv("SMX_PRESENT_SYNC"))):1;
 if(presentSync>1)throw std::runtime_error("Experimental present sync must be 0 or 1");
 smx_log_fprintf989(stderr,"OWN_PRESENT sync=%u depth=%u\n",presentSync,presentDepth);
 DWM_TIMING_INFO timing{};timing.cbSize=sizeof(timing);HRESULT dwmHr=DwmGetCompositionTimingInfo(nullptr,&timing);
 smx_log_fprintf989(stderr,"COMPOSITION_RATE hr=%ld refresh=%u/%u compose=%u/%u\n",long(dwmHr),timing.rateRefresh.uiNumerator,timing.rateRefresh.uiDenominator,timing.rateCompose.uiNumerator,timing.rateCompose.uiDenominator);
 com_ptr<IDXGISwapChainMedia> media;HRESULT mediaHr=chain?chain->QueryInterface(__uuidof(IDXGISwapChainMedia),media.put_void()):E_NOINTERFACE;
 smx_log_fprintf989(stderr,"PRESENT_DURATION_INTERFACE hr=%ld\n",long(mediaHr));
 if(SUCCEEDED(mediaHr)){for(UINT requested: {83333u,166667u}){UINT smaller=0,larger=0;HRESULT hr=media->CheckPresentDurationSupport(requested,&smaller,&larger);smx_log_fprintf989(stderr,"PRESENT_DURATION_SUPPORT request=%u hr=%ld smaller=%u larger=%u\n",requested,long(hr),smaller,larger);}}


 
 unsigned ownPresentFlags=0;
 if(const char*value=getenv("SMX_PRESENT_DURATION_100NS")){
  UINT duration=unsigned(atoi(value)),smaller=0,larger=0;
  if(!media||!duration)throw std::runtime_error("Custom duration unavailable");
  smxdiag994::checkedHr((media->CheckPresentDurationSupport(duration,&smaller,&larger)),__LINE__);
  if(smaller!=duration||larger!=duration)throw std::runtime_error("Exact custom duration unsupported");
  smxdiag994::checkedHr((media->SetPresentDuration(duration)),__LINE__);ownPresentFlags=DXGI_PRESENT_USE_DURATION;
  smx_log_fprintf989(stderr,"OWN_DURATION_SET request=%u flags=%u; own swapchain only\n",duration,ownPresentFlags);
 }
com_ptr<IDCompositionDevice> composition;com_ptr<IDCompositionTarget> target;com_ptr<IDCompositionVisual> visual;com_ptr<IUnknown> scheduledContent871;std::unique_ptr<OutputBackground993> outputBackground993;
 if(!wsi958){smxdiag994::checkedHr((DCompositionCreateDevice(displayDxgi700.get(),__uuidof(IDCompositionDevice),composition.put_void())),__LINE__);smxdiag994::checkedHr((composition->CreateTargetForHwnd(output,TRUE,target.put())),__LINE__);smxdiag994::checkedHr((composition->CreateVisual(visual.put())),__LINE__);
 if(scheduled871){scheduledContent871=scheduledPresenter871->content(composition.get());smxdiag994::checkedHr((visual->SetContent(scheduledContent871.get())),__LINE__);}
 else smxdiag994::checkedHr((visual->SetContent(chain.get())),__LINE__);smxdiag994::checkedHr((target->SetRoot(visual.get())),__LINE__);
 if(outputPreview993()){if(scheduled871)throw std::runtime_error("Output preview requires DXGI/DComp");outputBackground993=std::make_unique<OutputBackground993>();outputBackground993->initialize(composition.get(),target.get(),visual.get(),displayDevice700.get(),displayContext700.get());}
 if(outputPreview993()||processingExtent992.resampled)smxdiag994::checkedHr((visual->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR)),__LINE__);
 smxdiag994::checkedHr((SMX_DIAG_CALL994(SMX_DIAG_GEOMETRY994,smxdiag994::fields(),SMX_DIAG_HRESULT994,composition->Commit())),__LINE__);}
 com_ptr<ID3D11Texture2D> back;if(chain)smxdiag994::checkedHr((chain->GetBuffer(0,__uuidof(ID3D11Texture2D),back.put_void())),__LINE__);
 GpuGraphBridge bridge(device.get(),context.get());if(splitDisplay700)bridge.setDisplayDevice700(displayDevice700.get());if(!getenv("SMX_PIPELINE905")||!smxDirectEnabled()||!smxDirectOutputEnabled()||getenv("SMX_INPUT_AUDIT")||getenv("SMX_LIVE_PROOF")||getenv("SMX_CLONE_ORIGINALS")||getenv("SMX_SNAPSHOT_INPUTS"))throw std::runtime_error("R948 requires bounded direct async path without audit, clone, readback or serial snapshot");
 bridge.enableNativeSync948();bridge.prepareAsync930();if(wsi958)bridge.startWSI958(output);
  // R988 prepares the observer before WGC starts. No model or GPU submission is added.
  if(getenv("SMX_PIPELINE905"))bridge.startCompletion906();
  const bool pixelProbeRequested988=exactOne976(getenv("SMX_PIXEL_PROBE976"));const bool pixelProbeEffective988=pixelProbeRequested988&&wsi958;
  FILE*effective988=smx_log_fopen989("app-requested-effective988.json","wb");if(effective988){smx_log_fprintf989(effective988,"{\"presentation_backend\":\"%s\",\"pixel_probe_requested\":%s,\"pixel_probe_effective\":%s,\"pixel_probe_reason\":\"%s\",\"scope\":\"Effective means instrumented backend initialized; successful pixel samples require the probe result files. DXGI and scheduled backends do not implement the WSI pixel probe.\"}\n",wsi958?"vulkan":scheduled871?"scheduled":"dxgi",pixelProbeRequested988?"true":"false",pixelProbeEffective988?"true":"false",!pixelProbeRequested988?"not_requested":wsi958?"WSI_probe_initialized":"selected_backend_has_no_WSI_probe");smx_log_fclose989(effective988);}

 // Native stays a texture copy. Resampling uses shipped DXBC on the capture context.
 std::unique_ptr<WindowNormalize992> normalizer992;if(processingExtent992.resampled)normalizer992=std::make_unique<WindowNormalize992>(device.get(),context.get());
 const unsigned frameMultiplier989=smx989::frameMultiplier();std::vector<std::shared_ptr<GpuFrames>> texturePool(frameMultiplier989==2?6:12);
 for(auto&entry:texturePool){entry=std::make_shared<GpuFrames>();if(benchmarkSource&&getenv("SMX_LIVE_PROOF")&&strcmp(getenv("SMX_LIVE_PROOF"),"1")==0)entry->first=bridge.makePresentationTexture();entry->middle=bridge.makeOutputTexture();}
 unsigned sw=0,sh=0; com_ptr<ID3D11Texture2D> clientTexture; unsigned clientTextureW=0,clientTextureH=0;
 size_t bytes=(size_t)width*height*4;
 std::mutex queueMutex;std::condition_variable queueChanged;LARGE_INTEGER timelineFrequency{};QueryPerformanceFrequency(&timelineFrequency);auto timelineNow=[&](){LARGE_INTEGER q{};QueryPerformanceCounter(&q);return double(q.QuadPart)/timelineFrequency.QuadPart;};
 double delayMs=getenv("SMX_TIMELINE_DELAY_MS")?atof(getenv("SMX_TIMELINE_DELAY_MS")):80.0;if(!std::isfinite(delayMs)||delayMs<10||delayMs>200)throw std::runtime_error("Presentation buffer must be 10..200 ms");
 BridgeHandle presentationChanged;presentationChanged.value=CreateEventW(nullptr,FALSE,FALSE,nullptr);if(!presentationChanged.value)throw std::runtime_error("Presentation event failed");
 const bool phaseLock971=getenv("SMX_PHASE_LOCK971")&&strcmp(getenv("SMX_PHASE_LOCK971"),"1")==0;
  OriginalTimeline<std::shared_ptr<GpuFrames>> timeline(delayMs*0.001,24,1.0/120.0,phaseLock971);
  const bool lifecycleDComp988=chain&&!wsi958&&!scheduledPresenter871;
  const bool displayFenceRequested988=exactOne976(getenv("SMX_DISPLAY_FENCE_PROBE988"));
  if(displayFenceRequested988&&!lifecycleDComp988)throw std::runtime_error("Display fence probe requires the DComp DXGI backend");
  std::unique_ptr<smx988::DisplayFenceProbe988> displayFenceProbe988;
  if(displayFenceRequested988)displayFenceProbe988=std::make_unique<smx988::DisplayFenceProbe988>(stopFlag,getenv("SMX_BENCH_DIR")?getenv("SMX_BENCH_DIR"):".");
  const bool originalRecoveryRequested988=exactOne976(getenv("SMX_ORIGINAL_RECOVERY988"));
  double recoveryOutputHz988=getenv("SMX_RECOVERY_OUTPUT_HZ988")?atof(getenv("SMX_RECOVERY_OUTPUT_HZ988")):0;
  bool reliableRecoveryPeriod988=std::isfinite(recoveryOutputHz988)&&recoveryOutputHz988>=10&&recoveryOutputHz988<=240;if(!std::isfinite(recoveryOutputHz988))recoveryOutputHz988=0;
  FILE*recoveryTrace988=smx_log_fopen989("app-original-recovery988.csv","wb");
  struct CloseRecovery988{FILE*f;~CloseRecovery988(){if(f)smx_log_fclose989(f);}}closeRecovery988{recoveryTrace988};
  const bool progressWindow988=exactOne976(getenv("SMX_RECOVERY_PROGRESS988"));
  if(progressWindow988&&!originalRecoveryRequested988)throw std::runtime_error("Recovery progress window requires explicit original recovery");
  const bool sustainedRecovery988=exactOne976(getenv("SMX_SUSTAINED_RECOVERY988"));
  if(sustainedRecovery988&&(!originalRecoveryRequested988||!lifecycleDComp988))throw std::runtime_error("Sustained recovery requires explicit original recovery");
  const bool claimedOriginalRecovery988=exactOne976(getenv("SMX_CLAIMED_ORIGINAL_RECOVERY988"));
  if(claimedOriginalRecovery988&&(!originalRecoveryRequested988||!sustainedRecovery988||!lifecycleDComp988))throw std::runtime_error("Claimed original recovery requires explicit original and sustained recovery on DComp");
  timeline.configureRecovery988(originalRecoveryRequested988&&lifecycleDComp988,reliableRecoveryPeriod988?1.0/recoveryOutputHz988:0,reliableRecoveryPeriod988,recoveryTrace988,progressWindow988,sustainedRecovery988,claimedOriginalRecovery988);
  double displayPrepareMs988=getenv("SMX_DISPLAY_PREPARE_MS988")?atof(getenv("SMX_DISPLAY_PREPARE_MS988")):6;
  if(!std::isfinite(displayPrepareMs988)||displayPrepareMs988<0||displayPrepareMs988>32)throw std::runtime_error("Own DComp display preparation lead must be 0..32 ms");
  struct Claim988{double qpc,stamp,due;uint64_t ticket;bool generated;};std::vector<Claim988>claimEvents988;claimEvents988.reserve(4096);uint64_t claimTruncated988=0;
  struct Admission988{double qpc,first,second,due,estimateMs;uint64_t ticket;const char*reason;};std::vector<Admission988>admissionEvents988;admissionEvents988.reserve(4096);uint64_t admissionTruncated988=0;
   struct DisplayCopyStage988{unsigned id;bool generated,keyed,native;double source,begin,copy,keyRelease,signal,markCopy,flush,accessReset;};std::vector<DisplayCopyStage988>copyStages988;copyStages988.reserve(4096);uint64_t copyStagesTruncated988=0;


  std::atomic<bool> captureActive988{false},lifecycleCaptureGate988{false};
  std::atomic<double> freshSourceAfter988{0};
  uint64_t lifecycleSerial988=0,lifecycleUiEpoch988=0; // queueMutex; only display closes.
  struct LifecycleEvent988 {double qpc;uint64_t epoch,uiEpoch;const char*action;const char*reason;unsigned lastSubmitted,observed;bool credit;};
  std::vector<LifecycleEvent988> lifecycleEvents988;lifecycleEvents988.reserve(128);
  std::atomic<bool> waitableCredit988{false};unsigned lastSubmitted988=0;bool freshSubmitPending988=false;
  smx988::NaturalEnd988 naturalEnd988;std::atomic<bool>presentMetadataPending988{false};

 auto hudSession988=std::make_shared<smx988::HudSession988>();hudSession988->width=width;hudSession988->height=height;hudSession988->backend=chain?"dxgi":"vulkan";
 {std::lock_guard<std::mutex>lock(hudSessionMutex988);hudSessionGlobal988=hudSession988;}
 // Begins before capture and the first Present. No query is made on Present's thread.
 smx988::HudMonitor988 hudMonitor988(hudSession988,chain.get(),getenv("SMX_BENCH_DIR")?getenv("SMX_BENCH_DIR"):".");

  FILE*phaseTrace971=nullptr;if(phaseLock971)smx_log_fopen_s989(&phaseTrace971,"app-phase971.csv","wb");struct PhaseTraceCleanup971{FILE*f;~PhaseTraceCleanup971(){if(f)smx_log_fclose989(f);}}phaseCleanup971{phaseTrace971};timeline.setPhaseTrace971(phaseTrace971);
  smx_log_fprintf989(stderr,"PHASE971 enabled=%u base_delay_ms=%.6f max_content_to_due_ms=%.6f phase_bound_ms=8 phase_step_max_ms=.25 period_step_max_ms=.02 estimated_source_rate=1 no_model_change=1\n",unsigned(phaseLock971),delayMs,delayMs+(phaseLock971?8:0));
 ComPtr<ID3D11Device5> captureDevice912;smxdiag994::checkedHr((device->QueryInterface(IID_PPV_ARGS(&captureDevice912))),__LINE__);
 ComPtr<ID3D11DeviceContext4> captureContext912;smxdiag994::checkedHr((context->QueryInterface(IID_PPV_ARGS(&captureContext912))),__LINE__);
 ComPtr<ID3D11Fence> captureFence912;smxdiag994::checkedHr((captureDevice912->CreateFence(0,D3D11_FENCE_FLAG_NONE,IID_PPV_ARGS(&captureFence912))),__LINE__);
 UINT64 captureValue912=0;
 bool cloneOriginal729=getenv("SMX_CLONE_ORIGINALS")&&strcmp(getenv("SMX_CLONE_ORIGINALS"),"1")==0;if(cloneOriginal729&&!splitDisplay700)throw std::runtime_error("Display clone requires split device");smx_log_fprintf989(stderr,"DISPLAY_CLONE729 enabled=%u extra_capacity_bytes=%llu extra_nominal_RW_bytes_per_capture=%llu\n",unsigned(cloneOriginal729),cloneOriginal729?16ull*width*height*4:0,cloneOriginal729?2ull*width*height*4:0);std::array<std::shared_ptr<GpuFrames>,16> originals;for(auto&entry:originals){entry=std::make_shared<GpuFrames>();entry->first=bridge.makeOriginalTexture();D3D11_QUERY_DESC q937{D3D11_QUERY_TIMESTAMP,0};smxdiag994::checkedHr((device->CreateQuery(&q937,&entry->copyStart937)),__LINE__);smxdiag994::checkedHr((device->CreateQuery(&q937,&entry->copyEnd937)),__LINE__);smxdiag994::checkedHr((device->CreateQuery(&q937,&entry->copyRelease938)),__LINE__);q937.Query=D3D11_QUERY_TIMESTAMP_DISJOINT;smxdiag994::checkedHr((device->CreateQuery(&q937,&entry->copyDomain937)),__LINE__);if(cloneOriginal729)entry->middle=bridge.makeDisplayOriginal729();entry->readyEvent912.value=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!entry->readyEvent912.value)throw std::runtime_error("Capture ready event creation failed");}

 unsigned originalCursor876=0,generatedCursor876=0;
 auto leaseOriginal876=[&]()->std::shared_ptr<GpuFrames>{
  for(unsigned i=0;i<originals.size();i++){unsigned j=(originalCursor876+i)%originals.size();if(originals[j].use_count()==1){originalCursor876=(j+1)%originals.size();return originals[j];}}
  return {};
 };
 smx_log_fprintf989(stderr,"POOL876 original_slots=%zu generated_slots=%zu round_robin=1 retained_until_display_copy_complete=1 resource_flags_unkeyed_shared_fence948=1\n",originals.size(),texturePool.size());
 // Commit retained allocations before capture begins; this is setup, never inference.
 // No dummy generated frames, and no display submission is counted here.
 if(getenv("SMX_PRIME_RESOURCES")){
  double p0=timelineNow();ComPtr<ID3D11Texture2D>seed744;D3D11_TEXTURE2D_DESC td{};td.Width=width;td.Height=height;td.MipLevels=td.ArraySize=1;td.Format=DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count=1;td.Usage=D3D11_USAGE_DEFAULT;td.BindFlags=D3D11_BIND_SHADER_RESOURCE;
  std::vector<unsigned char>zero744(bytes,0);D3D11_SUBRESOURCE_DATA sd{};sd.pSysMem=zero744.data();sd.SysMemPitch=width*4;smxdiag994::checkedHr((device->CreateTexture2D(&td,&sd,&seed744)),__LINE__);
  auto initialize744=[&](ID3D11Texture2D*t){if(!t)return;auto access=bridge.access948(t,context.get());ComPtr<IDXGIKeyedMutex>m;std::unique_ptr<BridgeLock>l;if(SUCCEEDED(t->QueryInterface(IID_PPV_ARGS(&m))))l=std::make_unique<BridgeLock>(m.Get());context->CopyResource(t,seed744.Get());if(access)access->signal();context->Flush();};
  for(auto&e:originals){initialize744(e->first.Get());initialize744(e->middle.Get());}for(auto&e:texturePool){initialize744(e->first.Get());initialize744(e->middle.Get());}
  D3D11_QUERY_DESC qd{D3D11_QUERY_EVENT,0};ComPtr<ID3D11Query>q;smxdiag994::checkedHr((device->CreateQuery(&qd,&q)),__LINE__);context->End(q.Get());context->Flush();HRESULT qr;double until=timelineNow()+2;do{qr=context->GetData(q.Get(),nullptr,0,0);if(qr==S_FALSE)Sleep(1);}while(qr==S_FALSE&&timelineNow()<until);if(qr!=S_OK)throw std::runtime_error("Resource initialization deadline failed; stop no retry");
  smx_log_fprintf989(stderr,"RESOURCE_PRIME744 setup_ms=%.6f eliminated_unused_first_bytes=%llu; no generated frames computed\n",(timelineNow()-p0)*1000,6ull*width*height*4);
 }
 const bool interactive775=!getenv("SMX_STRICT_TIMING_TEST");
 const bool startupWarmup775=sourceOverlay||windowTest795||getenv("SMX_STARTUP_WARMUP");
 std::atomic<unsigned> stableOriginal775{0};std::atomic<double> recoveryUntil775{0};
 unsigned startupSkipped775=0,recoverySkipped775=0;double startupComplete775=0;
 bool startupFinished775=!startupWarmup775,performanceStopped775=false;
 std::atomic<bool> displayDone{false};std::string displayError;bool audit724=benchmarkSource&&getenv("SMX_INPUT_AUDIT");com_ptr<ID3D11Texture2D>atlas724;unsigned auditPairs724=0;unsigned inferReturned700=0;double drainStart700=0;std::atomic<bool> latenessFailure700{false};
 if(audit724){D3D11_TEXTURE2D_DESC ad{};ad.Width=720;ad.Height=1024;ad.MipLevels=ad.ArraySize=1;ad.Format=DXGI_FORMAT_B8G8R8A8_UNORM;ad.SampleDesc.Count=1;ad.Usage=D3D11_USAGE_STAGING;ad.CPUAccessFlags=D3D11_CPU_ACCESS_READ;smxdiag994::checkedHr((device->CreateTexture2D(&ad,nullptr,atlas724.put())),__LINE__);}
 std::atomic<unsigned>latePresents{0},droppedPairs{0};unsigned sourceGaps=0;

 bool outputGuard=getenv("SMX_OUTPUT_GUARD")&&strcmp(getenv("SMX_OUTPUT_GUARD"),"1")==0;
 bool resourceEnabled=getenv("SMX_RESOURCE_BUDGET")&&strcmp(getenv("SMX_RESOURCE_BUDGET"),"1")==0;if(resourceEnabled)outputGuard=false;
 overlayBypass=(outputGuard||resourceEnabled)&&sourceOverlay;bypassCaptured=0;activeCaptured=0;
 double previousCaptureStamp=0;std::atomic<double> capturePeriodMs{1000.0/60};
 unsigned callbacks838=0,rawFrames838=0,paused838=0,invalid838=0,recreate838=0,extent838=0,crop838=0,restart838=0;
 FILE*copyTrace926=smx_log_fopen989("app-copy926.csv","wb");struct CloseCopy926{FILE*f;~CloseCopy926(){if(f)smx_log_fclose989(f);}}closeCopy926{copyTrace926};
 if(copyTrace926)smx_log_fprintf989(copyTrace926,"sequence,source_qpc,copy_begin_qpc,publish_qpc,acquire_cpu_ms,copy_cpu_ms,release_cpu_ms,signal_flush_cpu_ms,ready_fence\n");
 auto release926=[&](std::unique_ptr<BridgeLock>&lock){if(lock&&lock->mutex){HRESULT hr=lock->mutex->ReleaseSync(0);lock->mutex=nullptr;smxdiag994::checkedHr((hr),__LINE__);}lock.reset();};
 smx_log_fprintf989(stderr,"CAPTURE_HANDOFF926 copy_release_signal_flush=1 per_resource_gpu_fence948=1 ready_after_release=1\n");
 CaptureReadyMonitor933 producer933(captureFence912.Get(),context.get(),event);
  const bool captureTrace988=getenv("SMX_CAPTURE_TRACE988")&&strcmp(getenv("SMX_CAPTURE_TRACE988"),"1")==0;
  bool rawProofDone989=false;
  auto processCapture989=[&,inbox,event](auto&& nextFrame989)noexcept{
  const double callbackEntry988=captureTrace988?timelineNow():0;
   std::lock_guard inboxLock(inbox->mutex);const double inboxAcquired988=captureTrace988?timelineNow():0;++callbacks838;if(!inbox->active)return;
  try{for(;;){const double frameGetBegin988=captureTrace988?timelineNow():0;smxdiag994::Scope acquire994(SMX_DIAG_WGC_TRYGET994,smxdiag994::fields(inbox->arrived+1));CaptureFrame949 frame=nextFrame989();acquire994.end(SMX_DIAG_SEMANTIC994,frame?1:0);const double frameGetEnd988=captureTrace988?timelineNow():0;if(!frame)break;
   ++rawFrames838;
   if((sourceOverlay&&capturePaused989())||lifecycleCaptureGate988.load()||(freshSourceAfter988.load()>0&&frame.SystemRelativeTime().count()*1e-7<freshSourceAfter988.load())){++paused838;frame.Close();continue;}
   auto actual=frame.ContentSize();if(actual.Width<=0||actual.Height<=0){++invalid838;frame.Close();continue;}
   if(recreating795){++recreate838;frame.Close();continue;}
   if(!validExtent795(actual.Width,actual.Height))throw std::runtime_error("Capture extent exceeds bounded window support");
   uint64_t extent795=packExtent795(unsigned(actual.Width),unsigned(actual.Height));
   if(extent795!=poolExtent795.load()){smxdiag994::instant(SMX_DIAG_GEOMETRY994,smxdiag994::fields(inbox->arrived+1,0,extent795,poolExtent795.load(),0,1,0));++extent838;geometrySkipped795++;if(processingExtent992.resampled){++restart838;nativeRestart830=true;stopFlag=true;}else pendingExtent795=extent795;frame.Close();SetEvent(event);continue;}
   if(overlayBypass){
    double stamp=frame.SystemRelativeTime().count()*1e-7;
    double dt=previousCaptureStamp?(stamp-previousCaptureStamp)*1000:0;previousCaptureStamp=stamp;
    if(dt>1&&dt<200)capturePeriodMs=capturePeriodMs.load()*.8+dt*.2;
    inbox->arrived++;capturedOriginal++;hudSession988->captured++;hudSession988->metrics.recordCapture(stamp);bypassCaptured++;
    if(inbox->stamps.size()<72000)inbox->stamps.push_back(frame.SystemRelativeTime().count());
    frame.Close();continue;
   }
   auto texture=frame.Texture989();
   D3D11_TEXTURE2D_DESC td{};texture->GetDesc(&td);unsigned cw=std::min(td.Width,unsigned(actual.Width)),ch=std::min(td.Height,unsigned(actual.Height));
   RECT cr{},wr{},fr{};POINT origin{0,0};if(!GetClientRect(selected,&cr)||!ClientToScreen(selected,&origin)||!GetWindowRect(selected,&wr))throw std::runtime_error("Cannot query original client rectangle");
   if(inbox->arrived==0)smx_log_fprintf989(stderr,"FIRST_CAPTURE texture=%ux%u content=%dx%d format=%u client=%ldx%ld window=%ld,%ld,%ld,%ld origin=%ld,%ld\n",td.Width,td.Height,actual.Width,actual.Height,unsigned(td.Format),cr.right,cr.bottom,wr.left,wr.top,wr.right,wr.bottom,origin.x,origin.y);
   RECT captureRect=wr;if(desktopCapture989)captureRect=desktopSource989->captureRect();else{HRESULT hr=DwmGetWindowAttribute(selected,DWMWA_EXTENDED_FRAME_BOUNDS,&fr,sizeof(fr));
   if(SUCCEEDED(hr)&&fr.right-fr.left==long(cw)&&fr.bottom-fr.top==long(ch))captureRect=fr;
   else if(wr.right-wr.left!=long(cw)||wr.bottom-wr.top!=long(ch)){++crop838;geometrySkipped795++;frame.Close();continue;}}
   long ox=origin.x-captureRect.left,oy=origin.y-captureRect.top;
   if(!validExtent795(cr.right,cr.bottom)||ox<0||oy<0||uint64_t(ox)+cr.right>cw||uint64_t(oy)+cr.bottom>ch){++crop838;geometrySkipped795++;frame.Close();continue;}
   if(td.Format!=DXGI_FORMAT_B8G8R8A8_UNORM)throw std::runtime_error("Window capture requires BGRA8");
   if(cr.right!=long(sourceWidth992)||cr.bottom!=long(sourceHeight992)){smxdiag994::instant(SMX_DIAG_GEOMETRY994,smxdiag994::fields(inbox->arrived+1,0,(uintptr_t)selected,0,0,cr.right,cr.bottom));++restart838;nativeRestart830=true;stopFlag=true;frame.Close();SetEvent(event);continue;}
   if(lastClientW795!=unsigned(cr.right)||lastClientH795!=unsigned(cr.bottom)){
    lastClientW795=unsigned(cr.right);lastClientH795=unsigned(cr.bottom);++geometryEpoch795;{std::lock_guard lock(queueMutex);timeline.resetPacing971("geometry");}
    for(auto&old795:inbox->frames){old795.frame.Close();inbox->dropped++;}inbox->frames.clear();
    smx_log_fprintf989(stderr,"WINDOW795 source=%ux%u model=%ux%u resampled=%u epoch=%u\n",lastClientW795,lastClientH795,width,height,unsigned(lastClientW795!=width||lastClientH795!=height),geometryEpoch795);
    status(L"Source "+std::to_wstring(lastClientW795)+L" x "+std::to_wstring(lastClientH795)+L" → processing "+std::to_wstring(width)+L" x "+std::to_wstring(height)+L". Overlay follows the game client.");
   }
   std::shared_ptr<GpuFrames>lease;
   {std::lock_guard lock(queueMutex);lease=leaseOriginal876();if(!lease){timeline.evictOldestOriginal();lease=leaseOriginal876();}}
   if(!lease)throw std::runtime_error("Original texture pool exhausted");
   D3D11_BOX crop{unsigned(ox),unsigned(oy),0,unsigned(ox)+unsigned(cr.right),unsigned(oy)+unsigned(cr.bottom),1};
   if(selftest&&!rawProofDone989&&exactOne976(getenv("SMX_PROOF989"))&&!smxPixelProofEligible989(unsigned(cr.right),unsigned(cr.bottom))){smxPixelProofSkipped989("app-capture-raw989",unsigned(cr.right),unsigned(cr.bottom));rawProofDone989=true;}
   if(normalizer992)normalizer992->prepareClient(texture.get(),unsigned(ox),unsigned(oy),unsigned(cr.right),unsigned(cr.bottom),lease->first.Get());
   SmxDiagFields994 captureFields994=smxdiag994::fields(inbox->arrived+1,0,(uintptr_t)lease->first.Get(),lease->readyValue912,frame.SystemRelativeTime().count()*1e-7,sourceWidth992,sourceHeight992);
   double copyBegin926=timelineNow();
   auto capture948=SMX_DIAG_OPERATION994(SMX_DIAG_CAPTURE_ACQUIRE994,captureFields994,bridge.access948(lease->first.Get(),context.get()));ComPtr<IDXGIKeyedMutex> captureMutex;std::unique_ptr<BridgeLock> captureLock;if(smxDirectEnabled()&&!capture948){smxdiag994::checkedHr((lease->first.As(&captureMutex)),__LINE__);captureLock=std::make_unique<BridgeLock>(captureMutex.Get());}
   ComPtr<IDXGIKeyedMutex>cloneMutex729;std::unique_ptr<BridgeLock>cloneLock729;if(cloneOriginal729){smxdiag994::checkedHr((lease->middle.As(&cloneMutex729)),__LINE__);cloneLock729=std::make_unique<BridgeLock>(cloneMutex729.Get());}
    double acquired926=timelineNow();
    if(selftest&&!rawProofDone989&&exactOne976(getenv("SMX_PROOF989"))){frame.BeginCopy989();smxCaptureRawProof989(device.get(),context.get(),texture.get(),crop);rawProofDone989=true;}frame.BeginCopy989();smxdiag994::Scope captureCopy994(SMX_DIAG_CAPTURE_COPY994,captureFields994);context->Begin(lease->copyDomain937.Get());context->End(lease->copyStart937.Get());if(normalizer992){normalizer992->copyPrepared();++scaledFrames795;}else{context->CopySubresourceRegion(lease->first.Get(),0,0,0,0,texture.get(),0,&crop);nativeFrames795++;}context->End(lease->copyEnd937.Get());captureCopy994.end();
    if(cloneOriginal729)context->CopyResource(lease->middle.Get(),lease->first.Get());lease->readyObserved809=false;lease->captureStamp809=frame.SystemRelativeTime().count()*1e-7;lease->readyStamp809=0;ResetEvent(lease->readyEvent912.value);lease->readyValue912=++captureValue912;
    double copied926=timelineNow();
    release926(cloneLock729);release926(captureLock);if(capture948)capture948->signal();context->End(lease->copyRelease938.Get());context->End(lease->copyDomain937.Get());
    double released926=timelineNow();
    smxdiag994::checkedHr((SMX_DIAG_CALL994(SMX_DIAG_CAPTURE_SIGNAL994,smxdiag994::fields(inbox->arrived+1,0,(uintptr_t)captureFence912.Get(),lease->readyValue912),SMX_DIAG_HRESULT994,captureContext912->Signal(captureFence912.Get(),lease->readyValue912))),__LINE__);
    smxdiag994::checkedHr((captureFence912->SetEventOnCompletion(lease->readyValue912,lease->readyEvent912.value)),__LINE__);
    SMX_DIAG_COMMAND994(SMX_DIAG_CAPTURE_FLUSH994,captureFields994,context->Flush());capture948.reset();double flushed926=timelineNow();producer933.enqueue(frame,lease,inbox->arrived+1,flushed926,callbackEntry988,inboxAcquired988,frameGetBegin988,frameGetEnd988,captureTrace988?copyBegin926:0);activeCaptured++;
    if(copyTrace926)smx_log_fprintf989(copyTrace926,"%u,%.9f,%.9f,%.9f,%.6f,%.6f,%.6f,%.6f,%llu\n",inbox->arrived+1,lease->captureStamp809,copyBegin926,flushed926,(acquired926-copyBegin926)*1000,(copied926-acquired926)*1000,(released926-copied926)*1000,(flushed926-released926)*1000,(unsigned long long)lease->readyValue912);
   double stamp=frame.SystemRelativeTime().count()*1e-7;
   double captureDt=previousCaptureStamp?(stamp-previousCaptureStamp)*1000:0;previousCaptureStamp=stamp;
   if(captureDt>1&&captureDt<200)capturePeriodMs=capturePeriodMs.load()*.8+captureDt*.2;
   bool capturePublished988=false;{std::lock_guard lock(queueMutex);if(!lifecycleCaptureGate988.load()){timeline.push(lease,stamp,false,timelineNow());capturePublished988=true;}}if(!capturePublished988){++paused838;frame.Close();continue;}SetEvent(presentationChanged.value);queueChanged.notify_one();
   inbox->arrived++;capturedOriginal++;hudSession988->captured++;hudSession988->metrics.recordCapture(stamp);if(inbox->stamps.size()<72000)inbox->stamps.push_back(frame.SystemRelativeTime().count());
   if(inbox->frames.size()>=4){inbox->frames.front().frame.Close();inbox->frames.pop_front();inbox->dropped++;}inbox->frames.push_back({std::move(frame),std::move(lease),inbox->arrived,geometryEpoch795});
  }}catch(const hresult_error&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,int32_t(e.code()),SMX_DIAG_HRESULT994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));inbox->error=e.code();inbox->errorText="Capture HRESULT "+std::to_string(uint32_t(e.code()))+": "+to_string(e.message());}catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));inbox->error=E_FAIL;inbox->errorText=e.what();}catch(...){inbox->error=E_FAIL;inbox->errorText="Unknown capture callback error";}if(FAILED(inbox->error)){smx_log_fprintf989(stderr,"CAPTURE_ERROR: %s\n",inbox->errorText.c_str());fflush(stderr);}SetEvent(event);
 };
 winrt::event_token token{};if(pool)token=pool.FrameArrived([&](auto const&sender,auto const&){processCapture989([&](){return CaptureFrame949(sender.TryGetNextFrame());});});
 auto stopCapture989=[&](){smxdiag994::Scope close994(SMX_DIAG_POOL_CLOSE994);if(desktopSource989)desktopSource989->close();if(session)SMX_DIAG_COMMAND994(SMX_DIAG_POOL_CLOSE994,smxdiag994::fields(0,0,0,0,0,1),session.Close());if(pool)pool.FrameArrived(token);close994.end();};
 struct CallbackCleanup{std::function<void()>cleanup;bool active=true;~CallbackCleanup(){if(active)cleanup();}} callbackCleanup{[&,inbox](){try{stopCapture989();}catch(...){}std::lock_guard lock(inbox->mutex);inbox->active=false;for(auto&f:inbox->frames){try{f.frame.Close();}catch(...){}}inbox->frames.clear();}};
 auto recordLifecycle988=[&](const char*action,SmxLifecycleReason980 why){ // caller owns queueMutex
   smxdiag994::instant(why==SmxLifecycleReason980::Stop?SMX_DIAG_STOP994:(strcmp(action,"closed")==0?SMX_DIAG_PAUSE994:SMX_DIAG_RESUME994),smxdiag994::fields(lastSubmitted988,lifecycleSerial988,lifecycleUiEpoch988,0,0,strcmp(action,"closed")==0?0:1));
   DXGI_FRAME_STATISTICS stats{};HRESULT hr=E_NOINTERFACE;hudSession988->getLatest(stats,hr);
   if(lifecycleEvents988.size()<4096)lifecycleEvents988.push_back({timelineNow(),lifecycleSerial988,lifecycleUiEpoch988,action,why==SmxLifecycleReason980::Stop?"stop":"pause",lastSubmitted988,hr==S_OK?stats.PresentCount:0,waitableCredit988.load()});
  };
  auto closeRequested988=[&](const OriginalTimeline<std::shared_ptr<GpuFrames>>::Entry*claim=nullptr,bool forceFailureStop988=false){ // display thread only, queueMutex held
   if(!lifecycleDComp988||(!captureActive988.load()&&!forceFailureStop988))return false;
   if(timeline.lifecycleClosed980()&&timeline.lifecycleReason980()==SmxLifecycleReason980::Stop)return true;
   auto uiEpoch=overlayEpoch.load();bool stop=stopFlag.load()||forceFailureStop988;bool request=stop||(sourceOverlay&&(capturePaused989()||uiEpoch!=lifecycleUiEpoch988));
   if(request&&(!timeline.lifecycleClosed980()||uiEpoch!=lifecycleUiEpoch988||(stop&&timeline.lifecycleReason980()!=SmxLifecycleReason980::Stop))){
    lifecycleUiEpoch988=uiEpoch;++lifecycleSerial988;lifecycleCaptureGate988=true;
    timeline.closeLifecycle980(lifecycleSerial988,stop?SmxLifecycleReason980::Stop:SmxLifecycleReason980::Pause,claim);
    recordLifecycle988("closed",stop?SmxLifecycleReason980::Stop:SmxLifecycleReason980::Pause);SetEvent(event);
   }
   return timeline.lifecycleClosed980();
  };
  std::thread display([&]{
  std::optional<OriginalTimeline<std::shared_ptr<GpuFrames>>::Entry> frame;
  smx988::DisplayStage988 displayStage988=smx988::DisplayStage988::Empty;
  auto failureClose988=[&]{
   stopFlag=true;SetEvent(presentationChanged.value);SetEvent(event);
   if(!lifecycleDComp988)return; // Other backends retain their existing failure boundary.
   auto action=smx988::failureClose988(displayStage988,frame.has_value());
   smx_log_fprintf989(stderr,"DISPLAY_FAILURE988 stage=%s has_entry=%u action=%s qpc=%.9f\n",smx988::displayStageName988(displayStage988),unsigned(frame.has_value()),action==smx988::FailureClose988::FailStop?"fail_stop":action==smx988::FailureClose988::CancelClaim?"stop_close_claim":"stop_close_no_claim",timelineNow());fflush(stderr);
   if(action==smx988::FailureClose988::FailStop)smxBridgeFailure555("Display failed during incomplete GPU ownership or unacknowledged Present; preserving in-flight resources",ERROR_INVALID_STATE);
   try{
    std::lock_guard lock(queueMutex);
    closeRequested988(action==smx988::FailureClose988::CancelClaim?&*frame:nullptr,true);
    // Stop is already established. An ACK with failed ID/metadata is an error,
    // never a successful natural drain or a fabricated submitted/observed ID.
    const bool metadataPendingAtFailure988=presentMetadataPending988.exchange(false);
    if(metadataPendingAtFailure988){smx_log_fprintf989(stderr,"DISPLAY_FAILURE988 metadata_incomplete=1 stop_closed=1 last_verified_submitted_id=%u\n",lastSubmitted988);fflush(stderr);}
    frame.reset();displayStage988=smx988::DisplayStage988::Empty;
   }catch(...){smxBridgeFailure555("Display failure Stop-close could not establish safe lifecycle retirement",ERROR_INVALID_STATE);}
  };
  try{
   BridgeHandle presentationTimer;presentationTimer.value=CreateWaitableTimerExW(nullptr,nullptr,2,TIMER_ALL_ACCESS);if(!presentationTimer.value)throw std::runtime_error("Presentation high resolution timer failed");
   struct SubmittedFrame{UINT id;bool generated;};std::deque<SubmittedFrame> submittedFrames;UINT lastObservedId=0;
   FrameScheduling displayScheduling(false);const double prepareLead835=(scheduledPresenter871||wsi958)?.032:displayPrepareMs988*.001;smx_log_fprintf989(stderr,"DISPLAY_PREPARE971 lead_ms=%.3f timeline_base_delay_ms=%.3f max_content_to_due_ms=%.3f phase_lock=%u early_copy_only=1\n",prepareLead835*1000,delayMs,delayMs+(phaseLock971?8:0),unsigned(phaseLock971));double firstSource=0;bool primed=false;int lastW=0,lastH=0;
   FILE*trace=nullptr;smx_log_fopen_s989(&trace,"app-present.csv","wb");struct CloseTrace{FILE*f;~CloseTrace(){if(f)smx_log_fclose989(f);}}closeTrace{trace};
   if(trace)smx_log_fprintf989(trace,"kind,source_time,present_call_ms,target_ms,late_ms,stats_hr,displayed_present_count,present_refresh_count,sync_refresh_count,sync_qpc,ready_wait_ms,copy_submit_ms,present_ms,source_latency_at_call_ms,submitted_present_id,media_hr,media_mode,approved_duration_100ns,swapchain_wait_ms,texture_mutex_wait_ms,present_id_query_ms,frame_stats_query_ms,media_stats_query_ms,prepared_wait_ms,content_to_due_ms,phase_offset_ms,present_begin_qpc,present_return_qpc,ack_lock_acquired_qpc,ack_complete_qpc,metadata_complete_qpc,log_begin_qpc,present_api_ms,present_return_to_ack_ms,ack_to_metadata_ms,waitable_begin_qpc,waitable_ready_qpc,display_access_ready_qpc,copy_flush_qpc,presentation_due_qpc\n");
   for(;;){
    frame.reset();displayStage988=smx988::DisplayStage988::Empty;
    {std::unique_lock lock(queueMutex);while(true){
      if(closeRequested988()){
        if(displayDone)break;
        lock.unlock();WaitForSingleObject(presentationChanged.value,20);lock.lock();continue;
       }
       if(stopFlag&&displayDone){timeline.discardPending();if(!drainStart700)drainStart700=timelineNow();if(timelineNow()-drainStart700>.2)throw std::runtime_error("Completed output drain exceeded 200 ms");}
      if(scheduledPresenter871)scheduledPresenter871->poll();if(wsi958)bridge.pollWSI968();double now=timelineNow();timeline.recoverQueuedOriginals988(now);frame=timeline.takeDue(now+prepareLead835);if(frame){displayStage988=smx988::DisplayStage988::Claimed;if(claimEvents988.size()<72000)claimEvents988.push_back({timelineNow(),frame->stamp,frame->due,frame->ticket,frame->generated});else ++claimTruncated988;break;}if(displayDone&&timeline.empty())break;
      auto due=timeline.nextDue();auto wait=due?std::chrono::duration<double>(std::clamp(*due-prepareLead835-now,0.0001,0.05)):std::chrono::duration<double>(0.05);
      DWORD wake;
      if(due){
       LARGE_INTEGER timerDue{};timerDue.QuadPart=-std::max<LONGLONG>(1,LONGLONG(wait.count()*10000000));
       if(!SetWaitableTimer(presentationTimer.value,&timerDue,0,nullptr,nullptr,FALSE))throw std::runtime_error("Presentation timer arm failed");
       HANDLE handles[]={presentationChanged.value,presentationTimer.value};
       lock.unlock();wake=WaitForMultipleObjects(2,handles,FALSE,50);lock.lock();
      }else{
       lock.unlock();wake=WaitForSingleObject(presentationChanged.value,50);lock.lock();
      }
      if(wake==WAIT_FAILED)throw std::runtime_error("Presentation deadline wait failed");
     }}
    if(!frame)break;
    if(!primed){firstSource=frame->stamp;primed=true;}
    RECT client{};GetClientRect(output,&client);
    
    if(visual&&(client.right!=lastW||client.bottom!=lastH)){smx_log_fprintf989(stderr,"NATIVE_OUTPUT715 client=%ldx%ld backbuffer=%ux%u\n",client.right,client.bottom,width,height);float fit=std::min(float(client.right)/width,float(client.bottom)/height);D2D_MATRIX_3X2_F transform{fit,0,0,fit,(client.right-width*fit)*0.5f,(client.bottom-height*fit)*0.5f};if(sourceOverlay&&!outputPreview993())transform={float(client.right)/width,0,0,float(client.bottom)/height,0,0};
     if(outputPreview993()){auto content=smx993::fitContent({0,0,client.right,client.bottom},sourceWidth992,sourceHeight992);transform={float(content.width())/width,0,0,float(content.height())/height,float(content.left),float(content.top)};outputBackground993->resize(unsigned(client.right),unsigned(client.bottom));}
     smxdiag994::checkedHr((SMX_DIAG_CALL994(SMX_DIAG_GEOMETRY994,smxdiag994::fields(),SMX_DIAG_HRESULT994,visual->SetTransform(transform))),__LINE__);smxdiag994::checkedHr((SMX_DIAG_CALL994(SMX_DIAG_GEOMETRY994,smxdiag994::fields(),SMX_DIAG_HRESULT994,composition->Commit())),__LINE__);lastW=client.right;lastH=client.bottom;}
    SmxDiagFields994 displayFields994=smxdiag994::fields(0,frame->ticket,(uintptr_t)chain.get(),0,frame->stamp,frame->generated?1:0);
    double readyStart=timelineNow();
    double swapchainReady651=readyStart,readyEnd=readyStart,copyEnd=readyStart;double copyReturn988=0,keyRelease988=0,signalReturn988=0,markCopyReturn988=0,flushReturn988=0;bool hadKeyed988=false,hadNative988=false;
    ComPtr<ID3D11Fence> probeFence988;uint64_t probeFenceValue988=0,probeSequence988=0;
    if(wsi958){displayStage988=smx988::DisplayStage988::AccessStarted;bridge.prepareWSI958((frame->generated||cloneOriginal729)?frame->payload->middle.Get():frame->payload->first.Get(),frame->payload,frame->generated);swapchainReady651=readyEnd=copyEnd=timelineNow();}
    else{
    if(scheduledPresenter871){back=scheduledPresenter871->acquire(stopFlag);}
    else {DWORD ready=waitableCredit988?WAIT_OBJECT_0:WAIT_TIMEOUT;
      while(ready==WAIT_TIMEOUT){ready=SMX_DIAG_CALL994(SMX_DIAG_DISPLAY_CREDIT994,displayFields994,SMX_DIAG_WIN32994,WaitForSingleObject(outputReady.value,50));if(ready==WAIT_FAILED)throw std::runtime_error("Display waitable failed; lifecycle cannot hide this error");if(ready==WAIT_OBJECT_0)waitableCredit988=true;
       {std::lock_guard lock(queueMutex);if(closeRequested988(&*frame)){frame.reset();break;}}
       if(timelineNow()-readyStart>1.0)throw std::runtime_error("Display queue stalled for one second");
      }
      if(!frame)continue;
      if(ready!=WAIT_OBJECT_0)throw std::runtime_error("Display queue wait failed");
      {std::lock_guard lock(queueMutex);
       if(closeRequested988(&*frame))frame.reset();
       else if(!lifecycleCaptureGate988.load()&&claimedOriginalRecovery988&&!frame->generated&&timeline.recoverClaimedOriginalAfterCredit988(*frame,timelineNow(),prepareLead835,true,false)){
        // Only an unaccessed original claim is discarded. No ACK, copy, texture lookup,
        // or ownership command has occurred. Keep the acquired swapchain credit for
        // the ready generated successor in the next normal iteration.
        frame.reset();
       }
      }if(!frame)continue;
     }
    displayStage988=smx988::DisplayStage988::AccessStarted;swapchainReady651=timelineNow();ComPtr<IDXGIKeyedMutex> originalMutex;std::unique_ptr<BridgeLock> originalLock;auto*displayTexture=bridge.displayTexture700((frame->generated||cloneOriginal729)?frame->payload->middle.Get():frame->payload->first.Get());auto display948=SMX_DIAG_OPERATION994(SMX_DIAG_DISPLAY_ACCESS994,displayFields994,bridge.access948(displayTexture,displayContext700.get()));if(!display948&&SUCCEEDED(displayTexture->QueryInterface(IID_PPV_ARGS(&originalMutex))))originalLock=std::make_unique<BridgeLock>(originalMutex.Get());
    readyEnd=timelineNow();displayFields994.resource=(uintptr_t)displayTexture;SMX_DIAG_COMMAND994(SMX_DIAG_DISPLAY_COPY994,displayFields994,displayContext700->CopyResource(back.get(),displayTexture));copyReturn988=timelineNow();hadKeyed988=bool(originalLock);hadNative988=bool(display948);
    // Include ownership release in the same explicit flush. Query retirement follows
    // both the copy and release; every keyed-mutex pair remains intact.
    if(originalLock){auto*m932=originalLock->mutex;originalLock->mutex=nullptr;originalLock.reset();smxdiag994::checkedHr((m932->ReleaseSync(0)),__LINE__);}keyRelease988=timelineNow();
    if(display948){display948->signal();if(displayFenceProbe988){
     // Immutable snapshot while the existing NativeD3DAccess resource lock is held.
     probeFence988=display948->fence;probeFenceValue988=display948->state->value;
    }}signalReturn988=timelineNow();
    if(scheduledPresenter871)scheduledPresenter871->markCopy(frame->payload);markCopyReturn988=timelineNow();
    SMX_DIAG_COMMAND994(SMX_DIAG_DISPLAY_FLUSH994,displayFields994,displayContext700->Flush());flushReturn988=timelineNow();display948.reset();copyEnd=timelineNow();displayStage988=smx988::DisplayStage988::CopyReleased;
    if(displayFenceProbe988){smx988::FenceRecord988 r;r.generated=frame->generated;r.source=frame->stamp;r.due=frame->due;r.copyBegin=readyEnd;r.submit=keyRelease988;r.signalReturn=signalReturn988;r.flushReturn=flushReturn988;r.fenceValue=probeFenceValue988;probeSequence988=displayFenceProbe988->enqueue(std::move(probeFence988),r);}
    }
    // Every acquired NativeD3DAccess has signalled and Flush completed above. A cancelled
    // claim may now release its CPU lease; the per-resource GPU fence still protects reuse.
    {std::lock_guard lock(queueMutex);if(closeRequested988(&*frame))frame.reset();}if(!frame)continue;
    // Prepare copy/ownership before the original deadline; do not move presentation earlier or extend latency.
    double presentTarget835=frame->due;
    while(!scheduledPresenter871&&timelineNow()<presentTarget835){
     const double armQpc988=timelineNow();LARGE_INTEGER due835{};due835.QuadPart=-std::max<LONGLONG>(1,LONGLONG((presentTarget835-armQpc988)*10000000));
     if(!SetWaitableTimer(presentationTimer.value,&due835,0,nullptr,nullptr,FALSE)){DWORD timerError988=GetLastError();smx_log_fprintf989(stderr,"PREPARED_WAIT988 operation=arm result=0 error=%lu arm_qpc=%.9f due_qpc=%.9f relative_100ns=%lld timeout_ms=100\n",timerError988,armQpc988,presentTarget835,(long long)due835.QuadPart);fflush(stderr);throw std::runtime_error("Prepared present timer arm failed error="+std::to_string(timerError988));}
     const double waitBegin988=timelineNow();DWORD waitResult988=SMX_DIAG_CALL994(SMX_DIAG_DISPLAY_CREDIT994,displayFields994,SMX_DIAG_WIN32994,WaitForSingleObject(presentationTimer.value,100));DWORD waitError988=waitResult988==WAIT_FAILED?GetLastError():ERROR_SUCCESS;const double wakeQpc988=timelineNow();
     auto decision988=smx988::classifyPreparedWait988(waitResult988,wakeQpc988,presentTarget835);
     if(waitResult988!=WAIT_OBJECT_0){smx_log_fprintf989(stderr,"PREPARED_WAIT988 operation=wait result=%lu error=%lu arm_qpc=%.9f wait_begin_qpc=%.9f wake_qpc=%.9f due_qpc=%.9f relative_100ns=%lld timeout_ms=100 elapsed_ms=%.6f action=%s\n",waitResult988,waitError988,armQpc988,waitBegin988,wakeQpc988,presentTarget835,(long long)due835.QuadPart,(wakeQpc988-waitBegin988)*1000,decision988==smx988::PreparedWait988::TimeoutAfterDue?"same_reserved_present_late":"failure");fflush(stderr);}
     if(decision988==smx988::PreparedWait988::Failure)throw std::runtime_error("Prepared present deadline wait failed rc="+std::to_string(waitResult988)+" error="+std::to_string(waitError988));
     if(decision988==smx988::PreparedWait988::TimeoutAfterDue)break;
    }
    {std::lock_guard lock(queueMutex);if(closeRequested988(&*frame))frame.reset();}if(!frame)continue;
    displayStage988=smx988::DisplayStage988::PresentUnacknowledged;presentMetadataPending988=true;UINT scheduledId871=0;double presentBegin835=timelineNow();
    if(scheduledPresenter871)scheduledId871=scheduledPresenter871->submit(frame->stamp,presentTarget835,frame->generated);
    else if(wsi958)scheduledId871=bridge.presentWSI958();else {HRESULT presentHr988=SMX_DIAG_CALL994(SMX_DIAG_PRESENT994,displayFields994,SMX_DIAG_HRESULT994,chain->Present(presentSync,ownPresentFlags));waitableCredit988=false;smxdiag994::checkedHr((presentHr988),__LINE__);}
     const double presentReturn988=timelineNow();double ackLock988=0;
     {std::lock_guard lock(queueMutex);ackLock988=timelineNow();timeline.acknowledge(*frame,ackLock988);displayStage988=smx988::DisplayStage988::Acknowledged;timeline.notePresentationRecovery988(ackLock988,frame->due,swapchainReady651-readyStart);hudSession988->metrics.setLateCounts(timeline.counts.lateCompleted,timeline.counts.lateSubmitted);}const double ackComplete988=timelineNow();SetEvent(presentationChanged.value);presented++;if(frame->generated)generatedPresented++;else originalPresented++;
    double idQueryStart720=timelineNow();UINT submittedId=scheduledId871;if(chain)smxdiag994::checkedHr((SMX_DIAG_CALL994(SMX_DIAG_DISPLAY_STATS994,displayFields994,SMX_DIAG_HRESULT994,chain->GetLastPresentCount(&submittedId))),__LINE__);if(displayFenceProbe988)displayFenceProbe988->bind(probeSequence988,submittedId);hudSession988->recordSubmitted(submittedId,frame->generated,frame->stamp,frame->due,presentBegin835);displayFields994.frame=submittedId;smxdiag994::instant(SMX_DIAG_PRESENT994,displayFields994);
     {std::lock_guard lock(queueMutex);lastSubmitted988=submittedId;if(freshSubmitPending988){recordLifecycle988("first_fresh_present_submitted",SmxLifecycleReason980::Pause);freshSubmitPending988=false;}presentMetadataPending988=false;}
     if(!wsi958){if(copyStages988.size()<72000)copyStages988.push_back({submittedId,frame->generated,hadKeyed988,hadNative988,frame->stamp,readyEnd,copyReturn988,keyRelease988,signalReturn988,markCopyReturn988,flushReturn988,copyEnd});else ++copyStagesTruncated988;}
      submittedFrames.push_back({submittedId,frame->generated});while(submittedFrames.size()>512)submittedFrames.pop_front();
    double idQueryEnd720=timelineNow();double after=timelineNow(),due=frame->due,late=(after-due)*1000;if(!frame->generated){if(late<=8.333334)stableOriginal775++;else stableOriginal775=0;}
    if(late>8.333334){latePresents++;if(frame->generated){latenessFailure700=true;performanceWarning775=true;if(interactive775){recoveryUntil775=0;status(L"Presentation was late; retrying with the next fresh input. Completed frames remain ordered. Timing test has not passed.");}else if(!naturalEnd988.requested())stopFlag=true;}}
    double statsStart720=timelineNow();DXGI_FRAME_STATISTICS stats{};HRESULT statsHr=E_NOINTERFACE;hudSession988->getLatest(stats,statsHr);
    if(scheduledPresenter871){scheduledPresenter871->poll();observedGenerated=scheduledPresenter871->confirmedGenerated();observedOriginal=scheduledPresenter871->confirmedOriginal();}
    if(statsHr==S_OK&&stats.PresentCount>lastObservedId){
     auto found=std::find_if(submittedFrames.begin(),submittedFrames.end(),[&](const SubmittedFrame&e){return e.id==stats.PresentCount;});
     if(found!=submittedFrames.end()){if(found->generated)observedGenerated++;else observedOriginal++;}else unknownObserved++;
     lastObservedId=stats.PresentCount;
     while(!submittedFrames.empty()&&submittedFrames.front().id<=lastObservedId)submittedFrames.pop_front();
    }

    double frameStatsEnd720=timelineNow();DXGI_FRAME_STATISTICS_MEDIA mediaStats{};HRESULT mediaStatsHr=media?media->GetFrameStatisticsMedia(&mediaStats):E_NOINTERFACE;
    double mediaStatsEnd720=timelineNow();const double presentLogBegin988=timelineNow();if(trace&&presented.load()<=72000)smx_log_fprintf989(trace,"%u,%.9f,%.6f,%.6f,%.6f,%ld,%u,%u,%u,%lld,%.6f,%.6f,%.6f,%.6f,%u,%ld,%u,%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.6f,%.6f,%.6f,%.9f,%.9f,%.9f,%.9f,%.9f\n",unsigned(frame->generated),frame->stamp,(after-firstSource)*1000,(due-firstSource)*1000,late,long(statsHr),stats.PresentCount,stats.PresentRefreshCount,stats.SyncRefreshCount,stats.SyncQPCTime.QuadPart,(readyEnd-readyStart)*1000,(copyEnd-readyEnd)*1000,(after-presentBegin835)*1000,(after-frame->stamp)*1000,submittedId,long(mediaStatsHr),unsigned(mediaStats.CompositionMode),mediaStats.ApprovedPresentDuration,(swapchainReady651-readyStart)*1000,(readyEnd-swapchainReady651)*1000,(idQueryEnd720-idQueryStart720)*1000,(frameStatsEnd720-statsStart720)*1000,(mediaStatsEnd720-frameStatsEnd720)*1000,(presentBegin835-copyEnd)*1000,(frame->due-frame->stamp)*1000,(frame->due-frame->stamp)*1000-delayMs,presentBegin835,presentReturn988,ackLock988,ackComplete988,idQueryEnd720,presentLogBegin988,(presentReturn988-presentBegin835)*1000,(ackComplete988-presentReturn988)*1000,(idQueryEnd720-ackComplete988)*1000,readyStart,swapchainReady651,readyEnd,copyEnd,frame->due);
   }
   if(wsi958)bridge.drainWSI958();if(scheduledPresenter871){scheduledPresenter871->drain(150);observedGenerated=scheduledPresenter871->confirmedGenerated();observedOriginal=scheduledPresenter871->confirmedOriginal();}
  }catch(const winrt::hresult_error&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,int32_t(e.code()),SMX_DIAG_HRESULT994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));displayError="Display HRESULT "+std::to_string(unsigned(e.code()))+": "+winrt::to_string(e.message());smx_log_fprintf989(stderr,"DISPLAY871_ERROR %s\n",displayError.c_str());fflush(stderr);failureClose988();}catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));displayError=e.what();smx_log_fprintf989(stderr,"DISPLAY871_ERROR %s\n",displayError.c_str());fflush(stderr);failureClose988();}catch(...){displayError="Independent presentation device error";smx_log_fprintf989(stderr,"DISPLAY871_ERROR unknown\n");fflush(stderr);failureClose988();}
 });
 smx_log_fprintf989(stderr,"DISPLAY_HANDOFF932 copy_release_retirement_flush=1 keyed_mutex_retained=1\n");smx_log_fprintf989(stderr,"Start capture\n");struct DisplayJoin {std::atomic<bool>&done;std::condition_variable&changed;std::thread&thread;~DisplayJoin(){stopFlag=true;done=true;changed.notify_all();if(thread.joinable())thread.join();}} displayJoin{displayDone,queueChanged,display};
  if(sourceOverlay){
   overlayReady=false;overlayPaused=true;PostMessageW(control,WM_APP+4,benchmarkSource?0:1,0);
   setPreparationStage992(PreparationStage992::WaitForeground);
   preparationMarker862("waiting_source_foreground");
   status(L"模型已准备完成，正在等待游戏回到前台。此时不会生成游戏帧；Ctrl+Alt+Esc 可停止。");
   const bool gate710=benchmarkSource&&getenv("SMX_PREPARE_GATE")&&strcmp(getenv("SMX_PREPARE_GATE"),"1")==0;
   const double foregroundTimeout862=smx992::foregroundTimeout(hostConfig988.hosted,getenv("SMX_PREPARE_SECONDS862"));
   smxdiag994::Scope foreground994(SMX_DIAG_FOREGROUND_WAIT994,smxdiag994::fields(0,0,(uintptr_t)selected));const double waitBegin992=timelineNow();smx992::WaitResult result992=smx992::WaitResult::Waiting;
   std::string go710;
   if(gate710){
    const char*bd=getenv("SMX_BENCH_DIR");if(!bd)throw std::runtime_error("Preparation gate requires own log folder");
    std::string ready710=std::string(bd)+"\\app-ready710.json";go710=std::string(bd)+"\\START_CAPTURE";
    FILE*r710=smx_log_fopen989(ready710.c_str(),"wb");if(!r710)throw std::runtime_error("Cannot write preparation marker");smx_log_fprintf989(r710,"{\"ready\":true,\"capture_started\":false,\"selected_hwnd\":%lld}\n",(long long)(INT_PTR)selected);smx_log_fclose989(r710);
   }
   smx_log_fprintf989(stderr,"PREPARATION992 foreground_wait hosted=%u timeout_seconds=%.3f indefinite=%u hosted_no_foreground_activation=%u\n",unsigned(hostConfig988.hosted),gate710?60:foregroundTimeout862,unsigned(!gate710&&foregroundTimeout862==0),unsigned(hostConfig988.hosted));
   for(;;){
    const bool alive992=targetIdentity990();if(!alive992)targetClosedEvent990();
    const bool keepCapture992=benchmarkSource&&getenv("SMX_BENCH_KEEP_CAPTURE")&&strcmp(getenv("SMX_BENCH_KEEP_CAPTURE"),"1")==0;
    const bool foregroundReady992=overlayReady.load()&&!capturePaused989()&&(keepCapture992||targetForeground990());
    const bool ready992=(!gate710||GetFileAttributesA(go710.c_str())!=INVALID_FILE_ATTRIBUTES)&&foregroundReady992;
    result992=smx992::poll(timelineNow()-waitBegin992,gate710?60:foregroundTimeout862,stopFlag.load(),alive992,ready992);
    if(result992!=smx992::WaitResult::Waiting)break;
    Sleep(25); // UI/pipe thread remains responsive; no capture or model submission is active.
   }
   foregroundWaitSeconds992=timelineNow()-waitBegin992;foreground994.end(SMX_DIAG_SEMANTIC994,int64_t(result992));
   if(result992!=smx992::WaitResult::Ready){
    const bool cancelled992=result992==smx992::WaitResult::Cancelled;
    preparationMarker862(cancelled992?(targetClosed990?"target_closed_before_capture":"stopped_before_capture"):"foreground_wait_expired");
    // No capture was started. Join the idle display worker before classifying this cancellation;
    // an actual display initialization error must not be hidden by Stop/target closure.
    stopFlag=true;displayDone=true;SetEvent(presentationChanged.value);SetEvent(event);queueChanged.notify_all();if(display.joinable())display.join();
    if(!displayError.empty())throw std::runtime_error(displayError);
    if(!cancelled992)throw std::runtime_error(gate710?"Preparation gate/foreground diagnostic deadline expired; capture was not attempted":"Configured foreground-wait diagnostic deadline expired; capture was not attempted");
    const char*reason992=targetClosed990?"target_closed":"stop_requested";
    if(FILE*f=smx_log_fopen989("app-preparation-stop992.json","wb")){smx_log_fprintf989(f,"{\"reason\":\"%s\",\"capture_started\":false,\"foreground_wait_seconds\":%.6f,\"normal_cancellation\":true,\"model_initialization_may_have_used_gpu\":true}\n",reason992,foregroundWaitSeconds992.load());smx_log_fclose989(f);}
    status(targetClosed990?L"游戏已关闭，已安全取消启动。":L"已停止等待游戏前台。");return 0;
   }
  }
 {std::lock_guard lock(queueMutex);lifecycleUiEpoch988=overlayEpoch.load();seenOverlayEpoch=lifecycleUiEpoch988;captureActive988=true;}
  if(desktopSource989)desktopSource989->start([&](CaptureFrame949 frame){processCapture989([&](){return std::exchange(frame,CaptureFrame949{});});},[&,inbox](HRESULT hr,const char*message){std::lock_guard lock(inbox->mutex);inbox->error=hr;inbox->errorText=message;SetEvent(event);},[&](bool paused){if(captureBackendPaused989.exchange(paused)!=paused){++overlayEpoch;SetEvent(event);SetEvent(presentationChanged.value);}},[&](){targetClosedEvent990();SetEvent(event);SetEvent(presentationChanged.value);});else SMX_DIAG_COMMAND994(SMX_DIAG_CAPTURE_ACQUIRE994,smxdiag994::fields(0,0,(uintptr_t)selected,0,0,3),session.StartCapture());processingState992.set(processingExtent992);preparationMarker862("capture_started",true);setPreparationStage992(PreparationStage992::Capturing);status(L"Capturing. F11: fullscreen; Esc: stop. Original weights / Intel XMX. Selected processing size; output follows game client. Independent originals; 80 ms presentation buffer by default. Every inference reserves an ordered presentation slot; completed outputs are acknowledged after successful Present.");
 FILE*captureTrace=nullptr;if(true)smx_log_fopen_s989(&captureTrace,"app-capture.csv","wb");
 struct CloseCaptureTrace{FILE*f;~CloseCaptureTrace(){if(f)smx_log_fclose989(f);}}closeCaptureTrace{captureTrace};
 if(captureTrace)smx_log_fprintf989(captureTrace,"pair,source_interval_ms,source_age_ms,gpu_ms,worker_ms,qpc_seconds,input_lock_ms,input_submit_ms,vk_submit_ms,vk_fence_wait_ms,query_ms,output_lock_ms,output_copy_submit_ms\n");
 LARGE_INTEGER qpcFreq{};QueryPerformanceFrequency(&qpcFreq);
 bool adaptive=!getenv("SMX_DISABLE_ADMISSION");
 SmxFrameBudget budget;smx_budget_init(&budget,getenv("SMX_MODEL_DUTY")?atof(getenv("SMX_MODEL_DUTY")):.65);
 FILE*budgetTrace=nullptr;smx_log_fopen_s989(&budgetTrace,"app-budget.csv","wb");
 struct CloseBudget{FILE*f;~CloseBudget(){if(f)smx_log_fclose989(f);}}closeBudget{budgetTrace};
 if(budgetTrace)smx_log_fprintf989(budgetTrace,"qpc_seconds,decision,available_ms,predicted_ms,duty,source_interval_ms,source_age_ms,worker_ms,on_time\n");
 double budgetEpoch=timelineNow()*1000;unsigned lastSequence=0;int primedAdmission=0;
 
 if(outputGuard||resourceEnabled)adaptive=false;
 std::unique_ptr<SmxSensors> sensors;std::unique_ptr<SmxResourceBudget> resource;
 if(resourceEnabled){sensors=std::make_unique<SmxSensors>();resource=std::make_unique<SmxResourceBudget>(timelineNow(),sensors->limitW);}
 SmxOutputBalance outputBalance;smx_output_init(&outputBalance,timelineNow()*1000);
 unsigned outputSkips=0,expiredSkips=0;DeadlineGuard930 deadlineGuard664;bool enableDeadline664=true;bool autoStopped695=false;
 FILE*outputTrace=nullptr;if(outputGuard)smx_log_fopen_s989(&outputTrace,"app-output-balance.csv","wb");
 struct CloseOutputTrace{FILE*f;~CloseOutputTrace(){if(f)smx_log_fclose989(f);}}closeOutputTrace{outputTrace};
 if(outputTrace)smx_log_fprintf989(outputTrace,"qpc_seconds,state,reference_observed_output_hz,observed_original_hz,observed_generated_hz,observed_original_ids,observed_generated_ids,executions\n");

 std::shared_ptr<GpuFrames> cachedOriginal;double cachedStamp=0;unsigned cachedSequence=0,bridgeSecondSequence=0,pairUploads=0,seenGeometry795=0,testAwaitGeometry795=0;
 double passthroughSeconds=0,lastPump=timelineNow(),captureStarted=lastPump;
 double heartbeat775=0,budgetPauseStart775=0;unsigned readyPolls809=0,readyWaits809=0;bool waitingBudget809=false;
 const bool readyGate809=!(getenv("SMX_READY_INPUT809")&&strcmp(getenv("SMX_READY_INPUT809"),"0")==0);
 auto inputReady809=[&](const std::shared_ptr<GpuFrames>&p){
    if(!readyGate809)return true;
    if(!p||!p->readyValue912||!p->readyEvent912.value)throw std::runtime_error("Capture-ready fence missing");
    if(p->readyObserved809)return true;
    ++readyPolls809;UINT64 completed=captureFence912->GetCompletedValue();
    if(completed==UINT64_MAX)throw std::runtime_error("Capture fence device removed; stop");
    if(completed>=p->readyValue912){p->readyObserved809=true;p->readyStamp809=timelineNow();}
     // R949 input availability is its enqueued per-resource GPU timeline signal.
     // No fabricated completion timestamp: observer logs CPU fence completion separately.
     return true;
 };
 FILE*readyEventTrace912=smx_log_fopen989("app-ready912.csv","wb");struct CloseReady912{FILE*f;~CloseReady912(){if(f)smx_log_fclose989(f);}}closeReady912{readyEventTrace912};
 if(readyEventTrace912)smx_log_fprintf989(readyEventTrace912,"qpc,sequence,fence_value,wait_ms,wake,new_capture_or_ready_or_timeout\n");
 smx_log_fprintf989(stderr,"CAPTURE_READY912 monotonic_fence=1 per_slot_manual_event=16 timer_poll_removed=1 per_resource_gpu_wait948=1 cpu_ready_gate_removed949=1 wgc_lifetime_until_copy_fence=1\n");
 FILE*readyTrace809=smx_log_fopen989("app-ready809.csv","wb");struct CloseReady809{FILE*f;~CloseReady809(){if(f)smx_log_fclose989(f);}}closeReady809{readyTrace809};
 if(readyTrace809)smx_log_fprintf989(readyTrace809,"pair,available_ms,admit_estimate_ms,input_ready_minus_source_ms_signed,input_ready_to_admit_ms_signed,source_age_at_admit_ms,worker_ms,model_gpu_ms,revalidation,geometry_epoch\n");
 auto writeLive775=[&](const char*state){double t=timelineNow();if(t-heartbeat775<.5)return;heartbeat775=t;size_t outstanding988=0;const auto c775=[&](){std::lock_guard lock(queueMutex);outstanding988=timeline.completedOutstanding();return timeline.counts;}();FILE*lf=smx_log_fopen989("app-live775.json","wb");if(lf){smx_log_fprintf989(lf,R"({"qpc_seconds":%.9f,"state":"%s","captured":%u,"inference_returned":%u,"completed":%llu,"submitted_generated":%llu,"outstanding_generated":%llu,"late_completed":%llu,"late_submitted":%llu,"startup_source_frames_without_inference":%u,"recovery_source_frames_without_inference":%u,"deadline_rejections":%u,"startup_elapsed_seconds":%.6f,"overlay_paused":%s,"scope":"Live accounting, not performance acceptance; warmup and skipped work never count as generated frames."})",t,state,capturedOriginal.load(),inferReturned700,(unsigned long long)c775.completed,(unsigned long long)c775.submitted,(unsigned long long)outstanding988,(unsigned long long)c775.lateCompleted,(unsigned long long)c775.lateSubmitted,startupSkipped775,recoverySkipped775,deadlineGuard664.skips,startupComplete775,capturePaused989()?"true":"false");smx_log_fclose989(lf);}};
 CaptureFrame949 heldFirst{nullptr};double lastTime=0,gpuSum=0,captureSum=0,conversionSum=0,runWallSum=0;unsigned captureCount=0,poolWaitCount965=0,poolSkipCount965=0;double poolWaitMs965=0;auto started=std::chrono::steady_clock::now(),lastStatus=started;bool failed=false;std::wstring errorMessage;

 const bool async905=getenv("SMX_PIPELINE905")!=nullptr;if(async905)bridge.setWake906(event);
 if(async905&&(!smxDirectEnabled()||!smxDirectOutputEnabled()||audit724||getenv("SMX_LIVE_PROOF")))throw std::runtime_error("Async path requires direct retained textures without readback");
 struct Pending905{unsigned slot;std::function<void()> complete;double predictedDone;};
 FILE*jobInputs933=smx_log_fopen989("app-job-inputs933.csv","wb");struct CloseInputs933{FILE*f;~CloseInputs933(){if(f)smx_log_fclose989(f);}}closeInputs933{jobInputs933};if(jobInputs933)smx_log_fprintf989(jobInputs933,"job,slot,first_ready_fence,second_ready_fence,first_source_qpc,second_source_qpc,admit_qpc,after_submit_qpc,capture_completed_at_submit\n");unsigned jobInputId933=0;
 std::deque<Pending905> pending905;unsigned nextSlot905=0,maxPending905=0;FILE*prediction930=smx_log_fopen989("app-prediction930.csv","wb");struct ClosePrediction930{FILE*f;~ClosePrediction930(){if(f)smx_log_fclose989(f);}}closePrediction930{prediction930};if(prediction930)smx_log_fprintf989(prediction930,"qpc,busy,ahead_ms,increment_ms,available_ms,admitted,idle_samples,warm_samples\n");
 auto drain905=[&](bool wait){while(!pending905.empty()&&(wait||bridge.ready905(pending905.front().slot))){auto p=std::move(pending905.front());pending905.pop_front();p.complete();hudSession988->metrics.setInflight(unsigned(pending905.size()));}};
  auto finishProbe989=[&](){if(lifecycleDComp988){std::lock_guard lock(queueMutex);lifecycleCaptureGate988=true;naturalEnd988.begin(timelineNow(),.250);}else stopFlag=true;SetEvent(presentationChanged.value);};
  try{
  while(!stopFlag){
   if(benchmarkSource&&std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count()>=(getenv("SMX_BENCH_SECONDS")?atof(getenv("SMX_BENCH_SECONDS")):60.0)){
    if(lifecycleDComp988){std::lock_guard lock(queueMutex);lifecycleCaptureGate988=true;naturalEnd988.begin(timelineNow(),.250);}else stopFlag=true;
    SetEvent(presentationChanged.value);break;
   }
   producer933.check();
   bool closed988=false;uint64_t closedEpoch988=0;SmxLifecycleReason980 closedReason988=SmxLifecycleReason980::Pause;
   {std::lock_guard lock(queueMutex);closed988=timeline.lifecycleClosed980();if(closed988){closedEpoch988=timeline.lifecycleEpoch980();closedReason988=timeline.lifecycleReason980();}}
   if(closed988){
    // Never retire an input/output lease before the model's original fence completes.
    if(async905)drain905(true);
    {std::lock_guard lock(inbox->mutex);for(auto&entry:inbox->frames){entry.frame.Close();inbox->dropped++;}inbox->frames.clear();}
    if(heldFirst){heldFirst.Close();heldFirst=nullptr;}cachedOriginal.reset();cachedStamp=0;cachedSequence=0;bridgeSecondSequence=0;lastTime=0;lastSequence=0;primedAdmission=0;
    deadlineGuard664.resetHistory();budgetPauseStart775=0;waitingBudget809=false;recoveryUntil775=0;stableOriginal775=0;startupFinished775=!startupWarmup775;
    {std::lock_guard lock(queueMutex);
     if(!stopFlag&&closedReason988==SmxLifecycleReason980::Pause&&!capturePaused989()&&overlayEpoch.load()==lifecycleUiEpoch988&&timeline.lifecycleClosed980()&&timeline.lifecycleEpoch980()==closedEpoch988){
      timeline.resetPacing971("lifecycle_resume");freshSourceAfter988=timelineNow();captureStarted=timelineNow();
      timeline.resumeLifecycle980(closedEpoch988);freshSubmitPending988=true;seenOverlayEpoch=lifecycleUiEpoch988;
      recordLifecycle988("resumed_internal_queue",SmxLifecycleReason980::Pause);lifecycleCaptureGate988=false;SetEvent(presentationChanged.value);
     }
    }
    writeLive775("lifecycle_pause");WaitForSingleObject(event,20);continue;
   }
   if(async905){drain905(false);if(selftest&&!windowTest795&&frameMultiplier989==2){if(generated.load()>=3){finishProbe989();break;}if(generated.load()+pending905.size()>=3){WaitForSingleObject(event,20);continue;}}if(pending905.size()>=2){WaitForSingleObject(event,50);continue;}}

   if(auto extent795=pendingExtent795.exchange(0)){
    recreating795=true;
    {std::lock_guard lock(inbox->mutex);for(auto&old795:inbox->frames){old795.frame.Close();inbox->dropped++;}inbox->frames.clear();}
    if(heldFirst){heldFirst.Close();heldFirst=nullptr;}cachedOriginal.reset();cachedSequence=0;bridgeSecondSequence=0;lastTime=0;
    winrt::Windows::Graphics::SizeInt32 nextSize795{int(extent795>>32),int(extent795&0xffffffffu)};
    if(!pool)throw std::runtime_error("DXGI monitor extent changed; restart capture explicitly");pool.Recreate(captureDevice,DirectXPixelFormat::B8G8R8A8UIntNormalized,6,nextSize795);poolExtent795=extent795;poolRecreates795++;recreating795=false;
    stableOriginal775=0;startupFinished775=!startupWarmup775;captureStarted=timelineNow();
    smx_log_fprintf989(stderr,"WINDOW795 frame_pool_recreated=%dx%d; model retained; no generated result discarded\n",nextSize795.Width,nextSize795.Height);
   }
   writeLive775(capturePaused989()?"source_background":!startupFinished775?"preparing_display":waitingBudget809?"waiting_for_budget":timelineNow()<recoveryUntil775.load()?"recovering_deadline":"running");
   
   bool outputAllowed=true;
   double pumpNow=timelineNow();if(overlayBypass)passthroughSeconds+=pumpNow-lastPump;lastPump=pumpNow;
   if((outputGuard||resourceEnabled)&&!(sourceOverlay&&capturePaused989())){
    auto before=outputBalance.state;
    bool baseline=before==SMX_BASELINE;
    outputAllowed=resourceEnabled?(sensors->poll(pumpNow),resource->update(pumpNow,capturedOriginal.load(),generated.load(),*sensors)):smx_output_admit(&outputBalance,pumpNow*1000,capturedOriginal.load(),baseline?capturedOriginal.load():observedOriginal.load(),baseline?0:observedGenerated.load())!=0;
    bool bypass=sourceOverlay&&!outputAllowed;
    if(overlayBypass.exchange(bypass)!=bypass){
     {std::lock_guard lock(queueMutex);timeline.discardPending();}
     {std::lock_guard lock(inbox->mutex);for(auto&f:inbox->frames){f.frame.Close();inbox->dropped++;}inbox->frames.clear();}
     cachedOriginal.reset();cachedSequence=0;bridgeSecondSequence=0;lastTime=0;
     if(heldFirst){heldFirst.Close();heldFirst=nullptr;}
     SetEvent(presentationChanged.value);PostMessageW(control,WM_APP+4,0,0);
    }
    if(outputTrace&&before!=outputBalance.state)smx_log_fprintf989(outputTrace,"%.9f,%u,%.6f,%.6f,%.6f,%u,%u,%u\n",pumpNow,unsigned(outputBalance.state),outputBalance.referenceHz,outputBalance.lastOriginalHz,outputBalance.lastGeneratedHz,observedOriginal.load(),observedGenerated.load(),generated.load());
    if(!outputAllowed){outputSkips++;WaitForSingleObject(event,50);continue;}
   }
   CaptureFrame949 latest{nullptr};std::shared_ptr<GpuFrames> latestOriginal;unsigned latestSequence=0,latestGeometry795=0;std::shared_ptr<GpuFrames> pairFirstOriginal;double pairFirstStamp=0;unsigned pairFirstSequence=0;
   bool pendingReady809=false;std::shared_ptr<GpuFrames>pendingFrame912;unsigned pendingSequence912=0;
   {std::lock_guard lock(inbox->mutex);if(FAILED(inbox->error)&&!inbox->errorText.empty())throw std::runtime_error(inbox->errorText);smxdiag994::checkedHr((inbox->error),__LINE__);
    int chosen=-1;for(int i=int(inbox->frames.size())-1;i>=0;--i){if(inputReady809(inbox->frames[i].original)){chosen=i;break;}pendingReady809=true;if(!pendingFrame912){pendingFrame912=inbox->frames[i].original;pendingSequence912=inbox->frames[i].sequence;}}
    if(chosen>=0){auto&entry=inbox->frames[size_t(chosen)];latest=std::move(entry.frame);latestOriginal=std::move(entry.original);latestSequence=entry.sequence;latestGeometry795=entry.geometryEpoch;
     if(chosen>0){auto&first=inbox->frames[size_t(chosen)-1];if(first.sequence+1==latestSequence&&first.geometryEpoch==latestGeometry795&&inputReady809(first.original)){pairFirstOriginal=first.original;pairFirstStamp=first.frame.SystemRelativeTime().count()*1e-7;pairFirstSequence=first.sequence;}}
     for(int i=0;i<=chosen;++i){if(inbox->frames.front().frame){inbox->frames.front().frame.Close();++inbox->dropped;}inbox->frames.pop_front();}
    }
   }
   if(latest&&latestGeometry795!=seenGeometry795){seenGeometry795=latestGeometry795;
    // A new capture geometry starts a fresh timing estimate; cumulative accounting is preserved.
    deadlineGuard664.resetHistory();budgetPauseStart775=0;waitingBudget809=false;recoveryUntil775=0;
    smx_log_fprintf989(stderr,"WINDOW795 deadline_history_reset epoch=%u; cumulative execution/late counters retained\n",seenGeometry795);cachedOriginal.reset();cachedSequence=0;bridgeSecondSequence=0;lastTime=0;if(heldFirst){heldFirst.Close();heldFirst=nullptr;}}
   if(sourceOverlay&&!lifecycleDComp988){
    auto epoch=overlayEpoch.load();
    if(epoch!=seenOverlayEpoch){{std::lock_guard lock(queueMutex);timeline.resetPacing971("foreground");}lastTime=0;seenOverlayEpoch=epoch;cachedOriginal.reset();cachedSequence=0;bridgeSecondSequence=0;if(outputGuard)smx_output_init(&outputBalance,timelineNow()*1000);if(heldFirst){heldFirst.Close();heldFirst=nullptr;}}
    if(capturePaused989()){if(latest)latest.Close();lastTime=0;WaitForSingleObject(event,100);continue;}
   }
   if(!latest){
    if(pendingReady809&&pendingFrame912){
      ++readyWaits809;HANDLE waitHandles912[2]={event,pendingFrame912->readyEvent912.value};double waitStart912=timelineNow();
      DWORD wake912=WaitForMultipleObjects(2,waitHandles912,FALSE,100);
      if(readyEventTrace912)smx_log_fprintf989(readyEventTrace912,"%.9f,%u,%llu,%.6f,%lu,%s\n",timelineNow(),pendingSequence912,(unsigned long long)pendingFrame912->readyValue912,(timelineNow()-waitStart912)*1000,wake912,wake912==WAIT_OBJECT_0?"new_capture":wake912==WAIT_OBJECT_0+1?"copy_ready":wake912==WAIT_TIMEOUT?"timeout":"error");
      if(wake912==WAIT_FAILED)throw std::runtime_error("Capture-ready wait failed");
    }else WaitForSingleObject(event,100);continue;
   }
   if(windowTest795&&testAwaitGeometry795){if(latestGeometry795==testAwaitGeometry795){latest.Close();continue;}testAwaitGeometry795=0;}
   if(!startupFinished775){
    // Prime the actual WGC / overlay / DWM path using originals only, before any generated work.
    if(stableOriginal775.load()>=4&&timelineNow()-captureStarted>=.25){startupFinished775=true;startupComplete775=timelineNow()-captureStarted;smx_log_fprintf989(stderr,"STARTUP775 display_ready_seconds=%.6f original_warmup_frames=%u; no generated work discarded\n",startupComplete775,startupSkipped775);status(L"Capture and display ready. Starting native-size interpolation.");}
    else {latest.Close();lastTime=0;cachedOriginal.reset();cachedSequence=0;if(heldFirst){heldFirst.Close();heldFirst=nullptr;}startupSkipped775++;continue;}
   }
   if(interactive775&&timelineNow()<recoveryUntil775.load()){latest.Close();lastTime=0;cachedOriginal.reset();cachedSequence=0;if(heldFirst){heldFirst.Close();heldFirst=nullptr;}recoverySkipped775++;continue;}
   auto actual=latest.ContentSize();
   if(actual.Width<=0||actual.Height<=0){latest.Close();continue;}
   com_ptr<ID3D11Texture2D> tex;if(desktopCapture989){if(!latestOriginal)throw std::runtime_error("DXGI cropped lease missing");tex.copy_from(latestOriginal->first.Get());}else tex=latest.Texture989();
   D3D11_TEXTURE2D_DESC td;tex->GetDesc(&td);
   sw=td.Width;sh=td.Height;
   double now=latest.SystemRelativeTime().count()*1e-7;
   if(now<=lastTime){latest.Close();continue;}
   if(!pairFirstOriginal&&cachedOriginal&&cachedSequence+1==latestSequence&&inputReady809(cachedOriginal)){pairFirstOriginal=cachedOriginal;pairFirstStamp=cachedStamp;pairFirstSequence=cachedSequence;}
   cachedOriginal=latestOriginal;cachedStamp=now;cachedSequence=latestSequence;
   if(!pairFirstOriginal){latest.Close();lastTime=0;continue;}
   lastTime=pairFirstStamp;lastSequence=pairFirstSequence;

   double sourcePeriodMs=capturePeriodMs.load();
   double sourceAgeMs=fmax(0.0,(timelineNow()-now)*1000);
   // Only generate for adjacent timestamps. Skipping invalidates the old pair.
   double pairIntervalMs=lastTime?(now-lastTime)*1000:sourcePeriodMs;
   if(lastTime&&latestSequence!=lastSequence+1){
    lastTime=0;primedAdmission=0;pairIntervalMs=sourcePeriodMs;
    if(heldFirst){heldFirst.Close();heldFirst=nullptr;}
   }
   double availableMs=delayMs-sourceAgeMs-pairIntervalMs*.5;
    if(phaseLock971){std::optional<double>pairDue971;{std::lock_guard lock(queueMutex);pairDue971=timeline.pairDue971(lastTime,now);}if(!pairDue971){if(admissionEvents988.size()<72000)admissionEvents988.push_back({timelineNow(),lastTime,now,0,deadlineGuard664.lastEstimate,0,"phase_no_due_before_admission"});else ++admissionTruncated988;latest.Close();lastTime=0;continue;}availableMs=(*pairDue971-timelineNow())*1000;}
   double budgetNow=timelineNow()*1000-budgetEpoch;
   if(!outputAllowed||availableMs<=0){
    if(!outputAllowed)outputSkips++;else expiredSkips++;
    latest.Close();lastTime=0;primedAdmission=0;if(heldFirst){heldFirst.Close();heldFirst=nullptr;}continue;
   }
   double now930=timelineNow();bool busy930=async905&&!pending905.empty();double ahead930=busy930?std::max(0.0,(pending905.back().predictedDone-now930)*1000):0;
   unsigned priorRefresh809=deadlineGuard664.revalidations;
   // R941 bounded-capacity diagnostic: no adaptive forecast masquerading as GPU throughput.
    // Two immutable jobs remain the hard work bound. Existing expiry, order, and completion protections remain.
    bool admit930;
    if(async905){deadlineGuard664.lastBusy930=busy930;deadlineGuard664.lastAhead930=ahead930;deadlineGuard664.lastIncrement930=std::clamp(sourcePeriodMs,16.0,34.0);deadlineGuard664.lastEstimate=deadlineGuard664.lastIncrement930;deadlineGuard664.probePending=false;admit930=std::isfinite(availableMs)&&availableMs>=deadlineGuard664.lastEstimate;if(!admit930)++deadlineGuard664.skips;}
    else admit930=deadlineGuard664.admit(availableMs,now930);
   if(async905&&prediction930)smx_log_fprintf989(prediction930,"%.9f,%u,%.6f,%.6f,%.6f,%u,%u,%u\n",now930,unsigned(busy930),deadlineGuard664.lastAhead930,deadlineGuard664.lastIncrement930,availableMs,unsigned(admit930),deadlineGuard664.idle930.n,deadlineGuard664.warm930.n);
   if(enableDeadline664&&!admit930){latest.Close();lastTime=0;primedAdmission=0;if(heldFirst){heldFirst.Close();heldFirst=nullptr;}
    if(interactive775){performanceWarning775=true;waitingBudget809=true;if(!budgetPauseStart775)budgetPauseStart775=timelineNow();recoveryUntil775=0;
     wchar_t msg[320];swprintf_s(msg,L"Waiting for GPU budget: %.1f ms available, %.1f ms estimated. Rechecked at every fresh input; bounded capacity diagnostic; no generated output discarded.",availableMs,deadlineGuard664.lastEstimate);status(msg);continue;}
    autoStopped695=true;stopFlag=true;status(L"Interpolation stopped: GPU processing cannot fit the display budget. Capture itself succeeded.");break;
   }
   bool revalidation809=deadlineGuard664.probePending;
   if(priorRefresh809!=deadlineGuard664.revalidations)smx_log_fprintf989(stderr,"RECOVERY831 continuous-age forecast admitted fresh pair; late counters retained\n");
   budgetPauseStart775=0;waitingBudget809=false;double estimateAtAdmit809=deadlineGuard664.lastEstimate;
   int decision=primedAdmission==2?2:(adaptive?smx_budget_admit(&budget,budgetNow,availableMs):1);primedAdmission=0;
   if(resourceEnabled&&!resource->admit(timelineNow(),availableMs))decision=0;
   if(!decision){
    if(budgetTrace)smx_log_fprintf989(budgetTrace,"%.9f,skip,%.6f,%.6f,%.6f,%.6f,%.6f,0,0\n",timelineNow(),availableMs,smx_budget_predict(&budget),budget.duty,sourcePeriodMs,sourceAgeMs);
    latest.Close();lastTime=0;primedAdmission=0;if(heldFirst){heldFirst.Close();heldFirst=nullptr;}continue;
   }
   unsigned cw=width,ch=height;auto captureStart=std::chrono::steady_clock::now();
   if(!latestOriginal)throw std::runtime_error("Missing retained native original texture");
   ID3D11Texture2D* inputTexture=latestOriginal->first.Get();
   bool ready;
   if(smxDirectEnabled()){bridge.setDirectPair(pairFirstOriginal->first.Get(),inputTexture);ready=true;bridgeSecondSequence=latestSequence;}
   else{
    if(bridgeSecondSequence!=pairFirstSequence){bridge.pushFrame(pairFirstOriginal->first.Get(),cw,ch);pairUploads++;}
    ready=bridge.pushFrame(inputTexture,cw,ch);pairUploads++;bridgeSecondSequence=latestSequence;
   }
   if(generated.load()<2)smx_log_fprintf989(stderr,"ADJACENT_INPUT_PAIR first=%u second=%u interval_ms=%.6f uploads=%u\n",pairFirstSequence,latestSequence,(now-pairFirstStamp)*1000,pairUploads);

   conversionSum+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-captureStart).count();captureCount++;
   
   if(!ready||lastTime==0){heldFirst=latest;lastTime=now;lastSequence=latestSequence;primedAdmission=decision;continue;}
   #include "fp16-next537/candidate994/multiphase989-loop.inc"
   std::shared_ptr<GpuFrames> leased;
   for(unsigned i=0;i<texturePool.size();i++){unsigned j=(generatedCursor876+i)%texturePool.size();if(texturePool[j].use_count()==1){leased=texturePool[j];generatedCursor876=(j+1)%texturePool.size();break;}}
   // Bounded pre-compute backpressure; never discard a generated result or grow the pool.
    if(!leased){++poolWaitCount965;double poolBegin965=timelineNow();
     do{if(async905)drain905(false);WaitForSingleObject(event,2);
      for(unsigned i=0;i<texturePool.size();i++){unsigned j=(generatedCursor876+i)%texturePool.size();if(texturePool[j].use_count()==1){leased=texturePool[j];generatedCursor876=(j+1)%texturePool.size();break;}}
     }while(!leased&&!stopFlag&&!capturePaused989()&&timelineNow()-poolBegin965<.008);
     poolWaitMs965+=(timelineNow()-poolBegin965)*1000;
    }
    if(!leased||stopFlag||capturePaused989()){++poolSkipCount965;latest.Close();lastTime=0;primedAdmission=0;if(heldFirst){heldFirst.Close();heldFirst=nullptr;}continue;}

   double reservationDue971=(now+lastTime)*.5+delayMs*.001;uint64_t reservation700=0;{
     std::lock_guard lock(queueMutex);auto pairDue971=timeline.pairDue971(lastTime,now);if(pairDue971)reservationDue971=*pairDue971;double reserveNow988=timelineNow(),reserveEstimate988=std::max(8.0,deadlineGuard664.lastEstimate);reservation700=timeline.reservePair971(lastTime,now,reserveNow988,reserveEstimate988*.001,4);
     if(admissionEvents988.size()<72000)admissionEvents988.push_back({reserveNow988,lastTime,now,reservationDue971,reserveEstimate988,reservation700,timeline.lastReserveReject988()});else ++admissionTruncated988;
    }
    if(!reservation700){latest.Close();lastTime=0;primedAdmission=0;if(heldFirst){heldFirst.Close();heldFirst=nullptr;}continue;}
    CallbackCleanup reservationCleanup700{[&](){std::lock_guard lock(queueMutex);timeline.cancelBeforeComplete(reservation700);SetEvent(presentationChanged.value);}};
    SetEvent(presentationChanged.value);
    if(smxDirectOutputEnabled())bridge.setDirectOutput(leased->middle.Get());
   if(audit724){if(auditPairs724>=512)throw std::runtime_error("Marker audit capacity");ComPtr<IDXGIKeyedMutex>a724,b724;smxdiag994::checkedHr((pairFirstOriginal->first.As(&a724)),__LINE__);smxdiag994::checkedHr((latestOriginal->first.As(&b724)),__LINE__);{BridgeLock a(a724.Get()),b(b724.Get());D3D11_BOX box{20,1025,0,740,1026,1};context->CopySubresourceRegion(atlas724.get(),0,0,auditPairs724*2,0,pairFirstOriginal->first.Get(),0,&box);context->CopySubresourceRegion(atlas724.get(),0,0,auditPairs724*2+1,0,latestOriginal->first.Get(),0,&box);context->Flush();}auditPairs724++;}
   if(stopFlag||capturePaused989()){latest.Close();lastTime=0;primedAdmission=0;if(heldFirst){heldFirst.Close();heldFirst=nullptr;}continue;}
   auto runStart=std::chrono::steady_clock::now();

   if(async905){
    double expectedDone930=now930+deadlineGuard664.lastEstimate*.001;unsigned slot905=nextSlot905;UINT64 captureCompleted949=captureFence912->GetCompletedValue();if(captureCompleted949==UINT64_MAX)throw std::runtime_error("R949 device removed");bridge.setPairKeys943(pairFirstOriginal->readyValue912,latestOriginal->readyValue912);bridge.submit905(slot905);if(jobInputs933)smx_log_fprintf989(jobInputs933,"%u,%u,%llu,%llu,%.9f,%.9f,%.9f,%.9f,%llu\n",++jobInputId933,slot905,(unsigned long long)pairFirstOriginal->readyValue912,(unsigned long long)latestOriginal->readyValue912,lastTime,now,now930,timelineNow(),(unsigned long long)captureCompleted949);nextSlot905^=1u;
    double firstTime905=lastTime;
    // Each completion closure retains both input leases and the output until the corresponding fence.
    pending905.push_back({slot905,[&,slot905,leased,pairFirstOriginal,latestOriginal,reservation700,reservationDue971,captureStart,runStart,now,firstTime905,sourceAgeMs,availableMs,estimateAtAdmit809,revalidation809,latestGeometry795,decision,sourcePeriodMs]()mutable{
     double gpu=bridge.finish905(slot905);inferReturned700++;
     double costMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-captureStart).count();runWallSum+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-runStart).count();gpuSum+=gpu;unsigned count=++generated;
     hudSession988->metrics.recordGeneration(gpu,costMs,bridge.queueWaitMs988);bool onTime=timelineNow()<=reservationDue971;deadlineGuard664.completePipeline(bridge.submitTime930(slot905),bridge.fenceTime930(slot905),costMs,!onTime,timelineNow());if(resourceEnabled)resource->complete(timelineNow(),costMs,onTime);if(adaptive)smx_budget_complete(&budget,timelineNow()*1000-budgetEpoch,costMs,onTime);
     if(readyTrace809)smx_log_fprintf989(readyTrace809,"%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%u,%u\n",count,availableMs,estimateAtAdmit809,latestOriginal->readyObserved809?(latestOriginal->readyStamp809-latestOriginal->captureStamp809)*1000:-1.0,latestOriginal->readyObserved809?sourceAgeMs-(latestOriginal->readyStamp809-latestOriginal->captureStamp809)*1000:-1.0,sourceAgeMs,costMs,gpu,unsigned(revalidation809),latestGeometry795);
     if(captureTrace&&count<=72000){LARGE_INTEGER qpc{};QueryPerformanceCounter(&qpc);smx_log_fprintf989(captureTrace,"%u,%.6f,%.6f,%.6f,%.6f,%.9f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",count,(now-firstTime905)*1000,(double(qpc.QuadPart)/qpcFreq.QuadPart-now)*1000,gpu,costMs,double(qpc.QuadPart)/qpcFreq.QuadPart,0.,0.,bridge.vkSubmitMs,bridge.vkFenceMs,bridge.queryMs,0.,0.);}
     {std::lock_guard lock(queueMutex);auto completion988=wsi958?timeline.completeGpuWithStatus980(reservation700,timelineNow()):timeline.completeWithStatus980(reservation700,std::move(leased),timelineNow());bool timely=completion988.onTime||completion988.lifecycleCancelled;hudSession988->metrics.setLateCounts(timeline.counts.lateCompleted,timeline.counts.lateSubmitted);if(!timely){latenessFailure700=true;performanceWarning775=true;if(!interactive775&&!naturalEnd988.requested()){autoStopped695=true;stopFlag=true;}}}SetEvent(presentationChanged.value);queueChanged.notify_one();
    },expectedDone930});
    // Only WSI consumes a timeline whose GPU-ready dependency is a shared semaphore.
    if(wsi958){{std::lock_guard lock(queueMutex);timeline.armGpu(reservation700,leased);}SetEvent(presentationChanged.value);}

    hudSession988->metrics.setInflight(unsigned(pending905.size()));maxPending905=std::max(maxPending905,unsigned(pending905.size()));reservationCleanup700.active=false;latest.Close();if(heldFirst){heldFirst.Close();heldFirst=nullptr;}lastTime=now;lastSequence=latestSequence;continue;
   }
   double gpu=bridge.infer();inferReturned700++;if(benchmarkSource&&getenv("SMX_LIVE_PROOF")&&strcmp(getenv("SMX_LIVE_PROOF"),"1")==0&&generated.load()==1)dumpLiveProof(device.get(),context.get(),inputTexture,"live-proof-input.rgba",true);latest.Close();if(heldFirst){heldFirst.Close();heldFirst=nullptr;}
   if(!desktopCapture989&&(actual.Width!=size.Width||actual.Height!=size.Height)){std::lock_guard lock(inbox->mutex);for(auto&f:inbox->frames)f.frame.Close();inbox->frames.clear();size=actual;pool.Recreate(captureDevice,DirectXPixelFormat::B8G8R8A8UIntNormalized,6,size);}
   if(benchmarkSource&&getenv("SMX_LIVE_PROOF")&&strcmp(getenv("SMX_LIVE_PROOF"),"1")==0)bridge.copyPair(leased->first.Get(),leased->middle.Get());else bridge.copyMiddle(leased->middle.Get());
   if(benchmarkSource&&getenv("SMX_LIVE_PROOF")&&strcmp(getenv("SMX_LIVE_PROOF"),"1")==0&&generated.load()==2){dumpLiveProof(device.get(),context.get(),leased->first.Get(),"live-proof-original.rgba",false);dumpLiveProof(device.get(),context.get(),leased->middle.Get(),"live-proof-generated.rgba",false);}
   runWallSum+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-runStart).count();gpuSum+=gpu;unsigned count=++generated;
   double costMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-captureStart).count();
   hudSession988->metrics.recordGeneration(gpu,costMs,bridge.queueWaitMs988);bool onTime=timelineNow()<=reservationDue971;
   deadlineGuard664.complete(costMs,!onTime,timelineNow());if(resourceEnabled)resource->complete(timelineNow(),costMs,onTime);
   if(adaptive)smx_budget_complete(&budget,timelineNow()*1000-budgetEpoch,costMs,onTime);
   if(readyTrace809){smx_log_fprintf989(readyTrace809,"%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%u,%u\n",count,availableMs,estimateAtAdmit809,latestOriginal->readyObserved809?(latestOriginal->readyStamp809-latestOriginal->captureStamp809)*1000:-1.0,latestOriginal->readyObserved809?sourceAgeMs-(latestOriginal->readyStamp809-latestOriginal->captureStamp809)*1000:-1.0,sourceAgeMs,costMs,gpu,unsigned(revalidation809),latestGeometry795);/* Flush at session close; per-frame synchronous disk flush is not part of inference. */}
   if(budgetTrace)smx_log_fprintf989(budgetTrace,"%.9f,%s,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d\n",timelineNow(),decision==2?"probe":"run",availableMs,smx_budget_predict(&budget),budget.duty,sourcePeriodMs,sourceAgeMs,costMs,onTime?1:0);
   if(now-lastTime>0.025)sourceGaps++;
   if(captureTrace&&count<=72000){LARGE_INTEGER qpc{};QueryPerformanceCounter(&qpc);smx_log_fprintf989(captureTrace,"%u,%.6f,%.6f,%.6f,%.6f,%.9f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",count,(now-lastTime)*1000,(double(qpc.QuadPart)/qpcFreq.QuadPart-now)*1000,gpu,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-captureStart).count(),double(qpc.QuadPart)/qpcFreq.QuadPart,bridge.inputLockMs,bridge.inputSubmitMs,bridge.vkSubmitMs,bridge.vkFenceMs,bridge.queryMs,bridge.outputLockMs,bridge.outputSubmitMs);}
   {std::lock_guard lock(queueMutex);auto completion988=timeline.completeWithStatus980(reservation700,std::move(leased),timelineNow());bool timely700=completion988.onTime||completion988.lifecycleCancelled;reservationCleanup700.active=false;hudSession988->metrics.setLateCounts(timeline.counts.lateCompleted,timeline.counts.lateSubmitted);if(!timely700){latenessFailure700=true;performanceWarning775=true;if(interactive775)recoveryUntil775=0;else{autoStopped695=true;stopFlag=true;}}}SetEvent(presentationChanged.value);queueChanged.notify_one();
   if(recoveryTest809&&count==1){deadlineGuard664.service=90;deadlineGuard664.deviation=0;deadlineGuard664.samples.fill(90);deadlineGuard664.count=1;deadlineGuard664.next=1;deadlineGuard664.lastCompletion=timelineNow();smx_log_fprintf989(stderr,"TEST809 injected 90 ms stale prediction only; real work and late counters unchanged; not performance evidence\n");}
   if(selftest&&!recoveryTest809&&!queueTest809&&(count==1||count==2)){if(windowTest795)testAwaitGeometry795=latestGeometry795;PostMessageW(control,WM_APP+3,count,0);}
   if((selftest||(benchmarkSource&&getenv("SMX_LIVE_PROOF")&&strcmp(getenv("SMX_LIVE_PROOF"),"1")==0))&&count==(queueTest809?(getenv("SMX_TEST_FRAMES833")?unsigned(std::clamp(atoi(getenv("SMX_TEST_FRAMES833")),3,1800)):24u):3u)){stopFlag=true;}
   lastTime=now;lastSequence=latestSequence;
   if(count<=2||std::chrono::duration<double>(std::chrono::steady_clock::now()-lastStatus).count()>=0.25){
    lastStatus=std::chrono::steady_clock::now();
   wchar_t message[512];double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
    swprintf_s(message,L"处理输入输出 %u × %u  ·  模型 GPU 时间跨度 %.2f ms\n累计生成 %u 张，提交 %u 张（提交不代表实际上屏）\n提交时间线 %.0f ms · 相位调整 ±%.0f ms（显示延迟见 HUD）  |  Ctrl+Alt+Esc 停止\nCtrl+Alt+Home 返回控制台",width,height,gpu,count,presented.load(),delayMs,phaseLock971?8.0:0.0);
   status(message);
   }
  }
 if(naturalEnd988.requested()){
   // One shared 250 ms deadline covers GPU completion, Present/ack and OS tail evidence.
   // Never hide/destroy the output merely because the last Present call returned.
   for(;;){
    if(async905)drain905(false);
    bool internallyDrained988=false,interrupted988=false;unsigned tailId988=0;
    {std::lock_guard lock(queueMutex);internallyDrained988=pending905.empty()&&!presentMetadataPending988.load()&&timeline.empty()&&timeline.completedOutstanding()==0&&timeline.gpuCompletionPending()==0;tailId988=lastSubmitted988;
     interrupted988=stopFlag.load()||timeline.lifecycleClosed980()||(sourceOverlay&&(capturePaused989()||overlayEpoch.load()!=lifecycleUiEpoch988));}
    DXGI_FRAME_STATISTICS tailStats988{};HRESULT tailHr988=E_NOINTERFACE;hudSession988->getLatest(tailStats988,tailHr988);
    auto state988=naturalEnd988.observe(timelineNow(),internallyDrained988,tailId988,tailHr988==S_OK,tailStats988.PresentCount,interrupted988);
    if(state988!=smx988::NaturalEnd988::State::Draining)break;
    WaitForSingleObject(event,2);
   }
   if(!naturalEnd988.complete()){
    stopFlag=true;SetEvent(presentationChanged.value);performanceWarning775=true;
    if(naturalEnd988.timedOut()&&!naturalEnd988.internalDrained()){failed=true;errorMessage=L"Natural capture deadline: retained frame drain exceeded 250 ms; remaining work will be safely cancelled, not counted as submitted.";}
   }
  }
  if(!naturalEnd988.complete()&&lifecycleDComp988&&captureActive988){SetEvent(presentationChanged.value);double stopCloseUntil988=timelineNow()+2;
   for(;;){bool closed;{std::lock_guard lock(queueMutex);closed=timeline.lifecycleClosed980()&&timeline.lifecycleReason980()==SmxLifecycleReason980::Stop;}if(closed)break;if(timelineNow()>stopCloseUntil988)smxBridgeFailure555("Display lifecycle Stop-close acknowledgement timeout; pending resources not unwound",WAIT_TIMEOUT);WaitForSingleObject(event,10);}
  }
  if(async905)drain905(true);
  smx_log_fprintf989(stderr,"POOL965 wait_events=%u precompute_skips=%u wait_ms=%.6f generated_pool_slots=%zu no_result_discard=1\n",poolWaitCount965,poolSkipCount965,poolWaitMs965,texturePool.size());
 }catch(const hresult_error&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,int32_t(e.code()),SMX_DIAG_HRESULT994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));failed=true;errorMessage=e.message().c_str();}catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));failed=true;errorMessage=to_hstring(e.what()).c_str();}
 if(async905){smx_log_fprintf989(stderr,"ASYNC942_SPLIT_INPUT_OUTPUT max_pending=%u remaining=%zu same_queue=1 immutable_slots=2 private_input_snapshots=4 inter_job_barrier=1\n",maxPending905,pending905.size());}
 stopCapture989();
  {std::lock_guard captureDrainLock912(inbox->mutex);
   if(captureValue912){BridgeHandle drain912;drain912.value=CreateEventW(nullptr,FALSE,FALSE,nullptr);if(!drain912.value)throw std::runtime_error("Capture drain event creation failed");smxdiag994::checkedHr((captureFence912->SetEventOnCompletion(captureValue912,drain912.value)),__LINE__);if(WaitForSingleObject(drain912.value,5000)!=WAIT_OBJECT_0)throw std::runtime_error("Capture fence drain timeout; no retry");}
  }
 {std::lock_guard lock(inbox->mutex);inbox->active=false;for(auto&f:inbox->frames)f.frame.Close();inbox->frames.clear();
 smx_log_fprintf989(stderr,"Capture inbox: arrived=%u dropped=%u\n",inbox->arrived,inbox->dropped);
  FILE*raw838=smx_log_fopen989("app-capture838.json","wb");if(raw838){smx_log_fprintf989(raw838,R"({"callbacks":%u,"raw_frames":%u,"accepted_frames":%u,"discard_paused":%u,"discard_invalid":%u,"discard_recreate":%u,"discard_extent":%u,"discard_crop":%u,"discard_native_restart":%u,"selected_hwnd":%lld,"foreground_hwnd_at_end":%lld,"source_minimized_at_end":%s})",callbacks838,rawFrames838,inbox->arrived,paused838,invalid838,recreate838,extent838,crop838,restart838,(long long)(INT_PTR)selected,(long long)(INT_PTR)GetForegroundWindow(),IsIconic(selected)?"true":"false");smx_log_fclose989(raw838);}
  smx_log_fprintf989(stderr,"CAPTURE838 callbacks=%u raw=%u accepted=%u paused=%u extent=%u crop=%u\n",callbacks838,rawFrames838,inbox->arrived,paused838,extent838,crop838);
 FILE*f=nullptr;smx_log_fopen_s989(&f,"app-arrivals.csv","wb");if(f){smx_log_fprintf989(f,"frame,source_time_100ns\n");for(size_t i=0;i<inbox->stamps.size();i++)smx_log_fprintf989(f,"%zu,%lld\n",i,inbox->stamps[i]);smx_log_fclose989(f);}}
 if(heldFirst){heldFirst.Close();heldFirst=nullptr;}callbackCleanup.active=false;if(pool)SMX_DIAG_COMMAND994(SMX_DIAG_POOL_CLOSE994,smxdiag994::fields(0,0,0,0,0,2),pool.Close());if(item)item.Closed(closed);bridge.setWake906(nullptr);
 displayDone=true;SetEvent(presentationChanged.value);queueChanged.notify_all();display.join();if(displayFenceProbe988){displayFenceProbe988->shutdown();if(displayFenceProbe988->failed())displayError="Display fence diagnostic failed; inspect app-display-fence988.json";}CloseHandle(event);captureEvent.value=nullptr;hudMonitor988.shutdown();{auto actual988=hudSession988->metrics.snapshot(timelineNow());observedOriginal=unsigned(actual988.originalDisplayCount);observedGenerated=unsigned(actual988.generatedDisplayCount);unknownObserved=unsigned(actual988.unmappedCount);}

  if(FILE*f=smx_log_fopen989("app-processing992.json","wb")){auto cost=smx992::scalingCost(processingExtent992,unsigned(size.Width),unsigned(size.Height),desktopCapture989?0:6);smx_log_fprintf989(f,"{%s,\"native_copy_frames\":%u,\"resampled_frames\":%u,\"direct_srv_draws\":%llu,\"fallback_copy_draws\":%llu,\"fallback_allocated_bytes\":%llu,\"source_client_rgba_bytes\":%llu,\"process_rgba_bytes\":%llu,\"capture_surface_width\":%d,\"capture_surface_height\":%d,\"capture_pool_extra_estimate_bytes\":%llu,\"fallback_reserve_bytes\":%llu,\"shader_source\":\"precompiled_DXBC_bilinear\",\"internal_grid\":\"768x432_unchanged\",\"presentation\":\"DComp_maps_process_image_to_selected_output_geometry\",\"scope\":\"Processing selection changes image detail and full-resolution copies/warp; it does not shrink the fixed XMX core. No performance or power acceptance implied.\"}\n",processingState992.fields().c_str(),nativeFrames795,scaledFrames795,(unsigned long long)(normalizer992?normalizer992->directFrames():0),(unsigned long long)(normalizer992?normalizer992->fallbackFrames():0),(unsigned long long)(normalizer992?normalizer992->fallbackBytes():0),(unsigned long long)cost.sourceClientRgba,(unsigned long long)cost.processRgba,size.Width,size.Height,(unsigned long long)cost.captureDeltaAligned,(unsigned long long)cost.fallbackAligned);smx_log_fclose989(f);}

  if(FILE*f=smx_log_fopen989("app-output993.json","wb")){smx_log_fprintf989(f,"{%s,\"preview_click_stops\":%u,\"black_background\":\"%s\",\"scope\":\"Own-window geometry and composition request only; preview has no absolute game mouse mapping, no global hooks, no display or game settings changes. Actual scanout/pixel correctness and game performance require separate verification.\"}\n",outputFields993().c_str(),previewClickStops993.load(),outputPreview993()?"one_opaque_DComp_surface_pixel":"not_added");smx_log_fclose989(f);}
  targetProcessEvidence993.record("capture_session_drained");
  FILE*windowReport795=smx_log_fopen989("app-window795.json","wb");if(windowReport795){smx_log_fprintf989(windowReport795,R"({"model_width":%u,"model_height":%u,"last_client_width":%u,"last_client_height":%u,"native_frames":%u,"resampled_frames":%u,"geometry_transition_skips":%u,"frame_pool_recreates":%u,"source_geometry_epochs":%u,"native_source_all_frames":%s,"scope":"Native mode copies source; explicit fit modes resample to reported model size and DComp maps output to game client. Original internal 768x432 grid retained. This is not a power or throughput pass."})",width,height,lastClientW795,lastClientH795,nativeFrames795,scaledFrames795,geometrySkipped795,poolRecreates795,geometryEpoch795,(nativeFrames795>0&&!scaledFrames795)?"true":"false");smx_log_fclose989(windowReport795);}
  if(!displayError.empty()){failed=true;errorMessage=to_hstring(displayError).c_str();}
 if(audit724){
  D3D11_QUERY_DESC qd{D3D11_QUERY_EVENT,0};com_ptr<ID3D11Query>q;smxdiag994::checkedHr((device->CreateQuery(&qd,q.put())),__LINE__);context->End(q.get());context->Flush();double until=timelineNow()+.5;HRESULT qr;do{qr=context->GetData(q.get(),nullptr,0,0);if(qr==S_FALSE)Sleep(1);}while(qr==S_FALSE&&timelineNow()<until);if(qr!=S_OK)throw std::runtime_error("Marker audit readback fence failed, no retry");
  D3D11_MAPPED_SUBRESOURCE m{};smxdiag994::checkedHr((context->Map(atlas724.get(),0,D3D11_MAP_READ,D3D11_MAP_FLAG_DO_NOT_WAIT,&m)),__LINE__);unsigned identical=0,adjacent=0,other=0;FILE*af=smx_log_fopen989("app-input-marker724.csv","wb");if(af)smx_log_fprintf989(af,"pair,first_source_id,second_source_id,distinct,adjacent\n");
  for(unsigned i=0;i<auditPairs724;i++){unsigned a=0,b=0;for(unsigned bit=0;bit<16;bit++){size_t x=(10+bit*45)*4;a|=(((unsigned char*)m.pData)[i*2*m.RowPitch+x]>127?1u:0u)<<bit;b|=(((unsigned char*)m.pData)[(i*2+1)*m.RowPitch+x]>127?1u:0u)<<bit;}bool ad=((a+1)&65535)==b;identical+=a==b;adjacent+=ad;other+=a!=b&&!ad;if(af)smx_log_fprintf989(af,"%u,%u,%u,%u,%u\n",i+1,a,b,unsigned(a!=b),unsigned(ad));}
  if(af)smx_log_fclose989(af);context->Unmap(atlas724.get(),0);FILE*ar=smx_log_fopen989("app-input-marker724.json","wb");if(ar){smx_log_fprintf989(ar,R"({"pairs_audited":%u,"identical_source_pairs":%u,"adjacent_source_pairs":%u,"distinct_nonadjacent_pairs":%u,"scope":"Synthetic source IDs. Extra 720x1 copies and post-run readback change workload; not power acceptance."})",auditPairs724,identical,adjacent,other);smx_log_fclose989(ar);}
 }
 FILE*gpuQueue960=smx_log_fopen989("app-gpuqueue960.json","wb");if(gpuQueue960){smx_log_fprintf989(gpuQueue960,R"({"gpu_armed":%llu,"cpu_completion_observed":%llu,"present_submitted":%llu,"present_submitted_before_cpu_completion_observed":%llu,"gpu_completion_pending":%llu,"scope":"Armed means explicit GPU semaphore dependency; not complete or physically displayed. CPU completion and Present are counted independently."})",(unsigned long long)timeline.counts.gpuArmed,(unsigned long long)timeline.counts.completed,(unsigned long long)timeline.counts.submitted,(unsigned long long)timeline.counts.submittedBeforeCompletion,(unsigned long long)timeline.gpuCompletionPending());smx_log_fclose989(gpuQueue960);}
  FILE*naturalLog988=smx_log_fopen989("app-natural-end988.json","wb");if(naturalLog988){smx_log_fprintf989(naturalLog988,R"({"requested":%s,"state":"%s","deadline_seconds":0.250,"start_qpc":%.9f,"deadline_qpc":%.9f,"finish_qpc":%.9f,"elapsed_ms":%.6f,"internal_drain_pass":%s,"internal_drain_qpc":%.9f,"tail_submitted_id":%u,"tail_observed_id":%u,"tail_os_observed":%s,"timed_out":%s,"interrupted":%s,"normal_end_pass":%s,"completed":%llu,"submitted":%llu,"completed_cancelled":%llu,"scope":"Natural bounded capture end only. One shared deadline covers internal drain and observing the last submitted ID. OS ID observation is not physical scanout or proof every preceding frame displayed. Timeout does not release pending GPU leases; existing fence retirement remains mandatory. User Stop and foreground pause retain cancellation semantics."})",naturalEnd988.requested()?"true":"false",naturalEnd988.stateName(),naturalEnd988.start(),naturalEnd988.deadline(),naturalEnd988.finish(),naturalEnd988.elapsedMs(),naturalEnd988.internalDrained()?"true":"false",naturalEnd988.internalTime(),naturalEnd988.tailId(),naturalEnd988.observedId(),naturalEnd988.tailObserved()?"true":"false",naturalEnd988.timedOut()?"true":"false",naturalEnd988.interrupted()?"true":"false",naturalEnd988.complete()?"true":"false",(unsigned long long)timeline.counts.completed,(unsigned long long)timeline.counts.submitted,(unsigned long long)timeline.lifecycle980.completedCancelled());smx_log_fclose989(naturalLog988);}
  FILE*copyStagesFile988=smx_log_fopen989("app-copy-stages988.csv","wb");if(copyStagesFile988){smx_log_fprintf989(copyStagesFile988,"submitted_present_id,generated,source_qpc,had_keyed_mutex,had_native_access,copy_begin_qpc,copy_return_qpc,key_release_return_qpc,native_signal_return_qpc,mark_copy_return_qpc,flush_return_qpc,access_reset_return_qpc,copy_api_ms,key_release_cpu_ms,native_signal_cpu_ms,mark_copy_cpu_ms,flush_cpu_ms,access_reset_cpu_ms,total_cpu_ms\n");for(const auto&e:copyStages988)smx_log_fprintf989(copyStagesFile988,"%u,%u,%.9f,%u,%u,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",e.id,unsigned(e.generated),e.source,unsigned(e.keyed),unsigned(e.native),e.begin,e.copy,e.keyRelease,e.signal,e.markCopy,e.flush,e.accessReset,(e.copy-e.begin)*1000,(e.keyRelease-e.copy)*1000,(e.signal-e.keyRelease)*1000,(e.markCopy-e.signal)*1000,(e.flush-e.markCopy)*1000,(e.accessReset-e.flush)*1000,(e.accessReset-e.begin)*1000);smx_log_fclose989(copyStagesFile988);}smx_log_fprintf989(stderr,"COPY_STAGES988 records=%zu truncated=%llu scope=CPU_API_spans_not_GPU_execution GetBuffer_is_initialization_only\n",copyStages988.size(),(unsigned long long)copyStagesTruncated988);
  FILE*claims988=smx_log_fopen989("app-claims988.csv","wb");if(claims988){smx_log_fprintf989(claims988,"claim_qpc,source_qpc,due_qpc,ticket,generated,actual_lead_ms\n");for(const auto&e:claimEvents988)smx_log_fprintf989(claims988,"%.9f,%.9f,%.9f,%llu,%u,%.6f\n",e.qpc,e.stamp,e.due,(unsigned long long)e.ticket,unsigned(e.generated),(e.due-e.qpc)*1000);smx_log_fclose989(claims988);}
  FILE*admissions988=smx_log_fopen989("app-admissions988.csv","wb");if(admissions988){smx_log_fprintf989(admissions988,"attempt_qpc,first_source_qpc,second_source_qpc,mid_due_qpc,estimate_ms,ticket,reason\n");for(const auto&e:admissionEvents988)smx_log_fprintf989(admissions988,"%.9f,%.9f,%.9f,%.9f,%.6f,%llu,%s\n",e.qpc,e.first,e.second,e.due,e.estimateMs,(unsigned long long)e.ticket,e.reason);smx_log_fclose989(admissions988);}
  FILE*prepare988=smx_log_fopen989("app-display-prepare988.json","wb");if(prepare988){smx_log_fprintf989(prepare988,R"({"requested_dcomp_prepare_ms":%.6f,"effective_prepare_ms":%.6f,"claim_log_truncated":%llu,"admission_log_truncated":%llu,"original_due_unchanged":true,"present_waits_original_due":true,"scope":"Advance claim, waitable credit and copy only. Does not advance Present or retime source. Early original claims can reject a later unreserved middle frame through the existing ordering floor; logs preserve this tradeoff. Fewer generated frames are not an optimization pass."})",displayPrepareMs988,(scheduledPresenter871||wsi958)?32.0:displayPrepareMs988,(unsigned long long)claimTruncated988,(unsigned long long)admissionTruncated988);smx_log_fclose989(prepare988);}
  const auto recoveryState988=timeline.recovery988();
  FILE*claimedLog988=smx_log_fopen989("app-claimed-recovery988.json","wb");if(claimedLog988){smx_log_fprintf989(claimedLog988,R"({"requested":%s,"effective":%s,"claimed_originals_omitted":%llu,"queued_originals_omitted":%llu,"safety_blocks":%llu,"successor_blocks":%llu,"credit_retention_policy":"keep acquired credit until next real Present","generated_omitted":0,"present_acknowledgements_fabricated":0,"source_due_unchanged":true,"natural_end_new_claim_recovery_disabled":true,"scope":"Only matching unaccessed original claims after successful swapchain credit acquisition. Requires a ready generated queue-head successor within preparation lead. Existing shared token/episode budget applies; credit stays held until a real Present. Pause and Stop take priority."})",claimedOriginalRecovery988?"true":"false",recoveryState988.claimedRecoveryEnabled?"true":"false",(unsigned long long)recoveryState988.claimedDrops,(unsigned long long)recoveryState988.queuedDrops,(unsigned long long)recoveryState988.claimedSafetyBlocks,(unsigned long long)recoveryState988.claimedSuccessorBlocks);smx_log_fclose989(claimedLog988);}

  char refillQpc988[64]="null",episodeEndQpc988[64]="null",omissionRate988[64]="null";
  if(std::isfinite(recoveryState988.lastRefillQpc))snprintf(refillQpc988,sizeof(refillQpc988),"%.9f",recoveryState988.lastRefillQpc);
  if(std::isfinite(recoveryState988.lastEpisodeEnd))snprintf(episodeEndQpc988,sizeof(episodeEndQpc988),"%.9f",recoveryState988.lastEpisodeEnd);
  if(timeline.counts.originalQueued)snprintf(omissionRate988,sizeof(omissionRate988),"%.9f",double(recoveryState988.originalDrops)/double(timeline.counts.originalQueued));
  FILE*sustainedFile988=smx_log_fopen989("app-sustained-recovery988.json","wb");if(sustainedFile988){smx_log_fprintf989(sustainedFile988,R"({"requested":%s,"effective":%s,"token_capacity":%.6f,"token_refill_per_second":%.6f,"tokens_remaining":%.6f,"refilled_tokens":%.6f,"last_refill_qpc":%s,"last_episode_end_qpc":%s,"rearm_cooldown_ms":%.6f,"sustained_rearms":%llu,"queued_originals_omitted":%llu,"claimed_originals_omitted":%llu,"claimed_recovery_implemented":true,"token_budget_blocks":%llu,"token_rearm_blocks":%llu,"cooldown_blocks":%llu,"original_entries_accepted":%llu,"recovery_omission_fraction_of_accepted_originals":%s,"generated_omitted_by_recovery":0,"source_due_unchanged":true,"scope":"Only overdue originals are eligible; an explicitly enabled claimed original may be cancelled only after credit and before any GPU access, with a ready generated successor. Omissions change displayed workload and are not equal-workload inference or energy improvement. Completed generated jobs remain ordered."})",sustainedRecovery988?"true":"false",recoveryState988.sustainedRecoveryEnabled&&recoveryState988.enabled?"true":"false",recoveryState988.tokenCapacity,recoveryState988.tokenRefillPerSecond,recoveryState988.tokens,recoveryState988.refilledTokens,refillQpc988,episodeEndQpc988,recoveryState988.rearmCooldown*1000,(unsigned long long)recoveryState988.sustainedRearms,(unsigned long long)recoveryState988.queuedDrops,(unsigned long long)recoveryState988.claimedDrops,(unsigned long long)recoveryState988.tokenBudgetBlocks,(unsigned long long)recoveryState988.tokenRearmBlocks,(unsigned long long)recoveryState988.cooldownBlocks,(unsigned long long)timeline.counts.originalQueued,omissionRate988);smx_log_fclose989(sustainedFile988);}

  FILE*rw988=smx_log_fopen989("app-recovery-window988.json","wb");if(rw988){smx_log_fprintf989(rw988,R"({"progress_window_requested":%s,"progress_window_enabled":%s,"absolute_limit_ms":%.6f,"idle_since_actual_drop_limit_ms":%.6f,"expired_idle":%llu,"expired_absolute":%llu,"max_original_omissions_per_episode":3,"scope":"Opt-in bounded recovery window. Healthy/rearm test uses the existing 8.333ms deadline tolerance; no late threshold is relaxed. Generated frames and due remain unchanged."})",progressWindow988?"true":"false",recoveryState988.progressWindowEnabled?"true":"false",recoveryState988.episodeHardLimit*1000,recoveryState988.episodeIdleLimit*1000,(unsigned long long)recoveryState988.expiredIdleEpisodes,(unsigned long long)recoveryState988.expiredAbsoluteEpisodes);smx_log_fclose989(rw988);}
  FILE*rr988=smx_log_fopen989("app-original-recovery988.json","wb");if(rr988){smx_log_fprintf989(rr988,R"({"requested":%s,"effective":%s,"period_reliable":%s,"explicit_output_hz":%.6f,"output_period_ms":%.6f,"episodes":%llu,"originals_omitted":%llu,"drop_calls":%llu,"exhausted_episodes":%llu,"expired_episodes":%llu,"healthy_rearms":%llu,"generated_head_blocks":%llu,"claimed_blocks":%llu,"throttled_calls":%llu,"lifecycle_interruptions":%llu,"maximum_per_call":2,"maximum_per_episode":3,"episode_timeout_scope":"see app-recovery-window988.json","generated_dropped_by_recovery":0,"due_retimed_by_recovery":false,"scope":"Exceptional backlog protection; overdue queued originals or explicitly enabled untouched pre-copy original claims may be omitted. Every generated ticket and original due is preserved. This is not equal-workload inference speed or energy improvement and not a throughput pass."})",originalRecoveryRequested988?"true":"false",recoveryState988.enabled?"true":"false",recoveryState988.periodReliable?"true":"false",recoveryOutputHz988,recoveryState988.outputPeriod*1000,(unsigned long long)recoveryState988.episodes,(unsigned long long)recoveryState988.originalDrops,(unsigned long long)recoveryState988.dropCalls,(unsigned long long)recoveryState988.exhaustedEpisodes,(unsigned long long)recoveryState988.expiredEpisodes,(unsigned long long)recoveryState988.healthyRecoveries,(unsigned long long)recoveryState988.generatedHeadBlocks,(unsigned long long)recoveryState988.claimedBlocks,(unsigned long long)recoveryState988.throttledCalls,(unsigned long long)recoveryState988.lifecycleInterruptions);smx_log_fclose989(rr988);}
  const auto cancelledCompleted988=timeline.lifecycle980.completedCancelled();
  bool lifecycleIntegrity988=timeline.gpuCompletionPending()==0&&inferReturned700==timeline.counts.completed&&timeline.counts.completed==timeline.counts.submitted+cancelledCompleted988&&timeline.completedOutstanding()==0&&timeline.pendingLifecycleCompletions980()==0;
  bool committed700=lifecycleIntegrity988&&inferReturned700>0&&cancelledCompleted988==0;
  FILE*life988=smx_log_fopen989("app-lifecycle988.json","wb");if(life988){smx_log_fprintf989(life988,R"({"enabled":%s,"pause_epochs":%llu,"stop_epochs":%llu,"completed_cancelled_pause":%llu,"completed_cancelled_stop":%llu,"original_cancelled_pause":%llu,"original_cancelled_stop":%llu,"completed":%llu,"submitted":%llu,"outstanding":%llu,"pending_cancelled_gpu":%llu,"lifecycle_integrity_pass":%s,"normal_completion_commit_pass":%s,"throughput_eligible":%s,"scope":"DComp only. Cancelled completed work retains compute and energy cost and is excluded from performance acceptance. Unsubmitted old entries do not replay. Already Present/acked DXGI buffers and old frontbuffer cannot be cancelled; first fresh Present does not certify first fresh OS display."})",lifecycleDComp988?"true":"false",(unsigned long long)timeline.lifecycle980.pauseEpochs,(unsigned long long)timeline.lifecycle980.stopEpochs,(unsigned long long)timeline.lifecycle980.completedPause,(unsigned long long)timeline.lifecycle980.completedStop,(unsigned long long)timeline.lifecycle980.originalPause,(unsigned long long)timeline.lifecycle980.originalStop,(unsigned long long)timeline.counts.completed,(unsigned long long)timeline.counts.submitted,(unsigned long long)timeline.completedOutstanding(),(unsigned long long)timeline.pendingLifecycleCompletions980(),lifecycleIntegrity988?"true":"false",committed700?"true":"false",(committed700&&!timeline.lifecycle980.pauseEpochs&&(!naturalEnd988.requested()||naturalEnd988.complete()))?"true":"false");smx_log_fclose989(life988);}
  FILE*lifeTrace988=smx_log_fopen989("app-lifecycle988.csv","wb");if(lifeTrace988){smx_log_fprintf989(lifeTrace988,"qpc,epoch,overlay_epoch,action,reason,last_submitted_id,latest_observed_id,waitable_credit_held\n");for(const auto&e:lifecycleEvents988)smx_log_fprintf989(lifeTrace988,"%.9f,%llu,%llu,%s,%s,%u,%u,%u\n",e.qpc,(unsigned long long)e.epoch,(unsigned long long)e.uiEpoch,e.action,e.reason,e.lastSubmitted,e.observed,unsigned(e.credit));smx_log_fclose989(lifeTrace988);}

 bool timingFailure775=latenessFailure700||timeline.counts.lateSubmitted||autoStopped695;
  if((inferReturned700>0&&!lifecycleIntegrity988)||(!interactive775&&(!lifecycleIntegrity988||timingFailure775))){failed=true;if(errorMessage.empty())errorMessage=(inferReturned700>0&&!committed700)?L"Generated-frame commit contract failed; completed results were not all submitted.":L"Performance timing failed. Capture succeeded; this run is not throughput acceptance.";}
  if(timingFailure775)performanceWarning775=true;
  heartbeat775=0;writeLive775(failed?"error":performanceStopped775?"stopped_gpu_budget":"stopped");
  FILE*startup775=smx_log_fopen989("app-startup775.json","wb");if(startup775){smx_log_fprintf989(startup775,R"({"interactive_mode":%s,"warmup_enabled":%s,"warmup_finished":%s,"warmup_seconds":%.6f,"warmup_source_frames":%u,"recovery_source_frames":%u,"performance_stopped":%s,"performance_warning":%s,"scope":"Startup and recovery exclusions are explicit; session operational success is not timing or throughput acceptance."})",interactive775?"true":"false",startupWarmup775?"true":"false",startupFinished775?"true":"false",startupComplete775,startupSkipped775,recoverySkipped775,performanceStopped775?"true":"false",performanceWarning775?"true":"false");smx_log_fclose989(startup775);}
 FILE*commit700=smx_log_fopen989("app-commit700.json","wb");if(commit700){smx_log_fprintf989(commit700,R"({"reserved":%llu,"inference_returned":%u,"completed":%llu,"submitted":%llu,"outstanding":%llu,"cancelled_before_complete":%llu,"pre_submit_rejected":%llu,"late_completed":%llu,"late_submitted":%llu,"nonzero_completion_commit_pass":%s,"timing_pass":%s,"split_display_device":%s,"scope":"Successful Present calls only. Zero work is not a pass. No guarantee of physical scanout. Late drain fails timing acceptance. GPU/device failure is a failure, never masked."})",(unsigned long long)timeline.counts.reserved,inferReturned700,(unsigned long long)timeline.counts.completed,(unsigned long long)timeline.counts.submitted,(unsigned long long)timeline.completedOutstanding(),(unsigned long long)timeline.counts.cancelled,(unsigned long long)timeline.counts.preSubmitRejected,(unsigned long long)timeline.counts.lateCompleted,(unsigned long long)timeline.counts.lateSubmitted,committed700?"true":"false",(!latenessFailure700&&!timeline.counts.lateSubmitted&&committed700)?"true":"false",splitDisplay700?"true":"false");smx_log_fclose989(commit700);}
 FILE*bypassReport=nullptr;smx_log_fopen_s989(&bypassReport,"app-passthrough.json","wb");if(bypassReport){smx_log_fprintf989(bypassReport,R"({"enabled":%s,"start_qpc":%.9f,"end_qpc":%.9f,"bypass_seconds":%.6f,"bypass_captured_frames":%u,"active_cropped_frames":%u,"scope":"Bypass hides own overlay and skips explicit GPU crop/queue/inference. WGC and model allocations remain. OFF baseline uses capture rate proxy; no scanout or physical input guarantee."})",(outputGuard||resourceEnabled)?"true":"false",captureStarted,timelineNow(),passthroughSeconds,bypassCaptured.load(),activeCaptured.load());smx_log_fclose989(bypassReport);}
 FILE*observedReport=nullptr;smx_log_fopen_s989(&observedReport,"app-observed-output.json","wb");if(observedReport){smx_log_fprintf989(observedReport,R"({"hud988_canonical_logs":"app-display988.csv and app-hud988.jsonl","observed_counter_scope":"current statistics epoch; unknown count is trailing observation window; Vulkan has no confirmed display counts","captured_frames":%u,"submitted_original":%u,"submitted_generated":%u,"observed_original_ids":%u,"observed_generated_ids":%u,"unknown_observed_ids":%u,"reference_capture_hz":%.6f,"reference_observed_output_hz":%.6f,"last_capture_hz":%.6f,"scope":"Unique displayed Present IDs matched to explicit GetLastPresentCount IDs. Polling may miss displayed frames; capture cadence is a game-FPS proxy. Not optical verification."})",capturedOriginal.load(),originalPresented.load(),generatedPresented.load(),observedOriginal.load(),observedGenerated.load(),unknownObserved.load(),outputBalance.referenceBaseHz,outputBalance.referenceHz,outputBalance.lastBaseHz);smx_log_fclose989(observedReport);}
 FILE*outputReport=nullptr;smx_log_fopen_s989(&outputReport,"app-output-balance.json","wb");if(outputReport){smx_log_fprintf989(outputReport,R"({"enabled":%s,"state":%u,"reference_observed_output_hz":%.6f,"last_original_hz":%.6f,"last_generated_hz":%.6f,"original_presents":%u,"generated_presents":%u,"executions":%u,"model_skipped_inputs_or_bypass_polls":%u,"already_expired_inputs":%u,"accepted_windows":%u,"rejected_windows":%u,"base_floor_rejections":%u,"scope":"Capture base proxy and unique observed display IDs; short inference-off reference, not resource reservation"})",outputGuard?"true":"false",unsigned(outputBalance.state),outputBalance.referenceHz,outputBalance.lastOriginalHz,outputBalance.lastGeneratedHz,originalPresented.load(),generatedPresented.load(),generated.load(),outputSkips,expiredSkips,outputBalance.accepted,outputBalance.rejected,outputBalance.baseFloorRejects);smx_log_fclose989(outputReport);}
 FILE*budgetReport=nullptr;smx_log_fopen_s989(&budgetReport,"app-budget.json","wb");if(budgetReport){smx_log_fprintf989(budgetReport,"{\"enabled\":%s,\"max_service_duty\":%.6f,\"final_service_duty\":%.6f,\"executed\":%u,\"budget_skips\":%u,\"deadline_skips\":%u,\"probes\":%u,\"late\":%u,\"scope\":\"wall-time service budget; not hardware occupancy; no game-specific rules\"}\n",adaptive?"true":"false",budget.maxDuty,budget.duty,budget.executed,budget.skippedBudget,budget.skippedDeadline,budget.probes,budget.late);smx_log_fclose989(budgetReport);}
 FILE*accept695=smx_log_fopen989("app-acceptance695.json","wb");if(accept695){smx_log_fprintf989(accept695,"{\"auto_stopped_for_deadline\":%s,\"completed\":%u,\"generated_submitted\":%u,\"scope\":\"Clean process exit is not performance acceptance. Deadline stop hides overlay; no delayed-original-only fallback is promoted as interpolation. First cold job and sudden preemption are not guaranteed.\"}\n",autoStopped695?"true":"false",generated.load(),generatedPresented.load());smx_log_fclose989(accept695);}
 FILE*rr809=smx_log_fopen989("app-recovery809.json","wb");if(rr809){smx_log_fprintf989(rr809,"{\"input_ready_gate\":%s,\"query_polls\":%u,\"query_wait_iterations\":%u,\"stale_history_revalidations\":%u,\"scope\":\"R912 fence values avoid admission before producer GPU writes complete; completion events replace 2ms timer polling; readiness age includes host observation delay; signed values may be negative when WGC timestamps lead the CPU observation. Forecast continuously ages; retries are source-event driven. This is not throughput acceptance.\"}",readyGate809?"true":"false",readyPolls809,readyWaits809,deadlineGuard664.revalidations);smx_log_fclose989(rr809);}
 FILE*guard664=smx_log_fopen989("app-deadline664.json","wb");if(guard664){smx_log_fprintf989(guard664,"{\"enabled\":%s,\"pre_submit_skips\":%u,\"executed\":%u,\"completed_past_deadline\":%u,\"last_service_forecast_ms\":%.6f,\"scope\":\"R941 CAPACITY DIAGNOSTIC: max2 in-flight jobs, fresh input admission requires one source period of remaining budget (16-34ms), adaptive forecast not used. 80ms timeline/order/commit protections unchanged. Does not imply throughput or latency acceptance.\"}\n",enableDeadline664?"true":"false",deadlineGuard664.skips,deadlineGuard664.executed,deadlineGuard664.completedLate,deadlineGuard664.estimate());smx_log_fclose989(guard664);}
 FILE*phaseReport971=smx_log_fopen989("app-phase971.json","wb");if(phaseReport971){const auto&p971=timeline.phase971();smx_log_fprintf989(phaseReport971,"{\"enabled\":%s,\"base_delay_ms\":%.6f,\"max_content_to_due_ms\":%.6f,\"phase_bound_ms\":8,\"phase_step_max_ms\":0.25,\"period_step_max_ms\":0.02,\"estimated_period_ms\":%.6f,\"reset_count\":%llu,\"bound_clips\":%llu,\"order_clips\":%llu,\"scope\":\"Causal presentation phase only; content stamps/model math unchanged; not model throughput or actual display proof\"}\n",phaseLock971?"true":"false",delayMs,delayMs+(phaseLock971?8:0),p971.period()*1000,(unsigned long long)p971.epochs,(unsigned long long)p971.boundClips,(unsigned long long)p971.orderClips);smx_log_fclose989(phaseReport971);}
  FILE*policy=nullptr;smx_log_fopen_s989(&policy,"app-timeline.json","wb");if(policy){smx_log_fprintf989(policy,"{\"delay_ms\":%.6f,\"original_queued\":%llu,\"generated_queued\":%llu,\"original_dropped\":%llu,\"generated_dropped\":%llu,\"invalid_timestamps\":%llu}\n",delayMs,(unsigned long long)timeline.counts.originalQueued,(unsigned long long)timeline.counts.generatedQueued,(unsigned long long)timeline.counts.originalDropped,(unsigned long long)timeline.counts.generatedDropped,(unsigned long long)timeline.counts.invalid);smx_log_fclose989(policy);}
 FILE*pacing=nullptr;smx_log_fopen_s989(&pacing,"app-pacing.json","wb");if(pacing){smx_log_fprintf989(pacing,"{\"present_calls\":%u,\"calls_over_one_refresh_late\":%u,\"dropped_pairs\":%u,\"source_intervals_over_25ms\":%u,\"scope\":\"Present call timing; not physical display scanout validation\"}\n",presented.load(),latePresents.load(),droppedPairs.load(),sourceGaps);smx_log_fclose989(pacing);}
 if(selftest&&!windowTest795&&generated.load()==0){failed=true;errorMessage=L"Portable capture probe completed without any generated frames";}
 FILE* report=nullptr;smx_log_fopen_s989(&report,"app-session.json","wb");if(report){smx_log_fprintf989(report,"{\"width\":%u,\"height\":%u,\"generated\":%u,\"presented\":%u,\"mean_gpu_ms\":%.6f,\"selftest\":%s,\"success\":%s}\n",width,height,generated.load(),presented.load(),generated?gpuSum/generated.load():0,selftest?"true":"false",failed?"false":"true");smx_log_fclose989(report);
 FILE*perf=nullptr;smx_log_fopen_s989(&perf,"app-profile.json","wb");if(perf){smx_log_fprintf989(perf,"{\"captured\":%u,\"mean_capture_prepare_ms\":%.6f,\"mean_gpu_conversion_submit_ms\":%.6f,\"mean_gpu_transfer_model_and_output_copy_ms\":%.6f}\n",captureCount,captureCount?captureSum/captureCount:0,captureCount?conversionSum/captureCount:0,generated?runWallSum/generated.load():0);smx_log_fclose989(perf);}
 FILE*ui=nullptr;smx_log_fopen_s989(&ui,"app-ui-checks.json","wb");if(ui){smx_log_fprintf989(ui,"{\"fullscreen_entries\":%u,\"fullscreen_exits\":%u,\"source_hit_test_passes\":%u,\"source_overlay\":%s,\"overlay_entries\":%u,\"overlay_hides\":%u,\"overlay_hit_test_passes\":%u,\"overlay_hit_test_failures\":%u}\n",fullscreenEntries.load(),fullscreenExits.load(),passthroughChecks.load(),sourceOverlay?"true":"false",overlayEntries.load(),overlayHides.load(),overlayHitPasses.load(),overlayHitFailures.load());smx_log_fclose989(ui);}}
 if(outputGeometryFailed993.load()){failed=true;errorMessage=lastCaptureError;}
 if(failed){lastCaptureError=errorMessage;smx_log_fprintf989(stderr,"SESSION_ERROR: %s\n",to_string(errorMessage).c_str());fflush(stderr);status(L"Stopped: "+errorMessage);return 1;}
 return 0;
 }catch(const hresult_error&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,int32_t(e.code()),SMX_DIAG_HRESULT994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));smx_log_fprintf989(stderr,"Capture error phase=%s hr=0x%08x: %s\n",capturePhase989,(unsigned)e.code(),to_string(e.message()).c_str());status(L"Capture error: "+std::wstring(e.message()));return 1;}
 catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));FILE*failure852=smx_log_fopen989("app-failure852.txt","wb");if(failure852){smx_log_fprintf989(failure852,"%s\n",e.what());smx_log_fclose989(failure852);}smx_log_fprintf989(stderr,"App error phase=%s: %s\n",capturePhase989,e.what());status(L"Error: "+std::wstring(to_hstring(e.what())));return 1;}
}
static BOOL CALLBACK enumWindows(HWND h,LPARAM){
 if(h==control||h==output||!IsWindowVisible(h)||GetWindow(h,GW_OWNER))return TRUE;
 wchar_t title[256];GetWindowTextW(h,title,256);if(!title[0])return TRUE;RECT r;GetClientRect(h,&r);if(r.right<32||r.bottom<32)return TRUE;
 windows.push_back(h);SendMessageW(combo,CB_ADDSTRING,0,(LPARAM)title);return TRUE;
}
static void refresh(){
 HWND prior=selected,fg=GetForegroundWindow();SendMessageW(combo,CB_RESETCONTENT,0,0);windows.clear();EnumWindows(enumWindows,0);int best=-1;
 for(size_t i=0;i<windows.size();i++){wchar_t title[512]{};GetWindowTextW(windows[i],title,512);if(best<0)best=int(i);if(windows[i]==fg)best=int(i);if(wcsstr(title,L"Forza Horizon"))best=int(i);}
 for(size_t i=0;i<windows.size();i++)if(windows[i]==prior)best=int(i);
 if(best>=0)SendMessageW(combo,CB_SETCURSEL,best,0);
}
static void launch(){
 if(running)return;
 try{outputMode993=smx993::outputMode(getenv("SMX_OUTPUT_MODE993"));
  if(outputPreview993()&&(smx989::desktopCapture(getenv("SMX_CAPTURE_BACKEND989"))||(getenv("SMX_PRESENT_BACKEND")&&strcmp(getenv("SMX_PRESENT_BACKEND"),"dxgi")!=0)||opaqueWindow976))throw std::runtime_error("Output preview requires WGC capture, DXGI/DComp and Layered own-window mode");
 }catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));status(L"Output option rejected: "+std::wstring(winrt::to_hstring(e.what()).c_str()));return;}
 outputState993.clear(outputMode993);outputGeometryFailed993=false;outputProcessingWidth993=0;outputProcessingHeight993=0;previewClickStops993=0;
 controlMinimized964=false;start964=GetTickCount64();lastHud964=0;oldCapture964=oldGen964=oldPresent964=0;hzCapture964=hzGen964=hzPresent964=0;SetTimer(control,8,1000,nullptr);
 int i=(int)SendMessageW(combo,CB_GETCURSEL,0,0);if(i<0||i>=(int)windows.size()){status(L"Select a source window.");return;}selected=windows[i];
 DWORD identity990=0;GetWindowThreadProcessId(selected,&identity990);targetPid990=identity990;targetClosed990=false;processingState992.clear();lastWindowState990.clear();windowStatePath990=std::string(getenv("SMX_MODEL_DIR")?getenv("SMX_MODEL_DIR"):".")+"\\app-window-state990.jsonl";
 targetProcessEvidence993.begin(identity990,getenv("SMX_MODEL_DIR"));targetProcessEvidence993.record("selected");
 RECT sourceClient{},sourceOuter{};
 if(!GetClientRect(selected,&sourceClient)||!GetWindowRect(selected,&sourceOuter)){status(L"Cannot read selected source size.");return;}
 if(!validExtent795(sourceClient.right,sourceClient.bottom)){status(L"Restore the source window to a visible, supported size.");return;}
 
 selectedSize=1;useSession120=false;EnableWindow(refreshModeCheck,FALSE);
 sourceOverlay=(selftest&&(!opaqueWindow976||smx989::desktopCapture(getenv("SMX_CAPTURE_BACKEND989"))))||(!selftest&&!demo&&(!benchmarkSource||(getenv("SMX_BENCH_OVERLAY_TEST")&&strcmp(getenv("SMX_BENCH_OVERLAY_TEST"),"1")==0))&&SendMessageW(overlayCheck,BM_GETCHECK,0,0)==BST_CHECKED);
 if(outputPreview993())sourceOverlay=true;
 EnableWindow(overlayCheck,FALSE);captureBackendPaused989=false;overlayReady=false;overlayPaused=sourceOverlay;overlayEpoch=0;seenOverlayEpoch=0;
 overlayEntries=0;overlayHides=0;overlayHitPasses=0;overlayHitFailures=0;overlayVisible=false;overlayRect={};
 if(sourceOverlay){
  auto eventPath=baseDir+L"\\model1080\\app-window-events.csv";if(const char*md=getenv("SMX_MODEL_DIR"))eventPath=std::wstring(md,md+strlen(md))+L"\\app-window-events.csv";
  _wfopen_s(&overlayEvents,eventPath.c_str(),L"wb");
  if(overlayEvents)smx_log_fprintf989(overlayEvents,"qpc_seconds,event,foreground_hwnd,minimized,left,top,right,bottom\n");
 }
 if(!hostConfig988.hosted)smx_diag_session994(smx_diag_current_session994()+1);
 stopFlag=false;performanceWarning775=false;generated=0;presented=0;originalPresented=0;generatedPresented=0;capturedOriginal=0;observedOriginal=0;observedGenerated=0;unknownObserved=0;fullscreenEntries=0;fullscreenExits=0;passthroughChecks=0;running=true;EnableWindow(startButton,FALSE);EnableWindow(stopButton,TRUE);
 if(!output)output=CreateWindowExW(outputExStyle993(sourceOverlay,windowTest795),L"SMXOutput",L"Smooth Motion R994",outputStyle993(sourceOverlay),80,160,900,620,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
 if(!output){running=false;EnableWindow(startButton,TRUE);EnableWindow(stopButton,FALSE);EnableWindow(overlayCheck,TRUE);if(overlayEvents){smx_log_fclose989(overlayEvents);overlayEvents=nullptr;}status(L"Cannot create output window.");return;}
 SetWindowLongPtrW(output,GWL_STYLE,outputStyle993(sourceOverlay));
 SetWindowTextW(output,outputPreview993()?L"Smooth Motion · 输出预览 — 点击停止":L"Smooth Motion · 游戏叠层");
 SetWindowLongPtrW(output,GWL_EXSTYLE,outputExStyle993(sourceOverlay,windowTest795));
 if(!sourceOverlay||!opaqueWindow976)SetLayeredWindowAttributes(output,0,255,LWA_ALPHA);
  recordWindow976(output,"output","style_configured");
 if(!setAffinity976(output,"output",((sourceOverlay&&!visualProof964)||(smx989::desktopCapture(getenv("SMX_CAPTURE_BACKEND989"))&&!visualProof964)||(!demo&&!selftest&&(!benchmarkSource||sourceOverlay)&&!visualProof964))?WDA_EXCLUDEFROMCAPTURE:WDA_NONE)){
  running=false;EnableWindow(startButton,TRUE);EnableWindow(stopButton,FALSE);EnableWindow(overlayCheck,TRUE);
  if(overlayEvents){smx_log_fclose989(overlayEvents);overlayEvents=nullptr;}
  ShowWindow(output,SW_HIDE);status(L"Output display affinity setup failed. See app-window976.jsonl; capture was not started.");return;
 }
 if(sourceOverlay&&opaqueWindow976)smx_log_fprintf989(stderr,"WINDOW976 opaque diagnostic selected; cross-process input forwarding unverified; no system display settings changed\n");
 if(sourceOverlay){if(getenv("SMX_PRESENT_BACKEND")&&strcmp(getenv("SMX_PRESENT_BACKEND"),"vulkan")==0){POINT pos958{0,0};ClientToScreen(selected,&pos958);if(!SetWindowPos(output,nullptr,pos958.x,pos958.y,sourceClient.right,sourceClient.bottom,SWP_NOACTIVATE|SWP_NOZORDER))throw std::runtime_error("WSI overlay initial geometry");}ShowWindow(output,SW_HIDE);SetTimer(control,7,100,nullptr);startWindowHooks990();if(!hostConfig988.hosted&&!benchmarkSource)SetForegroundWindow(selected);}
 else{RECT nativeOutput715{0,0,1920,1080};if(benchmarkSource){if(!AdjustWindowRectEx(&nativeOutput715,WS_OVERLAPPEDWINDOW,FALSE,WS_EX_LAYERED))throw std::runtime_error("Cannot size own native output");}SetWindowPos(output,HWND_NOTOPMOST,80,160,benchmarkSource?nativeOutput715.right-nativeOutput715.left:900,benchmarkSource?nativeOutput715.bottom-nativeOutput715.top:620,SWP_FRAMECHANGED|SWP_SHOWWINDOW|SWP_NOACTIVATE);ShowWindow(output,SW_SHOWNOACTIVATE);}
 status(outputPreview993()?L"正在准备输出预览；游戏须保持前台。\n点击预览或 Ctrl+Alt+Esc 停止；预览不映射游戏菜单鼠标。":L"正在准备模型与显示管线，完成后自动覆盖游戏。\n键鼠仍由游戏接收；Ctrl+Alt+Esc 可随时停止。");
 {std::lock_guard lock(engineMutex783);if(engineIdle783){modelReady992=true;foregroundWaitSeconds992=0;captureSetupSeconds992=0;setPreparationStage992(PreparationStage992::CaptureSetup);lastCaptureError.clear();restart783=true;engineCv783.notify_one();status(L"正在复用已准备的模型，恢复捕获与叠层。");return;}}
 if(worker.joinable())worker.join();
 {std::lock_guard lock(engineMutex783);quit783=false;restart783=false;session783=0;}
 modelReady992=false;modelPreparationSeconds992=0;captureSetupSeconds992=0;foregroundWaitSeconds992=0;setPreparationStage992(PreparationStage992::ModelInitializing);
 worker=std::thread([generation=++generation783]{
  std::wstring directory=baseDir+(selectedSize==1?L"\\model1080":L"\\model512");if(const char*md=getenv("SMX_MODEL_DIR"))directory=std::wstring(md,md+strlen(md));
  SetCurrentDirectoryW(directory.c_str());
  lastCaptureError.clear();FILE*resetLog=nullptr;smx_log_freopen_s989(&resetLog,"NUL","w",stderr);FILE*log=smx_log_fsopen989("app-engine.log","w",_SH_DENYNO);if(log){_dup2(_fileno(log),_fileno(stderr));smx_log_fclose989(log);smx_log_reset989(stderr,"app-engine.log");setvbuf(stderr,nullptr,_IONBF,0);}
  char name[]="smooth-motion-xmx",runs[]="1";std::string manifest=getenv("SMX_MANIFEST")?getenv("SMX_MANIFEST"):"graph-planaddr.bin";char*args[]={name,manifest.data(),runs};int code=SMX_DIAG_CALL994(SMX_DIAG_ENGINE994,smxdiag994::fields(),SMX_DIAG_SEMANTIC994,smx_graph_main(3,args));engineExit989=code;
  PostMessageW(control,WM_APP+2,code,(LPARAM)generation);
 });
}

static std::string hostResponse988(bool ok=true,const std::string&error={}){
 bool retained=false;{std::lock_guard<std::mutex>l(engineMutex783);retained=engineIdle783;}
 bool active=running.load();std::string state=hostError988?"error":!active?"idle":stopFlag.load()||hostQuitting988?"stopping":!hostCaptureStarted988.load()?(preparationStage992.load()==PreparationStage992::WaitForeground?"wait_foreground":"preparing"):capturePaused989()?"paused":"running";
 std::string message=host988::utf8(hostMessage988);if(state=="wait_foreground")message="Model ready; waiting for the selected game foreground. Stop is available.";if(state=="paused")message=std::string("Capture paused: ")+pauseReason990();if(targetClosed990&&!hostError988)message=active?"Target game closed; stopping safely":"Target game closed; capture stopped normally";
 return "{\"ok\":"+std::string(ok?"true":"false")+",\"error\":"+host988::quote(error)+",\"message\":"+host988::quote(message)+",\"state\":"+host988::quote(state)+",\"running\":"+(active?"true":"false")+",\"safeIdle\":"+(!active?"true":"false")+",\"targetHwnd\":"+std::to_string(uint64_t(uintptr_t(selected)))+",\"sessionSequence\":"+std::to_string(hostSession988)+",\"modelRetained\":"+(retained?"true":"false")+",\"hudEnabled\":"+(hudEnabled964?"true":"false")+",\"detailsEnabled\":"+(hudDetails988?"true":"false")+",\"emergencyHotkeyRegistered\":"+(hostEmergencyRegistered988?"true":"false")+",\"protocolVersion\":1,"+windowFields990()+","+preparationFields992()+","+processingFields992()+","+outputFields993()+"}";
}
static std::string validateTarget988(const host988::Command&c){
 HWND target=(HWND)(uintptr_t)c.hwnd;DWORD pid=0;GetWindowThreadProcessId(target,&pid);
 if(!IsWindow(target)||GetAncestor(target,GA_ROOT)!=target||pid!=c.pid||pid==GetCurrentProcessId()||pid==hostConfig988.pid)return "target_identity_mismatch";
 HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid);bool match=host988::processMatches(process,c.creation);if(process)CloseHandle(process);if(!match)return "target_process_creation_mismatch";
 if(!IsWindowVisible(target)||IsIconic(target))return "target_not_visible";HWND fg=GetForegroundWindow();if(!fg||GetAncestor(fg,GA_ROOT)!=target)return "target_not_foreground";
 RECT r{};if(!GetClientRect(target,&r)||r.right<1||r.bottom<1||r.right>16384||r.bottom>16384)return "target_extent_unsupported";
 return {};
}
static void hostDispatch988(){
 auto request=hostServer988?hostServer988->take():nullptr;if(!request||request->cancelled.load())return;
 auto c=request->command;bool ok=true;std::string error;
 switch(c.kind){
 case host988::Command::Ping:case host988::Command::Status:break;
 case host988::Command::Start:{
  if(running||hostQuitting988){ok=false;error="engine_busy";break;}
  error=validateTarget988(c);if(!error.empty()){ok=false;break;}
  hostError988=false;hostCaptureStarted988=false;hostMessage988=L"Preparing model and display";
  selected=(HWND)(uintptr_t)c.hwnd;windows.clear();windows.push_back(selected);SendMessageW(combo,CB_RESETCONTENT,0,0);SendMessageW(combo,CB_ADDSTRING,0,(LPARAM)L"Hosted target");SendMessageW(combo,CB_SETCURSEL,0,0);
  smx_diag_session994(hostSession988+1);smxdiag994::instant(SMX_DIAG_SESSION994,smxdiag994::fields(0,0,0,0,0,hostSession988+1,0));try{launch();if(running)++hostSession988;else{ok=false;error="start_rejected";hostError988=true;smx_diag_session994(hostSession988);}}catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));smx_diag_session994(hostSession988);stopFlag=true;ok=false;error=e.what();hostError988=true;if(!worker.joinable()){running=false;hostCaptureStarted988=false;KillTimer(control,7);KillTimer(control,8);ShowWindow(output,SW_HIDE);hideHud964();if(overlayEvents){smx_log_fclose989(overlayEvents);overlayEvents=nullptr;}}}break;
 }
 case host988::Command::Stop:if(running){stopFlag=true;hideHud964();hostMessage988=L"Stopping safely; waiting for GPU retirement";}break;
 case host988::Command::Hud:hudEnabled964=c.enabled;updateSourceOverlay();break;
 case host988::Command::Detail:hudDetails988=c.enabled;smx988::releaseHud();lastHud964=0;updateSourceOverlay();break;
 case host988::Command::Quit:hostQuitting988=true;if(running)stopFlag=true;break;
 default:ok=false;error="invalid_command";break;
 }
 request->response=hostResponse988(ok,error);SetEvent(request->done);
}

static LRESULT CALLBACK controlProc(HWND h,UINT m,WPARAM w,LPARAM l){
 if(m==host988::CommandMessage){hostDispatch988();return 0;}
 if(m==host988::ShutdownMessage){hostMessage988=L"Host disconnected; stopping safely";PostMessageW(h,WM_CLOSE,0,0);return 0;}
 if(m==WM_TIMER&&w==7){updateSourceOverlay();return 0;}
 if(m==WM_TIMER&&w==8){
  if(running&&!benchmarkSource){unsigned seconds=0;try{seconds=smx989::sessionSeconds(getenv("SMX_UI_SECONDS964"));}catch(...){stopFlag=true;status(L"Invalid test-session duration; choose continuous or 10–14400 seconds.");return 0;}if(seconds&&GetTickCount64()-start964>=seconds*1000ull){stopFlag=true;hideHud964();status(L"定时测试已结束，可重新开始；普通游玩选择持续模式。");}}
  return 0;
 }
 if(m==WM_CTLCOLORSTATIC){HDC dc=(HDC)w;SetBkColor(dc,RGB(24,29,38));SetTextColor(dc,RGB(221,230,239));return (LRESULT)panel964;}
 if(m==WM_APP+10){updateSourceOverlay();recordWindowState990();return 0;}
 if(m==WM_APP+4){if(w==1){BOOL focused=hostConfig988.hosted?FALSE:SetForegroundWindow(selected);smx_log_fprintf989(stderr,"PREPARE706 selected=%lld foreground=%lld SetForeground=%u\n",(long long)(INT_PTR)selected,(long long)(INT_PTR)GetForegroundWindow(),unsigned(focused));}overlayReady=true;updateSourceOverlay();return 0;}
 if(m==WM_HOTKEY){
 if(w==1||w==3){hudEnabled964=!hudEnabled964;updateSourceOverlay();}
 if(w==2){stopFlag=true;hideHud964();ShowWindow(output,SW_HIDE);if(!hostConfig988.hosted)ShowWindow(control,SW_RESTORE);status(L"正在安全停止，游戏已恢复显示。");}
 if(w==4&&!hostConfig988.hosted){ShowWindow(control,SW_RESTORE);SetForegroundWindow(control);}
 if(w==5){hudDetails988=!hudDetails988;smx988::releaseHud();lastHud964=0;updateSourceOverlay();}
 return 0;}
 if(m==WM_APP+3){if(windowTest795){unsigned count=unsigned(w);SetWindowPos(sourceTest,nullptr,0,0,count==1?1024:1280,count==1?768:720,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);}else toggleFullscreen();return 0;}
 if(m==WM_COMMAND){switch(LOWORD(w)){case 101:refresh();break;case 102:launch();break;case 103:stopFlag=true;hideHud964();status(L"正在安全停止，等待当前 GPU 工作结束。");break;case 104:hudEnabled964=!hudEnabled964;updateSourceOverlay();break;case 108:if(IsWindow(selected)){SetForegroundWindow(selected);updateSourceOverlay();}break;}return 0;}
 if(m==WM_APP+1){auto p=(std::wstring*)l;hostMessage988=*p;SetWindowTextW(statusText,p->c_str());delete p;return 0;}
 if(m==WM_APP+8){smxdiag994::instant(SMX_DIAG_SESSION994,smxdiag994::fields(0,0,0,0,0,hostSession988,2),SMX_DIAG_SEMANTIC994,w);if((unsigned long)l!=generation783.load())return 0;stopWindowHooks990();setPreparationStage992(PreparationStage992::Idle);KillTimer(h,7);KillTimer(h,8);hideHud964();overlayReady=false;overlayPaused=true;if(fullscreen)toggleFullscreen();ShowWindow(output,SW_HIDE);running=false;hostCaptureStarted988=false;overlayEvent("stop",overlayRect);if(overlayEvents){smx_log_fclose989(overlayEvents);overlayEvents=nullptr;}EnableWindow(overlayCheck,TRUE);EnableWindow(startButton,TRUE);EnableWindow(stopButton,FALSE);status(performanceWarning775?L"已停止，有超期记录；模型保留 120 秒，可快速重新开始。":L"已停止。模型保留 120 秒，点击开始可快速恢复；关闭窗口释放资源。");return 0;}
 if(m==WM_APP+2){smxdiag994::instant(SMX_DIAG_SESSION994,smxdiag994::fields(0,0,0,0,0,hostSession988,3),SMX_DIAG_SEMANTIC994,w);if((unsigned long)l!=generation783.load())return 0;stopWindowHooks990();modelReady992=false;setPreparationStage992(PreparationStage992::Idle);KillTimer(h,7);KillTimer(h,8);hideHud964();overlayReady=false;overlayPaused=true;if(fullscreen)toggleFullscreen();ShowWindow(output,SW_HIDE);if(worker.joinable())worker.join();running=false;hostCaptureStarted988=false;if(hostConfig988.hosted)hostError988=w!=0;overlayEvent("stop",overlayRect);if(overlayEvents){smx_log_fclose989(overlayEvents);overlayEvents=nullptr;}EnableWindow(overlayCheck,TRUE);EnableWindow(startButton,TRUE);EnableWindow(stopButton,FALSE);if(w==0)status(performanceWarning775?L"已停止，存在时序警告，本次不能作为稳定性能达标。":L"已停止，完成的生成帧已按顺序提交。");else status(lastCaptureError.empty()?L"Engine startup failed. See app-engine.log and startup-verification.json.":L"会话停止："+lastCaptureError);if(selftest||closePending)DestroyWindow(h);return 0;}
 if(m==WM_CLOSE){hostQuitting988=true;{std::lock_guard lock(engineMutex783);quit783=true;engineCv783.notify_one();}if(running){closePending=true;stopFlag=true;status(L"Closing after GPU cleanup.");}else DestroyWindow(h);return 0;}
 if(m==WM_DESTROY){stopWindowHooks990();UnregisterHotKey(h,1);UnregisterHotKey(h,2);UnregisterHotKey(h,3);UnregisterHotKey(h,4);UnregisterHotKey(h,5);PostQuitMessage(0);return 0;}
 return DefWindowProcW(h,m,w,l);
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR command,int){
 if(wcsstr(command,L"--device-capabilities995"))return smx_device_capabilities_cli995();
 if(wcsstr(command,L"--diagnostic-integration-selftest994"))return diagnosticIntegrationSelftest994();
 if(wcsstr(command,L"--diagnostics-selftest994"))return int(smx_diag_selftest994(L"."));
 if(wcsstr(command,L"--output-geometry-selftest993")){try{unsigned checks=outputGeometrySelftest993();FILE*f=smx_log_fopen989("output-geometry993-selftest.json","wb");if(!f)return 2;smx_log_fprintf989(f,"{\"pass\":true,\"checks\":%u,\"gpu_used\":false,\"windows_created\":false}\n",checks);smx_log_fclose989(f);return 0;}catch(...){return 1;}}
 if(wcsstr(command,L"--telemetry-lifetime-selftest993")){try{unsigned checks=smx993::collectorLifetimeSelfTest();FILE*f=smx_log_fopen989("telemetry-lifetime993-selftest.json","wb");if(!f)return 2;smx_log_fprintf989(f,"{\"pass\":true,\"checks\":%u,\"gpu_used\":false,\"windows_created\":false,\"blocked_query_mock\":true,\"late_return_mock\":true,\"max_isolated_groups\":1}\n",checks);smx_log_fclose989(f);return 0;}catch(...){return 1;}}
 if(wcsstr(command,L"--foreground-wait-selftest992")){try{unsigned checks=smx992::selfTest();FILE*f=smx_log_fopen989("foreground-wait992-selftest.json","wb");if(!f)return 2;smx_log_fprintf989(f,"{\"pass\":true,\"checks\":%u,\"gpu_used\":false,\"windows_created\":false,\"hosted_default_unbounded\":true,\"cli_default_seconds\":30}\n",checks);smx_log_fclose989(f);return 0;}catch(...){return 1;}}
 if(wcsstr(command,L"--window-state-selftest990")){try{unsigned checks=smx990::selfTest();FILE*f=smx_log_fopen989("window-state990-selftest.json","wb");if(!f)return 2;smx_log_fprintf989(f,"{\"pass\":true,\"checks\":%u,\"gpu_used\":false,\"window_mutations\":false}\n",checks);smx_log_fclose989(f);return 0;}catch(...){return 1;}}
 if(wcsstr(command,L"--capture-contract-selftest"))return captureContractSelftest989();
 if(wcsstr(command,L"--host-selftest")){bool pass=host988::selfTest();FILE*f=smx_log_fopen989("host-selftest988.json","wb");if(f){smx_log_fprintf989(f,"{\"pass\":%s,\"gpu_used\":false}\n",pass?"true":"false");smx_log_fclose989(f);}return pass?0:1;}
 smxdiag994::Runtime diagnostics994;
 try{hostConfig988=host988::config();}catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));return 70;}

 if(wcsstr(command,L"--hud-power-probe"))return hudPowerProbe988();
 if(wcsstr(command,L"--hud-selftest"))return hudSelftest988();
 if(wcsstr(command,L"--verify-window-config976"))return windowConfigSelftest976();
 _putenv_s("SMX_ALLOW_NUMERIC_DIFF","1");
 if(!getenv("SMX_RESOURCE_BUDGET"))_putenv_s("SMX_RESOURCE_BUDGET","0");
 if(!getenv("SMX_DIRECT_ORIGINALS"))_putenv_s("SMX_DIRECT_ORIGINALS","1");
 if(!getenv("SMX_QUEUE_PRIORITY"))_putenv_s("SMX_QUEUE_PRIORITY","high");
 if(!getenv("SMX_MM_SCHED"))_putenv_s("SMX_MM_SCHED","1");
 if(!getenv("SMX_PCORE"))_putenv_s("SMX_PCORE","1");
 if(!getenv("SMX_PRESENT_DEPTH"))_putenv_s("SMX_PRESENT_DEPTH","3");
 if(!getenv("SMX_PRESENT_PREFILL"))_putenv_s("SMX_PRESENT_PREFILL","2");
 if(!getenv("SMX_NATIVE_DOWNSCALE"))_putenv_s("SMX_NATIVE_DOWNSCALE","1");
 if(!getenv("SMX_WARP_FLAVOR"))_putenv_s("SMX_WARP_FLAVOR","native-flow");
 SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
 uiAuto964=!hostConfig988.hosted&&wcsstr(command,L"--overlay-auto")!=nullptr;
 _putenv_s("SMX_MARKER971",""); // R971 typo is never a valid input.
 if(exactOne976(getenv("SMX_MARKER967")))_putenv_s("SMX_VISUAL_PROOF964","1");
 visualProof964=visualRequested976(getenv("SMX_VISUAL_PROOF964"),getenv("SMX_MARKER967"));
 if(!parseWindowMode976(getenv("SMX_WINDOW_MODE976"),opaqueWindow976))return 65;
 font964=CreateFontW(-18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Microsoft YaHei UI");
 panel964=CreateSolidBrush(RGB(24,29,38));
 wchar_t path[32768];GetModuleFileNameW(nullptr,path,32768);baseDir=path;baseDir.resize(baseDir.find_last_of(L"\\/"));
 if(wcsstr(command,L"--verify-display-guard")){
  SetCurrentDirectoryW((baseDir+L"\\model1080").c_str());FILE*log=nullptr;smx_log_freopen_s989(&log,"display-guard-test.log","w",stderr);
  try{CaptureDisplaySession displaySession(false,baseDir);smx_log_fprintf989(stderr,"Controlled unexpected exit for restoration verification\n");fflush(stderr);Sleep(1500);TerminateProcess(GetCurrentProcess(),77);return 77;}catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));smx_log_fprintf989(stderr,"%s\n",e.what());return 1;}
 }
 if(wcsstr(command,L"--verify-gpu-1080")||wcsstr(command,L"--verify-512")||wcsstr(command,L"--verify-1080")||wcsstr(command,L"--verify-capture")){
  verifyGpuOnly=wcsstr(command,L"--verify-gpu-1080")!=nullptr;verifyOnly=!verifyGpuOnly;bool hd=verifyGpuOnly||wcsstr(command,L"--verify-1080")!=nullptr;
  std::wstring directory=baseDir+(hd?L"\\model1080":L"\\model512");if(const char*md=getenv("SMX_MODEL_DIR"))directory=std::wstring(md,md+strlen(md));
  if(!SetCurrentDirectoryW(directory.c_str()))return 6;
  FILE*log=nullptr;std::string logPath=getenv("SMX_BENCH_DIR")?std::string(getenv("SMX_BENCH_DIR"))+"\\app-verify-engine.log":"app-verify-engine.log";smx_log_freopen_s989(&log,logPath.c_str(),"w",stderr);
  char name[]="smooth-motion-xmx",runs[]="1";char standard[]="graph.bin",capture[]="cases/app-capture.bin";char*args[]={name,wcsstr(command,L"--verify-capture")?capture:(getenv("SMX_MANIFEST")?getenv("SMX_MANIFEST"):standard),runs};return smx_graph_main(3,args);
 }
 benchmarkSource=wcsstr(command,L"--capture-native-test")!=nullptr||wcsstr(command,L"--capture-forza-test")!=nullptr;
 recoveryTest809=wcsstr(command,L"--recovery-self-test809")!=nullptr;queueTest809=wcsstr(command,L"--queue-self-test809")!=nullptr;windowTest795=recoveryTest809||queueTest809||wcsstr(command,L"--windowed-self-test795")!=nullptr;selftest=windowTest795||wcsstr(command,L"--self-test")!=nullptr;demo=wcsstr(command,L"--demo")!=nullptr;
 WNDCLASSW wc{};wc.hInstance=instance;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=panel964;wc.lpszClassName=L"SMXControl";wc.lpfnWndProc=controlProc;RegisterClassW(&wc);wc.hbrBackground=nullptr;wc.lpszClassName=L"SMXOutput";wc.lpfnWndProc=outputProc;RegisterClassW(&wc);wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);wc.lpszClassName=L"SMXTest";wc.lpfnWndProc=testProc;RegisterClassW(&wc);wc.lpszClassName=L"SMXHud964";wc.lpfnWndProc=hudProc964;wc.hbrBackground=panel964;RegisterClassW(&wc);
 control=CreateWindowExW((windowTest795||hostConfig988.hosted)?(WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW):0,L"SMXControl",L"Smooth Motion R988  |  Intel XMX 游戏叠层",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|((benchmarkSource||hostConfig988.hosted)?0:WS_VISIBLE),60,60,800,410,nullptr,nullptr,instance,nullptr);
 recordWindow976(control,"control","created");
 CreateWindowW(L"STATIC",L"游戏窗口",WS_CHILD|WS_VISIBLE,18,16,120,22,control,nullptr,instance,nullptr);
 combo=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL,18,44,590,260,control,nullptr,instance,nullptr);
 CreateWindowW(L"BUTTON",L"刷新",WS_CHILD|WS_VISIBLE,620,44,100,28,control,(HMENU)101,instance,nullptr);
 sizeCombo=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,18,88,220,120,control,nullptr,instance,nullptr);
 SendMessageW(sizeCombo,CB_ADDSTRING,0,(LPARAM)L"跟随游戏原生尺寸");SendMessageW(sizeCombo,CB_SETCURSEL,0,0);EnableWindow(sizeCombo,FALSE);
 startButton=CreateWindowW(L"BUTTON",L"开始插帧",WS_CHILD|WS_VISIBLE,260,88,100,28,control,(HMENU)102,instance,nullptr);
 stopButton=CreateWindowW(L"BUTTON",L"停止",WS_CHILD|WS_VISIBLE|WS_DISABLED,375,88,100,28,control,(HMENU)103,instance,nullptr);
 fullButton=CreateWindowW(L"BUTTON",L"显示 / 隐藏状态条",WS_CHILD|WS_VISIBLE,490,88,170,28,control,(HMENU)104,instance,nullptr);
 refreshModeCheck=CreateWindowW(L"BUTTON",L"保留现有桌面分辨率、刷新率和 VRR",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,18,126,705,24,control,(HMENU)105,instance,nullptr);
 SendMessageW(refreshModeCheck,BM_SETCHECK,BST_CHECKED,0);EnableWindow(refreshModeCheck,FALSE);
 overlayCheck=CreateWindowW(L"BUTTON",opaqueWindow976?L"不透明诊断叠层：鼠标输入穿透尚未验证":L"覆盖游戏内容区，鼠标与键盘输入穿透到游戏",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,18,154,705,24,control,(HMENU)106,instance,nullptr);
 SendMessageW(overlayCheck,BM_SETCHECK,BST_CHECKED,0);
 statusText=CreateWindowW(L"STATIC",L"选择游戏后点击「开始插帧」，准备完成将自动显示叠层。\nCtrl+Alt+Esc 停止  ·  Ctrl+Alt+F8 状态条\nCtrl+Alt+Home 返回此窗口  ·  时间线缓冲 80 ms",WS_CHILD|WS_VISIBLE,18,190,705,110,control,nullptr,instance,nullptr);
 CreateWindowW(L"BUTTON",L"返回游戏",WS_CHILD|WS_VISIBLE,18,310,140,32,control,(HMENU)108,instance,nullptr);
 EnumChildWindows(control,[](HWND child,LPARAM)->BOOL{SendMessageW(child,WM_SETFONT,(WPARAM)font964,TRUE);return TRUE;},0);
 if(selftest||demo){sourceTest=CreateWindowExW(windowTest795?WS_EX_NOACTIVATE:0,L"SMXTest",L"SMX controlled moving-pattern capture",WS_POPUP|WS_VISIBLE,1040,80,getenv("SMX_TEST_WIDTH831")?atoi(getenv("SMX_TEST_WIDTH831")):(recoveryTest809||queueTest809)?1280:windowTest795?640:512,getenv("SMX_TEST_HEIGHT831")?atoi(getenv("SMX_TEST_HEIGHT831")):(recoveryTest809||queueTest809)?720:windowTest795?360:512,nullptr,nullptr,instance,nullptr);SetTimer(sourceTest,1,30,nullptr);RedrawWindow(sourceTest,nullptr,nullptr,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);UpdateWindow(sourceTest);}
 if(benchmarkSource&&wcsstr(command,L"--capture-forza-test")){HWND game=FindWindowW(nullptr,L"Forza Horizon 6");if(game&&IsIconic(game)){ShowWindowAsync(game,SW_RESTORE);for(unsigned k=0;k<20&&IsIconic(game);k++)Sleep(50);}}
 refresh();if(selftest||demo){for(size_t i=0;i<windows.size();i++)if(windows[i]==sourceTest)SendMessageW(combo,CB_SETCURSEL,i,0);launch();}
 if(benchmarkSource){HWND test=wcsstr(command,L"--capture-forza-test")?FindWindowW(nullptr,L"Forza Horizon 6"):FindWindowW(L"SMXNativeSource",nullptr);bool found=false;for(size_t i=0;i<windows.size();i++)if(windows[i]==test){SendMessageW(combo,CB_SETCURSEL,i,0);found=true;break;}if(found){closePending=true;launch();}else{const char*dir=getenv("SMX_BENCH_DIR");if(dir){std::string path=std::string(dir)+"\\app-startup851.txt";FILE*f=smx_log_fopen989(path.c_str(),"wb");if(f){smx_log_fprintf989(f,"Source selection failed; hwnd=%llu visible=%u minimized=%u enumerated=%zu\n",(unsigned long long)(UINT_PTR)test,unsigned(IsWindowVisible(test)),unsigned(IsIconic(test)),windows.size());smx_log_fclose989(f);}}DestroyWindow(control);return 7;}}
 if(hostConfig988.hosted){
  hostEmergencyRegistered988=RegisterHotKey(control,2,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,VK_ESCAPE)!=FALSE;
  try{hostServer988=std::make_unique<host988::Server>(hostConfig988,control);}catch(const std::exception&e){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,1,SMX_DIAG_SEMANTIC994,smxdiag994::fields(0,0,0,0,0,__LINE__,1));FILE*f=smx_log_fopen989("host-startup988.json","wb");if(f){smx_log_fprintf989(f,"{\"ok\":false,\"error\":%s}\n",host988::quote(e.what()).c_str());smx_log_fclose989(f);}DestroyWindow(control);return 71;}
 }else{RegisterHotKey(control,1,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,VK_F11);RegisterHotKey(control,2,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,VK_ESCAPE);RegisterHotKey(control,3,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,VK_F8);RegisterHotKey(control,4,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,VK_HOME);RegisterHotKey(control,5,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,VK_F9);}
 if(uiAuto964&&!benchmarkSource)PostMessageW(control,WM_COMMAND,102,0);
 MSG message;while(GetMessageW(&message,nullptr,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
 if(hostServer988)hostServer988->stop();stopFlag=true;{std::lock_guard lock(engineMutex783);quit783=true;engineCv783.notify_one();}if(worker.joinable())worker.join();hostServer988.reset();if(sourceTest)DestroyWindow(sourceTest);if(hud964)DestroyWindow(hud964);if(output)DestroyWindow(output);DeleteObject(font964);DeleteObject(panel964);if(selftest&&generated.load()==0&&engineExit989.load()==0)return 8;return (selftest||benchmarkSource||demo)?engineExit989.load():0;
}
