#pragma once
#include <array>
#include <algorithm>
#include <cmath>
struct DeadlineGuard830 {
 std::array<double,32> samples{};unsigned count=0,next=0,skips=0,executed=0,completedLate=0,revalidations=0;
 double lastEstimate=30,lastCompletion=-1,service=30,deviation=0;bool probePending=false;
 void resetHistory(){samples.fill(0);count=next=0;lastEstimate=service=30;deviation=0;lastCompletion=-1;probePending=false;}
 double estimate()const{return count?service+std::min(deviation,8.0)+2.0:30.0;}
 double forecast(double now)const{double e=estimate();if(lastCompletion<0)return e;double age=std::max(0.0,now-lastCompletion-0.05);return 30.0+(e-30.0)*std::exp(-age/0.20);}
 bool admit(double available,double now){lastEstimate=forecast(now);probePending=count&&lastCompletion>=0&&now-lastCompletion>.25;if(probePending)++revalidations;if(!std::isfinite(available)||available<std::max(8.0,lastEstimate)){++skips;return false;}return true;}
 void complete(double cost,bool late,double now){if(std::isfinite(cost)&&cost>0){double error=std::abs(cost-service);service=count?service*.5+cost*.5:cost;deviation=count?deviation*.5+error*.5:0;samples[next]=cost;next=(next+1)%samples.size();count=std::min(unsigned(samples.size()),count+1);}lastCompletion=now;probePending=false;++executed;if(late)++completedLate;}
};
