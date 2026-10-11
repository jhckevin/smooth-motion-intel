#include "bounded_stdio989.h"
#pragma once
#include "hud_metrics988.hpp"
#include "hud_power988.hpp"
#include "telemetry_lifetime993.hpp"
#include "diagnostic994.hpp"
#include <dxgi1_3.h>
#include <wrl/client.h>
#include <thread>
#include <atomic>
#include <memory>
#include <string>
#include <deque>
#include <unordered_map>
#include <cstdio>
#include <share.h>
#include <algorithm>
#include <stdexcept>
namespace smx988 {
// One CRT writer, concurrent read-only desktop telemetry and archive readers.
inline FILE* openHudLog988(const char*path){return smx_log_fsopen989(path,"wb",_SH_DENYWR);}
struct HudSession988 {
 HudMetrics metrics;
 struct Meta {bool generated=false;double source=0,due=0,presentCpu=0;uint64_t idEpoch=0;};
 struct Event {double queryQpc=0;DXGI_FRAME_STATISTICS stats{};int32_t hr=0;uint64_t epoch=0,gap=0,idEpoch=0;double frequency=0;};
 std::mutex rawMutex,powerMutex;std::deque<Event>raw;std::unordered_map<uint64_t,Meta>meta;std::deque<uint64_t>metaOrder;
 DXGI_FRAME_STATISTICS latestStats{};HRESULT latestHr=E_NOINTERFACE;double latestQuery=0;PowerSample988 power;
 std::atomic<uint64_t>rawDropped{0},captured{0};std::atomic<bool>observerFailed{false},telemetryDisabled993{false},displayDisabled993{false},sensorDisabled993{false};
 uint64_t metadataEpoch=0,lastSubmittedId=0;double lastSubmittedCpu=0;std::deque<double>pollMs,queryMs;bool highResolutionTimer=false;
 uint32_t width=0,height=0;std::string backend;double started=qpcNow988();
 void recordSubmitted(uint64_t id,bool generated,double source,double due,double presentCpu){
  {std::lock_guard<std::mutex>lock(rawMutex);if(lastSubmittedId&&id<lastSubmittedId){++metadataEpoch;meta.clear();metaOrder.clear();}lastSubmittedId=id;lastSubmittedCpu=presentCpu;if(!meta.count(id)){if(meta.size()>=2048){meta.erase(metaOrder.front());metaOrder.pop_front();}meta.emplace(id,Meta{generated,source,due,presentCpu,metadataEpoch});metaOrder.push_back(id);}}
  metrics.recordSubmitted(id,generated,source,due,presentCpu);
 }
 void pollJson(FILE*f,double now){if(!f)return;std::vector<double>p,q;bool high=false;{std::lock_guard<std::mutex>lock(rawMutex);p.assign(pollMs.begin(),pollMs.end());q.assign(queryMs.begin(),queryMs.end());high=highResolutionTimer;}auto value=[](std::vector<double>v,int kind){if(v.empty())return -1.;if(kind==0){double sum=0;for(auto x:v)sum+=x;return sum/v.size();}std::sort(v.begin(),v.end());return kind==1?v[size_t(.95*(v.size()-1))]:v.back();};smx_log_fprintf989(f,"{\"type\":\"collector\",\"qpc\":%.9f,\"high_resolution_timer\":%s,\"poll_samples\":%zu,\"poll_mean_ms\":%.6f,\"poll_p95_ms\":%.6f,\"poll_max_ms\":%.6f,\"query_mean_ms\":%.6f,\"query_p95_ms\":%.6f,\"query_max_ms\":%.6f,\"observer_failed\":%s}\n",now,high?"true":"false",p.size(),value(p,0),value(p,1),value(p,2),value(q,0),value(q,1),value(q,2),observerFailed?"true":"false");}
 HudMetrics::Snapshot displaySnapshot993(double now){auto s=metrics.snapshot(now);if(telemetryDisabled993||displayDisabled993){s.displayValid=false;s.low1PercentValid=false;s.latencyValid=false;s.incomplete=true;s.stalled=true;s.displayHz=s.originalHz=s.generatedHz=s.coverage=0;s.graph.clear();}return s;}
  PowerSample988 powerSnapshot(double now){if(telemetryDisabled993||sensorDisabled993)return PowerSample988{};std::lock_guard<std::mutex>lock(powerMutex);auto p=power;if(now-p.sampleQpc>3||now<p.sampleQpc){p.powerValid=p.frequencyValid=p.throttleValid=p.cpuPackageValid=false;p.watts=p.gpuMHz=p.cpuPackageWatts=-1;}if(p.cpuPackageValid&&(now-p.cpuPackageSampleQpc>3||now<p.cpuPackageSampleQpc)){p.cpuPackageValid=false;p.cpuPackageWatts=-1;}return p;}
 void getLatest(DXGI_FRAME_STATISTICS&s,HRESULT&hr){std::lock_guard<std::mutex>lock(rawMutex);s=latestStats;hr=latestHr;}
 void flushRaw(FILE*f){
  struct Row{Event e;bool mapped=false;Meta m;};std::vector<Row>rows;
  {std::lock_guard<std::mutex>lock(rawMutex);rows.reserve(raw.size());while(!raw.empty()){Row row{raw.front()};raw.pop_front();auto it=meta.find(row.e.stats.PresentCount);if(row.e.hr==S_OK&&it!=meta.end()&&it->second.presentCpu<=row.e.queryQpc&&it->second.idEpoch==row.e.idEpoch){row.mapped=true;row.m=it->second;}rows.push_back(row);}}
  if(!f)return;for(auto&r:rows)smx_log_fprintf989(f,"%.9f,%llu,%ld,%u,%u,%u,%lld,%lld,%llu,%u,%d,%.9f,%.9f,%.9f,%llu,%.0f\n",r.e.queryQpc,(unsigned long long)r.e.epoch,long(r.e.hr),r.e.stats.PresentCount,r.e.stats.PresentRefreshCount,r.e.stats.SyncRefreshCount,r.e.stats.SyncQPCTime.QuadPart,r.e.stats.SyncGPUTime.QuadPart,(unsigned long long)r.e.gap,unsigned(r.mapped),r.mapped?int(r.m.generated):-1,r.m.source,r.m.due,r.m.presentCpu,(unsigned long long)r.e.idEpoch,r.e.frequency);fflush(f);
 }
 void json(FILE*f,double now){if(!f)return;auto s=displaySnapshot993(now);auto p=powerSnapshot(now);std::string gpuWatts=p.powerValid?std::to_string(p.watts):"null";
  std::string cpuWatts=p.cpuPackageValid?std::to_string(p.cpuPackageWatts):"null";std::string cpuRaw=std::isfinite(p.cpuPackageRawMilliwatts)&&p.cpuPackageRawMilliwatts>=0?std::to_string(p.cpuPackageRawMilliwatts):"null";
  smx_log_fprintf989(f,"{\"type\":\"snapshot\",\"qpc\":%.9f,\"backend\":\"%s\",\"telemetry_available993\":%s,\"display_observer_available993\":%s,\"sensor_available993\":%s,\"display_valid\":%s,\"display_hz\":%.6f,\"original_hz\":%.6f,\"generated_hz\":%.6f,\"coverage\":%.6f,\"low1_valid\":%s,\"low1_hz\":%.6f,\"interval_p95_ms\":%.6f,\"incomplete\":%s,\"warming_up\":%s,\"stalled\":%s,\"latency_valid\":%s,\"content_to_display_ms\":%.6f,\"present_to_display_ms\":%.6f,\"observed_count\":%llu,\"observed_original\":%llu,\"observed_generated\":%llu,\"missing_ids\":%llu,\"unmapped\":%llu,\"untimed\":%llu,\"valid_intervals\":%llu,\"epoch_submitted\":%llu,\"submitted\":%llu,\"generated_submitted\":%llu,\"generation_completed\":%llu,\"capture_count\":%llu,\"capture_hz\":%.6f,\"inflight\":%u,\"late_completed\":%llu,\"late_submitted\":%llu,\"model_timing_valid\":%s,\"model_gpu_ms\":%.6f,\"service_ms\":%.6f,\"queue_valid\":%s,\"queue_ms\":%.6f,\"stats_errors\":%llu,\"disjoint_count\":%llu,\"reset_count\":%llu,\"raw_events_dropped\":%llu,\"power_valid\":%s,\"device_matched\":%s,\"power_domain_enum\":%u,\"energy_uj\":%llu,\"energy_timestamp_us\":%llu,\"power_watts\":%.6f,\"power_sample_age_s\":%.6f,\"power_driver_interval_s\":%.6f,\"power_result\":%u,\"frequency_valid\":%s,\"gpu_mhz\":%.6f,\"frequency_result\":%u,\"driver_flag_valid\":%s,\"driver_flag\":%u,\"sensor_poll_ms\":%.6f,\"gpu_power\":{\"valid\":%s,\"watts\":%s,\"unit\":\"W\",\"source\":\"LevelZero.Sysman.zesPowerGetEnergyCounter\",\"domain_enum\":%u,\"physical_scope\":\"uncalibrated GPU-driver domain; not established as CPU package or system\",\"fg_exclusive\":false},\"cpu_package_power\":{\"valid\":%s,\"watts\":%s,\"unit\":\"W\",\"source\":\"Windows.PDH.EnergyMeter.RAPL_Package0_PKG.Power\",\"domain\":\"RAPL_Package0_PKG\",\"raw_milliwatts\":%s,\"status\":%u,\"counter_status\":%u,\"available\":%s,\"warming_up\":%s,\"sample_qpc\":%.9f,\"age_s\":%.6f,\"conversion\":\"PDH_FMT_DOUBLE cooked mW / 1000; default scale\",\"fg_exclusive\":false},\"scope\":\"DXGI OS-display observations, not optical scanout. Missing/untimed IDs are not invented. queue_ms is calibrated submit-to-model-begin, including preceding GPU work and resource synchronization; not pure queue wait. GPU driver energy has uncalibrated physical scope, not established as CPU package; it is not a whole-system, FG-exclusive or delta-power measurement. CPU package is independently read from Windows RAPL PKG in mW, converted to W; no value is inferred from limits or duplicated from GPU. Driver throttle flags do not identify PL1.\"}\n",now,backend.c_str(),telemetryDisabled993?"false":"true",(telemetryDisabled993||displayDisabled993)?"false":"true",(telemetryDisabled993||sensorDisabled993)?"false":"true",s.displayValid?"true":"false",s.displayHz,s.originalHz,s.generatedHz,s.coverage,s.low1PercentValid?"true":"false",s.low1PercentHz,s.displayIntervalP95Ms,s.incomplete?"true":"false",s.warmingUp?"true":"false",s.stalled?"true":"false",s.latencyValid?"true":"false",s.contentToDisplayMs,s.presentToDisplayMs,(unsigned long long)s.observedDisplayCount,(unsigned long long)s.originalDisplayCount,(unsigned long long)s.generatedDisplayCount,(unsigned long long)s.missingIds,(unsigned long long)s.unmappedCount,(unsigned long long)s.untimedCount,(unsigned long long)s.validIntervals,(unsigned long long)s.epochSubmittedCount,(unsigned long long)s.submittedCount,(unsigned long long)s.generatedSubmittedCount,(unsigned long long)s.generationCompleteCount,(unsigned long long)captured.load(),s.captureHz,s.inflight,(unsigned long long)s.lateCompleted,(unsigned long long)s.lateSubmitted,s.modelTimingValid?"true":"false",s.modelGpuMs,s.serviceMs,s.queueWaitValid?"true":"false",s.queueWaitMs,(unsigned long long)s.statsErrorCount,(unsigned long long)s.disjointCount,(unsigned long long)s.resetCount,(unsigned long long)rawDropped.load(),p.powerValid?"true":"false",p.deviceMatched?"true":"false",p.domain,(unsigned long long)p.energyMicrojoules,(unsigned long long)p.driverMicroseconds,p.watts,p.sampleQpc?now-p.sampleQpc:-1.,p.driverIntervalSeconds,p.powerResult,p.frequencyValid?"true":"false",p.gpuMHz,p.frequencyResult,p.throttleValid?"true":"false",p.throttleFlags,p.pollMs,p.powerValid?"true":"false",gpuWatts.c_str(),p.domain,p.cpuPackageValid?"true":"false",cpuWatts.c_str(),cpuRaw.c_str(),p.cpuPackageStatus,p.cpuPackageCounterStatus,p.cpuPackageAvailable?"true":"false",p.cpuPackageWarmup?"true":"false",p.cpuPackageSampleQpc,p.cpuPackageSampleQpc?now-p.cpuPackageSampleQpc:-1.);fflush(f);
 }
};
struct HudCollectorContext993 {
 std::shared_ptr<HudSession988>state_;std::string directory_;
 std::shared_ptr<smx993::OwnedEvent>firstQuery=std::make_shared<smx993::OwnedEvent>();
 std::array<smx993::WorkerPhase,3>phases;std::atomic<bool>statsRetired993{false};
 HudCollectorContext993(std::shared_ptr<HudSession988>s,std::string directory):state_(std::move(s)),directory_(std::move(directory)){}
 static void sensorPhase(void*ctx,const char*name){static_cast<HudCollectorContext993*>(ctx)->phases[1].set(name);}
 void powerLoop(HANDLE stop_){
  try{phases[1].set("sensors.initialize");{
   PowerSampler988 sampler(&sensorPhase,this);for(;;){if(WaitForSingleObject(stop_,0)==WAIT_OBJECT_0)break;
    auto power=sampler.sample();if(WaitForSingleObject(stop_,0)==WAIT_OBJECT_0)break;
    phases[1].set("sensors.publish");{std::lock_guard<std::mutex>lock(state_->powerMutex);state_->power=power;}
    phases[1].set("sensors.wait");if(WaitForSingleObject(stop_,1000)==WAIT_OBJECT_0)break;
   }phases[1].set("sensors.cleanup");}
  }catch(...){state_->observerFailed=true;state_->sensorDisabled993=true;phases[1].set("sensors.exception");}
  phases[1].set("sensors.exited");
 }
 void statsLoop(HANDLE stop_,Microsoft::WRL::ComPtr<IDXGISwapChain1> chain_){
  LARGE_INTEGER frequency{};QueryPerformanceFrequency(&frequency);uint64_t epoch=0,lastId=0,total=0,idEpoch=0,lastNonzeroId=0;HRESULT lastHr=E_PENDING;double previousQuery=0;
  HANDLE timer=CreateWaitableTimerExW(nullptr,nullptr,CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,TIMER_ALL_ACCESS);bool highRes=timer!=nullptr;
  if(!timer)timer=CreateWaitableTimerExW(nullptr,nullptr,0,TIMER_ALL_ACCESS);
  {std::lock_guard<std::mutex>lock(state_->rawMutex);state_->highResolutionTimer=highRes;}
  try{for(;;){
   if(WaitForSingleObject(stop_,0)==WAIT_OBJECT_0||statsRetired993.load())break;phases[0].set("dxgi.GetFrameStatistics");double begin=qpcNow988();DXGI_FRAME_STATISTICS stats{};smxdiag994::Scope diagnosticCall994(SMX_DIAG_DISPLAY_STATS994);HRESULT hr=chain_->GetFrameStatistics(&stats);if(FAILED(hr))diagnosticCall994.fail(SMX_DIAG_HRESULT994,int64_t(hr));else diagnosticCall994.end(SMX_DIAG_HRESULT994,int64_t(hr));double now=qpcNow988();if(WaitForSingleObject(stop_,0)==WAIT_OBJECT_0||statsRetired993.load())break;phases[0].set("stats.publish");
   state_->metrics.observeDisplay(stats.PresentCount,stats.PresentRefreshCount,stats.SyncRefreshCount,stats.SyncQPCTime.QuadPart,int32_t(hr),now,double(frequency.QuadPart));SetEvent(firstQuery->value);
   {std::lock_guard<std::mutex>lock(state_->rawMutex);
    if(previousQuery){state_->pollMs.push_back((begin-previousQuery)*1000);if(state_->pollMs.size()>4096)state_->pollMs.pop_front();}previousQuery=begin;
    state_->queryMs.push_back((now-begin)*1000);if(state_->queryMs.size()>4096)state_->queryMs.pop_front();
    state_->latestStats=stats;state_->latestHr=hr;state_->latestQuery=now;
    if(hr!=lastHr||(hr==S_OK&&stats.PresentCount>0&&stats.PresentCount!=lastId)){
     uint64_t gap=0;if(hr==S_OK&&stats.PresentCount>0&&lastNonzeroId&&stats.PresentCount<lastNonzeroId){++idEpoch;lastNonzeroId=0;}if(hr==S_OK&&stats.PresentCount)lastNonzeroId=stats.PresentCount;if(hr==DXGI_ERROR_FRAME_STATISTICS_DISJOINT||(hr==S_OK&&stats.PresentCount>0&&stats.PresentCount<lastId)){++epoch;lastId=0;}
     if(hr==S_OK&&lastId&&stats.PresentCount>lastId+1)gap=stats.PresentCount-lastId-1;
     if(total<72000&&state_->raw.size()<2048){state_->raw.push_back({now,stats,int32_t(hr),epoch,gap,idEpoch,double(frequency.QuadPart)});++total;}else ++state_->rawDropped;
     if(hr==S_OK&&stats.PresentCount)lastId=stats.PresentCount;lastHr=hr;
    }
   }
   phases[0].set("stats.wait");if(timer){LARGE_INTEGER due{};due.QuadPart=-50000;if(!SetWaitableTimer(timer,&due,0,nullptr,nullptr,FALSE))throw std::runtime_error("HUD poll timer failed");HANDLE waits[]={stop_,timer};DWORD result=WaitForMultipleObjects(2,waits,FALSE,50);if(result==WAIT_OBJECT_0)break;if(result!=WAIT_OBJECT_0+1)throw std::runtime_error("HUD poll timer timeout");}
   else if(WaitForSingleObject(stop_,5)==WAIT_OBJECT_0)break;
  }}catch(...){state_->observerFailed=true;state_->displayDisabled993=true;phases[0].set("stats.exception");}
  if(timer)CloseHandle(timer);phases[0].set("dxgi.chain_release");chain_.Reset();phases[0].set("stats.exited");SetEvent(firstQuery->value);
 }
 // Preserve the previous capture session before opening fixed canonical filenames.
 // No replace flag: a failed archive never licenses truncating prior evidence.
 bool archivePrevious(const std::string&name,const std::string&suffix){std::string from=directory_+"\\"+name;DWORD attr=GetFileAttributesA(from.c_str());if(attr==INVALID_FILE_ATTRIBUTES)return GetLastError()==ERROR_FILE_NOT_FOUND;if(attr&FILE_ATTRIBUTE_DIRECTORY)return false;std::string to=from+suffix;return MoveFileExA(from.c_str(),to.c_str(),0)!=FALSE;}

