#include "../../sensor_adapter995.hpp"
#pragma once
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "../../deps/level-zero/zes_api.h"
struct SmxSensors {
 double busy=-1,compute=-1,watts=-1,mhz=-1,limitW=25,pollMs=-1;
 unsigned throttle=0;bool validEnergy=false,validEngine=false;
 HMODULE lib=nullptr;zes_device_handle_t dev=nullptr;
 zes_engine_handle_t engines[16]{};zes_engine_stats_t old[16]{};
 zes_engine_group_t types[16]{};unsigned ne=0;
 zes_pwr_handle_t power=nullptr;zes_freq_handle_t frequency=nullptr;
 zes_power_energy_counter_t energy{};double last=-1;
#define SMX_ZE(F) decltype(&::F) F=nullptr;
 SMX_ZE(zesEngineGetActivity) SMX_ZE(zesPowerGetEnergyCounter) SMX_ZE(zesFrequencyGetState)
#undef SMX_ZE
 template<class T> T fn(const char*n){return reinterpret_cast<T>(GetProcAddress(lib,n));}
 SmxSensors(){
  lib=LoadLibraryExW(L"ze_loader.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!lib)return;
  auto init=fn<decltype(&::zesInit)>("zesInit");auto drivers=fn<decltype(&::zesDriverGet)>("zesDriverGet");
  auto devices=fn<decltype(&::zesDeviceGet)>("zesDeviceGet");auto props=fn<decltype(&::zesDeviceGetProperties)>("zesDeviceGetProperties");
  if(!init||!drivers||!devices||!props||init(0))return;
  dev=smxSysmanAdapter995(lib);
  if(!dev)return;
  zesEngineGetActivity=fn<decltype(zesEngineGetActivity)>("zesEngineGetActivity");
  zesPowerGetEnergyCounter=fn<decltype(zesPowerGetEnergyCounter)>("zesPowerGetEnergyCounter");
  zesFrequencyGetState=fn<decltype(zesFrequencyGetState)>("zesFrequencyGetState");
  auto ee=fn<decltype(&::zesDeviceEnumEngineGroups)>("zesDeviceEnumEngineGroups");
  auto ep=fn<decltype(&::zesEngineGetProperties)>("zesEngineGetProperties");
  uint32_t n=16;if(ee&&ep&&zesEngineGetActivity&&!ee(dev,&n,engines)&&n<=16){ne=n;for(unsigned i=0;i<ne;i++){
   zes_engine_properties_t p{ZES_STRUCTURE_TYPE_ENGINE_PROPERTIES};if(ep(engines[i],&p)){ne=0;break;}types[i]=p.type;zesEngineGetActivity(engines[i],&old[i]);}}
  auto pe=fn<decltype(&::zesDeviceEnumPowerDomains)>("zesDeviceEnumPowerDomains");
  auto pp=fn<decltype(&::zesPowerGetProperties)>("zesPowerGetProperties");
  zes_pwr_handle_t ps[16]{};n=16;if(pe&&pp&&zesPowerGetEnergyCounter&&!pe(dev,&n,ps)&&n<=16)for(unsigned i=0;i<n;i++){
   zes_power_properties_t p{ZES_STRUCTURE_TYPE_POWER_PROPERTIES};if(!pp(ps[i],&p)&&!p.onSubdevice){power=ps[i];if(p.defaultLimit>0)limitW=p.defaultLimit/1000.;zesPowerGetEnergyCounter(power,&energy);break;}}
  auto pl=fn<decltype(&::zesPowerGetLimits)>("zesPowerGetLimits");
  if(power&&pl){zes_power_sustained_limit_t s{};zes_power_burst_limit_t b{};zes_power_peak_limit_t p{};
   if(!pl(power,&s,&b,&p)&&s.enabled&&s.power>0)limitW=s.power/1000.;}
  auto fe=fn<decltype(&::zesDeviceEnumFrequencyDomains)>("zesDeviceEnumFrequencyDomains");
  auto fp=fn<decltype(&::zesFrequencyGetProperties)>("zesFrequencyGetProperties");
  zes_freq_handle_t fs[16]{};n=16;if(fe&&fp&&zesFrequencyGetState&&!fe(dev,&n,fs)&&n<=16)for(unsigned i=0;i<n;i++){
   zes_freq_properties_t p{ZES_STRUCTURE_TYPE_FREQ_PROPERTIES};if(!fp(fs[i],&p)&&p.type==ZES_FREQ_DOMAIN_GPU){frequency=fs[i];break;}}
 }
 ~SmxSensors(){if(lib)FreeLibrary(lib);}
 bool poll(double now){
  if(last>=0&&now-last<.5)return false;last=now;LARGE_INTEGER q0{},q1{},f{};QueryPerformanceCounter(&q0);QueryPerformanceFrequency(&f);
  validEngine=false;busy=-1;compute=-1;
  if(zesEngineGetActivity)for(unsigned i=0;i<ne;i++){zes_engine_stats_t s{};
   if(!zesEngineGetActivity(engines[i],&s)){
    uint64_t dt=s.timestamp-old[i].timestamp,da=s.activeTime-old[i].activeTime;
    if(dt>=100000&&dt<=5000000&&s.activeTime>=old[i].activeTime&&da<=dt){
     double v=double(da)/dt;if(types[i]==ZES_ENGINE_GROUP_ALL){busy=v;validEngine=true;}if(types[i]==ZES_ENGINE_GROUP_COMPUTE_ALL)compute=v;}
    old[i]=s;}}
  validEnergy=false;watts=-1;
  if(power&&zesPowerGetEnergyCounter){zes_power_energy_counter_t s{};if(!zesPowerGetEnergyCounter(power,&s)){
   if(s.timestamp>energy.timestamp&&s.energy>=energy.energy){double dt=double(s.timestamp-energy.timestamp);double w=double(s.energy-energy.energy)/dt;
    if(dt>=100000&&dt<=5000000&&std::isfinite(w)&&w>=0&&w<=250){watts=w;validEnergy=true;}}energy=s;}}
  if(frequency&&zesFrequencyGetState){zes_freq_state_t s{ZES_STRUCTURE_TYPE_FREQ_STATE};if(!zesFrequencyGetState(frequency,&s)){mhz=s.actual;throttle=s.throttleReasons;}}
  QueryPerformanceCounter(&q1);pollMs=1000.*(q1.QuadPart-q0.QuadPart)/f.QuadPart;return true;
 }
};
struct SmxResourceBudget {
 double epoch=0,window=0,nextRun=0,backoff=0,lastProgress=0,baseW=-1,baseHz=0,hz=0,duty=.25,costMs=10,joule=.15,budgetW=2.5;
 uint64_t baseCount=0,execCount=0;double creditMs=10,creditTime=0,reservedMs=0;double lateFraction=0;unsigned skips=0,late=0,off=0,runs=0;bool baseline=true;
 FILE* trace=nullptr;
 SmxResourceBudget(double now,double limit=25){epoch=window=creditTime=lastProgress=now;budgetW=std::clamp(limit*.25,1.,10.);const char*s=getenv("SMX_FG_BUDGET_W");if(s&&atof(s)>0)budgetW=std::clamp(atof(s),.25,15.);
  fopen_s(&trace,"app-resource-budget.csv","wb");if(trace)fprintf(trace,"qpc_seconds,baseline,busy,compute,package_w,limit_w,mhz,throttle,base_hz,reference_base_hz,model_hz,estimated_incremental_j_per_inference,budget_w,service_duty,predict_ms,poll_ms,late_fraction\n");}
 ~SmxResourceBudget(){if(trace)fclose(trace);}
 bool update(double now,uint64_t captured,uint64_t executed,const SmxSensors&s){
  double span=now-window;if(span<.5)return !baseline;
  hz=(captured-baseCount)/span;double mh=(executed-execCount)/span;
  if(executed>execCount)lastProgress=now;
  baseCount=captured;execCount=executed;window=now;
  if(baseline){
   if(now-epoch>.6){baseHz=baseHz?baseHz*.7+hz*.3:hz;if(s.validEnergy)baseW=baseW<0?s.watts:baseW*.7+s.watts*.3;}
   if(now-epoch>=2.5&&now>=backoff&&baseHz>=1){baseline=false;epoch=lastProgress=now;duty=.25;lateFraction=0;costMs=std::min(costMs,10.);nextRun=now;creditMs=costMs;creditTime=now;}
  }else{
   double target=.75;if(mh>0&&lateFraction>.2)target=std::min(target,duty*.8);
   if(s.validEngine){if(s.busy>.92&&hz<baseHz*.95)target=std::min(target,duty*.80);else target=std::min(target,duty+.035);}
   else target=std::min(target,duty+.015);
   if(s.throttle&&((mh>0&&lateFraction>.2)||(baseHz>1&&hz<baseHz*.92)))target=std::min(target,duty*.90);
   if(baseHz>1&&hz<baseHz*.92)target=std::min(target,duty*.75);
   if(s.validEnergy&&baseW>=0&&mh>=2&&now-epoch>1){
    double estimate=std::max(0.,s.watts-baseW)/mh;if(estimate>0)joule=joule*.85+estimate*.15;
    double ownBudget=std::min(budgetW,std::max(0.,s.limitW*.85-baseW));
    target=std::min(target,ownBudget/std::max(.01,joule)*costMs/1000.);
   }
   duty=std::clamp(duty*.5+target*.5,0.,.75);
   if((now-lastProgress>5)||duty<.035||(s.validEngine&&s.busy>.98&&hz<baseHz*.88)||now-epoch>30){
    baseline=true;epoch=now;backoff=now+(duty<.035?5.:0.);off++;baseHz=0;baseW=-1;}
  }
  if(trace){fprintf(trace,"%.9f,%d,%.6f,%.6f,%.6f,%.6f,%.1f,%u,%.6f,%.6f,%.6f,%.9f,%.6f,%.6f,%.6f,%.6f,%.6f\n",now,baseline,s.busy,s.compute,s.watts,s.limitW,s.mhz,s.throttle,hz,baseHz,mh,joule,budgetW,duty,costMs,s.pollMs,lateFraction);fflush(trace);}
  return !baseline;
 }
 void refill(double now){creditMs=std::min(costMs*2.,creditMs+std::max(0.,now-creditTime)*1000.*duty);creditTime=now;}
 bool admit(double now,double available){
  refill(now);
  if(baseline||creditMs<costMs||available<10){skips++;return false;}
  if(costMs*1.15+1>available){if(now<nextRun){skips++;return false;}nextRun=now+1.;}
  reservedMs=costMs;creditMs-=reservedMs;return true;
 }
 void complete(double now,double ms,bool onTime){lastProgress=now;refill(now);runs++;creditMs+=reservedMs-ms;reservedMs=0;lateFraction=lateFraction*.9+(onTime?0.:.1);
  costMs=costMs*.8+std::clamp(ms,.01,1000.)*.2;creditMs=std::max(creditMs,-costMs*2.);
  if(!onTime){late++;duty*=.8;}
 }

};
