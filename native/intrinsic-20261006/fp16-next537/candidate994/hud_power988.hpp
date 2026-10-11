#include "../../../sensor_adapter995.hpp"
#include "bounded_stdio989.h"
#pragma once
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include "../../../deps/level-zero/zes_api.h"
#include <cmath>
#include <cstdint>
namespace smx988 {
inline double qpcNow988(){LARGE_INTEGER q{},f{};QueryPerformanceCounter(&q);QueryPerformanceFrequency(&f);return double(q.QuadPart)/f.QuadPart;}
struct PowerSample988 {
 bool deviceMatched=false,powerValid=false,frequencyValid=false,throttleValid=false;
 // Independent Windows Energy Meter RAPL package counter; cooked power is mW.
 bool cpuPackageValid=false,cpuPackageAvailable=false,cpuPackageWarmup=true;double cpuPackageWatts=-1,cpuPackageRawMilliwatts=-1,cpuPackageSampleQpc=0;
 uint32_t cpuPackageStatus=0xffffffffu,cpuPackageCounterStatus=0xffffffffu;
 uint32_t domain=0,throttleFlags=0,powerResult=0xffffffffu,frequencyResult=0xffffffffu;
 double watts=-1,gpuMHz=-1,sampleQpc=0,driverIntervalSeconds=0,pollMs=0;
 uint64_t energyMicrojoules=0,driverMicroseconds=0;
};
class PdhCpuPackage988 {
  void(*phase_)(void*,const char*)=nullptr;void*phaseContext_=nullptr;
  void phase(const char*s){if(phase_)phase_(phaseContext_,s);}
 HMODULE library_=nullptr;PDH_HQUERY query_=nullptr;PDH_HCOUNTER counter_=nullptr;bool warmed_=false;
 PDH_STATUS startup_=PDH_FUNCTION_NOT_FOUND;
 decltype(&::PdhCollectQueryData) collect_=nullptr;decltype(&::PdhGetFormattedCounterValue) formatted_=nullptr;decltype(&::PdhCloseQuery) close_=nullptr;
 template<class T>T fn(const char*n){return reinterpret_cast<T>(GetProcAddress(library_,n));}
public:
 static bool milliwattsToWatts(uint32_t apiStatus,uint32_t counterStatus,double raw,double&watts){
  watts=-1;if(apiStatus!=ERROR_SUCCESS||(counterStatus!=PDH_CSTATUS_VALID_DATA&&counterStatus!=PDH_CSTATUS_NEW_DATA)||!std::isfinite(raw)||raw<0||raw>500000)return false;
  watts=raw/1000.;return true;
 }
 static bool selfTest(){double watts=-1;
  if(!milliwattsToWatts(0,PDH_CSTATUS_VALID_DATA,24228.685,watts)||std::abs(watts-24.228685)>1e-9)return false;
  if(!milliwattsToWatts(0,PDH_CSTATUS_NEW_DATA,0,watts)||watts!=0)return false;
  if(!milliwattsToWatts(0,PDH_CSTATUS_VALID_DATA,500000,watts)||watts!=500)return false;
  for(double bad:{-1.,500000.01,HUGE_VAL})if(milliwattsToWatts(0,0,bad,watts)||watts!=-1)return false;
  if(milliwattsToWatts(PDH_INVALID_DATA,0,20000,watts)||watts!=-1)return false;
  if(milliwattsToWatts(0,PDH_CSTATUS_NO_INSTANCE,20000,watts)||watts!=-1)return false;
  return true;
 }
 PdhCpuPackage988(void(*phaseCallback)(void*,const char*)=nullptr,void*context=nullptr):phase_(phaseCallback),phaseContext_(context){
   phase("pdh.initialize");
  library_=LoadLibraryExW(L"pdh.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!library_)return;
  auto open=fn<decltype(&::PdhOpenQueryW)>("PdhOpenQueryW");auto add=fn<decltype(&::PdhAddEnglishCounterW)>("PdhAddEnglishCounterW");
  collect_=fn<decltype(collect_)>("PdhCollectQueryData");formatted_=fn<decltype(formatted_)>("PdhGetFormattedCounterValue");close_=fn<decltype(close_)>("PdhCloseQuery");
  if(!open||!add||!collect_||!formatted_||!close_)return;
  startup_=open(nullptr,0,&query_);if(startup_!=ERROR_SUCCESS)return;
  startup_=add(query_,L"\\Energy Meter(RAPL_Package0_PKG)\\Power",0,&counter_);
 }
 ~PdhCpuPackage988(){if(query_&&close_){phase("pdh.CloseQuery");close_(query_);}if(library_){phase("pdh.FreeLibrary");FreeLibrary(library_);}}
 PdhCpuPackage988(const PdhCpuPackage988&)=delete;PdhCpuPackage988&operator=(const PdhCpuPackage988&)=delete;
 void sample(PowerSample988&r){
  r.cpuPackageStatus=uint32_t(startup_);r.cpuPackageAvailable=startup_==ERROR_SUCCESS&&counter_;
  if(!r.cpuPackageAvailable)return;
  phase("pdh.CollectQueryData");const auto status=collect_(query_);r.cpuPackageStatus=uint32_t(status);r.cpuPackageSampleQpc=qpcNow988();
  if(status!=ERROR_SUCCESS){warmed_=false;return;}
  if(!warmed_){warmed_=true;return;}r.cpuPackageWarmup=false;
  PDH_FMT_COUNTERVALUE value{};DWORD type=0;phase("pdh.GetFormattedCounterValue");auto result=formatted_(counter_,PDH_FMT_DOUBLE,&type,&value);
  r.cpuPackageStatus=uint32_t(result);r.cpuPackageCounterStatus=value.CStatus;
  if(result==ERROR_SUCCESS&&std::isfinite(value.doubleValue))r.cpuPackageRawMilliwatts=value.doubleValue;
  r.cpuPackageValid=milliwattsToWatts(uint32_t(result),value.CStatus,value.doubleValue,r.cpuPackageWatts);
 }
};
class PowerSampler988 {
  void(*phase_)(void*,const char*)=nullptr;void*phaseContext_=nullptr;
  void phase(const char*s){if(phase_)phase_(phaseContext_,s);}
 PdhCpuPackage988 cpuPackage_;
 HMODULE lib_=nullptr;zes_device_handle_t dev_=nullptr;zes_pwr_handle_t power_=nullptr;zes_freq_handle_t frequency_=nullptr;
 zes_power_energy_counter_t previous_{};bool havePrevious_=false;uint32_t domain_=0;
 decltype(&::zesPowerGetEnergyCounter) energy_=nullptr;decltype(&::zesFrequencyGetState) frequencyState_=nullptr;
 template<class T>T fn(const char*n){return reinterpret_cast<T>(GetProcAddress(lib_,n));}
public:
 static bool energyWatts(const zes_power_energy_counter_t&a,const zes_power_energy_counter_t&b,double&w){
  w=-1;if(b.timestamp<=a.timestamp||b.energy<a.energy)return false;
  uint64_t dt=b.timestamp-a.timestamp;if(dt<100000||dt>3000000)return false;
  double result=double(b.energy-a.energy)/double(dt);if(!std::isfinite(result)||result<0||result>300)return false;w=result;return true;
 }
 static bool selfTest(){if(!PdhCpuPackage988::selfTest())return false;zes_power_energy_counter_t a{1000000,1000000},b{21000000,2000000};double w=-1;if(!energyWatts(a,b,w)||std::abs(w-20)>1e-9)return false;b.timestamp=a.timestamp;if(energyWatts(a,b,w)||w!=-1)return false;b.timestamp=5000000;if(energyWatts(a,b,w))return false;b={0,2000000};if(energyWatts(a,b,w))return false;return true;}
 PowerSampler988(void(*phaseCallback)(void*,const char*)=nullptr,void*context=nullptr):phase_(phaseCallback),phaseContext_(context),cpuPackage_(phaseCallback,context){
   phase("sysman.initialize");
  lib_=LoadLibraryExW(L"ze_loader.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!lib_)return;
  auto init=fn<decltype(&::zesInit)>("zesInit");auto drivers=fn<decltype(&::zesDriverGet)>("zesDriverGet");auto devices=fn<decltype(&::zesDeviceGet)>("zesDeviceGet");auto properties=fn<decltype(&::zesDeviceGetProperties)>("zesDeviceGetProperties");
  if(!init||!drivers||!devices||!properties||init(0)!=ZE_RESULT_SUCCESS)return;
  dev_=smxSysmanAdapter995(lib_);
  if(!dev_)return;
  energy_=fn<decltype(energy_)>("zesPowerGetEnergyCounter");frequencyState_=fn<decltype(frequencyState_)>("zesFrequencyGetState");
  auto powerEnum=fn<decltype(&::zesDeviceEnumPowerDomains)>("zesDeviceEnumPowerDomains");auto powerProps=fn<decltype(&::zesPowerGetProperties)>("zesPowerGetProperties");
  uint32_t np=16;zes_pwr_handle_t ps[16]{};
  if(energy_&&powerEnum&&powerProps&&powerEnum(dev_,&np,ps)==ZE_RESULT_SUCCESS&&np<=16){for(uint32_t i=0;i<np;++i){zes_power_ext_properties_t ext{ZES_STRUCTURE_TYPE_POWER_EXT_PROPERTIES};ext.domain=ZES_POWER_DOMAIN_UNKNOWN;zes_power_properties_t p{ZES_STRUCTURE_TYPE_POWER_PROPERTIES};p.pNext=&ext;if(powerProps(ps[i],&p)!=ZE_RESULT_SUCCESS||p.onSubdevice||ext.domain==ZES_POWER_DOMAIN_UNKNOWN)continue;if(!power_||ext.domain==ZES_POWER_DOMAIN_PACKAGE){power_=ps[i];domain_=uint32_t(ext.domain);}if(ext.domain==ZES_POWER_DOMAIN_PACKAGE)break;}}
  auto freqEnum=fn<decltype(&::zesDeviceEnumFrequencyDomains)>("zesDeviceEnumFrequencyDomains");auto freqProps=fn<decltype(&::zesFrequencyGetProperties)>("zesFrequencyGetProperties");
  uint32_t nf=16;zes_freq_handle_t fs[16]{};if(frequencyState_&&freqEnum&&freqProps&&freqEnum(dev_,&nf,fs)==ZE_RESULT_SUCCESS&&nf<=16){for(uint32_t i=0;i<nf;++i){zes_freq_properties_t p{ZES_STRUCTURE_TYPE_FREQ_PROPERTIES};if(freqProps(fs[i],&p)==ZE_RESULT_SUCCESS&&!p.onSubdevice&&p.type==ZES_FREQ_DOMAIN_GPU){frequency_=fs[i];break;}}}
 }
 ~PowerSampler988(){if(lib_){phase("sysman.FreeLibrary");FreeLibrary(lib_);}}
 PowerSampler988(const PowerSampler988&)=delete;PowerSampler988&operator=(const PowerSampler988&)=delete;
 PowerSample988 sample(){
  const double begin=qpcNow988();PowerSample988 result{};result.deviceMatched=dev_!=nullptr;result.domain=domain_;
  if(power_&&energy_){zes_power_energy_counter_t e{};phase("sysman.PowerGetEnergyCounter");auto status=energy_(power_,&e);result.powerResult=uint32_t(status);if(status==ZE_RESULT_SUCCESS){result.energyMicrojoules=e.energy;result.driverMicroseconds=e.timestamp;if(havePrevious_){result.powerValid=energyWatts(previous_,e,result.watts);if(e.timestamp>previous_.timestamp)result.driverIntervalSeconds=double(e.timestamp-previous_.timestamp)*1e-6;}previous_=e;havePrevious_=true;}else havePrevious_=false;}
  if(frequency_&&frequencyState_){zes_freq_state_t f{ZES_STRUCTURE_TYPE_FREQ_STATE};phase("sysman.FrequencyGetState");auto status=frequencyState_(frequency_,&f);result.frequencyResult=uint32_t(status);if(status==ZE_RESULT_SUCCESS){result.frequencyValid=std::isfinite(f.actual)&&f.actual>0;if(result.frequencyValid)result.gpuMHz=f.actual;result.throttleValid=true;result.throttleFlags=f.throttleReasons;}}
  cpuPackage_.sample(result);result.sampleQpc=qpcNow988();result.pollMs=(result.sampleQpc-begin)*1000;return result;
 }
};
inline const wchar_t*powerDomainName988(uint32_t d){switch(d){case ZES_POWER_DOMAIN_CARD:return L"Card";case ZES_POWER_DOMAIN_PACKAGE:return L"Package";case ZES_POWER_DOMAIN_STACK:return L"Stack";case ZES_POWER_DOMAIN_MEMORY:return L"Memory";case ZES_POWER_DOMAIN_GPU:return L"GPU domain";default:return L"Unknown";}}
}
