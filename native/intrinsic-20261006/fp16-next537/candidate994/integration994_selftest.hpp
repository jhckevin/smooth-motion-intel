#pragma once
#include "integration994.hpp"
#include <memory>
#include <stdexcept>
inline int diagnosticIntegrationSelftest994() {
 unsigned checks=0;auto check=[&](bool b){++checks;if(!b)throw std::runtime_error("diagnostic transparent-call mock failure");};
 if(!smx_diag_open994(L".","cpu-integration994","R994-diagnostics-v1"))return 2;
 try{
  LARGE_INTEGER fq{};QueryPerformanceFrequency(&fq);smxdiag994::frequency994=fq.QuadPart;
  unsigned called=0;SetLastError(123);errno=41;
  HRESULT hr=smxdiag994::call(SMX_DIAG_PRESENT994,{},SMX_DIAG_HRESULT994,[&]{++called;SetLastError(456);errno=42;return E_FAIL;});
  check(called==1&&hr==E_FAIL);check(GetLastError()==456&&errno==42);
  int vk=smxdiag994::call(SMX_DIAG_MODEL_SUBMIT994,{},SMX_DIAG_VKRESULT994,[&]{++called;return -4;});check(vk==-4&&called==2);
  auto value=smxdiag994::operation(SMX_DIAG_CAPTURE_ACQUIRE994,{},[&]{++called;return std::make_unique<int>(7);});check(value&&*value==7&&called==3);
  smxdiag994::command(SMX_DIAG_DISPLAY_COPY994,{},[&]{++called;SetLastError(987);errno=34;});check(called==4);check(GetLastError()==987&&errno==34);
  bool threw=false;try{smxdiag994::command(SMX_DIAG_CAPTURE_COPY994,{},[&]{++called;throw std::runtime_error("mock CPU exception");});}catch(const std::runtime_error&){threw=true;}check(threw&&called==5);
  DWORD wait=smxdiag994::call(SMX_DIAG_DISPLAY_CREDIT994,{},SMX_DIAG_WIN32994,[&]{++called;SetLastError(ERROR_INVALID_HANDLE);return DWORD(WAIT_FAILED);});check(wait==WAIT_FAILED&&called==6);check(GetLastError()==ERROR_INVALID_HANDLE);
  bool hrThrew=false;try{smxdiag994::checkedHr(DXGI_ERROR_DEVICE_REMOVED,123);}catch(const winrt::hresult_error&e){hrThrew=e.code()==DXGI_ERROR_DEVICE_REMOVED;}check(hrThrew);
  LARGE_INTEGER a{},z{};QueryPerformanceCounter(&a);for(unsigned i=0;i<100000;++i){HRESULT x=smxdiag994::call(SMX_DIAG_PRESENT994,smxdiag994::fields(i),SMX_DIAG_HRESULT994,[]{return S_OK;});if(x!=S_OK)throw std::runtime_error("mock changed result");}QueryPerformanceCounter(&z);
  double ns=1e9*double(z.QuadPart-a.QuadPart)/double(fq.QuadPart)/100000.;
  smx_diag_close994();smxdiag994::instant(SMX_DIAG_END_PROCESS994);check(true);
  FILE*f=nullptr;fopen_s(&f,"diagnostic-integration994-selftest.json","wb");if(!f)return 3;fprintf(f,"{\"pass\":true,\"checks\":%u,\"gpu_used\":false,\"mock_calls\":100000,\"mean_begin_end_ns\":%.3f,\"scope\":\"remote CPU observation overhead only; not game performance acceptance\",\"transparent_results\":true,\"last_error_errno_preserved\":true,\"late_event_after_close_safe\":true}\n",checks,ns);fclose(f);return 0;
 }catch(...){smx_diag_close994();return 1;}
}
