#pragma once
#include "deadline_guard830.hpp"
struct DeadlineGuard930:DeadlineGuard830 {
 struct Sample {unsigned n=0;double mean=30,dev=0;
  double predict()const{return n?mean+std::min(dev,8.0)+2.0:30.0;}
  void add(double v){double error=std::abs(v-mean);mean=n?.5*mean+.5*v:v;dev=n?.5*dev+.5*error:0;++n;}
 };
 Sample idle930,warm930;double previousFence930=0,lastIncrement930=30,lastAhead930=0;bool lastBusy930=false;
 void resetHistory(){DeadlineGuard830::resetHistory();idle930=Sample{};warm930=Sample{};previousFence930=0;}
 bool admitPipeline(double available,double now,double ahead,bool busy){
  lastBusy930=busy;lastAhead930=busy?std::max(2.0,ahead):0.0;
  lastIncrement930=busy?warm930.predict():idle930.predict();if(!busy&&lastCompletion>=0){double age=std::max(0.0,now-lastCompletion-.05);lastIncrement930=30.0+(lastIncrement930-30.0)*std::exp(-age/.20);}lastEstimate=lastAhead930+lastIncrement930;probePending=false;
  if(!std::isfinite(available)||available<std::max(8.0,lastEstimate)){++skips;return false;}return true;
 }
 void completePipeline(double submitted,double fence,double wall,bool late,double now){
  bool warm=submitted<previousFence930;double incremental=1000*(fence-std::max(submitted,previousFence930));
  if(std::isfinite(incremental)&&incremental>0&&incremental<5000)(warm?warm930:idle930).add(incremental);
  previousFence930=std::max(previousFence930,fence);DeadlineGuard830::complete(wall,late,now);
 }
};
