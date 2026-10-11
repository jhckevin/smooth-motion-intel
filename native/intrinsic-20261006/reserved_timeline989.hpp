#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
// R980: lifecycle cancellation is explicit and is never counted as a successful Present.
// All methods must still be serialized by the caller. closeLifecycle980 is called
// only by the display owner after a native D3D access has signalled/flushed, before
// Present starts. It does NOT cancel GPU commands: pending905 must retain all leases.
enum class SmxLifecycleReason980 { Pause, Stop };
struct SmxCompletion980 { bool onTime=false,lifecycleCancelled=false;SmxLifecycleReason980 reason=SmxLifecycleReason980::Pause; };
struct SmxLifecycleCounts980 {
 uint64_t pauseEpochs=0,stopEpochs=0,originalPause=0,originalStop=0;
 uint64_t completedPause=0,completedStop=0,beforeExecutionPause=0,beforeExecutionStop=0;
 uint64_t rejectedOriginal=0,rejectedGeneration=0;
 uint64_t completedCancelled()const{return completedPause+completedStop;}
};
// Content stamps remain untouched. This estimates presentation phase only.
class SourcePhase971 {
 std::deque<double> history_;
 double period_=0,phase_=0,lastDue_=-std::numeric_limits<double>::infinity();
 static double median(std::vector<double> v){std::sort(v.begin(),v.end());size_t m=v.size()/2;return v.size()%2?v[m]:(v[m-1]+v[m])*.5;}
 public:
 uint64_t epochs=0,boundClips=0,orderClips=0,rateResets=0;
 double delay=.080,bound=.008,maxDelay=.088;
 explicit SourcePhase971(double d):delay(d),maxDelay(d+.008){}
 void reset(){history_.clear();period_=phase_=0;++epochs;}
 double period()const{return period_;}
 double advance(double t){
  if(!std::isfinite(t))throw std::invalid_argument("phase source clock");
  if(!history_.empty()&&period_&&(t-history_.back()>1.75*period_||t-history_.back()<.4*period_))reset();
  history_.push_back(t);while(history_.size()>32)history_.pop_front();
  double candidate=t+delay;
  if(history_.size()<9)phase_=t;
  else{
   std::vector<double> slopes;for(size_t i=8;i<history_.size();++i)slopes.push_back((history_[i]-history_[i-8])/8);
   double estimate=median(std::move(slopes));
   if(estimate<.004||estimate>.200){reset();history_.push_back(t);phase_=t;}
   else if(!period_){period_=estimate;phase_=t;}
   else{
    period_+=std::clamp(estimate-period_,-.000020,.000020);
    double predicted=phase_+period_,error=t-predicted;
    double corrected=predicted+std::clamp(error/16,-.000250,.000250);
    double limited=std::clamp(corrected,t-bound,t+bound);limited=std::min(limited,t+maxDelay-delay);
    if(std::abs(limited-corrected)>1e-9)++boundClips;
    phase_=limited;
   }
   candidate=phase_+delay;
  }
  // A reset never moves a new original before a previously accepted original.
  // The absolute per-content cap, not lastDue+period, prevents cumulative latency growth.
  double floor=std::nextafter(lastDue_,std::numeric_limits<double>::infinity());
  if(candidate<floor){candidate=floor;++orderClips;}
  if(candidate>t+maxDelay+1e-9)throw std::runtime_error("Phase order cannot fit absolute latency cap");
  lastDue_=candidate;return candidate;
 }
};
// Caller serializes ALL methods. Successful Present acknowledgement is not display proof.
template<class Payload> class OriginalTimeline {
 public:
 struct Entry {Payload payload{};double stamp=0;bool generated=false;uint64_t ticket=0;bool ready=true;double due=0;};
 struct Counters {size_t originalQueued=0,generatedQueued=0,originalDropped=0,generatedDropped=0,invalid=0,reserved=0,completed=0,submitted=0,cancelled=0,preSubmitRejected=0,lateCompleted=0,lateSubmitted=0,gpuArmed=0,submittedBeforeCompletion=0;};
 explicit OriginalTimeline(double delay,size_t capacity=24,double tolerance=1.0/120.0,bool phaseLock971=false):delay_(delay),tolerance_(tolerance),capacity_(capacity),phaseEnabled_(phaseLock971),phase_(delay){if(!std::isfinite(delay)||delay<0||!capacity||!std::isfinite(tolerance)||tolerance<0)throw std::invalid_argument("timeline settings");}
 void setPhaseTrace971(FILE*f){phaseTrace_=f;if(f)fprintf(f,"event,epoch,content_stamp,presentation_due,phase_offset_ms,estimated_period_ms,max_content_to_due_ms,ticket\n");}
 void resetPacing971(const char*reason){if(!phaseEnabled_)return;phase_.reset();sourceDues_.clear();if(phaseTrace_)fprintf(phaseTrace_,"reset_%s,%llu,0,0,0,0,%.6f,0\n",reason,(unsigned long long)phase_.epochs,phase_.maxDelay*1000);}
 bool phaseEnabled971()const{return phaseEnabled_;}
 const SourcePhase971&phase971()const{return phase_;}
 std::optional<double> pairDue971(double first,double second)const{
  if(!std::isfinite(first)||!std::isfinite(second)||second<=first)return {};
  if(!phaseEnabled_)return (first+second)*.5+delay_;
  auto a=std::find_if(sourceDues_.begin(),sourceDues_.end(),[&](const SourceDue&e){return std::abs(e.stamp-first)<1e-7;});
  auto b=std::find_if(sourceDues_.begin(),sourceDues_.end(),[&](const SourceDue&e){return std::abs(e.stamp-second)<1e-7;});
  if(a==sourceDues_.end()||b==sourceDues_.end())return {};
  return (a->due+b->due)*.5;
 }
 uint64_t reservePair971(double first,double second,double now,double expectedSeconds,size_t maxPending=4){if(lifecycleClosed_980){lastReserveReject988_="closed";++counts.preSubmitRejected;++lifecycle980.rejectedGeneration;return 0;}auto due=pairDue971(first,second);if(!due){lastReserveReject988_=(!std::isfinite(first)||!std::isfinite(second)||second<=first)?"invalid":"phase_no_due";++counts.preSubmitRejected;return 0;}return reserveDue971((first+second)*.5,*due,now,expectedSeconds,maxPending);}

 // R989: reserve genuine subframe timestamps using the same source phase mapping.
 // Caller holds its existing queue lock; midpoint R988 remains unchanged.
 std::optional<double> pairDueFraction989(double first,double second,unsigned numerator,unsigned denominator)const{
  if(!std::isfinite(first)||!std::isfinite(second)||second<=first||denominator<2||numerator==0||numerator>=denominator)return {};
  const double fraction=double(numerator)/denominator;
  if(!phaseEnabled_)return first+(second-first)*fraction+delay_;
  auto a=std::find_if(sourceDues_.begin(),sourceDues_.end(),[&](const SourceDue&e){return std::abs(e.stamp-first)<1e-7;});
  auto b=std::find_if(sourceDues_.begin(),sourceDues_.end(),[&](const SourceDue&e){return std::abs(e.stamp-second)<1e-7;});
  if(a==sourceDues_.end()||b==sourceDues_.end())return {};
  return a->due+(b->due-a->due)*fraction;
 }
 uint64_t reserveFraction989(double first,double second,unsigned numerator,unsigned denominator,double now,double expectedSeconds,size_t maxPending=16){
  auto due=pairDueFraction989(first,second,numerator,denominator);
  if(!due){lastReserveReject988_="invalid_fraction_or_phase";++counts.preSubmitRejected;return 0;}
  return reserveDue971(first+(second-first)*double(numerator)/denominator,*due,now,expectedSeconds,maxPending);
 }
 bool push(Payload payload,double stamp,bool generated,double now){
  if(lifecycleClosed_980){if(generated)throw std::logic_error("Generated output requires reservation");++counts.originalDropped;++lifecycle980.rejectedOriginal;return false;}
  if(generated)throw std::logic_error("Generated output requires a pre-submit reservation");
  if(!std::isfinite(stamp)||!std::isfinite(now)){++counts.invalid;return false;}
  if(stamp<=floor()||stamp<=lastOriginal_){++counts.originalDropped;return false;}
  if(!makeRoom()){++counts.originalDropped;return false;}
  double due=phaseEnabled_?phase_.advance(stamp):stamp+delay_;
  lastOriginal_=stamp;sourceDues_.push_back({stamp,due});while(sourceDues_.size()>128)sourceDues_.pop_front();insert(Entry{std::move(payload),stamp,false,0,true,due});++counts.originalQueued;trace971("original",stamp,due,0);return true;
 }
 // Compatibility for callers that do not supply a source pair. New paced generation uses reservePair971.
 uint64_t reserve(double stamp,double now,double expectedSeconds,size_t maxPending=4){if(phaseEnabled_)throw std::logic_error("Paced generation must reserve an explicit source pair");return reserveDue971(stamp,stamp+delay_,now,expectedSeconds,maxPending);}
 void armGpu(uint64_t ticket,Payload payload){auto cancelled=findCancelled980(ticket);if(cancelled!=cancelledTickets980_.end()){gpuRecords_.push_back({ticket,cancelled->stamp,cancelled->due,false,false,true,cancelled->reason});cancelledTickets980_.erase(cancelled);++counts.gpuArmed;return;}auto it=find(ticket);if(it==queue_.end()||it->ready)throw std::logic_error("Invalid GPU arm");gpuRecords_.push_back({ticket,it->stamp,it->due,false,false});it->payload=std::move(payload);it->ready=true;++counts.generatedQueued;++counts.gpuArmed;}
 SmxCompletion980 completeGpuWithStatus980(uint64_t ticket,double now){auto it=findGpu(ticket);if(it==gpuRecords_.end()||it->completed)throw std::logic_error("Unknown or duplicate GPU completion");
  if(it->lifecycleCancelled980){auto reason=it->cancelReason980;++counts.completed;countCompletedCancellation980(reason);gpuRecords_.erase(it);return {false,true,reason};}
  bool onTime=now<=it->due+tolerance_;if(!onTime)++counts.lateCompleted;++counts.completed;it->completed=true;if(it->submitted)gpuRecords_.erase(it);return {onTime,false,SmxLifecycleReason980::Pause};}
 bool completeGpu(uint64_t ticket,double now){auto result=completeGpuWithStatus980(ticket,now);return result.onTime||result.lifecycleCancelled;}
 size_t gpuCompletionPending()const{return std::count_if(gpuRecords_.begin(),gpuRecords_.end(),[](const GpuRecord&e){return !e.completed;});}
 SmxCompletion980 completeWithStatus980(uint64_t ticket,Payload payload,double now){
  auto cancelled=findCancelled980(ticket);if(cancelled!=cancelledTickets980_.end()){auto reason=cancelled->reason;cancelledTickets980_.erase(cancelled);++counts.completed;countCompletedCancellation980(reason);return {false,true,reason};}
  auto it=find(ticket);if(it==queue_.end()||it->ready)throw std::logic_error("Unknown or duplicate reservation completion");it->payload=std::move(payload);it->ready=true;++counts.completed;++counts.generatedQueued;bool onTime=now<=it->due+tolerance_;if(!onTime)++counts.lateCompleted;return {onTime,false,SmxLifecycleReason980::Pause};}
 // Compatibility only: R980 integration uses explicit completion status, not this bool.
 bool complete(uint64_t ticket,Payload payload,double now){auto result=completeWithStatus980(ticket,std::move(payload),now);return result.onTime||result.lifecycleCancelled;}
 bool cancelBeforeComplete(uint64_t ticket){auto cancelled=findCancelled980(ticket);if(cancelled!=cancelledTickets980_.end()){countBeforeExecutionCancellation980(cancelled->reason);cancelledTickets980_.erase(cancelled);++counts.cancelled;return true;}auto it=find(ticket);if(it==queue_.end())return false;if(it->ready)throw std::logic_error("Cannot cancel completed work");queue_.erase(it);++counts.cancelled;return true;}
 std::optional<Entry> takeDue(double now){if(!std::isfinite(now))throw std::invalid_argument("timeline clock");if(claimed_||queue_.empty())return {};auto&e=queue_.front();if(!e.ready||e.due>now)return {};if(e.stamp<=lastSubmitted_)throw std::logic_error("Timeline ordering violated");claimed_=std::move(e);queue_.pop_front();return claimed_;}
 void acknowledge(const Entry&e,double now){if(!claimed_||claimed_->stamp!=e.stamp||claimed_->ticket!=e.ticket)throw std::logic_error("Invalid Present acknowledgement");lastSubmitted_=e.stamp;if(e.generated){++counts.submitted;if(now>e.due+tolerance_)++counts.lateSubmitted;auto g=findGpu(e.ticket);if(g!=gpuRecords_.end()){g->submitted=true;if(g->completed)gpuRecords_.erase(g);else ++counts.submittedBeforeCompletion;}}claimed_.reset();}
 void releaseClaim(){if(claimed_){queue_.push_front(std::move(*claimed_));claimed_.reset();}}
 std::optional<double> nextDue()const{if(claimed_||queue_.empty()||!queue_.front().ready)return {};return queue_.front().due;}
 bool evictOldestOriginal(){auto it=std::find_if(queue_.begin(),queue_.end(),[](const Entry&e){return !e.generated;});if(it==queue_.end())return false;queue_.erase(it);++counts.originalDropped;return true;}
 // Do not touch committed generated entries or their due times during a foreground reset.
 void discardPending(){for(auto it=queue_.begin();it!=queue_.end();)if(!it->generated){it=queue_.erase(it);++counts.originalDropped;}else ++it;resetPacing971("pending_originals_discarded");}
 bool empty()const{return queue_.empty()&&!claimed_;}
 size_t size()const{return queue_.size()+(claimed_?1:0);}
 size_t pendingGenerated()const{return std::count_if(queue_.begin(),queue_.end(),[](const Entry&e){return e.generated;})+(claimed_&&claimed_->generated?1:0);}
 size_t completedOutstanding()const{size_t early=std::count_if(gpuRecords_.begin(),gpuRecords_.end(),[](const GpuRecord&e){return e.submitted&&!e.completed;});size_t retired=counts.submitted+size_t(lifecycle980.completedCancelled());if(counts.completed+early<retired)throw std::logic_error("Invalid completion accounting");return counts.completed+early-retired;}
 bool lifecycleClosed980()const{return lifecycleClosed_980;}
 uint64_t lifecycleEpoch980()const{return lifecycleEpoch_980;}
 SmxLifecycleReason980 lifecycleReason980()const{return lifecycleReason_980;}
 size_t pendingLifecycleCompletions980()const{return cancelledTickets980_.size()+std::count_if(gpuRecords_.begin(),gpuRecords_.end(),[](const GpuRecord&e){return e.lifecycleCancelled980&&!e.completed;});}
 // Call only from the thread that owns activeClaim, at a point with no active
 // D3D native transaction and no Present already begun. The caller resets its
 // optional<Entry> after this returns. No lease may be released before its GPU fence.
 void closeLifecycle980(uint64_t epoch,SmxLifecycleReason980 reason,const Entry*activeClaim=nullptr){
  if(reason!=SmxLifecycleReason980::Pause&&reason!=SmxLifecycleReason980::Stop)throw std::invalid_argument("Lifecycle reason");
  if(!epoch||epoch<lifecycleEpoch_980)throw std::logic_error("Stale lifecycle close");
  if(lifecycleClosed_980&&epoch==lifecycleEpoch_980){if(reason!=lifecycleReason_980)throw std::logic_error("Different cause requires a new lifecycle epoch");return;}
  if(epoch==lifecycleEpoch_980)throw std::logic_error("Reuse of resumed lifecycle epoch");
  if(claimed_&&(!activeClaim||activeClaim->ticket!=claimed_->ticket||activeClaim->stamp!=claimed_->stamp||activeClaim->generated!=claimed_->generated))throw std::logic_error("Lifecycle close must own active claim");
  if(!claimed_&&activeClaim)throw std::logic_error("Lifecycle close has stale claim");
  size_t newTickets=std::count_if(queue_.begin(),queue_.end(),[](const Entry&e){return e.generated&&!e.ready;});
  if(cancelledTickets980_.size()+newTickets>capacity_)throw std::logic_error("Lifecycle cancellation capacity");
  lifecycleClosed_980=true;lifecycleEpoch_980=epoch;lifecycleReason_980=reason;if(reason==SmxLifecycleReason980::Pause)++lifecycle980.pauseEpochs;else ++lifecycle980.stopEpochs;
  for(const auto&e:queue_){cancelEntry980(e,reason);}
  queue_.clear();if(claimed_){cancelEntry980(*claimed_,reason);claimed_.reset();}resetPacing971(reason==SmxLifecycleReason980::Pause?"lifecycle_pause":"lifecycle_stop");
 }
 // Admission remains closed until all prior GPU jobs have really retired and
 // the worker has cleared stale inbox/pair caches. The UI becoming foreground
 // alone is insufficient to open this epoch.
 void resumeLifecycle980(uint64_t epoch){if(!lifecycleClosed_980||epoch!=lifecycleEpoch_980)throw std::logic_error("Stale lifecycle resume");if(lifecycleReason_980==SmxLifecycleReason980::Stop)throw std::logic_error("Stopped timeline requires a new session");if(!queue_.empty()||claimed_||pendingLifecycleCompletions980()||gpuCompletionPending())throw std::logic_error("Lifecycle resume before retirement");lifecycleClosed_980=false;resetPacing971("lifecycle_resume");}
 SmxLifecycleCounts980 lifecycle980;


 // R988 is optional display-thread-only backlog protection. All calls, including
 // notePresentationRecovery988, remain under the caller's queue mutex. Clock is
 // actual QPC seconds, never now+prepareLead. This does not change model work/due.
 struct RecoveryState988 {
  bool requested=false,enabled=false,periodReliable=false,active=false,rearmAllowed=true,progressWindowEnabled=false,sustainedRecoveryEnabled=false,claimedRecoveryEnabled=false;
  double tokens=3,tokenCapacity=3,tokenRefillPerSecond=1,refilledTokens=0,lastRefillQpc=-std::numeric_limits<double>::infinity(),lastEpisodeEnd=-std::numeric_limits<double>::infinity(),rearmCooldown=.250;
  uint64_t claimedSafetyBlocks=0,claimedSuccessorBlocks=0;
  uint64_t queuedDrops=0,claimedDrops=0,tokenBudgetBlocks=0,tokenRearmBlocks=0,cooldownBlocks=0,sustainedRearms=0;
  double outputPeriod=0,episodeStart=0,episodeLastProgress=0,episodeHardLimit=.250,episodeIdleLimit=.250,lastDrop=-std::numeric_limits<double>::infinity();
  double lastAckLate=0,lastSwapWait=0,episodeInitialLate=0,episodeLastLate=0,maxAckLate=0;
  uint64_t episodes=0,originalDrops=0,dropCalls=0,exhaustedEpisodes=0,expiredEpisodes=0,expiredIdleEpisodes=0,expiredAbsoluteEpisodes=0,healthyRecoveries=0;
  uint64_t claimedBlocks=0,generatedHeadBlocks=0,throttledCalls=0,lifecycleBlocks=0,lifecycleInterruptions=0;
  unsigned episodeDrops=0,lateAckStreak=0,healthyAckStreak=0;
 };
 // A zero/unreliable/out-of-range output period leaves protection disabled. The
 // caller may explicitly supply 1/60 for a documented 30->60 diagnostic. It must
 // not silently assume all games/displays have this rate.
 void configureRecovery988(bool requested,double outputPeriod,bool reliable,FILE*trace=nullptr,bool progressWindow=false,bool sustainedRecovery=false,bool claimedRecovery=false){
  if(claimedRecovery&&(!sustainedRecovery||!requested))throw std::invalid_argument("Claimed recovery requires explicit sustained original recovery");
  if(recoveryConfigured988_)throw std::logic_error("Recovery configuration is immutable for a session");
  recoveryConfigured988_=true;recoveryTrace988_=trace;
  recovery988_.requested=requested;recovery988_.outputPeriod=outputPeriod;
  recovery988_.progressWindowEnabled=progressWindow;recovery988_.episodeHardLimit=progressWindow?.500:.250;
  recovery988_.periodReliable=reliable&&std::isfinite(outputPeriod)&&outputPeriod>=.004&&outputPeriod<=.100;
  recovery988_.enabled=requested&&recovery988_.periodReliable;
  recovery988_.sustainedRecoveryEnabled=sustainedRecovery&&recovery988_.enabled;
  recovery988_.claimedRecoveryEnabled=claimedRecovery&&recovery988_.sustainedRecoveryEnabled;
  if(trace){fprintf(trace,"event,qpc,episode,episode_drops,total_original_drops,stamp,due,original_late_ms,ack_late_ms,swap_wait_ms,output_period_ms,enabled,deadline_tolerance_ms,hard_episode_limit_ms,idle_limit_ms,progress_window,sustained,tokens,token_capacity,refill_per_second,refilled_tokens,cooldown_remaining_ms,queued_drops,claimed_drops,token_budget_blocks,token_rearm_blocks\n");recoveryTraceEvent988("configured",0,0,0);}
 }
 const RecoveryState988& recovery988()const{return recovery988_;}
 // Read immediately after reserve under the same queueMutex. Stable literals; no IO/allocation.
 const char*lastReserveReject988()const noexcept{return lastReserveReject988_;}
 // Call exactly once after each successful Present acknowledgement. due remains
 // the original immutable deadline. Pending work and lost observations do not
 // create synthetic acknowledgements.
 void notePresentationRecovery988(double now,double due,double swapWaitSeconds){
  if(!recovery988_.enabled)return;
  if(!std::isfinite(due)||!std::isfinite(swapWaitSeconds)||swapWaitSeconds<0)throw std::invalid_argument("Recovery acknowledgement values");
  recoveryCheckClock988(now);recoveryObserveLifecycle988(now);recoveryRefill988(now);recoveryExpire988(now);
  auto&r=recovery988_;r.lastAckLate=now-due;r.lastSwapWait=swapWaitSeconds;
  r.episodeLastLate=r.lastAckLate;r.maxAckLate=std::max(r.maxAckLate,r.lastAckLate);
  // Use the existing acceptance comparison, including its floating-point boundary.
  // A within-budget ACK is healthy even if taking queue credit consumed part of it.
  bool healthy=now<=due+tolerance_;
  r.healthyAckStreak=healthy?std::min(r.healthyAckStreak+1,8u):0;
  if(r.healthyAckStreak>=8){
   if(!r.rearmAllowed){++r.healthyRecoveries;if(r.sustainedRecoveryEnabled)r.lastEpisodeEnd=now;recoveryTraceEvent988("healthy_rearmed",now,0,0);}
   r.active=false;r.rearmAllowed=true;r.lateAckStreak=0;return;
  }
  bool pressured=now>due+tolerance_&&swapWaitSeconds>r.outputPeriod*.5;
  if(r.sustainedRecoveryEnabled&&!r.active){
   // Require two fresh pressured ACKs AFTER cooldown, never cached pre-expiry evidence.
   if(lifecycleClosed_980){r.lateAckStreak=0;return;}
   if(now<r.lastEpisodeEnd+r.rearmCooldown){r.lateAckStreak=0;if(pressured)++r.cooldownBlocks;return;}
   r.lateAckStreak=pressured?std::min(r.lateAckStreak+1,2u):0;
   if(r.lateAckStreak>=2&&!r.rearmAllowed){
    if(r.tokens<1){++r.tokenRearmBlocks;return;}
    r.rearmAllowed=true;++r.sustainedRearms;recoveryTraceEvent988("sustained_rearmed",now,0,0);
   }
  }else r.lateAckStreak=pressured?std::min(r.lateAckStreak+1,2u):0;
  if(!lifecycleClosed_980&&r.rearmAllowed&&r.lateAckStreak>=2){
   if(r.sustainedRecoveryEnabled&&r.tokens<1){++r.tokenRearmBlocks;return;}
   r.active=true;r.rearmAllowed=false;r.episodeStart=r.episodeLastProgress=now;r.episodeDrops=0;
   r.episodeInitialLate=r.lastAckLate;++r.episodes;recoveryTraceEvent988("episode_begin",now,0,0);
  }
 }
 // Only unclaimed, queue-head originals already beyond the unchanged ACK tolerance qualify.
 // Never walks across a generated entry, never erases a ticket/lease record,
 // never retimes a frame, and never advances lastSubmitted_. Per-call 2, per-
 // episode 3, >=one output period between drop calls. Default <=250ms; explicit
 // progressWindow permits <=500ms absolute AND <=250ms since last actual drop. After
 // exhaustion/expiry keeps R983 healthy-only rearming unless sustained mode is
 // explicitly enabled. Sustained mode adds a shared cap-3 token bucket refilling
 // 1/s, fresh pressured ACKs and cooldown. This is NOT an inference speedup.
 size_t recoverQueuedOriginals988(double now){
  if(!recovery988_.enabled)return 0;
  recoveryCheckClock988(now);recoveryObserveLifecycle988(now);recoveryRefill988(now);recoveryExpire988(now);auto&r=recovery988_;
  if(lifecycleClosed_980){++r.lifecycleBlocks;return 0;}
  if(!r.active)return 0;
  if(claimed_){++r.claimedBlocks;return 0;}
  if(now-r.lastDrop<r.outputPeriod){++r.throttledCalls;return 0;}
  size_t dropped=0;
  while(dropped<2&&r.episodeDrops<3&&!queue_.empty()){
   const auto&e=queue_.front();
   if(e.generated){++r.generatedHeadBlocks;break;}
   if(!e.ready||now<=e.due+tolerance_)break;
   if(r.sustainedRecoveryEnabled&&r.tokens<1){++r.tokenBudgetBlocks;break;}
   // Copy diagnostic scalars before releasing only this queue's payload ref.
   double stamp=e.stamp,due=e.due;queue_.pop_front();++counts.originalDropped;
   ++r.originalDrops;++r.queuedDrops;++r.episodeDrops;++dropped;if(r.sustainedRecoveryEnabled)r.tokens-=1;
   recoveryTraceEvent988("original_dropped",now,stamp,due);
   trace971("recovery_original_dropped",stamp,due,0);
  }
  if(dropped){r.lastDrop=r.episodeLastProgress=now;++r.dropCalls;}
  if(r.episodeDrops>=3){recoveryEndEpisode988(now);++r.exhaustedEpisodes;recoveryTraceEvent988("episode_budget_exhausted",now,0,0);}
  return dropped;
 }
 // Display-thread only, under queueMutex. The caller must have acquired DXGI
 // credit and MUST NOT have begun displayTexture/access/Copy/Present for this claim.
 // On success reset only the caller's Entry ref; KEEP the credit for the next loop.
 // Natural capture drain and pending pause/stop must be checked before calling.
 // This helper never transfers claim, touches a generated ticket, or retimes due.
 bool recoverClaimedOriginalAfterCredit988(const Entry&e,double now,double prepareLead,
                                            bool creditAcquired,bool gpuAccessStarted){
  auto&r=recovery988_;if(!r.claimedRecoveryEnabled)return false;
  if(!creditAcquired||gpuAccessStarted){++r.claimedSafetyBlocks;return false;}
  if(!std::isfinite(prepareLead)||prepareLead<0||prepareLead>.032)throw std::invalid_argument("Claimed recovery preparation lead");
  recoveryCheckClock988(now);recoveryObserveLifecycle988(now);recoveryRefill988(now);recoveryExpire988(now);
  if(lifecycleClosed_980){++r.lifecycleBlocks;return false;}
  if(!claimed_||claimed_->stamp!=e.stamp||claimed_->ticket!=e.ticket||claimed_->generated!=e.generated||claimed_->due!=e.due)throw std::logic_error("Claimed recovery must own exact active entry");
  if(e.generated){++r.claimedSafetyBlocks;return false;}
  if(!r.active||now<=e.due+tolerance_)return false;
  if(now-r.lastDrop<r.outputPeriod){++r.throttledCalls;return false;}
  if(r.episodeDrops>=3)return false;
  // Restrict the first implementation to making room for a ready generated frame.
  if(queue_.empty()||!queue_.front().generated||!queue_.front().ready||queue_.front().due>now+prepareLead){++r.claimedSuccessorBlocks;return false;}
  const auto&next=queue_.front();if(next.stamp<=e.stamp||next.due<e.due)throw std::logic_error("Claimed recovery successor ordering");
  if(r.tokens<1){++r.tokenBudgetBlocks;return false;}
  const double stamp=e.stamp,due=e.due;
  // The caller still owns its Entry ref until after return. Other model/capture
  // leases and the shared-resource GPU fence retain their original lifetimes.
  claimed_.reset();++counts.originalDropped;++r.originalDrops;++r.claimedDrops;++r.episodeDrops;
  r.tokens-=1;r.lastDrop=r.episodeLastProgress=now;++r.dropCalls;
  recoveryTraceEvent988("claimed_original_dropped",now,stamp,due);trace971("recovery_claimed_original_dropped",stamp,due,0);
  if(r.episodeDrops>=3){recoveryEndEpisode988(now);++r.exhaustedEpisodes;recoveryTraceEvent988("episode_budget_exhausted",now,0,0);}
  return true;
 }
 Counters counts;
 private:

 const char*lastReserveReject988_="none";
 RecoveryState988 recovery988_;FILE*recoveryTrace988_=nullptr;bool recoveryConfigured988_=false;
 double recoveryLastClock988_=-std::numeric_limits<double>::infinity();
 uint64_t recoveryLifecycleEpoch988_=0;bool recoveryLifecycleClosed988_=false;
 void recoveryCheckClock988(double now){
  if(!std::isfinite(now)||now<0||now<recoveryLastClock988_)throw std::invalid_argument("Recovery clock invalid or regressed");
  recoveryLastClock988_=now;
 }
 void recoveryObserveLifecycle988(double now){
  if(recoveryLifecycleEpoch988_==lifecycleEpoch_980&&recoveryLifecycleClosed988_==lifecycleClosed_980)return;
  auto&r=recovery988_;if(r.active)++r.lifecycleInterruptions;
  r.active=false;r.rearmAllowed=false;r.lateAckStreak=r.healthyAckStreak=0;
  if(r.sustainedRecoveryEnabled){r.tokens=0;r.lastRefillQpc=now;r.lastEpisodeEnd=now;}
  recoveryLifecycleEpoch988_=lifecycleEpoch_980;recoveryLifecycleClosed988_=lifecycleClosed_980;
  recoveryTraceEvent988("lifecycle_control_reset",now,0,0);
 }
 void recoveryRefill988(double now){
  auto&r=recovery988_;if(!r.sustainedRecoveryEnabled)return;
  if(!std::isfinite(r.lastRefillQpc)){r.lastRefillQpc=now;return;}
  if(!lifecycleClosed_980){double credited=std::min(r.tokenCapacity-r.tokens,(now-r.lastRefillQpc)*r.tokenRefillPerSecond);r.tokens+=credited;r.refilledTokens+=credited;}
  r.lastRefillQpc=now;
 }
 void recoveryEndEpisode988(double now){
  auto&r=recovery988_;r.active=false;
  if(r.sustainedRecoveryEnabled){r.lastEpisodeEnd=now;r.rearmAllowed=false;r.lateAckStreak=r.healthyAckStreak=0;}
 }
 void recoveryExpire988(double now){
  auto&r=recovery988_;if(!r.active)return;
  if(now-r.episodeStart>r.episodeHardLimit){recoveryEndEpisode988(now);++r.expiredEpisodes;++r.expiredAbsoluteEpisodes;recoveryTraceEvent988("episode_absolute_expired",now,0,0);}
  else if(now-r.episodeLastProgress>r.episodeIdleLimit){recoveryEndEpisode988(now);++r.expiredEpisodes;++r.expiredIdleEpisodes;recoveryTraceEvent988("episode_idle_expired",now,0,0);}
 }
 void recoveryTraceEvent988(const char*event,double now,double stamp,double due){
  if(!recoveryTrace988_)return;const auto&r=recovery988_;
  fprintf(recoveryTrace988_,"%s,%.9f,%llu,%u,%llu,%.9f,%.9f,%.6f,%.6f,%.6f,%.6f,%u,%.6f,%.6f,%.6f,%u,%u,%.6f,%.6f,%.6f,%.6f,%.6f,%llu,%llu,%llu,%llu\n",event,now,(unsigned long long)r.episodes,r.episodeDrops,(unsigned long long)r.originalDrops,stamp,due,due?(now-due)*1000:0,r.lastAckLate*1000,r.lastSwapWait*1000,r.outputPeriod*1000,unsigned(r.enabled),tolerance_*1000,r.episodeHardLimit*1000,r.episodeIdleLimit*1000,unsigned(r.progressWindowEnabled),unsigned(r.sustainedRecoveryEnabled),r.tokens,r.tokenCapacity,r.tokenRefillPerSecond,r.refilledTokens,std::isfinite(r.lastEpisodeEnd)?std::max(0.0,(r.lastEpisodeEnd+r.rearmCooldown-now)*1000):0.0,(unsigned long long)r.queuedDrops,(unsigned long long)r.claimedDrops,(unsigned long long)r.tokenBudgetBlocks,(unsigned long long)r.tokenRearmBlocks);
 }
 struct SourceDue{double stamp,due;};std::deque<SourceDue>sourceDues_;
 struct GpuRecord{uint64_t ticket;double stamp,due;bool completed,submitted;bool lifecycleCancelled980=false;SmxLifecycleReason980 cancelReason980=SmxLifecycleReason980::Pause;};std::deque<GpuRecord>gpuRecords_;
 struct CancelledTicket980{uint64_t ticket;double stamp,due;SmxLifecycleReason980 reason;};
 std::deque<CancelledTicket980>cancelledTickets980_;bool lifecycleClosed_980=false;uint64_t lifecycleEpoch_980=0;SmxLifecycleReason980 lifecycleReason_980=SmxLifecycleReason980::Pause;
 auto findCancelled980(uint64_t ticket){return std::find_if(cancelledTickets980_.begin(),cancelledTickets980_.end(),[&](const CancelledTicket980&e){return e.ticket==ticket;});}
 void countCompletedCancellation980(SmxLifecycleReason980 reason){if(reason==SmxLifecycleReason980::Pause)++lifecycle980.completedPause;else ++lifecycle980.completedStop;}
 void countBeforeExecutionCancellation980(SmxLifecycleReason980 reason){if(reason==SmxLifecycleReason980::Pause)++lifecycle980.beforeExecutionPause;else ++lifecycle980.beforeExecutionStop;}
 void cancelEntry980(const Entry&e,SmxLifecycleReason980 reason){
  if(!e.generated){++counts.originalDropped;if(reason==SmxLifecycleReason980::Pause)++lifecycle980.originalPause;else ++lifecycle980.originalStop;return;}
  if(!e.ready){cancelledTickets980_.push_back({e.ticket,e.stamp,e.due,reason});return;}
  auto gpu=findGpu(e.ticket);if(gpu!=gpuRecords_.end()){if(gpu->submitted)throw std::logic_error("Cannot lifecycle-cancel an acknowledged Present");if(gpu->completed){countCompletedCancellation980(reason);gpuRecords_.erase(gpu);}else{gpu->lifecycleCancelled980=true;gpu->cancelReason980=reason;}return;}
  countCompletedCancellation980(reason);
 }
 uint64_t reserveDue971(double stamp,double due,double now,double expectedSeconds,size_t maxPending){
  if(lifecycleClosed_980){lastReserveReject988_="closed";++counts.preSubmitRejected;++lifecycle980.rejectedGeneration;return 0;}
  if(!std::isfinite(stamp)||!std::isfinite(due)||!std::isfinite(now)||!std::isfinite(expectedSeconds)||expectedSeconds<0){lastReserveReject988_="invalid";++counts.invalid;return 0;}
  // Preserve the original short-circuit order and admission predicates exactly.
  const char*reject=stamp<=floor()?"floor":due<now+expectedSeconds?"deadline":(pendingGenerated()>=maxPending||queue_.size()+(claimed_?1:0)>=capacity_)?"capacity":nullptr;
  if(reject){lastReserveReject988_=reject;++counts.preSubmitRejected;return 0;}
  if(std::any_of(queue_.begin(),queue_.end(),[&](const Entry&e){return std::abs(e.stamp-stamp)<1e-9;})){lastReserveReject988_="duplicate";++counts.preSubmitRejected;return 0;}
  uint64_t id=++serial_;insert(Entry{{},stamp,true,id,false,due});++counts.reserved;lastReserveReject988_="none";trace971("generated_reserved",stamp,due,id);return id;
 }
 void trace971(const char*event,double stamp,double due,uint64_t ticket){if(phaseTrace_)fprintf(phaseTrace_,"%s,%llu,%.9f,%.9f,%.6f,%.6f,%.6f,%llu\n",event,(unsigned long long)phase_.epochs,stamp,due,(due-stamp-delay_)*1000,phase_.period()*1000,phase_.maxDelay*1000,(unsigned long long)ticket);}
 auto findGpu(uint64_t ticket){return std::find_if(gpuRecords_.begin(),gpuRecords_.end(),[&](const GpuRecord&e){return e.ticket==ticket;});}
 double floor()const{return claimed_?std::max(lastSubmitted_,claimed_->stamp):lastSubmitted_;}
 bool makeRoom(){while(size()>=capacity_)if(!evictOldestOriginal())return false;return true;}
 void insert(Entry e){auto p=std::upper_bound(queue_.begin(),queue_.end(),e.stamp,[](double t,const Entry&e){return t<e.stamp;});queue_.insert(p,std::move(e));}
 auto find(uint64_t t){return std::find_if(queue_.begin(),queue_.end(),[&](const Entry&e){return e.generated&&e.ticket==t;});}
 std::deque<Entry>queue_;std::optional<Entry>claimed_;double delay_,tolerance_,lastSubmitted_=-std::numeric_limits<double>::infinity(),lastOriginal_=-std::numeric_limits<double>::infinity();size_t capacity_;uint64_t serial_=0;bool phaseEnabled_=false;SourcePhase971 phase_;FILE*phaseTrace_=nullptr;
};
