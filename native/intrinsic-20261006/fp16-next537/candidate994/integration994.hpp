#pragma once
#include "diagnostic994.hpp"
#include <windows.h>
#include <winrt/base.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
namespace smxdiag994 {
inline int64_t frequency994=0;
inline SmxDiagFields994 fields(uint64_t frame=0,uint64_t job=0,uint64_t resource=0,uint64_t fence=0,double sourceSeconds=0,int64_t v0=0,int64_t v1=0) noexcept {
 return {frame,job,resource,fence,sourceSeconds>0?int64_t(sourceSeconds*double(frequency994)):0,v0,v1};
}
inline void instant(uint32_t stage,SmxDiagFields994 f={},uint16_t domain=SMX_DIAG_NONE994,int64_t result=0) noexcept {
 smx_diag_event994(stage,SMX_DIAG_INSTANT994,0,domain,result,&f);
}
inline void error(uint32_t stage,int64_t result,uint16_t domain,SmxDiagFields994 f={}) noexcept {
 smx_diag_event994(stage,SMX_DIAG_FAIL994,0,domain,result,&f);
}
template<class F> auto call(uint32_t stage,SmxDiagFields994 f,uint16_t domain,F&& fn) -> decltype(fn()) {
 Scope scope(stage,f);auto result=fn();const DWORD last=GetLastError();const int oldErrno=errno;
 if(domain==SMX_DIAG_WIN32994&&uint64_t(result)==uint64_t(WAIT_FAILED)){f.value1=last;scope.update(f);scope.fail(domain,int64_t(result));}
 else if((domain==SMX_DIAG_HRESULT994&&int64_t(result)<0)||(domain==SMX_DIAG_VKRESULT994&&int64_t(result)<0))scope.fail(domain,int64_t(result));else scope.end(domain,int64_t(result));
 SetLastError(last);errno=oldErrno;return result;
}
template<class F> auto operation(uint32_t stage,SmxDiagFields994 f,F&& fn) -> decltype(fn()) {
 Scope scope(stage,f);auto result=fn();const DWORD last=GetLastError();const int oldErrno=errno;scope.end();SetLastError(last);errno=oldErrno;return result;
}
template<class F> void command(uint32_t stage,SmxDiagFields994 f,F&& fn) {
 Scope scope(stage,f);fn();const DWORD last=GetLastError();const int oldErrno=errno;scope.end();SetLastError(last);errno=oldErrno;
}
inline void checkedHr(HRESULT result,int line) {
 if(FAILED(result))error(SMX_DIAG_EXCEPTION_CAUGHT994,int64_t(result),SMX_DIAG_HRESULT994,fields(0,0,0,0,0,line,1));
 winrt::check_hresult(result);
}
struct Runtime {
 bool opened=false;
 Runtime() noexcept {
  LARGE_INTEGER f{};QueryPerformanceFrequency(&f);frequency994=f.QuadPart;
  const wchar_t*dir=_wgetenv(L"SMX_DIAGNOSTIC_DIR994");
  const char*id=getenv("SMX_DIAGNOSTIC_SESSION994");
  if(!dir||!*dir)dir=L".";
  opened=smx_diag_open994(dir,id?id:"unhosted","public995-capability-runtime-v1");
  if(!opened){std::fprintf(stderr,"DIAGNOSTIC994 unavailable original_error=%lu; inference behavior unchanged\n",(unsigned long)smx_diag_open_error994());}
  if(opened){smx_diag_install_handlers994();smx_diag_refresh_modules994();instant(SMX_DIAG_ENGINE994,fields(0,0,0,0,0,994,GetCurrentProcessId()));}
 }
 ~Runtime(){if(opened){instant(SMX_DIAG_END_PROCESS994);smx_diag_close994();}}
};
}
#define SMX_DIAG_CALL994(stage,fields,domain,expression) smxdiag994::call(stage,fields,domain,[&](){return (expression);})
#define SMX_DIAG_COMMAND994(stage,fields,expression) smxdiag994::command(stage,fields,[&](){expression;})

#define SMX_DIAG_OPERATION994(stage,fields,expression) smxdiag994::operation(stage,fields,[&](){return (expression);})
