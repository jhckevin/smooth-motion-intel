#include "../d3d_adapter995.hpp"
#include "fp16-next537/candidate994/bounded_stdio989.h"
#pragma once
#include "gpu_graph994.hpp"
#include <cstring>
#include "media_scheduler.hpp"
#include "frame_quality_named.h"
#include <d3d11_4.h>
#include <thread>
#include <atomic>
#include <array>
inline int verifyGpuPipelined(void*frames,const void*result,uint32_t W,uint32_t H,void*graph,double(*run)(void*)){
 FrameScheduling scheduling;
 const size_t bytes=size_t(W)*H*4;
 std::vector<unsigned char>original((unsigned char*)frames,(unsigned char*)frames+bytes*2),expected((const unsigned char*)result,(const unsigned char*)result+bytes),bgra(original.size());
 std::vector<unsigned char>reverse(bytes);
 memcpy(frames,original.data()+bytes,bytes);memcpy((unsigned char*)frames+bytes,original.data(),bytes);run(graph);memcpy(reverse.data(),result,bytes);
 auto check=[](HRESULT h){if(FAILED(h))throw std::runtime_error("GPU bridge validation D3D error");};
 ComPtr<ID3D11Device>d;ComPtr<ID3D11DeviceContext>c;D3D_FEATURE_LEVEL level;
 check(D3D11CreateDevice(smxD3DAdapter995().Get(),D3D_DRIVER_TYPE_UNKNOWN,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,nullptr,0,D3D11_SDK_VERSION,&d,&level,&c));
 GpuGraphBridge bridge(d.Get(),c.Get());
 for(size_t i=0;i<bytes*2;i+=4){bgra[i]=original[i+2];bgra[i+1]=original[i+1];bgra[i+2]=original[i];bgra[i+3]=original[i+3];}
 D3D11_TEXTURE2D_DESC td{};td.Width=W;td.Height=H;td.MipLevels=1;td.ArraySize=1;td.Format=DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count=1;td.Usage=D3D11_USAGE_DEFAULT;if(getenv("SMX_SOURCE_SRV"))td.BindFlags=D3D11_BIND_SHADER_RESOURCE;
 ComPtr<ID3D11Texture2D>sources[2];
 for(unsigned i=0;i<2;i++){D3D11_SUBRESOURCE_DATA init{bgra.data()+i*bytes,W*4,0};if(smxDirectEnabled()){sources[i]=bridge.makeOriginalTexture();ComPtr<IDXGIKeyedMutex> m;check(sources[i].As(&m));BridgeLock lock(m.Get());c->UpdateSubresource(sources[i].Get(),0,nullptr,init.pSysMem,init.SysMemPitch,0);c->Flush();}else check(d->CreateTexture2D(&td,&init,&sources[i]));}
 std::array<ComPtr<ID3D11Texture2D>,4> firstPool,middlePool;
 for(unsigned i=0;i<4;i++){firstPool[i]=bridge.makePresentationTexture();middlePool[i]=bridge.makeOutputTexture();}
 auto first=firstPool[0],middle=middlePool[0];
 memset(frames,0,bytes*2);memset(const_cast<void*>(result),0,bytes);

 auto runArm=[&]()->int {
 double gpuSum=0,wallSum=0;unsigned runs=30;const char*env=getenv("SMX_BRIDGE_RUNS");if(env)runs=std::clamp((unsigned)strtoul(env,nullptr,10),1u,10000u);
 double bridgeSeconds=getenv("SMX_BRIDGE_SECONDS")?std::clamp(atof(getenv("SMX_BRIDGE_SECONDS")),3.,120.):0.;unsigned plannedRuns=runs,actualRuns=0;
 BridgeHandle timer;timer.value=CreateWaitableTimerExW(nullptr,nullptr,2,TIMER_ALL_ACCESS);if(!timer.value)throw std::runtime_error("Pace timer failed");
 ComPtr<ID3D11Device5>d5;ComPtr<ID3D11DeviceContext4>c4;check(d.As(&d5));check(c.As(&c4));
 ComPtr<ID3D11Fence>done;check(d5->CreateFence(0,D3D11_FENCE_FLAG_NONE,IID_PPV_ARGS(&done)));
 BridgeHandle completed;completed.value=CreateEventW(nullptr,FALSE,FALSE,nullptr);if(!completed.value)throw std::runtime_error("D3D completion event failed");uint64_t completedValue=0;
 if(!smxDirectEnabled())bridge.pushFrame(sources[0].Get(),W,H);
 struct Timing{double gpu,start,push,infer,submit,lock,copy;};
 std::vector<Timing>timings(runs);std::vector<double>completionTimes(runs);
 BridgeHandle recycle;recycle.value=CreateEventW(nullptr,FALSE,FALSE,nullptr);if(!recycle.value)throw std::runtime_error("Recycle event failed");
 std::atomic<HRESULT>observerError{S_OK};
 LARGE_INTEGER absoluteQpc{},absoluteFrequency{};QueryPerformanceCounter(&absoluteQpc);QueryPerformanceFrequency(&absoluteFrequency);double epochQpc=double(absoluteQpc.QuadPart)/absoluteFrequency.QuadPart;
 double bridgeHz=getenv("SMX_BRIDGE_HZ")?std::clamp(atof(getenv("SMX_BRIDGE_HZ")),15.,120.):60.;
 auto epoch=std::chrono::steady_clock::now();double lastComplete=0;
 auto since=[&](){return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-epoch).count();};
 std::jthread observer([&](std::stop_token stop){
  for(unsigned i=0;i<plannedRuns&&!stop.stop_requested();i++){
   HRESULT hr=done->SetEventOnCompletion(uint64_t(i)+1,completed.value);if(FAILED(hr)){observerError=hr;return;}
   auto limit=std::chrono::steady_clock::now()+std::chrono::seconds(10);
   for(;;){DWORD wait=WaitForSingleObject(completed.value,50);if(wait==WAIT_OBJECT_0)break;
    if(stop.stop_requested())return;
    if(wait!=WAIT_TIMEOUT||std::chrono::steady_clock::now()>=limit){observerError=E_FAIL;return;}}
   completionTimes[i]=since();
  }
 });
 for(unsigned i=0;i<plannedRuns;i++){
  if((i&15u)==0u&&getenv("SMX_COMPARE_DIR")){std::string stopPath=std::string(getenv("SMX_COMPARE_DIR"))+"\\STOP";if(GetFileAttributesA(stopPath.c_str())!=INVALID_FILE_ATTRIBUTES)throw std::runtime_error("Comparison STOP requested");}
  if(bridgeSeconds>0.0&&i>0&&since()>=bridgeSeconds*1000.0)break;
  auto scheduled=epoch+std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(double(i)/bridgeHz));
  auto remain=scheduled-std::chrono::steady_clock::now();
  if(remain>std::chrono::steady_clock::duration::zero()){
   LARGE_INTEGER due;due.QuadPart=-std::max<LONGLONG>(1,std::chrono::duration_cast<std::chrono::nanoseconds>(remain).count()/100);
   if(!SetWaitableTimer(timer.value,&due,0,nullptr,nullptr,FALSE)||WaitForSingleObject(timer.value,INFINITE)!=WAIT_OBJECT_0)throw std::runtime_error("Pace timer wait failed");
  }
  check(observerError.load());auto&t=timings[i];t.start=since();
  if(i>=4&&done->GetCompletedValue()<uint64_t(i-3)){
   check(done->SetEventOnCompletion(uint64_t(i-3),recycle.value));
   if(WaitForSingleObject(recycle.value,5000)!=WAIT_OBJECT_0)throw std::runtime_error("Ring recycle timeout");
  }
  first=firstPool[i%4];middle=middlePool[i%4];
  if(smxDirectEnabled())bridge.setDirectPair(sources[i%2].Get(),sources[(i+1)%2].Get());
  else if(!bridge.pushFrame(sources[(i+1)%2].Get(),W,H))throw std::runtime_error("Pair not ready");
  if(smxDirectOutputEnabled())bridge.setDirectOutput(middle.Get());
  t.push=since();t.gpu=bridge.infer();gpuSum+=t.gpu;t.infer=since();
  first=sources[i%2];bridge.copyMiddle(middle.Get());t.lock=bridge.outputLockMs;t.copy=bridge.outputSubmitMs;check(c4->Signal(done.Get(),++completedValue));c->Flush();t.submit=since();actualRuns=i+1;
 }
 if(bridgeSeconds>0.0){check(done->SetEventOnCompletion(completedValue,recycle.value));if(WaitForSingleObject(recycle.value,5000)!=WAIT_OBJECT_0)throw std::runtime_error("Final GPU completion timeout");double finalComplete=since();observer.request_stop();observer.join();check(observerError.load());runs=actualRuns;timings.resize(runs);completionTimes.resize(runs);if(completionTimes.back()==0.0)completionTimes.back()=finalComplete;}else{observer.join();check(observerError.load());}lastComplete=completionTimes.back();
 printf("{\"epoch_qpc\":%.9f,",epochQpc);printf("\"scope\":\"bounded four-slot GPU transport; CPU observed D3D fence completion; no WGC; includes six diagnostic WSI copies/readbacks/presents, not physical display confirmation\",\"source_shader_readable\":%s,\"width\":%u,\"height\":%u,\"source_hz\":%.6f,\"ring_slots\":4,\"runs\":[",getenv("SMX_SOURCE_SRV")?"true":"false",W,H,bridgeHz);
 for(unsigned i=0;i<runs;i++){const auto&t=timings[i];double wall=completionTimes[i]-t.start;wallSum+=wall;
  printf("%s{\"gpu_ms\":%.6f,\"wall_ms\":%.6f,\"submit_wall_ms\":%.6f,\"start_ms\":%.6f,\"submit_ms\":%.6f,\"complete_ms\":%.6f,\"push_ms\":%.6f,\"infer_wait_ms\":%.6f,\"copy_submit_ms\":%.6f}",i?",\n":"",t.gpu,wall,t.submit-t.start,t.start,t.submit,completionTimes[i],t.push-t.start,t.infer-t.push,t.submit-t.infer);
 }
 printf("],");double lockMs=0,copyMs=0;for(const auto&t:timings){lockMs+=t.lock;copyMs+=t.copy;}printf("\"direct_output\":%s,\"mean_output_lock_ms\":%.6f,\"mean_output_submit_ms\":%.6f,",smxDirectOutputEnabled()?"true":"false",lockMs/runs,copyMs/runs);
 size_t inputErrors=0;for(size_t i=0;i<original.size();i++)inputErrors+=((unsigned char*)frames)[i]!=0;
 td.Format=DXGI_FORMAT_B8G8R8A8_UNORM;td.BindFlags=0;td.Usage=D3D11_USAGE_STAGING;td.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
 ComPtr<ID3D11Texture2D>staging;check(d->CreateTexture2D(&td,nullptr,&staging));
 std::vector<unsigned char>observed(bytes);
 auto compare=[&](ID3D11Texture2D*texture,const unsigned char*reference){
  auto nativeRead948=bridge.access948(texture,c.Get());ComPtr<IDXGIKeyedMutex>m;std::unique_ptr<BridgeLock>lock;if(!nativeRead948&&SUCCEEDED(texture->QueryInterface(IID_PPV_ARGS(&m))))lock=std::make_unique<BridgeLock>(m.Get());c->CopyResource(staging.Get(),texture);D3D11_MAPPED_SUBRESOURCE map{};check(c->Map(staging.Get(),0,D3D11_MAP_READ,0,&map));size_t errors=0;for(uint32_t y=0;y<H;y++)memcpy(observed.data()+size_t(y)*W*4,(unsigned char*)map.pData+size_t(y)*map.RowPitch,W*4);
  for(size_t i=0;i<bytes;i+=4)std::swap(observed[i],observed[i+2]);for(size_t i=0;i<bytes;i++)errors+=observed[i]!=reference[i];
  c->Unmap(staging.Get(),0);if(nativeRead948){nativeRead948->signal();c->Flush();}return errors;
 };
 size_t firstErrors=compare(first.Get(),original.data()+((runs+1)%2)*bytes),middleErrors=compare(middle.Get(),runs%2?expected.data():reverse.data());
 double mse=0,mae=0;uint64_t alpha=0;const auto&ref=runs%2?expected:reverse;for(size_t i=0;i<bytes;i++){int diff=int(observed[i])-int(ref[i]);if((i&3)==3)alpha+=diff!=0;else{mse+=diff*diff;mae+=abs(diff);}}mse/=(bytes/4)*3;mae/=(bytes/4)*3;
 printf("\"quality\":");print_frame_quality(observed.data(),ref.data(),W,H,"selected backend buffer-fed output");printf(",");
 if(getenv("SMX_DUMP_FRAME")){std::string dumpPath=getenv("SMX_BENCH_DIR")?std::string(getenv("SMX_BENCH_DIR"))+"\\sampled-generated.rgba":"sampled-generated.rgba";FILE*f=smx_log_fopen989(dumpPath.c_str(),"wb");if(f){fwrite(observed.data(),1,bytes,f);smx_log_fclose989(f);}}
 
 size_t asyncErrors905=0;
 if(getenv("SMX_PIPELINE905")){
  // Capture serial texture-backed outputs for both orientations, then check two simultaneously queued jobs twice.
  std::vector<unsigned char> serial905[2];
  for(unsigned s=0;s<2;s++){bridge.setDirectPair(sources[s].Get(),sources[1-s].Get());bridge.setDirectOutput(middlePool[s].Get());bridge.infer();compare(middlePool[s].Get(),expected.data());serial905[s]=observed;}
  for(unsigned round=0;round<3;round++){
   for(unsigned s=0;s<2;s++){unsigned orientation=(s+round)%2;bridge.setDirectPair(sources[orientation].Get(),sources[1-orientation].Get());bridge.setDirectOutput(middlePool[2+s].Get());bridge.setPairKeys943((round==2&&s==1?100:0)+orientation+1,2-orientation);bridge.submit905(s);}
   for(unsigned s=0;s<2;s++){bridge.finish905(s);asyncErrors905+=compare(middlePool[2+s].Get(),serial905[(s+round)%2].data());}
  }
  printf("\"two_job_async_byte_mismatches\":%llu,",(unsigned long long)asyncErrors905);
  printf("\"snapshot_reuse943_hits\":%llu,",(unsigned long long)bridge.reuseCount943());if(bridge.reuseCount943()<2)throw std::runtime_error("Snapshot reuse verification path was not exercised");
  printf("\"snapshot_copies944\":%llu,\"all_snapshot_layouts944\":%s,",(unsigned long long)bridge.copyCount943(),bridge.allSnapshotLayouts944()?"true":"false");if(bridge.copyCount943()+bridge.reuseCount943()!=12||!bridge.allSnapshotLayouts944())throw std::runtime_error("Snapshot copy/layout transition accounting failed");

  // Exercise the new resources, not just the legacy serial oracle above.
  bridge.enableNativeSync948();
  ComPtr<ID3D11Texture2D> nativeIn948[2],nativeOut948[2];
  for(unsigned k=0;k<2;k++){
   nativeIn948[k]=bridge.makeOriginalTexture();nativeOut948[k]=bridge.makeOutputTexture();
   ComPtr<IDXGIKeyedMutex> sourceMutex;check(sources[k].As(&sourceMutex));
   BridgeLock sourceLock(sourceMutex.Get());
   auto access=bridge.access948(nativeIn948[k].Get(),c.Get());
   if(!access)throw std::runtime_error("R948 verification failed to allocate native input");
   c->CopyResource(nativeIn948[k].Get(),sources[k].Get());access->signal();c->Flush();
  }
  WNDCLASSW wsiClass958{};wsiClass958.hInstance=GetModuleHandleW(nullptr);wsiClass958.lpfnWndProc=DefWindowProcW;wsiClass958.lpszClassName=L"SMXModelWsi959Verify";RegisterClassW(&wsiClass958);
  HWND wsiWindow958=CreateWindowExW(WS_EX_LAYERED|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW|WS_EX_TRANSPARENT,wsiClass958.lpszClassName,L"SMX native Vulkan output verification",WS_POPUP,20,20,W,H,nullptr,nullptr,wsiClass958.hInstance,nullptr);if(!wsiWindow958)throw std::runtime_error("WSI verify window");SetLayeredWindowAttributes(wsiWindow958,0,255,LWA_ALPHA);ShowWindow(wsiWindow958,SW_SHOWNOACTIVATE);bridge.startWSI958(wsiWindow958,true);size_t wsiErrors958=0;
  size_t nativeErrors948=0;uint64_t reuseBefore948=bridge.reuseCount943(),copyBefore948=bridge.copyCount943();
  for(unsigned round=0;round<3;round++){
   for(unsigned k=0;k<2;k++){
    unsigned orientation=(k+round)%2;
    bridge.setDirectPair(nativeIn948[orientation].Get(),nativeIn948[1-orientation].Get());
    bridge.setDirectOutput(nativeOut948[k].Get());
    bridge.setPairKeys943(1000+(round==2&&k==1?100:0)+orientation+1,1000+2-orientation);
    bridge.submit905(k);
   }
   for(unsigned k=0;k<2;k++){bridge.prepareWSI958(nativeOut948[k].Get(),{});auto pix958=bridge.pixelsWSI958();const auto&ref958=serial905[(k+round)%2];for(size_t i=0;i<bytes;i++){size_t c958=i%4;size_t ix958=c958==0?i+2:c958==2?i-2:i;wsiErrors958+=pix958[ix958]!=ref958[i];}bridge.presentWSI958();bridge.finish905(k);nativeErrors948+=compare(nativeOut948[k].Get(),serial905[(k+round)%2].data());MSG wm958;while(PeekMessageW(&wm958,nullptr,0,0,PM_REMOVE)){TranslateMessage(&wm958);DispatchMessageW(&wm958);}}
  }
  bridge.drainWSI958();bridge.stopWSI958();DestroyWindow(wsiWindow958);UnregisterClassW(wsiClass958.lpszClassName,wsiClass958.hInstance);
  printf("\"wsi958_byte_mismatches\":%llu,\"wsi958_checked_frames\":6,",(unsigned long long)wsiErrors958);
  asyncErrors905+=nativeErrors948+wsiErrors958;
  uint64_t nativeReuse948=bridge.reuseCount943()-reuseBefore948,nativeCopies948=bridge.copyCount943()-copyBefore948;
  printf("\"native948_byte_mismatches\":%llu,\"native948_resources\":%llu,\"native948_submits\":%llu,\"native948_reuse\":%llu,\"native948_copies\":%llu,",
   (unsigned long long)nativeErrors948,(unsigned long long)bridge.nativeResourceCount948(),(unsigned long long)bridge.nativeSubmitCount948(),(unsigned long long)nativeReuse948,(unsigned long long)nativeCopies948);
  if(bridge.nativeResourceCount948()!=4||bridge.nativeSubmitCount948()!=6||nativeReuse948<2||nativeCopies948+nativeReuse948!=12)
   throw std::runtime_error("R948 native transaction coverage incomplete");
 }
 bool integrity=!inputErrors&&!firstErrors&&!alpha&&!asyncErrors905;
 printf("\"copy_integrity_pass\":%s,",integrity?"true":"false");
 bool pass=!inputErrors&&!firstErrors&&!alpha&&!asyncErrors905&&mse<=1.0&&mae<=0.5;
 printf("\"mean_model_gpu_ms\":%.6f,\"mean_complete_gpu_path_ms\":%.6f,\"generated_per_second\":%.6f,\"unexpected_cpu_input_writes\":%llu,\"display_original_mismatches\":%llu,\"display_generated_mismatches\":%llu,\"pass\":%s}\n",gpuSum/runs,wallSum/runs,runs*1000/lastComplete,(unsigned long long)inputErrors,(unsigned long long)firstErrors,(unsigned long long)middleErrors,pass?"true":"false");
 return pass?0:4;
 };
 const char*controlDir=getenv("SMX_COMPARE_DIR");const char*own=getenv("SMX_COMPARE_ID");
 if(!controlDir)return runArm();
 if(!own||(strcmp(own,"int8")&&strcmp(own,"fp16")))throw std::runtime_error("Invalid comparison ID");
 std::string base(controlDir),ready=base+"\\ready-"+own+".json",stop=base+"\\STOP",command=base+"\\command.txt";
 FILE*r=smx_log_fopen989(ready.c_str(),"wb");if(!r)throw std::runtime_error("Cannot write ready");smx_log_fprintf989(r,"{\"ready\":true,\"pid\":%lu,\"graph_and_bridge_loaded\":true,\"width\":%u,\"height\":%u}\n",GetCurrentProcessId(),W,H);smx_log_fclose989(r);
 BridgeHandle mutex;mutex.value=CreateMutexA(nullptr,FALSE,getenv("SMX_COMPARE_MUTEX"));if(!mutex.value)throw std::runtime_error("Comparison mutex creation");
 unsigned last=0;auto lifetime=std::chrono::steady_clock::now();
 while(GetFileAttributesA(stop.c_str())==INVALID_FILE_ATTRIBUTES){
  if(std::chrono::duration<double>(std::chrono::steady_clock::now()-lifetime).count()>1200)throw std::runtime_error("Persistent comparison lifetime timeout");
  unsigned seq=0,seconds=0,hz=0;char target[16]={};FILE*f=smx_log_fopen989(command.c_str(),"rb");if(f){int n=fscanf(f,"%u %15s %u %u",&seq,target,&seconds,&hz);smx_log_fclose989(f);if(n!=4)seq=0;}
  if(seq<=last||strcmp(target,own)){Sleep(50);continue;}
  if(seconds<3||seconds>60||(hz!=30&&hz!=60&&hz!=120))throw std::runtime_error("Comparison arm bounds");
  if(WaitForSingleObject(mutex.value,10000)!=WAIT_OBJECT_0)throw std::runtime_error("Concurrent inference rejected");
  struct Unlock{HANDLE m;~Unlock(){ReleaseMutex(m);}} unlock{mutex.value};
  std::string tag="arm-"+std::to_string(seq)+"-"+own,dir=base+"\\"+tag;CreateDirectoryA(dir.c_str(),nullptr);
  std::string path=dir+"\\app-gpu-verification.json";FILE*report=nullptr;if(smx_log_freopen_s989(&report,path.c_str(),"w",stdout))throw std::runtime_error("Arm stdout open");
  _putenv_s("SMX_BENCH_DIR",dir.c_str());_putenv_s("SMX_BRIDGE_SECONDS",std::to_string(seconds).c_str());_putenv_s("SMX_BRIDGE_HZ",std::to_string(hz).c_str());
  int resultCode=runArm();fflush(stdout);FILE* idleLog=nullptr;if(smx_log_freopen_s989(&idleLog,"NUL","w",stdout))throw std::runtime_error("Arm report close failed");
  std::string ack=base+"\\done-"+std::to_string(seq)+"-"+own+".json";FILE*a=smx_log_fopen989(ack.c_str(),"wb");if(!a)throw std::runtime_error("Arm acknowledgment open");smx_log_fprintf989(a,"{\"sequence\":%u,\"precision\":\"%s\",\"result\":%d,\"same_graph_pipelines_reused\":true}\n",seq,own,resultCode);smx_log_fclose989(a);last=seq;
  if(resultCode)return resultCode;
 }
 return 0;

}
