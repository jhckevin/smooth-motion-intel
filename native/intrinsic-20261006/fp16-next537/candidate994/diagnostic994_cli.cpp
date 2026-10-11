#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "diagnostic994.hpp"
#include <windows.h>
#include <cstdio>
#include <cwchar>
#include <cerrno>
#include <thread>
#include <atomic>
#include <vector>
__declspec(noinline) static void ownCpuFault994(){*reinterpret_cast<volatile LONG*>(uintptr_t(1))=7;}
int wmain(int argc,wchar_t**argv){
 if(argc==3&&!wcscmp(argv[1],L"--self-test"))return int(smx_diag_selftest994(argv[2]));
 if(argc==3&&!wcscmp(argv[1],L"--crash-test")){
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
  if(!smx_diag_open994(argv[2],"actual-cpu-av-test","diag994-cli"))return 2;
  if(!smx_diag_install_handlers994())return 3;SmxDiagFields994 f{};f.frame=777;f.job=888;
  smx_diag_begin994(SMX_DIAG_MODEL_SUBMIT994,&f);ownCpuFault994();return 4;
 }
 if(argc==3&&!wcscmp(argv[1],L"--abrupt-test")){
  if(!smx_diag_open994(argv[2],"abrupt-cpu-test","diag994-cli"))return 2;
  smx_diag_begin994(SMX_DIAG_DISPLAY_CREDIT994,nullptr);
  for(unsigned i=0;i<32;++i)smx_diag_event994(SMX_DIAG_CAPTURE_ACQUIRE994,SMX_DIAG_INSTANT994,0,SMX_DIAG_SEMANTIC994,i,nullptr);
  TerminateProcess(GetCurrentProcess(),87);return 4;
 }
 if(argc==3&&!wcscmp(argv[1],L"--shutdown-test")){
  if(!smx_diag_open994(argv[2],"concurrent-shutdown","diag994-cli"))return 2;smx_diag_session994(12);
  std::atomic<bool>stop{false};std::vector<std::thread>workers;
  for(unsigned t=0;t<4;++t)workers.emplace_back([&]{smxdiag994::Scope late(SMX_DIAG_DISPLAY_STATS994);while(!stop.load())smx_diag_event994(SMX_DIAG_CAPTURE_COPY994,SMX_DIAG_INSTANT994,0,SMX_DIAG_NONE994,0,nullptr);late.end();});
  Sleep(2);smx_diag_close994();stop=true;for(auto&t:workers)t.join();smx_diag_event994(SMX_DIAG_STOP994,SMX_DIAG_INSTANT994,0,SMX_DIAG_NONE994,0,nullptr);return 0;
 }
 if(argc==3&&!wcscmp(argv[1],L"--error-state-test")){
  SetLastError(12345);errno=987;
  if(!smx_diag_open994(argv[2],"errors","diag994-cli"))return 2;
  if(GetLastError()!=12345||errno!=987)return 3;auto c=smx_diag_begin994(SMX_DIAG_PRESENT994,nullptr);
  if(GetLastError()!=12345||errno!=987)return 4;smx_diag_event994(SMX_DIAG_PRESENT994,SMX_DIAG_END994,c,SMX_DIAG_HRESULT994,0,nullptr);
  if(GetLastError()!=12345||errno!=987)return 5;smx_diag_close994();
  return GetLastError()==12345&&errno==987?0:6;
 }
 if(argc==3)return int(smx_diag_export994(argv[1],argv[2]));
 fwprintf(stderr,L"Usage: diag-export994.exe input.bin output.jsonl\n");return 64;
}
