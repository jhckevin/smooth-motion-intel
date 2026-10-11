#include "bounded_stdio989.h"
#pragma once
#include <dxgi1_2.h>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <functional>
// Genuine DXGI desktop capture, not a fallback alias for WGC. One monitor,
// identity rotation, foreground unobscured target only. Desktop timestamps do
// not identify which game Present caused a composed update.
class DesktopDuplication989 {
 struct State {
  winrt::com_ptr<IDXGIOutputDuplication> duplication;std::mutex mutex;std::condition_variable cv;
  bool acquired=false;std::atomic<bool>stop{false};std::atomic<HRESULT>error{S_OK};
  std::atomic<uint64_t>acquires{0},releases{0},pointerOnly{0},unchanged{0},blocked{0},delivered{0};
  void release()noexcept{std::lock_guard lock(mutex);if(!acquired)return;HRESULT hr=SMX_DIAG_CALL994(SMX_DIAG_CAPTURE_RELEASE994,smxdiag994::fields(0,0,(uintptr_t)duplication.get(),0,0,0,2),SMX_DIAG_HRESULT994,duplication->ReleaseFrame());acquired=false;if(FAILED(hr))error=hr;++releases;cv.notify_all();}
 };
 std::shared_ptr<State> state_=std::make_shared<State>();std::thread thread_;HANDLE done_=nullptr;
 HWND target_=nullptr,output_=nullptr,hud_=nullptr;HMONITOR monitor_=nullptr;RECT rect_{};DXGI_OUTDUPL_DESC desc_{};
 std::string lastBlockReason_;HWND lastBlockWindow_=nullptr;
 bool blockReason989(const char*reason,HWND window){if(lastBlockReason_!=reason||lastBlockWindow_!=window){lastBlockReason_=reason;lastBlockWindow_=window;RECT r{};GetWindowRect(window,&r);DWORD pid=0;GetWindowThreadProcessId(window,&pid);LARGE_INTEGER q{},frequency{};QueryPerformanceCounter(&q);QueryPerformanceFrequency(&frequency);FILE*f=smx_log_fopen989("app-capture-obstruction989.jsonl","ab");if(f){smx_log_fprintf989(f,"{\"reason\":\"%s\",\"qpc_seconds\":%.9f,\"hwnd\":%llu,\"pid\":%lu,\"style\":%llu,\"ex_style\":%llu,\"left\":%ld,\"top\":%ld,\"right\":%ld,\"bottom\":%ld}\n",reason,double(q.QuadPart)/frequency.QuadPart,(unsigned long long)(uintptr_t)window,pid,(unsigned long long)GetWindowLongPtrW(window,GWL_STYLE),(unsigned long long)GetWindowLongPtrW(window,GWL_EXSTYLE),r.left,r.top,r.right,r.bottom);smx_log_fclose989(f);}}return strcmp(reason,"clear")!=0;}
 std::function<void(CaptureFrame949)> callback_;std::function<void(HRESULT,const char*)> failure_;std::function<void(bool)> pause_;
 bool obscured(){
  if(!IsWindow(target_))return blockReason989("target_closed",target_);if(IsIconic(target_))return blockReason989("target_minimized",target_);HWND foreground=GetAncestor(GetForegroundWindow(),GA_ROOT);if(foreground!=target_)return blockReason989("target_not_foreground",foreground);
  RECT client{};POINT origin{};if(!GetClientRect(target_,&client)||!ClientToScreen(target_,&origin))return blockReason989("target_geometry",target_);OffsetRect(&client,origin.x,origin.y);
  unsigned count=0;for(HWND w=GetTopWindow(nullptr);w&&w!=target_&&count++<512;w=GetWindow(w,GW_HWNDNEXT)){
   if(w==output_||w==hud_||!IsWindowVisible(w)||IsIconic(w))continue;DWORD ownerPid=0,affinity=0;GetWindowThreadProcessId(w,&ownerPid);if(ownerPid==GetCurrentProcessId()&&GetWindowDisplayAffinity(w,&affinity)&&affinity==0x11)continue;DWORD cloaked=0;if(SUCCEEDED(DwmGetWindowAttribute(w,DWMWA_CLOAKED,&cloaked,sizeof cloaked))&&cloaked)continue;
   RECT window{},overlap{};if(GetWindowRect(w,&window)&&IntersectRect(&overlap,&client,&window))return blockReason989("foreign_overlap",w);
  }
  return count>=512?blockReason989("zorder_scan_limit",nullptr):blockReason989("clear",nullptr);
 }
 void record(){FILE*f=smx_log_fopen989("app-capture-backend989.json","wb");if(f){smx_log_fprintf989(f,"{\"backend\":\"dxgi_desktop_duplication\",\"monitor_left\":%ld,\"monitor_top\":%ld,\"monitor_width\":%ld,\"monitor_height\":%ld,\"acquired\":%llu,\"released\":%llu,\"pointer_only_skipped\":%llu,\"unchanged_timestamp_skipped\":%llu,\"blocked_polls\":%llu,\"delivered_callbacks\":%llu,\"first_frame_timeout_seconds\":10,\"error_hr\":%ld,\"source_timestamp_scope\":\"desktop LastPresentTime, not identified game Present\",\"own_overlay_exclusion_required\":true,\"gpu_copy_fence_before_release\":true,\"fallback\":false}\n",rect_.left,rect_.top,rect_.right-rect_.left,rect_.bottom-rect_.top,(unsigned long long)state_->acquires.load(),(unsigned long long)state_->releases.load(),(unsigned long long)state_->pointerOnly.load(),(unsigned long long)state_->unchanged.load(),(unsigned long long)state_->blocked.load(),(unsigned long long)state_->delivered.load(),long(state_->error.load()));smx_log_fclose989(f);}}
public:
 DesktopDuplication989(ID3D11Device*device,HWND target,HWND output,HWND hud):target_(target),output_(output),hud_(hud){
  for(HWND w:{output,hud})if(w){DWORD affinity=0;if(!GetWindowDisplayAffinity(w,&affinity)||affinity!=0x11)throw std::runtime_error("DXGI capture requires capture exclusion on own output and HUD; visual-proof mode is incompatible");}
  monitor_=MonitorFromWindow(target,MONITOR_DEFAULTTONULL);if(!monitor_)throw std::runtime_error("DXGI target has no monitor");
  winrt::com_ptr<IDXGIDevice> dxgi;winrt::check_hresult(device->QueryInterface(IID_PPV_ARGS(dxgi.put())));winrt::com_ptr<IDXGIAdapter>adapter;winrt::check_hresult(dxgi->GetAdapter(adapter.put()));winrt::com_ptr<IDXGIOutput1> chosen;
  for(UINT i=0;;++i){winrt::com_ptr<IDXGIOutput> out;HRESULT hr=adapter->EnumOutputs(i,out.put());if(hr==DXGI_ERROR_NOT_FOUND)break;winrt::check_hresult(hr);DXGI_OUTPUT_DESC od{};winrt::check_hresult(out->GetDesc(&od));if(od.Monitor==monitor_){chosen=out.as<IDXGIOutput1>();rect_=od.DesktopCoordinates;break;}}
  if(!chosen)throw std::runtime_error("DXGI target monitor is not on capture GPU; cross-adapter capture is unsupported");
  winrt::check_hresult(chosen->DuplicateOutput(device,state_->duplication.put()));state_->duplication->GetDesc(&desc_);
  if(desc_.Rotation!=DXGI_MODE_ROTATION_IDENTITY||desc_.ModeDesc.Format!=DXGI_FORMAT_B8G8R8A8_UNORM)throw std::runtime_error("DXGI monitor rotation/format is unsupported; no automatic fallback");
  done_=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!done_)throw std::runtime_error("DXGI capture stop event failed");record();
 }
 winrt::Windows::Graphics::SizeInt32 size()const{return {int(desc_.ModeDesc.Width),int(desc_.ModeDesc.Height)};}
 RECT captureRect()const{return rect_;}
 void start(std::function<void(CaptureFrame949)> callback,std::function<void(HRESULT,const char*)> failure,std::function<void(bool)> pause,std::function<void()> targetClosed){
  if(thread_.joinable())throw std::runtime_error("DXGI capture already started");callback_=std::move(callback);failure_=std::move(failure);pause_=std::move(pause);
  thread_=std::thread([this,targetClosed=std::move(targetClosed)]{
   bool apartmentReady=false,unownedAcquired=false;LARGE_INTEGER frequency{};QueryPerformanceFrequency(&frequency);LONGLONG previous=0;ULONGLONG lastGuard=0,firstFrameStart=GetTickCount64();bool blocked=true;
   try{winrt::init_apartment(winrt::apartment_type::multi_threaded);apartmentReady=true;while(!state_->stop){
    if(!state_->delivered.load()&&GetTickCount64()-firstFrameStart>=10000)throw std::runtime_error("DXGI startup received no usable desktop frame within 10 seconds; inspect app-capture-obstruction989.jsonl; no retry or fallback");
    if(FAILED(state_->error.load()))winrt::throw_hresult(state_->error.load());
    if(!IsWindow(target_)){targetClosed();break;}if(IsIconic(target_)||!IsWindowVisible(target_)){pause_(true);++state_->blocked;std::unique_lock lock(state_->mutex);state_->cv.wait_for(lock,std::chrono::milliseconds(25),[&]{return state_->stop.load();});continue;}
    RECT client{};POINT origin{};if(!GetClientRect(target_,&client)||!ClientToScreen(target_,&origin))throw std::runtime_error("DXGI target geometry unavailable");
    if(MonitorFromWindow(target_,MONITOR_DEFAULTTONULL)!=monitor_||!smx989::clientInsideMonitor(rect_,origin,client.right,client.bottom))throw std::runtime_error("DXGI target moved outside its original monitor; stop and choose the target again");
    ULONGLONG now=GetTickCount64();if(now-lastGuard>=100){blocked=obscured();pause_(blocked);lastGuard=now;}
    if(blocked){++state_->blocked;std::unique_lock lock(state_->mutex);state_->cv.wait_for(lock,std::chrono::milliseconds(25),[&]{return state_->stop.load();});continue;}
    {std::unique_lock lock(state_->mutex);state_->cv.wait_for(lock,std::chrono::milliseconds(25),[&]{return state_->stop.load()||!state_->acquired;});if(state_->stop)break;if(state_->acquired)continue;}
    DXGI_OUTDUPL_FRAME_INFO info{};winrt::com_ptr<IDXGIResource> resource;HRESULT hr=SMX_DIAG_CALL994(SMX_DIAG_CAPTURE_ACQUIRE994,smxdiag994::fields(0,0,(uintptr_t)state_->duplication.get(),0,0,25,2),SMX_DIAG_HRESULT994,state_->duplication->AcquireNextFrame(25,&info,resource.put()));if(hr==DXGI_ERROR_WAIT_TIMEOUT)continue;winrt::check_hresult(hr);
    {std::lock_guard lock(state_->mutex);state_->acquired=true;}unownedAcquired=true;++state_->acquires;
    if(!info.LastPresentTime.QuadPart){++state_->pointerOnly;state_->release();unownedAcquired=false;continue;}if(info.LastPresentTime.QuadPart<=previous){++state_->unchanged;state_->release();unownedAcquired=false;continue;}previous=info.LastPresentTime.QuadPart;
    auto texture=resource.as<ID3D11Texture2D>();auto state=state_;auto stamp=winrt::Windows::Foundation::TimeSpan{LONGLONG((long double)info.LastPresentTime.QuadPart*10000000/frequency.QuadPart)};
    CaptureFrame949 frame(std::move(texture),size(),stamp,[state]{state->release();});unownedAcquired=false;callback_(std::move(frame));++state_->delivered;
   }}catch(const winrt::hresult_error&e){state_->error=e.code();failure_(e.code(),"DXGI capture lost access/device or acquisition failed; no fallback");}catch(const std::exception&e){state_->error=E_FAIL;failure_(E_FAIL,e.what());}catch(...){state_->error=E_FAIL;failure_(E_FAIL,"Unknown DXGI capture failure");}
   if(unownedAcquired)state_->release();pause_(false);if(apartmentReady)winrt::uninit_apartment();SetEvent(done_);
  });
 }
 void close(){state_->stop=true;state_->cv.notify_all();if(thread_.joinable()){if(WaitForSingleObject(done_,5000)!=WAIT_OBJECT_0)smxBridgeFailure555("DXGI capture thread did not stop; preserve pending GPU leases",WAIT_TIMEOUT);thread_.join();}record();}
 ~DesktopDuplication989(){close();if(done_)CloseHandle(done_);}
};