 void healthCheck993(FILE*json){
  const double now=qpcNow988();const char*phase=phases[0].name.load(std::memory_order_acquire);const double since=phases[0].since.load();
  if(smx993::statsQueryExpired(phase,since,now)&&!statsRetired993.exchange(true)){
   state_->displayDisabled993=true;state_->observerFailed=true;
   if(json)smx_log_fprintf989(json,"{\"type\":\"telemetry_health993\",\"qpc\":%.9f,\"worker\":\"dxgi_statistics\",\"phase\":\"%s\",\"phase_age_ms\":%.6f,\"retire_after_current_call\":true,\"query_retry\":false,\"sampling_period_s\":1,\"threshold_ms\":500,\"scope\":\"Observed long API call; not proof of GPU/game fault. Current call is not cancelled. Display evidence unavailable.\"}\n",now,phase,(now-since)*1000);
  }
 }
 void writerLoop(HANDLE stop_){
  phases[2].set("logs.open");
  LARGE_INTEGER archiveQpc{};QueryPerformanceCounter(&archiveQpc);std::string suffix=".previous-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(archiveQpc.QuadPart);bool archived=archivePrevious("app-display988.csv",suffix)&&archivePrevious("app-hud988.jsonl",suffix);
  FILE*raw=nullptr,*json=nullptr;if(archived){raw=openHudLog988((directory_+"\\app-display988.csv").c_str());json=openHudLog988((directory_+"\\app-hud988.jsonl").c_str());}else state_->observerFailed=true;
  if(!raw||!json){state_->observerFailed=true;}
  if(raw)smx_log_fprintf989(raw,"query_qpc,epoch,result,present_count,present_refresh_count,sync_refresh_count,sync_qpc,sync_gpu,gap_ids,mapped,kind,source_qpc,due_qpc,present_cpu_qpc,id_epoch,qpc_frequency\n");
  uint64_t rotations989=0,rotationFailures989=0;
  auto keepLatest989=[&](){
   uint64_t bytes=0,drops=0;smx_log_stream989(json,&bytes,&drops);
   if(json&&bytes+8192>smx_log_limit989()){
    smx_log_fclose989(json);json=nullptr;std::string current=directory_+"\\app-hud988.jsonl",backup=current+".rotation-previous";
    if(!MoveFileExA(current.c_str(),backup.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))++rotationFailures989;
    json=openHudLog988(current.c_str());++rotations989;
    if(!json)state_->observerFailed=true;
   }
   uint64_t total=0,omitted=0,omittedBytes=0;smx_log_totals989(&total,&omitted,&omittedBytes);
   FILE*budget=fopen((directory_+"\\app-log-budget989.json").c_str(),"wb");if(budget){fprintf(budget,"{\"per_stream_limit_bytes\":%llu,\"written_bytes\":%llu,\"omitted_records\":%llu,\"omitted_bytes\":%llu,\"hud_rotations\":%llu,\"rotation_failures\":%llu,\"evidence_complete\":%s,\"scope\":\"Diagnostic streams stop at bounded byte budgets; metrics continue. HUD retains current plus one bounded previous segment. Omitted/rotated history cannot support whole-session trace claims.\"}\n",(unsigned long long)smx_log_limit989(),(unsigned long long)total,(unsigned long long)omitted,(unsigned long long)omittedBytes,(unsigned long long)rotations989,(unsigned long long)rotationFailures989,(omitted||rotations989||rotationFailures989)?"false":"true");fclose(budget);}
  };

  try{for(;;){
   if(WaitForSingleObject(stop_,0)==WAIT_OBJECT_0)break;phases[2].set("logs.write");healthCheck993(json);keepLatest989();state_->flushRaw(raw);state_->json(json,qpcNow988());state_->pollJson(json,qpcNow988());phases[2].set("logs.wait");if(WaitForSingleObject(stop_,1000)==WAIT_OBJECT_0)break;
  }}catch(...){state_->observerFailed=true;phases[2].set("logs.exception");}
  // Writer does not wait on the driver collectors. It owns only its own files.
  phases[2].set("logs.final_write");keepLatest989();state_->flushRaw(raw);state_->json(json,qpcNow988());state_->pollJson(json,qpcNow988());
  phases[2].set("logs.close");if(raw)smx_log_fclose989(raw);if(json)smx_log_fclose989(json);phases[2].set("logs.exited");
 }

};
class HudMonitor988 {
 std::shared_ptr<HudSession988>state_;std::shared_ptr<HudCollectorContext993>context_;
 std::unique_ptr<smx993::CollectorGroup>group_;bool stopped_=false;std::string directory_;
 std::array<const char*,3>stopPhase_{{"not_started","not_started","not_started"}};std::array<double,3>stopPhaseAge_{};double stopQpc_=0;
 void report(const char*reason,const smx993::ShutdownResult&r){
  // This small per-session report is written only by the caller, never a late worker.
  FILE*f=openHudLog988((directory_+"\\app-telemetry-shutdown993.json").c_str());if(!f)return;
  const double now=qpcNow988();
  smx_log_fprintf989(f,"{\"reason\":\"%s\",\"isolated\":%s,\"wait_result\":%lu,\"wait_error\":%lu,\"pending_mask\":%u,\"shutdown_ms\":%.6f,\"max_groups\":1,\"gpu_protection_changed\":false,\"old_worker_may_still_own_driver_objects\":%s,\"shutdown_all_workers_retired\":%s,\"workers\":[",reason,r.isolated?"true":"false",r.waitResult,r.error,r.pendingMask,r.elapsedMs,r.isolated?"true":"false",r.isolated?"false":"true");
  for(unsigned i=0;i<3;++i){const char*phase=context_?context_->phases[i].name.load(std::memory_order_acquire):"not_started";double since=context_?context_->phases[i].since.load():0;smx_log_fprintf989(f,"%s{\"worker\":%u,\"phase\":\"%s\",\"phase_age_ms\":%.6f,\"stop_phase\":\"%s\",\"stop_phase_age_ms\":%.6f}",i?",":"",i,phase,since?(now-since)*1000:0,stopPhase_[i],stopPhaseAge_[i]);}
  smx_log_fprintf989(f,"]}\n");smx_log_fclose989(f);
 }
public:
 HudMonitor988(std::shared_ptr<HudSession988>s,IDXGISwapChain1*chain,const std::string&directory):state_(std::move(s)),directory_(directory){
  try{
   group_=std::make_unique<smx993::CollectorGroup>();
   if(!group_->available()){state_->observerFailed=true;state_->telemetryDisabled993=true;stopped_=true;smx993::ShutdownResult blocked;blocked.isolated=true;blocked.stopped=false;report("prior_collector_group_still_isolated",blocked);return;}
   context_=std::make_shared<HudCollectorContext993>(state_,directory_);auto ctx=context_;
   if(chain){Microsoft::WRL::ComPtr<IDXGISwapChain1>owned=chain;group_->start([ctx,owned=std::move(owned)](HANDLE stop)mutable{ctx->statsLoop(stop,std::move(owned));});}
   else{group_->start([ctx](HANDLE){ctx->phases[0].set("stats.not_supported");SetEvent(ctx->firstQuery->value);});}
   group_->start([ctx](HANDLE stop){ctx->powerLoop(stop);});
   group_->start([ctx](HANDLE stop){ctx->writerLoop(stop);});
   if(WaitForSingleObject(ctx->firstQuery->value,200)!=WAIT_OBJECT_0){
    state_->observerFailed=true;state_->telemetryDisabled993=true;shutdown("initial_stats_query_timeout");
   }
  }catch(...){state_->observerFailed=true;state_->telemetryDisabled993=true;shutdown("collector_start_failed");}
 }
 ~HudMonitor988(){shutdown();}
 void shutdown(const char*reason="session_stop"){
  if(stopped_)return;stopped_=true;stopQpc_=qpcNow988();
  if(context_)for(unsigned i=0;i<3;++i){stopPhase_[i]=context_->phases[i].name.load(std::memory_order_acquire);double since=context_->phases[i].since.load();stopPhaseAge_[i]=since?(stopQpc_-since)*1000:0;}
  smx993::ShutdownResult result;if(group_)result=group_->shutdown(700);
  if(result.isolated){state_->observerFailed=true;state_->telemetryDisabled993=true;}
  report(reason,result);group_.reset();context_.reset();
 }
};

}
