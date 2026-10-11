#include "bounded_stdio989.h"
#pragma once
#include "fence_probe988_core.hpp"
#include <windows.h>
#include <d3d11_4.h>
#include <wrl/client.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <string>
#include <cstdio>
#include <stdexcept>
namespace smx988 {
// Owns fence COM references only. Never retains NativeSync948, texture, access lock,
// GpuFrames lease, or immediate context. There are no GPU commands in this class.
class DisplayFenceProbe988 {
 using Fence=Microsoft::WRL::ComPtr<ID3D11Fence>;
 struct Poll {double begin,end,spacingMs,queryMs;unsigned reads,pending;};
 FenceCore988 core_;std::array<Fence,FenceCore988::Capacity> fences_{};
 std::vector<Poll>polls_;std::mutex mutex_;std::thread thread_;std::atomic<bool>&appStop_;
 std::atomic<bool>failed_{false};HANDLE stop_=nullptr,done_=nullptr,timer_=nullptr;bool highres_=false,joined_=false,written_=false;
 std::string directory_;double frequency_=0;uint64_t pollTruncated_=0;DWORD error_=0;
 double now()const{LARGE_INTEGER t;QueryPerformanceCounter(&t);return double(t.QuadPart)/frequency_;}
 void fail(DWORD error){error_=error;failed_=true;appStop_=true;SetEvent(stop_);}
 void loop()noexcept{
  double previous=0;
  try{
   HANDLE handles[2]={stop_,timer_};
   while(true){LARGE_INTEGER due;due.QuadPart=-50000; // 5 ms, relative, no global timer changes.
    if(!SetWaitableTimer(timer_,&due,0,nullptr,nullptr,FALSE)){fail(GetLastError());break;}
    DWORD wait=WaitForMultipleObjects(2,handles,FALSE,INFINITE);if(wait==WAIT_OBJECT_0)break;if(wait!=WAIT_OBJECT_0+1){fail(GetLastError());break;}
    const double begin=now();FenceCore988::Selection selection;
    {std::lock_guard lock(mutex_);selection=core_.select();}
    unsigned reads=0;double cost=0;
    for(size_t k=0;k<selection.count;++k){if(WaitForSingleObject(stop_,0)==WAIT_OBJECT_0)break;
     const size_t slot=selection.indices[k];Fence fence;uint64_t seq=0;
     {std::lock_guard lock(mutex_);seq=core_.slots[slot];fence=fences_[slot];}
     if(!seq||!fence)continue;
     double q0=now();const uint64_t value=fence->GetCompletedValue();double q1=now();++reads;cost+=(q1-q0)*1000;
     {std::lock_guard lock(mutex_);if(core_.observe(slot,seq,value,q0,q1))fences_[slot].Reset();}
     if(value==std::numeric_limits<uint64_t>::max()){fail(ERROR_DEVICE_REMOVED);break;}
    }
    unsigned pending;{std::lock_guard lock(mutex_);pending=unsigned(core_.pending());}
    if(polls_.size()<FenceCore988::LogLimit)polls_.push_back({begin,now(),previous?(begin-previous)*1000:0,cost,reads,pending});else ++pollTruncated_;previous=begin;
    if(failed_)break;
   }
  }catch(...){fail(ERROR_UNHANDLED_EXCEPTION);}
  SetEvent(done_);
 }
 void closeHandles(){if(timer_)CloseHandle(timer_);if(done_)CloseHandle(done_);if(stop_)CloseHandle(stop_);timer_=done_=stop_=nullptr;}
 static void number(FILE*f,double x){if(std::isfinite(x)&&x>=0)smx_log_fprintf989(f,"%.9f",x);else fputs("",f);}
 void write(){
  if(written_)return;written_=true;
  FILE*csv=smx_log_fopen989((directory_+"\\app-display-fence988.csv").c_str(),"wb");
  FILE*poll=smx_log_fopen989((directory_+"\\app-display-fence-polls988.csv").c_str(),"wb");
  FILE*json=smx_log_fopen989((directory_+"\\app-display-fence988.json").c_str(),"wb");
  if(!csv||!poll||!json){failed_=true;appStop_=true;error_=ERROR_WRITE_FAULT;smx_log_fprintf989(stderr,"DISPLAY_FENCE988 logging_failed\n");}
  size_t complete=0,unbound=0,errors=0;
  if(csv)fputs("probe_sequence,submitted_present_id,bound,generated,fence_tag,fence_value,last_observed_value,source_qpc,due_qpc,copy_begin_qpc,signal_submit_begin_qpc,signal_return_qpc,flush_return_qpc,published_qpc,last_incomplete_query_begin_qpc,last_incomplete_query_end_qpc,first_complete_query_begin_qpc,first_complete_query_end_qpc,completion_lower_bound_qpc,completion_upper_bound_qpc,complete,device_error,query_count,query_cpu_total_ms,query_cpu_max_ms\n",csv);
  for(const auto&r:core_.records){complete+=r.complete;unbound+=!r.presentId;errors+=r.deviceError;
   if(!csv)continue;
   smx_log_fprintf989(csv,"%llu,%llu,%u,%u,%llu,%llu,%llu,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,",(unsigned long long)r.sequence,(unsigned long long)r.presentId,unsigned(r.presentId!=0),unsigned(r.generated),(unsigned long long)r.fenceTag,(unsigned long long)r.fenceValue,(unsigned long long)r.lastValue,r.source,r.due,r.copyBegin,r.submit,r.signalReturn,r.flushReturn,r.published);
   for(double value:{r.lastNegativeBegin,r.lastNegativeEnd,r.firstPositiveBegin,r.firstPositiveEnd,r.lowerBound(),r.firstPositiveEnd}){number(csv,value);fputc(',',csv);}
   smx_log_fprintf989(csv,"%u,%u,%llu,%.9f,%.9f\n",unsigned(r.complete),unsigned(r.deviceError),(unsigned long long)r.queries,r.queryCostMs,r.maxQueryMs);
  }
  double querySum=0,queryMax=0,spacingMax=0,roundMax=0;uint64_t queryCount=0;
  if(poll)fputs("poll_begin_qpc,poll_end_qpc,spacing_ms,query_cpu_total_ms,reads,pending_after\n",poll);
  for(const auto&p:polls_){querySum+=p.queryMs;queryCount+=p.reads;queryMax=std::max(queryMax,p.queryMs);spacingMax=std::max(spacingMax,p.spacingMs);roundMax=std::max(roundMax,(p.end-p.begin)*1000);if(poll)smx_log_fprintf989(poll,"%.9f,%.9f,%.9f,%.9f,%u,%u\n",p.begin,p.end,p.spacingMs,p.queryMs,p.reads,p.pending);}
  if(json)smx_log_fprintf989(json,R"({"requested":true,"effective":true,"diagnostic_only":true,"failed":%s,"error_code":%lu,"ring_capacity":64,"max_reads_per_wake":16,"record_limit":72000,"high_resolution_timer":%s,"qpc_frequency":%.0f,"requested_poll_ms":5,"attempted":%llu,"recorded":%zu,"completed_observed":%zu,"unbound":%zu,"device_errors":%zu,"exit_pending":%zu,"ring_full_omissions":%llu,"log_full_omissions":%llu,"missing_fence":%llu,"invalid_clock_samples":%llu,"bind_errors":%llu,"poll_records":%zu,"poll_records_truncated":%llu,"query_count":%llu,"query_cpu_total_ms":%.9f,"poll_query_cpu_max_ms":%.9f,"poll_spacing_max_ms":%.9f,"poll_round_max_ms":%.9f,"scope":"CPU observation of existing display release fence only. Lower bound is last negative query BEGIN or signal-call BEGIN; upper bound is first positive query END. First-positive-only is left-censored by submission. Neither bound is an exact GPU timestamp. Completion includes prior resource waits and copy; it is not Present, DWM latch, scanout, or GPU execution duration. Observer makes no GPU commands and owns no frame leases. Overflow is diagnostic omission only. Probe changes CPU observation load; not performance acceptance."})",failed_?"true":"false",(unsigned long)error_,highres_?"true":"false",frequency_,(unsigned long long)core_.attempted,core_.records.size(),complete,unbound,errors,core_.pending(),(unsigned long long)core_.ringFull,(unsigned long long)core_.logFull,(unsigned long long)core_.missingFence,(unsigned long long)core_.invalidClock,(unsigned long long)core_.bindErrors,polls_.size(),(unsigned long long)pollTruncated_,(unsigned long long)queryCount,querySum,queryMax,spacingMax,roundMax);
  if(csv)smx_log_fclose989(csv);if(poll)smx_log_fclose989(poll);if(json)smx_log_fclose989(json);
 }
public:
 DisplayFenceProbe988(std::atomic<bool>&appStop,const char*directory):appStop_(appStop),directory_(directory){
  LARGE_INTEGER f;QueryPerformanceFrequency(&f);frequency_=double(f.QuadPart);polls_.reserve(FenceCore988::LogLimit);
  stop_=CreateEventW(nullptr,TRUE,FALSE,nullptr);done_=CreateEventW(nullptr,TRUE,FALSE,nullptr);timer_=CreateWaitableTimerExW(nullptr,nullptr,2,TIMER_ALL_ACCESS);highres_=timer_!=nullptr;
  if(!timer_)timer_=CreateWaitableTimerW(nullptr,FALSE,nullptr);
  if(!stop_||!done_||!timer_){closeHandles();throw std::runtime_error("Display fence observer initialization failed");}
  try{thread_=std::thread([this]{loop();});}catch(...){closeHandles();throw;}
 }
 uint64_t enqueue(Fence fence,FenceRecord988 record){std::lock_guard lock(mutex_);record.fenceTag=uint64_t(reinterpret_cast<uintptr_t>(fence.Get()));record.published=now();size_t slot=0;auto seq=core_.enqueue(record,slot);if(seq)fences_[slot]=std::move(fence);return seq;}
 void bind(uint64_t seq,uint64_t id){if(!seq)return;std::lock_guard lock(mutex_);core_.bind(seq,id);}
 bool failed()const noexcept{return failed_.load();}
 void shutdown(){if(joined_)return;SetEvent(stop_);if(WaitForSingleObject(done_,2000)!=WAIT_OBJECT_0){smx_log_fprintf989(stderr,"DISPLAY_FENCE988 observer shutdown timeout; terminating own process without releasing pending resources\n");fflush(stderr);TerminateProcess(GetCurrentProcess(),86);std::terminate();}if(thread_.joinable())thread_.join();joined_=true;write();closeHandles();}
 ~DisplayFenceProbe988(){shutdown();}
};
}
