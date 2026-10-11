#include "bounded_stdio989.h"
#pragma once
#include <cstdint>
#include <cmath>
namespace smx988 {
// Thread-local presentation progress. No GPU state is inferred from a CPU timer.
enum class DisplayStage988 {Empty,Claimed,AccessStarted,CopyReleased,PresentUnacknowledged,Acknowledged};
enum class FailureClose988 {NoClaim,CancelClaim,FailStop};
inline const char* displayStageName988(DisplayStage988 s){switch(s){case DisplayStage988::Empty:return "empty";case DisplayStage988::Claimed:return "claimed_unaccessed";case DisplayStage988::AccessStarted:return "ownership_or_copy_incomplete";case DisplayStage988::CopyReleased:return "copy_signal_flush_complete";case DisplayStage988::PresentUnacknowledged:return "present_result_unacknowledged";case DisplayStage988::Acknowledged:return "acknowledged";}return "invalid";}
inline FailureClose988 failureClose988(DisplayStage988 stage,bool hasEntry){
 if(stage==DisplayStage988::AccessStarted||stage==DisplayStage988::PresentUnacknowledged)return FailureClose988::FailStop;
 if(stage==DisplayStage988::Acknowledged)return FailureClose988::NoClaim;
 if(stage==DisplayStage988::Empty)return hasEntry?FailureClose988::FailStop:FailureClose988::NoClaim;
 return hasEntry?FailureClose988::CancelClaim:FailureClose988::NoClaim;
}
enum class PreparedWait988 {TimerSignalled,TimeoutAfterDue,Failure};
inline PreparedWait988 classifyPreparedWait988(uint32_t result,double now,double originalDue){
 if(!std::isfinite(now)||!std::isfinite(originalDue))return PreparedWait988::Failure;
 if(result==0)return PreparedWait988::TimerSignalled;
 // WAIT_TIMEOUT is a successful wait outcome, not a failed API call. Once the
 // original deadline has passed, the same reserved frame may be submitted late.
 if(result==0x102u&&now>=originalDue)return PreparedWait988::TimeoutAfterDue;
 return PreparedWait988::Failure;
}
}
