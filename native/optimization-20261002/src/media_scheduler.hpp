#pragma once
#include <windows.h>
#include <avrt.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
class FrameScheduling{
 std::vector<ULONG> previousCpuSets;bool restoreCpuSets=false;
 HANDLE task=nullptr;bool restore=false;PROCESS_POWER_THROTTLING_STATE previous{};
public:
 bool enabled=false;
 explicit FrameScheduling(bool process=true){
  const char*value=getenv("SMX_MM_SCHED");if(!value||value[0]=='0')return;
  if(process){
   previous.Version=PROCESS_POWER_THROTTLING_CURRENT_VERSION;
   restore=GetProcessInformation(GetCurrentProcess(),ProcessPowerThrottling,&previous,sizeof(previous))!=FALSE;
   PROCESS_POWER_THROTTLING_STATE request{};request.Version=PROCESS_POWER_THROTTLING_CURRENT_VERSION;
   request.ControlMask=PROCESS_POWER_THROTTLING_EXECUTION_SPEED|PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;request.StateMask=0;
   BOOL ok=SetProcessInformation(GetCurrentProcess(),ProcessPowerThrottling,&request,sizeof(request));
   fprintf(stderr,"FRAME_QOS process=%d error=%lu\n",int(ok),ok?0:GetLastError());
  }
  if(getenv("SMX_PCORE")&&getenv("SMX_PCORE")[0]!='0'){
   ULONG count=0;GetThreadSelectedCpuSets(GetCurrentThread(),nullptr,0,&count);previousCpuSets.resize(count);
   bool priorOk=!count||GetThreadSelectedCpuSets(GetCurrentThread(),previousCpuSets.data(),count,&count);
   ULONG bytes=0;GetSystemCpuSetInformation(nullptr,0,&bytes,GetCurrentProcess(),0);std::vector<unsigned char>buffer(bytes);std::vector<ULONG>chosen;
   if(priorOk&&bytes&&GetSystemCpuSetInformation((PSYSTEM_CPU_SET_INFORMATION)buffer.data(),bytes,&bytes,GetCurrentProcess(),0)){
    BYTE best=0;for(ULONG off=0;off<bytes;){auto*v=(PSYSTEM_CPU_SET_INFORMATION)(buffer.data()+off);if(!v->Size)break;if(v->Type==CpuSetInformation)best=std::max(best,v->CpuSet.EfficiencyClass);off+=v->Size;}
    for(ULONG off=0;off<bytes;){auto*v=(PSYSTEM_CPU_SET_INFORMATION)(buffer.data()+off);if(!v->Size)break;if(v->Type==CpuSetInformation&&v->CpuSet.EfficiencyClass==best&&!v->CpuSet.Allocated)chosen.push_back(v->CpuSet.Id);off+=v->Size;}
    if(!chosen.empty())restoreCpuSets=SetThreadSelectedCpuSets(GetCurrentThread(),chosen.data(),ULONG(chosen.size()))!=FALSE;
    fprintf(stderr,"FRAME_CPU_SETS count=%zu applied=%d error=%lu\n",chosen.size(),int(restoreCpuSets),restoreCpuSets?0:GetLastError());
   }
  }
  DWORD index=0;task=AvSetMmThreadCharacteristicsW(L"Games",&index);enabled=task!=nullptr;
  BOOL priority=task?AvSetMmThreadPriority(task,AVRT_PRIORITY_HIGH):FALSE;
  fprintf(stderr,"FRAME_SCHED enabled=%d priority=%d error=%lu\n",int(enabled),int(priority),priority?0:GetLastError());
 }
 ~FrameScheduling(){if(restoreCpuSets)SetThreadSelectedCpuSets(GetCurrentThread(),previousCpuSets.empty()?nullptr:previousCpuSets.data(),ULONG(previousCpuSets.size()));if(task)AvRevertMmThreadCharacteristics(task);if(restore)SetProcessInformation(GetCurrentProcess(),ProcessPowerThrottling,&previous,sizeof(previous));}
 FrameScheduling(const FrameScheduling&)=delete;
};
