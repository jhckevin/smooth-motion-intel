#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "bounded_stdio989.h"
#include <windows.h>
#include <share.h>
#include <io.h>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <string>
#include <algorithm>
namespace {
struct Entry{uint64_t bytes=0,dropped=0;std::string path;};
struct Registry{std::mutex mutex;std::unordered_map<FILE*,Entry> streams;uint64_t written=0,droppedRecords=0,droppedBytes=0;};
Registry& registry(){static Registry* const r=new Registry;return *r;}
void attach(FILE*f,const char*path,const char*mode){if(f&&mode&&(strchr(mode,'w')||strchr(mode,'a')||strchr(mode,'+'))){auto&r=registry();std::lock_guard lock(r.mutex);auto&e=r.streams[f];e={};e.path=path?path:"";if(strchr(mode,'a')){const __int64 existing=_filelengthi64(_fileno(f));e.bytes=existing>=0?uint64_t(existing):smx_log_limit989();}}}
}
extern "C" uint64_t smx_log_limit989(){static const uint64_t limit=[](){const char*s=getenv("SMX_LOG_LIMIT_MB989");unsigned long n=s?strtoul(s,nullptr,10):16;return uint64_t(std::clamp(n,1ul,256ul))*1024*1024;}();return limit;}
extern "C" void smx_log_reset989(FILE*f,const char*path){attach(f,path,"w");}
extern "C" int smx_log_fprintf989(FILE*f,const char*format,...){
 if(!f||!format)return -1;
 thread_local char buffer[65536];va_list args;va_start(args,format);int n=vsnprintf(buffer,sizeof buffer,format,args);va_end(args);if(n<0)return n;
 bool drop=false;{auto&r=registry();std::lock_guard lock(r.mutex);auto&e=r.streams[f];uint64_t amount=uint64_t(n);drop=amount>=sizeof buffer||e.bytes>smx_log_limit989()||amount>smx_log_limit989()-std::min(e.bytes,smx_log_limit989());if(drop){++e.dropped;++r.droppedRecords;r.droppedBytes+=amount;}else{e.bytes+=amount;r.written+=amount;}}
 if(drop)return n; // Diagnostic omission is counted, never mistaken for GPU failure.
 return fwrite(buffer,1,size_t(n),f)==size_t(n)?n:-1;
}
extern "C" FILE* smx_log_fopen989(const char*p,const char*m){FILE*f=fopen(p,m);attach(f,p,m);return f;}
extern "C" int smx_log_fopen_s989(FILE**f,const char*p,const char*m){int e=fopen_s(f,p,m);if(!e)attach(*f,p,m);return e;}
extern "C" FILE* smx_log_fsopen989(const char*p,const char*m,int share){FILE*f=_fsopen(p,m,share);attach(f,p,m);return f;}
extern "C" int smx_log_freopen_s989(FILE**f,const char*p,const char*m,FILE*old){auto&r=registry();{std::lock_guard lock(r.mutex);r.streams.erase(old);}int e=freopen_s(f,p,m,old);if(!e)attach(*f,p,m);return e;}
extern "C" int smx_log_fclose989(FILE*f){if(!f)return EOF;auto&r=registry();{std::lock_guard lock(r.mutex);r.streams.erase(f);}return fclose(f);}
extern "C" void smx_log_stream989(FILE*f,uint64_t*bytes,uint64_t*drops){auto&r=registry();std::lock_guard lock(r.mutex);auto it=r.streams.find(f);if(bytes)*bytes=it==r.streams.end()?0:it->second.bytes;if(drops)*drops=it==r.streams.end()?0:it->second.dropped;}
extern "C" void smx_log_totals989(uint64_t*bytes,uint64_t*records,uint64_t*droppedBytes){auto&r=registry();std::lock_guard lock(r.mutex);if(bytes)*bytes=r.written;if(records)*records=r.droppedRecords;if(droppedBytes)*droppedBytes=r.droppedBytes;}
