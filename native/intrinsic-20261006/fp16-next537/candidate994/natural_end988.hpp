#include "bounded_stdio989.h"
#pragma once
#include <atomic>
#include <cmath>
#include <stdexcept>
namespace smx988 {
// Control thread owns all state except requested(), which display may read.
// No GPU ownership or clock adjustment is performed here.
class NaturalEnd988 {
public:
 enum class State {Idle,Draining,Complete,TimedOut,Interrupted};
 void begin(double now,double budget){
  if(requested()||!std::isfinite(now)||!std::isfinite(budget)||budget<.200||budget>.300)throw std::invalid_argument("natural end request");
  start_=now;deadline_=now+budget;state_=State::Draining;requested_.store(true,std::memory_order_release);
 }
 State observe(double now,bool internal,unsigned tail,bool statsValid,unsigned observed,bool interruption){
  if(state_!=State::Draining)return state_;
  if(!std::isfinite(now)||now<start_)throw std::invalid_argument("natural end clock");
  tail_=tail;observed_=statsValid?observed:0;
  if(internal&&!internal_){internal_=true;internalAt_=now;}
  tailObserved_=internal_&&tail_>0&&statsValid&&observed_==tail_;
  // A concurrent explicit Stop/pause always wins over an otherwise empty queue.
  if(interruption){state_=State::Interrupted;finish_=now;}
  // Readiness first noticed after the deadline cannot retroactively pass.
  else if(now>deadline_){state_=State::TimedOut;finish_=now;}
  else if(internal_&&tailObserved_){state_=State::Complete;finish_=now;}
  return state_;
 }
 bool requested()const{return requested_.load(std::memory_order_acquire);}
 bool complete()const{return state_==State::Complete;}
 bool timedOut()const{return state_==State::TimedOut;}
 bool interrupted()const{return state_==State::Interrupted;}
 bool internalDrained()const{return internal_;}
 bool tailObserved()const{return tailObserved_;}
 double start()const{return start_;}double deadline()const{return deadline_;}double finish()const{return finish_;}
 double elapsedMs()const{return requested()?(finish_-start_)*1000:0;}
 double internalTime()const{return internalAt_;}
 unsigned tailId()const{return tail_;}unsigned observedId()const{return observed_;}
 const char*stateName()const{switch(state_){case State::Idle:return "not_requested";case State::Draining:return "draining";case State::Complete:return "complete";case State::TimedOut:return "timed_out";case State::Interrupted:return "interrupted";}return "invalid";}
private:
 std::atomic<bool>requested_{false};State state_=State::Idle;bool internal_=false,tailObserved_=false;
 double start_=0,deadline_=0,finish_=0,internalAt_=0;unsigned tail_=0,observed_=0;
};
}
