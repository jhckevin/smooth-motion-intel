#pragma once
#include <presentation.h>
#include <dcomp.h>
#include <vector>
#include <deque>
#include <set>
#include <limits>
#include <cmath>
#include <stdexcept>
#include <string>
// Optional Composition Swapchain APIs must not become process-load dependencies.
// SDK 10.0.26100 dcomp.lib specifies these three ordinals; named exports are preferred.
struct OptionalCompositionApi995 {
 HMODULE module=nullptr;
 decltype(&::CreatePresentationFactory) createFactory=nullptr;
 decltype(&::DCompositionGetStatistics) statistics=nullptr;
 decltype(&::DCompositionGetTargetStatistics) targetStatistics=nullptr;
 static constexpr unsigned missingMask(bool factory,bool stats,bool target) noexcept {
  return (factory?0u:1u)|(stats?0u:2u)|(target?0u:4u);
 }
 template<class T>T resolve(const char*name,WORD ordinal){
  FARPROC p=GetProcAddress(module,name);
  if(!p)p=GetProcAddress(module,MAKEINTRESOURCEA(ordinal));
  return reinterpret_cast<T>(p);
 }
 OptionalCompositionApi995(){
  module=LoadLibraryExW(L"dcomp.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
  if(!module){DWORD error=GetLastError();throw std::runtime_error("Optional scheduled presentation unavailable: system32 dcomp.dll load failed; Win32="+std::to_string(error));}
  createFactory=resolve<decltype(createFactory)>("CreatePresentationFactory",2005);
  statistics=resolve<decltype(statistics)>("DCompositionGetStatistics",1101);
  targetStatistics=resolve<decltype(targetStatistics)>("DCompositionGetTargetStatistics",1102);
  unsigned missing=missingMask(createFactory!=nullptr,statistics!=nullptr,targetStatistics!=nullptr);
  if(missing){FreeLibrary(module);module=nullptr;
   throw std::runtime_error("Optional scheduled presentation unavailable: required dcomp exports missing; mask="+std::to_string(missing)+" (1=CreatePresentationFactory, 2=DCompositionGetStatistics, 4=DCompositionGetTargetStatistics). Use the default DXGI/DComp backend.");}
 }
 OptionalCompositionApi995(const OptionalCompositionApi995&)=delete;
 OptionalCompositionApi995&operator=(const OptionalCompositionApi995&)=delete;
 ~OptionalCompositionApi995(){if(module)FreeLibrary(module);}
};
// CPU compile-time missing-export mocks; all required entries must be available.
static_assert(OptionalCompositionApi995::missingMask(true,true,true)==0);
static_assert(OptionalCompositionApi995::missingMask(false,true,true)==1);
static_assert(OptionalCompositionApi995::missingMask(true,false,true)==2);
static_assert(OptionalCompositionApi995::missingMask(true,true,false)==4);
static_assert(OptionalCompositionApi995::missingMask(false,false,true)==3);
static_assert(OptionalCompositionApi995::missingMask(false,true,false)==5);
static_assert(OptionalCompositionApi995::missingMask(true,false,false)==6);
static_assert(OptionalCompositionApi995::missingMask(false,false,false)==7);

// Own-process presentation backend. No global timing, display, VRR, or security changes.
class ScheduledPresenter871 {
 // Declared first so the module outlives every COM object and pending buffer.
 OptionalCompositionApi995 api995_;
 using Texture=winrt::com_ptr<ID3D11Texture2D>;
 struct Handle{HANDLE value=nullptr;~Handle(){if(value)CloseHandle(value);}Handle()=default;Handle(const Handle&)=delete;};
 struct Buffer{Texture texture;winrt::com_ptr<IPresentationBuffer> buffer;std::unique_ptr<Handle> ready;};
 struct Submission{UINT64 id;bool generated;double source,target,submit;};
 struct CopyPending{UINT64 id;double submitted;winrt::com_ptr<ID3D11Query> query;std::shared_ptr<void> lease;};
 std::deque<CopyPending> copies_;std::vector<winrt::com_ptr<ID3D11Query>> freeQueries876_;std::shared_ptr<void> copyLease876_;UINT64 retired876_=0;size_t peakCopies876_=0;winrt::com_ptr<ID3D11Device> device_;winrt::com_ptr<ID3D11DeviceContext> context_;winrt::com_ptr<ID3D11Query> copyQuery_;double copySubmit_=0;
 struct Pending{UINT64 id,frame;COMPOSITION_TARGET_ID target;double firstSeen;};
 winrt::com_ptr<IPresentationFactory> factory_;
 winrt::com_ptr<IPresentationManager> manager_;
 winrt::com_ptr<IPresentationSurface> surface_;
 Handle surfaceHandle_,lost_,statistics_;
 std::vector<Buffer> buffers_;
 std::deque<Submission> submissions_;
 std::deque<Pending> pending_;
 std::set<std::pair<UINT64,UINT64>> seenComposition_;
 std::set<UINT64> seenIndependent_;
 UINT current_=UINT_MAX;double offset_=0,uncertainty_=0,lastTarget_=0;
 FILE* log_=nullptr;
 UINT64 submitted_=0,skipped_=0,canceled_=0,compositionInstances_=0,independentUnique_=0,unknown_=0;
 unsigned confirmedGenerated_=0,confirmedOriginal_=0;
 static double now(){LARGE_INTEGER q,f;QueryPerformanceCounter(&q);QueryPerformanceFrequency(&f);return double(q.QuadPart)/f.QuadPart;}
 Submission* lookup(UINT64 id){for(auto&v:submissions_)if(v.id==id)return &v;return nullptr;}
 void row(const char* event,UINT64 id,UINT64 frame,unsigned detail,UINT64 time,unsigned pc=0,unsigned cc=0,HRESULT hr=S_OK){
  if(!log_)return;auto*s=lookup(id);
  fprintf(log_,"%s,%llu,%d,%.9f,%.9f,%.9f,%.9f,%llu,%u,%llu,%u,%u,0x%08x\n",
   event,(unsigned long long)id,s?int(s->generated):-1,s?s->source:0,s?s->target:0,s?s->submit:0,now(),
   (unsigned long long)frame,detail,(unsigned long long)time,pc,cc,unsigned(hr));
 }
 void timingPending(){
  for(auto i=pending_.begin();i!=pending_.end();){
   COMPOSITION_TARGET_STATS t{};HRESULT hr=api995_.targetStatistics(i->frame,&i->target,&t);
   if(SUCCEEDED(hr)&&t.presentedStats.presentCount&&t.completedStats.presentCount>=t.presentedStats.presentCount){
    // Preserve raw compositor counts. This is NOT independent per-buffer scanout proof.
    row("composition_timing",i->id,i->frame,t.outstandingPresents,t.completedStats.time,t.presentedStats.presentCount,t.completedStats.presentCount,hr);
    i=pending_.erase(i);
   }else if(now()-i->firstSeen>1.0){
    row("composition_timing_unresolved",i->id,i->frame,t.outstandingPresents,t.completedStats.time,t.presentedStats.presentCount,t.completedStats.presentCount,hr);i=pending_.erase(i);
   }else ++i;
  }
 }
public:
 ScheduledPresenter871(ID3D11Device*d,UINT width,UINT height,UINT count){
  device_.copy_from(d);d->GetImmediateContext(context_.put());
  for(unsigned i=0;i<16;i++){D3D11_QUERY_DESC desc{D3D11_QUERY_EVENT,0};winrt::com_ptr<ID3D11Query> q;winrt::check_hresult(d->CreateQuery(&desc,q.put()));freeQueries876_.push_back(std::move(q));}
  winrt::check_hresult(api995_.createFactory(d,__uuidof(IPresentationFactory),factory_.put_void()));
  if(!factory_->IsPresentationSupported())throw std::runtime_error("Composition presentation unsupported");
  winrt::check_hresult(factory_->CreatePresentationManager(manager_.put()));
  winrt::check_hresult(DCompositionCreateSurfaceHandle(COMPOSITIONOBJECT_ALL_ACCESS,nullptr,&surfaceHandle_.value));
  winrt::check_hresult(manager_->CreatePresentationSurface(surfaceHandle_.value,surface_.put()));
  surface_->SetTag(871);
  RECT sourceRect{0,0,LONG(width),LONG(height)};winrt::check_hresult(surface_->SetSourceRect(&sourceRect));
  PresentationTransform identity{1,0,0,1,0,0};winrt::check_hresult(surface_->SetTransform(&identity));
  winrt::check_hresult(surface_->SetLetterboxingMargins(0,0,0,0));
  winrt::check_hresult(surface_->SetAlphaMode(DXGI_ALPHA_MODE_IGNORE));
  winrt::check_hresult(surface_->SetColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709));
  winrt::check_hresult(manager_->GetLostEvent(&lost_.value));
  winrt::check_hresult(manager_->ForceVSyncInterrupt(true));
  winrt::com_ptr<IDXGIDevice> dx;winrt::check_hresult(d->QueryInterface(__uuidof(IDXGIDevice),dx.put_void()));
  INT prior=0;dx->GetGPUThreadPriority(&prior);INT priority=getenv("SMX_DISPLAY_PRIORITY")?std::clamp(atoi(getenv("SMX_DISPLAY_PRIORITY")),-7,7):3;
  winrt::check_hresult(dx->SetGPUThreadPriority(priority));fprintf(stderr,"DISPLAY872 own_relative_priority=%d previous=%d force_feedback=1 global_settings_changed=0\n",priority,prior);
  winrt::check_hresult(manager_->GetPresentStatisticsAvailableEvent(&statistics_.value));
  for(auto kind:{PresentStatisticsKind_PresentStatus,PresentStatisticsKind_CompositionFrame,PresentStatisticsKind_IndependentFlipFrame})
   winrt::check_hresult(manager_->EnablePresentStatisticsKind(kind,true));
  count=std::clamp(count,3u,5u);
  bool displayable=!(getenv("SMX_PRESENT_DISPLAYABLE")&&strcmp(getenv("SMX_PRESENT_DISPLAYABLE"),"0")==0);
  for(UINT i=0;i<count;i++){
   Buffer b;D3D11_TEXTURE2D_DESC t{};t.Width=width;t.Height=height;t.MipLevels=t.ArraySize=1;t.Format=DXGI_FORMAT_B8G8R8A8_UNORM;t.SampleDesc.Count=1;t.Usage=D3D11_USAGE_DEFAULT;
   t.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
   t.MiscFlags=D3D11_RESOURCE_MISC_SHARED|D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
   if(displayable)t.MiscFlags|=D3D11_RESOURCE_MISC_SHARED_DISPLAYABLE;
   winrt::check_hresult(d->CreateTexture2D(&t,nullptr,b.texture.put()));
   winrt::check_hresult(manager_->AddBufferFromResource(b.texture.get(),b.buffer.put()));
   b.ready=std::make_unique<Handle>();winrt::check_hresult(b.buffer->GetAvailableEvent(&b.ready->value));
   buffers_.push_back(std::move(b));
  }
  double best=1e9;
  for(int i=0;i<16;i++){double a=now();UINT64 q=0;QueryInterruptTimePrecise(&q);double b=now();if(b-a<best){best=b-a;offset_=q*1e-7-(a+b)*.5;}}
  uncertainty_=best*.5;
  if(uncertainty_>.001)throw std::runtime_error("Presentation clock mapping uncertainty exceeds 1 ms");
  fopen_s(&log_,"app-scheduled-present871.csv","wb");
  if(!log_)throw std::runtime_error("Presentation audit log creation failed");
  fprintf(log_,"event,present_id,generated,source_qpc_s,target_qpc_s,submit_qpc_s,observed_qpc_s,composition_frame,detail,raw_time,presented_count,completed_count,hr\n");
  fprintf(stderr,"SCHEDULED871 supported=1 independent_flip_capability=%u buffers=%u displayable=%u width=%u height=%u interrupt_minus_qpc_s=%.9f mapping_uncertainty_us=%.3f model_unchanged=1 target_clock=qpc_default explicit_surface_rect=1\n",
   unsigned(factory_->IsPresentationSupportedWithIndependentFlip()),count,unsigned(displayable),width,height,offset_,uncertainty_*1e6);
 }
 ~ScheduledPresenter871(){
  fprintf(stderr,"SCHEDULED871_SUMMARY submitted=%llu status_skipped=%llu status_canceled=%llu composition_instances=%llu independent_unique=%llu independent_generated=%u independent_original=%u unmatched_events=%llu pending_composition_timing=%zu actual_scanout_acceptance=not_automatic\n",
   (unsigned long long)submitted_,(unsigned long long)skipped_,(unsigned long long)canceled_,(unsigned long long)compositionInstances_,(unsigned long long)independentUnique_,confirmedGenerated_,confirmedOriginal_,(unsigned long long)unknown_,pending_.size());
  fprintf(stderr,"COPY_LIFETIME876 retired=%llu pending=%zu peak=%zu free_queries=%zu source_reuse_only_after_gpu_copy=1\n",(unsigned long long)retired876_,copies_.size(),peakCopies876_,freeQueries876_.size());if(log_)fclose(log_);
 }
 winrt::com_ptr<IUnknown> content(IDCompositionDevice*d){winrt::com_ptr<IUnknown>c;winrt::check_hresult(d->CreateSurfaceFromHandle(surfaceHandle_.value,c.put()));return c;}
 void poll(){
  if(WaitForSingleObject(lost_.value,0)==WAIT_OBJECT_0)throw std::runtime_error("Presentation manager lost");
  for(unsigned n=0;n<2048;n++){
   DWORD available=WaitForSingleObject(statistics_.value,0);if(available==WAIT_TIMEOUT)break;if(available!=WAIT_OBJECT_0)throw std::runtime_error("Presentation statistics event failed");
   winrt::com_ptr<IPresentStatistics>s;winrt::check_hresult(manager_->GetNextPresentStatistics(s.put()));if(!s)break;
   UINT64 id=s->GetPresentId();if(!lookup(id))unknown_++;
   if(s->GetKind()==PresentStatisticsKind_PresentStatus){
    auto st=s.as<IPresentStatusPresentStatistics>();auto status=st->GetPresentStatus();
    skipped_+=status==PresentStatus_Skipped;canceled_+=status==PresentStatus_Canceled;
    row("status",id,st->GetCompositionFrameId(),unsigned(status),0);
   }else if(s->GetKind()==PresentStatisticsKind_IndependentFlipFrame){
    auto st=s.as<IIndependentFlipFramePresentStatistics>();
    row("independent_display",id,0,unsigned(st->GetOutputVidPnSourceId()),st->GetDisplayedTime().value);
    row("independent_duration",id,0,unsigned(st->GetOutputVidPnSourceId()),st->GetPresentDuration().value);
    if(seenIndependent_.insert(id).second){independentUnique_++;if(auto*r=lookup(id)){if(r->generated)confirmedGenerated_++;else confirmedOriginal_++;}}
   }else if(s->GetKind()==PresentStatisticsKind_CompositionFrame){
    auto st=s.as<ICompositionFramePresentStatistics>();UINT count=0;const CompositionFrameDisplayInstance* instances=nullptr;
    st->GetDisplayInstanceArray(&count,&instances);UINT64 frame=st->GetCompositionFrameId();
    if(!seenComposition_.insert({id,frame}).second)continue;
    compositionInstances_+=count;
    COMPOSITION_FRAME_STATS fs{};COMPOSITION_TARGET_ID targets[COMPOSITION_STATS_MAX_TARGETS]{};UINT actual=0;
    HRESULT hr=api995_.statistics(frame,&fs,COMPOSITION_STATS_MAX_TARGETS,targets,&actual);
    row("composition_frame",id,frame,count,fs.targetTime,0,0,hr);
    for(UINT j=0;j<count;j++){
     const auto&v=instances[j];row("composition_instance",id,frame,unsigned(v.instanceKind),0,unsigned(v.requiredCrossAdapterCopy),v.displayVidPnSourceId);
     if(v.instanceKind==CompositionFrameInstanceKind_ComposedToIntermediate||FAILED(hr))continue;
     for(UINT k=0;k<std::min(actual,UINT(COMPOSITION_STATS_MAX_TARGETS));k++){
      auto&t=targets[k];
      if(t.displayAdapterLuid.LowPart!=v.displayAdapterLUID.LowPart||t.displayAdapterLuid.HighPart!=v.displayAdapterLUID.HighPart||t.vidPnSourceId!=v.displayVidPnSourceId||t.uniqueId!=v.displayUniqueId)continue;
      if(pending_.size()>=512){row("timing_overflow",pending_.front().id,pending_.front().frame,0,0);pending_.pop_front();}
      pending_.push_back({id,frame,t,now()});
     }
    }
   }
  }
  timingPending();
  for(auto i=copies_.begin();i!=copies_.end();){BOOL done=FALSE;HRESULT hr=context_->GetData(i->query.get(),&done,sizeof(done),D3D11_ASYNC_GETDATA_DONOTFLUSH);if(FAILED(hr))winrt::check_hresult(hr);if(hr==S_OK&&done){row("copy_complete_observed",i->id,0,0,UINT64((now()-i->submitted)*1e7));freeQueries876_.push_back(std::move(i->query));i=copies_.erase(i);retired876_++;}else ++i;}
  // Session is bounded by the capture controller; retain only a small association window.
  while(submissions_.size()>4096)submissions_.pop_front();
  while(seenComposition_.size()>8192)seenComposition_.erase(seenComposition_.begin());
  while(seenIndependent_.size()>4096)seenIndependent_.erase(seenIndependent_.begin());
 }
 Texture acquire(std::atomic<bool>&stop){
  double begin=now();
  for(;;){
   poll();if(stop&&now()-begin>.2)throw std::runtime_error("Presentation stop drain exceeded 200 ms");
   for(UINT i=0;i<buffers_.size();i++){
    boolean available=false;winrt::check_hresult(buffers_[i].buffer->IsAvailable(&available));
    if(available){current_=i;winrt::check_hresult(surface_->SetBuffer(buffers_[i].buffer.get()));return buffers_[i].texture;}
   }
   if(now()-begin>1.0)throw std::runtime_error("Presentation buffers stalled for one second");
   HANDLE events[7]{};UINT n=0;events[n++]=lost_.value;events[n++]=statistics_.value;
   for(auto&b:buffers_)events[n++]=b.ready->value;
   if(WaitForMultipleObjects(n,events,FALSE,10)==WAIT_FAILED)throw std::runtime_error("Presentation buffer event wait failed");
  }
 }
 void markCopy(std::shared_ptr<void> lease){
  if(!lease||copyQuery_||copyLease876_)throw std::runtime_error("Invalid copy source lifetime");
  if(freeQueries876_.empty())throw std::runtime_error("Bounded copy query pool exhausted");
  copyQuery_=std::move(freeQueries876_.back());freeQueries876_.pop_back();
  copyLease876_=std::move(lease);context_->End(copyQuery_.get());copySubmit_=now();
 }
 UINT submit(double source,double target,bool generated){
  if(current_==UINT_MAX)throw std::runtime_error("No presentation buffer reserved");
  if(target<=lastTarget_)throw std::runtime_error("Non-increasing presentation target");
  bool useInterrupt=getenv("SMX_PRESENT_CLOCK")&&strcmp(getenv("SMX_PRESENT_CLOCK"),"interrupt")==0;
  double interruptTarget=target+(useInterrupt?offset_:0);
  if(!(interruptTarget>0)||!std::isfinite(interruptTarget))throw std::runtime_error("Invalid presentation target");
  UINT64 id=manager_->GetNextPresentId();
  if(id>UINT_MAX)throw std::runtime_error("Presentation ID overflow");
  winrt::check_hresult(manager_->SetTargetTime({UINT64(std::llround(interruptTarget*1e7))}));
  submissions_.push_back({id,generated,source,target,now()});
  winrt::check_hresult(manager_->Present());if(copyQuery_){BOOL ready=FALSE;HRESULT hr=context_->GetData(copyQuery_.get(),&ready,sizeof(ready),D3D11_ASYNC_GETDATA_DONOTFLUSH);if(FAILED(hr))winrt::check_hresult(hr);row("copy_ready_at_submit",id,0,unsigned(hr==S_OK&&ready),0);copies_.push_back({id,copySubmit_,copyQuery_,std::move(copyLease876_)});peakCopies876_=std::max(peakCopies876_,copies_.size());copyQuery_=nullptr;}submitted_++;lastTarget_=target;current_=UINT_MAX;
  row("submit",id,0,0,UINT64(std::llround(interruptTarget*1e7)));return UINT(id);
 }
 unsigned confirmedGenerated()const{return confirmedGenerated_;}
 unsigned confirmedOriginal()const{return confirmedOriginal_;}
 void drain(unsigned ms){double end=now()+ms*.001;while(now()<end){poll();WaitForSingleObject(statistics_.value,10);}poll();if(log_)fflush(log_);}
};
