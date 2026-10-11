#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>
#include <stdexcept>
struct DisplaySnapshotHeader{uint64_t magic;uint32_t paths,modes;};
static constexpr uint64_t displaySnapshotMagic=0x3147464344584d53ull;
static LONG sessionDisplayQuery(std::vector<DISPLAYCONFIG_PATH_INFO>&p,std::vector<DISPLAYCONFIG_MODE_INFO>&m){
 const UINT32 flags=QDC_ONLY_ACTIVE_PATHS|QDC_VIRTUAL_MODE_AWARE|0x40;
 for(int attempt=0;attempt<3;attempt++){UINT32 np=0,nm=0;LONG r=GetDisplayConfigBufferSizes(flags,&np,&nm);if(r)return r;p.resize(np);m.resize(nm);r=QueryDisplayConfig(flags,&np,p.data(),&nm,m.data(),nullptr);if(r==ERROR_INSUFFICIENT_BUFFER)continue;p.resize(np);m.resize(nm);return r;}return ERROR_INSUFFICIENT_BUFFER;
}
static LONG sessionDisplayApply(std::vector<DISPLAYCONFIG_PATH_INFO>&p,std::vector<DISPLAYCONFIG_MODE_INFO>&m){return SetDisplayConfig(UINT32(p.size()),p.data(),UINT32(m.size()),m.data(),SDC_APPLY|SDC_USE_SUPPLIED_DISPLAY_CONFIG|SDC_VIRTUAL_MODE_AWARE|0x20000);}
static bool readDisplaySnapshot(const wchar_t*path,std::vector<DISPLAYCONFIG_PATH_INFO>&p,std::vector<DISPLAYCONFIG_MODE_INFO>&m){
 FILE*f=nullptr;_wfopen_s(&f,path,L"rb");if(!f)return false;DisplaySnapshotHeader h{};bool ok=fread(&h,1,sizeof(h),f)==sizeof(h)&&h.magic==displaySnapshotMagic&&h.paths>0&&h.paths<=16&&h.modes>0&&h.modes<=128;
 if(ok){p.resize(h.paths);m.resize(h.modes);ok=fread(p.data(),sizeof(p[0]),p.size(),f)==p.size()&&fread(m.data(),sizeof(m[0]),m.size(),f)==m.size()&&fgetc(f)==EOF;}fclose(f);return ok;
}
class CaptureDisplaySession{
 std::vector<DISPLAYCONFIG_PATH_INFO> paths;std::vector<DISPLAYCONFIG_MODE_INFO>modes;HANDLE finished=nullptr,guardian=nullptr;std::wstring snapshot;bool changed=false;
public:
 CaptureDisplaySession(bool enabled,const std::wstring&base){
  if(!enabled)return;LONG r=sessionDisplayQuery(paths,modes);if(r)throw std::runtime_error("Cannot inspect the virtual and physical display modes");
  if(paths.size()!=1)throw std::runtime_error("Automatic 120 Hz mode currently requires one display; disable the session display option");
  auto next=paths;auto&x=next[0];UINT32 i=x.targetInfo.targetModeInfoIdx;if(i>=modes.size()||modes[i].infoType!=DISPLAYCONFIG_MODE_INFO_TYPE_TARGET)throw std::runtime_error("Physical display mode unavailable");
  auto rate=modes[i].targetMode.targetVideoSignalInfo.vSyncFreq;double physical=rate.Denominator?double(rate.Numerator)/rate.Denominator:0;double virtualHz=x.targetInfo.refreshRate.Denominator?double(x.targetInfo.refreshRate.Numerator)/x.targetInfo.refreshRate.Denominator:0;
  fprintf(stderr,"SESSION_DISPLAY virtual=%.6f physical=%.6f\n",virtualHz,physical);
  if(physical<119||physical>121)throw std::runtime_error("Session 120 Hz mode requires an active 120 Hz physical signal");
  if(virtualHz>=119&&virtualHz<=121)return;
  std::wstring suffix=std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());snapshot=base+L"\\model1080\\display-session-"+suffix+L".bin";
  FILE*f=nullptr;_wfopen_s(&f,snapshot.c_str(),L"wbx");if(!f)throw std::runtime_error("Cannot save display restoration data");
  DisplaySnapshotHeader h{displaySnapshotMagic,UINT32(paths.size()),UINT32(modes.size())};bool ok=fwrite(&h,1,sizeof(h),f)==sizeof(h)&&fwrite(paths.data(),sizeof(paths[0]),paths.size(),f)==paths.size()&&fwrite(modes.data(),sizeof(modes[0]),modes.size(),f)==modes.size();fclose(f);
  if(!ok){DeleteFileW(snapshot.c_str());throw std::runtime_error("Display restoration data write failed");}
  std::wstring eventName=L"Local\\SMX-Display-Restored-"+suffix;finished=CreateEventW(nullptr,TRUE,FALSE,eventName.c_str());
  if(!finished){DeleteFileW(snapshot.c_str());throw std::runtime_error("Display restoration event failed");}
  HANDLE ready=CreateEventW(nullptr,TRUE,FALSE,(eventName+L"-Ready").c_str());if(!ready){CloseHandle(finished);finished=nullptr;DeleteFileW(snapshot.c_str());throw std::runtime_error("Display helper readiness event failed");}
  std::wstring exe=base+L"\\display-guard.exe";std::wstring command=L"\""+exe+L"\" "+std::to_wstring(GetCurrentProcessId())+L" \""+snapshot+L"\" \""+eventName+L"\"";
  STARTUPINFOW si{};si.cb=sizeof(si);PROCESS_INFORMATION pi{};std::vector<wchar_t>cmd(command.begin(),command.end());cmd.push_back(0);
  if(!CreateProcessW(exe.c_str(),cmd.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,base.c_str(),&si,&pi)){CloseHandle(ready);CloseHandle(finished);finished=nullptr;DeleteFileW(snapshot.c_str());throw std::runtime_error("Display restoration helper could not start");}
  CloseHandle(pi.hThread);guardian=pi.hProcess;DWORD readyResult=WaitForSingleObject(ready,5000);CloseHandle(ready);if(readyResult!=WAIT_OBJECT_0){SetEvent(finished);WaitForSingleObject(guardian,3000);CloseHandle(guardian);CloseHandle(finished);guardian=finished=nullptr;DeleteFileW(snapshot.c_str());throw std::runtime_error("Display restoration helper did not become ready");}
  x.targetInfo.refreshRate=rate;x.flags&=~UINT32(0x10);auto nextModes=modes;r=sessionDisplayApply(next,nextModes);
  if(r){SetEvent(finished);WaitForSingleObject(guardian,3000);CloseHandle(guardian);CloseHandle(finished);guardian=finished=nullptr;DeleteFileW(snapshot.c_str());throw std::runtime_error("Temporary 120 Hz display path was rejected");}
  changed=true;fprintf(stderr,"SESSION_DISPLAY applied=120 restore_guard=true guardian_pid=%lu\n",pi.dwProcessId);
 }
 ~CaptureDisplaySession(){
  if(changed){LONG r=sessionDisplayApply(paths,modes);fprintf(stderr,"SESSION_DISPLAY restore_result=%ld\n",r);if(!r&&finished)SetEvent(finished);}
  if(guardian){if(WaitForSingleObject(guardian,3000)==WAIT_OBJECT_0)DeleteFileW(snapshot.c_str());CloseHandle(guardian);}
  if(finished)CloseHandle(finished);
 }
 CaptureDisplaySession(const CaptureDisplaySession&)=delete;
};
