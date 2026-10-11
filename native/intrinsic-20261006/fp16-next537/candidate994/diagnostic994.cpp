#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#include "diagnostic994.hpp"
#include <windows.h>
#include <tlhelp32.h>
#include <array>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <thread>
#include <cerrno>
#include <limits>

namespace {
struct PreserveErrors {DWORD win=GetLastError();int crt=errno;~PreserveErrors(){errno=crt;SetLastError(win);}};
struct PreserveWinError {DWORD win=GetLastError();~PreserveWinError(){SetLastError(win);}};
constexpr uint32_t Schema=1,HeaderBytes=16384,FileBytes=4u*1024*1024,RecordBytes=128;
constexpr uint32_t Capacity=(FileBytes-HeaderBytes)/RecordBytes,ModuleCapacity=96;
struct Payload {
 int64_t qpc;uint64_t callId,frame,job,resource,fence;int64_t sourceQpc,value0,value1,result;
 uint32_t stage;uint16_t action,domain;uint32_t captureSession,tid;
};
struct alignas(64) Record {
 volatile LONG busy;uint32_t reserved;volatile LONG64 begin;
 Payload payload;uint32_t crc,reserved2;volatile LONG64 end;
};
struct Module {uint64_t base,size;char name[96];uint8_t reserved[16];};
struct Header {
 char magic[8];uint32_t schema,headerBytes,recordBytes,capacity;
 uint64_t fileBytes;int64_t frequency,startQpc;uint64_t utcFileTime;
 uint32_t pid,reserved;char build[64],session[96];
 volatile LONG64 nextSequence,committed,droppedBusy,overwritten,droppedStale;
 volatile LONG64 currentSession,sessionBeginGlobalSeq;
 volatile LONG moduleGeneration,moduleCount,moduleTruncated,normalClose;
 Module modules[ModuleCapacity];
 Record crash;
};
static_assert(sizeof(Payload)==96&&sizeof(Record)==RecordBytes);
static_assert(sizeof(Module)==128&&sizeof(Header)<=HeaderBytes);
struct Runtime {
 volatile LONG opened=0,enabled=0,writers=0,refreshing=0;uint32_t openError=0;
 volatile LONG64 currentSession=0;
 HANDLE file=INVALID_HANDLE_VALUE,mapping=nullptr;Header*header=nullptr;Record*records=nullptr;
 LPTOP_LEVEL_EXCEPTION_FILTER previous=nullptr;bool installed=false;
} g;
constexpr auto crcTable(){std::array<uint32_t,256>a{};for(uint32_t i=0;i<256;++i){uint32_t x=i;for(unsigned k=0;k<8;++k)x=(x>>1)^(0xedb88320u&uint32_t(-int32_t(x&1)));a[i]=x;}return a;}
constexpr auto CrcTable=crcTable();
uint32_t crc(const void*data,size_t bytes) noexcept {uint32_t c=~0u;auto*p=static_cast<const uint8_t*>(data);for(size_t i=0;i<bytes;++i)c=CrcTable[(c^p[i])&255]^(c>>8);return ~c;}
void ascii(char*dst,size_t capacity,const char*src) noexcept {size_t n=0;if(src)for(;n+1<capacity&&src[n];++n){unsigned char c=static_cast<unsigned char>(src[n]);dst[n]=c>=32&&c<=126?char(c):'_';}dst[n]=0;}
bool pathJoin(wchar_t*dst,size_t capacity,const wchar_t*dir,const wchar_t*name) noexcept {
 if(!dir||!*dir)return false;size_t n=wcslen(dir),m=wcslen(name);if(n+m+2>capacity)return false;
 memcpy(dst,dir,n*sizeof(wchar_t));if(n&&dir[n-1]!=L'\\'&&dir[n-1]!=L'/')dst[n++]=L'\\';memcpy(dst+n,name,(m+1)*sizeof(wchar_t));return true;
}
bool enter() noexcept {InterlockedIncrement(&g.writers);if(InterlockedCompareExchange(&g.enabled,0,0)!=1){InterlockedDecrement(&g.writers);return false;}return true;}
void leave() noexcept {InterlockedDecrement(&g.writers);}
bool commitRecord(Record&r,int64_t sequence,const Payload&p,bool countOverwrite=true,bool*stale=nullptr) noexcept {
 if(InterlockedCompareExchange(&r.busy,1,0)!=0)return false;
 const int64_t previous=r.end;
 if(previous>=sequence&&previous>0){if(stale)*stale=true;InterlockedExchange(&r.busy,0);return false;}
 InterlockedExchange64(&r.begin,-sequence);InterlockedExchange64(&r.end,0);
 memcpy(&r.payload,&p,sizeof p);r.crc=crc(&p,sizeof p);MemoryBarrier();
 InterlockedExchange64(&r.begin,sequence);InterlockedExchange64(&r.end,sequence);
 InterlockedExchange(&r.busy,0);
 if(countOverwrite&&previous>0)InterlockedIncrement64(&g.header->overwritten);
 return true;
}
uint64_t append(uint32_t stage,uint16_t action,uint64_t callId,uint16_t domain,int64_t result,const SmxDiagFields994*fields,bool begin=false) noexcept {
 if(!enter())return 0;
 const int64_t sequence=InterlockedIncrement64(&g.header->nextSequence);Payload p{};LARGE_INTEGER q{};QueryPerformanceCounter(&q);
 p.qpc=q.QuadPart;p.callId=begin?uint64_t(sequence):callId;p.stage=stage;p.action=action;p.domain=domain;p.result=result;p.captureSession=uint32_t(InterlockedCompareExchange64(&g.currentSession,0,0));p.tid=GetCurrentThreadId();
 if(fields){p.frame=fields->frame;p.job=fields->job;p.resource=fields->resource;p.fence=fields->fence;p.sourceQpc=fields->sourceQpc;p.value0=fields->value0;p.value1=fields->value1;if(fields->captureSession)p.captureSession=fields->captureSession<=UINT32_MAX?uint32_t(fields->captureSession):0;}
 bool stale=false;if(commitRecord(g.records[(uint64_t(sequence)-1)%Capacity],sequence,p,true,&stale))InterlockedIncrement64(&g.header->committed);else if(stale)InterlockedIncrement64(&g.header->droppedStale);else InterlockedIncrement64(&g.header->droppedBusy);
 leave();return uint64_t(sequence);
}
bool readRecord(const Record&r,int64_t&sequence,Payload&p,uint32_t&badKind) noexcept {
 const int64_t b=r.begin;MemoryBarrier();if(b==0){badKind=0;return false;}if(b<0){badKind=1;return false;}
 memcpy(&p,&r.payload,sizeof p);const uint32_t c=r.crc;MemoryBarrier();const int64_t e=r.end,b2=r.begin;
 if(b!=b2||b!=e||e<=0){badKind=1;return false;}if(crc(&p,sizeof p)!=c){badKind=2;return false;}sequence=b;badKind=0;return true;
}
void recordCrash(EXCEPTION_POINTERS*ep) noexcept {
 if(!ep||!ep->ExceptionRecord||!enter())return;
 Payload p{};LARGE_INTEGER q{};QueryPerformanceCounter(&q);p.qpc=q.QuadPart;p.stage=SMX_DIAG_UNHANDLED994;p.action=SMX_DIAG_FAIL994;p.domain=SMX_DIAG_EXCEPTION994;
 p.result=int64_t(uint32_t(ep->ExceptionRecord->ExceptionCode));p.resource=uint64_t(reinterpret_cast<uintptr_t>(ep->ExceptionRecord->ExceptionAddress));p.value0=ep->ExceptionRecord->ExceptionFlags;p.captureSession=uint32_t(InterlockedCompareExchange64(&g.currentSession,0,0));p.tid=GetCurrentThreadId();
 // Module catalog is preallocated and refreshed only outside the handler.
 const LONG generation=g.header->moduleGeneration;MemoryBarrier();
 if(!(generation&1)){LONG count=g.header->moduleCount;if(count>=0&&count<=LONG(ModuleCapacity))for(LONG i=0;i<count;++i){const auto&m=g.header->modules[i];if(p.resource>=m.base&&p.resource-m.base<m.size){p.job=m.base;p.fence=p.resource-m.base;break;}}}
 MemoryBarrier();if(g.header->moduleGeneration!=generation){p.job=0;p.fence=0;}
 const int64_t sequence=InterlockedIncrement64(&g.header->nextSequence);p.callId=0;
 // Independent reserved crash slot remains usable if the event ring's slot is busy.
 commitRecord(g.header->crash,sequence,p,false);
 bool stale=false;if(commitRecord(g.records[(uint64_t(sequence)-1)%Capacity],sequence,p,true,&stale))InterlockedIncrement64(&g.header->committed);else if(stale)InterlockedIncrement64(&g.header->droppedStale);else InterlockedIncrement64(&g.header->droppedBusy);
 leave();
}
LONG WINAPI exceptionHandler(EXCEPTION_POINTERS*ep) noexcept {
 PreserveWinError preserve;recordCrash(ep);auto previous=g.previous;if(previous&&previous!=&exceptionHandler)return previous(ep);return EXCEPTION_CONTINUE_SEARCH;
}
std::string quoted(const char*s){std::string out="\"";if(s)for(size_t n=0;s[n];++n){unsigned char c=static_cast<unsigned char>(s[n]);if(c=='"'||c=='\\'){out+='\\';out+=char(c);}else if(c<32){char b[7];sprintf_s(b,"\\u%04x",unsigned(c));out+=b;}else out+=char(c);}out+='"';return out;}
std::string utc(uint64_t base,int64_t qpc,int64_t start,int64_t frequency){long double delta=(static_cast<long double>(qpc)-static_cast<long double>(start))*10000000.L/frequency;long double total=static_cast<long double>(base)+delta;if(total<0||total>static_cast<long double>(std::numeric_limits<uint64_t>::max()))return "";uint64_t value=uint64_t(total);FILETIME ft{DWORD(value),DWORD(value>>32)};SYSTEMTIME st{};if(!FileTimeToSystemTime(&ft,&st))return "";char b[40];sprintf_s(b,"%04u-%02u-%02uT%02u:%02u:%02u.%03uZ",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);return b;}
const char*actionName(uint16_t a){switch(a){case SMX_DIAG_BEGIN994:return "begin";case SMX_DIAG_END994:return "end";case SMX_DIAG_FAIL994:return "fail";case SMX_DIAG_ABORT994:return "abort";default:return "instant";}}
const char*domainName(uint16_t d){switch(d){case SMX_DIAG_WIN32994:return "win32";case SMX_DIAG_HRESULT994:return "hresult";case SMX_DIAG_VKRESULT994:return "vkresult";case SMX_DIAG_SEMANTIC994:return "semantic";case SMX_DIAG_EXCEPTION994:return "exception";default:return "none";}}
}
extern "C" bool smx_diag_open994(const wchar_t*directory,const char*sessionId,const char*buildId) noexcept {PreserveErrors preserve;
 if(InterlockedCompareExchange(&g.opened,1,0)!=0){g.openError=ERROR_ALREADY_INITIALIZED;return false;}
 wchar_t path[1024],previous[1024];if(!pathJoin(path,1024,directory,L"app-flight994.bin")||!pathJoin(previous,1024,directory,L"app-flight994.previous.bin")){g.openError=ERROR_FILENAME_EXCED_RANGE;return false;}
 DWORD attr=GetFileAttributesW(path);if(attr!=INVALID_FILE_ATTRIBUTES&&(attr&FILE_ATTRIBUTE_DIRECTORY)){g.openError=ERROR_DIRECTORY;return false;}if(attr!=INVALID_FILE_ATTRIBUTES&&!MoveFileExW(path,previous,MOVEFILE_REPLACE_EXISTING)){g.openError=GetLastError();return false;}
 g.file=CreateFileW(path,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(g.file==INVALID_HANDLE_VALUE){g.openError=GetLastError();return false;}
 LARGE_INTEGER length{};length.QuadPart=FileBytes;
 if(!SetFilePointerEx(g.file,length,nullptr,FILE_BEGIN)||!SetEndOfFile(g.file)){g.openError=GetLastError();CloseHandle(g.file);g.file=INVALID_HANDLE_VALUE;return false;}
 g.mapping=CreateFileMappingW(g.file,nullptr,PAGE_READWRITE,0,FileBytes,nullptr);if(!g.mapping){g.openError=GetLastError();CloseHandle(g.file);g.file=INVALID_HANDLE_VALUE;return false;}
 auto*base=static_cast<uint8_t*>(MapViewOfFile(g.mapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,FileBytes));if(!base){g.openError=GetLastError();CloseHandle(g.mapping);CloseHandle(g.file);g.mapping=nullptr;g.file=INVALID_HANDLE_VALUE;return false;}
 g.header=reinterpret_cast<Header*>(base);g.records=reinterpret_cast<Record*>(base+HeaderBytes);memset(base,0,FileBytes);
 memcpy(g.header->magic,"SMXFL994",8);g.header->schema=Schema;g.header->headerBytes=HeaderBytes;g.header->recordBytes=RecordBytes;g.header->capacity=Capacity;g.header->fileBytes=FileBytes;
 LARGE_INTEGER q{},f{};QueryPerformanceFrequency(&f);QueryPerformanceCounter(&q);FILETIME ft{};GetSystemTimePreciseAsFileTime(&ft);
 g.header->frequency=f.QuadPart;g.header->startQpc=q.QuadPart;g.header->utcFileTime=uint64_t(ft.dwLowDateTime)|(uint64_t(ft.dwHighDateTime)<<32);g.header->pid=GetCurrentProcessId();
 ascii(g.header->session,sizeof g.header->session,sessionId);ascii(g.header->build,sizeof g.header->build,buildId);
 InterlockedExchange(&g.enabled,1);smx_diag_refresh_modules994();smx_diag_event994(SMX_DIAG_ENGINE994,SMX_DIAG_INSTANT994,0,SMX_DIAG_NONE994,0,nullptr);return true;
}
extern "C" uint32_t smx_diag_open_error994(void) noexcept{return g.openError;}
extern "C" uint64_t smx_diag_begin994(uint32_t stage,const SmxDiagFields994*f) noexcept{PreserveErrors preserve;return append(stage,SMX_DIAG_BEGIN994,0,SMX_DIAG_NONE994,0,f,true);}
extern "C" void smx_diag_event994(uint32_t stage,uint16_t action,uint64_t callId,uint16_t domain,int64_t result,const SmxDiagFields994*f) noexcept {PreserveErrors preserve;append(stage,action,callId,domain,result,f);}
extern "C" uint64_t smx_diag_current_session994(void) noexcept {PreserveErrors preserve;return uint64_t(InterlockedCompareExchange64(&g.currentSession,0,0));}
extern "C" void smx_diag_session994(uint64_t sessionSequence) noexcept {PreserveErrors preserve;
 const uint64_t selected=sessionSequence<=UINT32_MAX?sessionSequence:0;InterlockedExchange64(&g.currentSession,LONG64(selected));
 if(enter()){InterlockedExchange64(&g.header->currentSession,LONG64(selected));InterlockedExchange64(&g.header->sessionBeginGlobalSeq,g.header->nextSequence+1);leave();}
 SmxDiagFields994 f{};f.captureSession=selected?selected:UINT64_MAX;f.value0=int64_t(sessionSequence);smx_diag_event994(SMX_DIAG_SESSION994,SMX_DIAG_INSTANT994,0,SMX_DIAG_SEMANTIC994,selected==sessionSequence?0:ERROR_ARITHMETIC_OVERFLOW,&f);
}
extern "C" void smx_diag_refresh_modules994() noexcept {PreserveErrors preserve;
 if(!enter())return;if(InterlockedCompareExchange(&g.refreshing,1,0)!=0){leave();return;}
 HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,GetCurrentProcessId());
 if(snapshot!=INVALID_HANDLE_VALUE){InterlockedIncrement(&g.header->moduleGeneration);MemoryBarrier();g.header->moduleCount=0;g.header->moduleTruncated=0;
  MODULEENTRY32W m{};m.dwSize=sizeof m;if(Module32FirstW(snapshot,&m))do{LONG count=g.header->moduleCount;if(count>=LONG(ModuleCapacity)){g.header->moduleTruncated=1;break;}
   auto&out=g.header->modules[count];memset(&out,0,sizeof out);out.base=uint64_t(reinterpret_cast<uintptr_t>(m.modBaseAddr));out.size=m.modBaseSize;
   if(!WideCharToMultiByte(CP_UTF8,0,m.szModule,-1,out.name,int(sizeof out.name),nullptr,nullptr))ascii(out.name,sizeof out.name,"module_name_unavailable");out.name[sizeof out.name-1]=0;g.header->moduleCount=count+1;
  }while(Module32NextW(snapshot,&m));MemoryBarrier();InterlockedIncrement(&g.header->moduleGeneration);CloseHandle(snapshot);
 }
 InterlockedExchange(&g.refreshing,0);leave();
}
extern "C" bool smx_diag_install_handlers994() noexcept {PreserveErrors preserve;
 if(!g.header||g.installed)return g.installed;g.previous=SetUnhandledExceptionFilter(&exceptionHandler);g.installed=true;return true;
}
extern "C" void smx_diag_close994() noexcept {PreserveErrors preserve;
 smx_diag_event994(SMX_DIAG_END_PROCESS994,SMX_DIAG_INSTANT994,0,SMX_DIAG_NONE994,0,nullptr);
 if(InterlockedExchange(&g.enabled,0)!=1)return;
 if(g.header)InterlockedExchange(&g.header->normalClose,2);
 if(g.installed){SetUnhandledExceptionFilter(g.previous);g.installed=false;}
 // No wait or forced cancellation of a producer. If one is still executing, keep
 // the bounded mapping/handles alive until process teardown rather than unmap it.
 if(InterlockedCompareExchange(&g.writers,0,0)!=0)return;
 if(g.header){InterlockedExchange(&g.header->normalClose,1);FlushViewOfFile(g.header,FileBytes);UnmapViewOfFile(g.header);g.header=nullptr;g.records=nullptr;}
 if(g.mapping){CloseHandle(g.mapping);g.mapping=nullptr;}if(g.file!=INVALID_HANDLE_VALUE){CloseHandle(g.file);g.file=INVALID_HANDLE_VALUE;}
}
extern "C" const char*smx_diag_stage_name994(uint32_t s) noexcept {
 switch(s){
#define S(x,n) case x:return n
 S(SMX_DIAG_ENGINE994,"engine");S(SMX_DIAG_SESSION994,"session");S(SMX_DIAG_MODEL_INIT994,"model.init");S(SMX_DIAG_FOREGROUND_WAIT994,"foreground.wait");
 S(SMX_DIAG_CAPTURE_ACQUIRE994,"capture.acquire");S(SMX_DIAG_CAPTURE_COPY994,"capture.copy");S(SMX_DIAG_CAPTURE_SIGNAL994,"capture.signal");S(SMX_DIAG_CAPTURE_RELEASE994,"capture.release");
 S(SMX_DIAG_MODEL_SUBMIT994,"model.submit");S(SMX_DIAG_MODEL_FENCE994,"model.fence");S(SMX_DIAG_MODEL_COMPLETE994,"model.complete");
 S(SMX_DIAG_DISPLAY_CREDIT994,"display.credit");S(SMX_DIAG_DISPLAY_ACCESS994,"display.access");S(SMX_DIAG_DISPLAY_COPY994,"display.copy");S(SMX_DIAG_DISPLAY_SIGNAL994,"display.signal");S(SMX_DIAG_DISPLAY_FLUSH994,"display.flush");S(SMX_DIAG_PRESENT994,"display.present");S(SMX_DIAG_DISPLAY_STATS994,"display.statistics");S(SMX_DIAG_DISPLAY_CLOSE994,"display.close");
 S(SMX_DIAG_DEVICE_REASON994,"device.removed_reason");S(SMX_DIAG_TARGET_CLOSED994,"target.closed");S(SMX_DIAG_PAUSE994,"pause");S(SMX_DIAG_RESUME994,"resume");S(SMX_DIAG_STOP994,"stop");S(SMX_DIAG_FATAL994,"fatal");S(SMX_DIAG_UNHANDLED994,"exception.unhandled");S(SMX_DIAG_TELEMETRY994,"telemetry");S(SMX_DIAG_MODULES994,"modules");S(SMX_DIAG_END_PROCESS994,"process.end");
 S(SMX_DIAG_CAPTURE_FLUSH994,"capture.flush");S(SMX_DIAG_POOL_CLOSE994,"capture.pool_close");S(SMX_DIAG_GEOMETRY994,"output.geometry");S(SMX_DIAG_D3D_WAIT994,"d3d.wait");S(SMX_DIAG_KEYED_ACQUIRE994,"keyed.acquire");S(SMX_DIAG_KEYED_RELEASE994,"keyed.release");S(SMX_DIAG_MODEL_QUERY994,"model.query");S(SMX_DIAG_MODEL_IDLE994,"model.idle");S(SMX_DIAG_WGC_TRYGET994,"wgc.try_get");S(SMX_DIAG_EXCEPTION_CAUGHT994,"exception.caught");
#undef S
 default:return "custom";
 }
}
extern "C" uint32_t smx_diag_export994(const wchar_t*input,const wchar_t*output) noexcept {PreserveErrors preserve;
 HANDLE file=INVALID_HANDLE_VALUE,mapping=nullptr;const uint8_t*view=nullptr;FILE*out=nullptr;
 auto cleanup=[&](){if(out){fclose(out);out=nullptr;}if(view){UnmapViewOfFile(view);view=nullptr;}if(mapping){CloseHandle(mapping);mapping=nullptr;}if(file!=INVALID_HANDLE_VALUE){CloseHandle(file);file=INVALID_HANDLE_VALUE;}};
 try{
  file=CreateFileW(input,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)return GetLastError();
  HANDLE existing=CreateFileW(output,0,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(existing!=INVALID_HANDLE_VALUE){BY_HANDLE_FILE_INFORMATION a{},b{};bool same=GetFileInformationByHandle(file,&a)&&GetFileInformationByHandle(existing,&b)&&a.dwVolumeSerialNumber==b.dwVolumeSerialNumber&&a.nFileIndexHigh==b.nFileIndexHigh&&a.nFileIndexLow==b.nFileIndexLow;CloseHandle(existing);if(same){cleanup();return ERROR_INVALID_PARAMETER;}}
  LARGE_INTEGER length{};if(!GetFileSizeEx(file,&length)||length.QuadPart!=FileBytes){cleanup();return ERROR_BAD_FORMAT;}
  mapping=CreateFileMappingW(file,nullptr,PAGE_READONLY,0,0,nullptr);if(!mapping){DWORD e=GetLastError();cleanup();return e;}
  view=static_cast<const uint8_t*>(MapViewOfFile(mapping,FILE_MAP_READ,0,0,FileBytes));if(!view){DWORD e=GetLastError();cleanup();return e;}
  const auto*h=reinterpret_cast<const Header*>(view);
  if(memcmp(h->magic,"SMXFL994",8)||h->schema!=Schema||h->headerBytes!=HeaderBytes||h->recordBytes!=RecordBytes||h->capacity!=Capacity||h->frequency<=0){cleanup();return ERROR_BAD_FORMAT;}
  struct Row{int64_t sequence;Payload p;};std::vector<Row>rows;rows.reserve(Capacity);
  const int64_t maxSequence=h->nextSequence;uint64_t empty=0,torn=0,badCrc=0,future=0;
  for(uint32_t i=0;i<Capacity;++i){const auto&r=reinterpret_cast<const Record*>(view+HeaderBytes)[i];Payload p{};int64_t sequence=0;uint32_t bad=0;
   if(readRecord(r,sequence,p,bad)){if(sequence>maxSequence){++future;continue;}if((uint64_t(sequence)-1)%Capacity!=i){++torn;continue;}rows.push_back({sequence,p});}
   else if(bad==1)++torn;else if(bad==2)++badCrc;else ++empty;
  }
  std::sort(rows.begin(),rows.end(),[](const Row&a,const Row&b){return a.sequence<b.sequence;});
  std::map<uint64_t,Row>open;uint64_t orphanEnds=0,sequenceGaps=0;int64_t prev=0;
  for(const auto&r:rows){if(prev&&r.sequence>prev+1)sequenceGaps+=uint64_t(r.sequence-prev-1);prev=r.sequence;
   if(r.p.action==SMX_DIAG_BEGIN994)open[r.p.callId]=r;
   else if(r.p.action==SMX_DIAG_END994||r.p.action==SMX_DIAG_FAIL994||r.p.action==SMX_DIAG_ABORT994){if(r.p.callId&&!open.erase(r.p.callId))++orphanEnds;}
  }
  Module modules[ModuleCapacity]{};LONG moduleCount=0;bool modulesStable=false;
  for(unsigned attempt=0;attempt<3&&!modulesStable;++attempt){LONG begin=h->moduleGeneration;MemoryBarrier();if(begin&1)continue;LONG count=h->moduleCount;if(count<0||count>LONG(ModuleCapacity))break;memcpy(modules,h->modules,size_t(count)*sizeof(Module));MemoryBarrier();modulesStable=h->moduleGeneration==begin;if(modulesStable)moduleCount=count;}
  Payload crash{};int64_t crashSequence=0;uint32_t crashBad=0;bool hasCrash=readRecord(h->crash,crashSequence,crash,crashBad);
  if(_wfopen_s(&out,output,L"wb")||!out){cleanup();return ERROR_ACCESS_DENIED;}
  char build[65]{},session[97]{};memcpy(build,h->build,64);memcpy(session,h->session,96);
  const bool incomplete=h->normalClose!=1||h->overwritten||h->droppedBusy||h->droppedStale||torn||badCrc||future;
  fprintf(out,"{\"type\":\"summary\",\"schema\":1,\"build_id\":%s,\"engine_run_id\":%s,\"pid\":%u,\"current_session_sequence\":%lld,\"session_begin_global_sequence_hint\":%lld,\"file_bytes\":%u,\"record_bytes\":%u,\"capacity\":%u,\"qpc_frequency\":%lld,\"start_qpc\":%lld,\"utc_filetime_base\":%llu,\"utc_derivation\":\"start calibration plus QPC delta; later wall-clock changes not tracked\",\"normal_close\":%s,\"live_or_abnormal_snapshot\":%s,\"requested_sequence\":%lld,\"committed_total\":%lld,\"overwritten_total\":%lld,\"dropped_contention\":%lld,\"dropped_stale_writer\":%lld,\"valid_records\":%zu,\"empty_slots\":%llu,\"torn_slots\":%llu,\"crc_invalid_slots\":%llu,\"future_slots\":%llu,\"retained_sequence_gaps\":%llu,\"orphan_ends\":%llu,\"open_begin_count\":%zu,\"crash_present\":%s,\"crash_slot_invalid\":%u,\"module_catalog_stable\":%s,\"module_catalog_truncated\":%s,\"truncated_or_missing\":%s,\"scope\":\"Bounded in-process CPU breadcrumbs, not GPU utilization or closed-source kernel root cause. Open begin means no matching retained terminal record; overwrite, exception or still-running call are alternatives. Process crash evidence may survive; OS crash/power loss final cache durability is not guaranteed.\"}\n",
   quoted(build).c_str(),quoted(session).c_str(),h->pid,(long long)h->currentSession,(long long)h->sessionBeginGlobalSeq,FileBytes,RecordBytes,Capacity,(long long)h->frequency,(long long)h->startQpc,(unsigned long long)h->utcFileTime,h->normalClose==1?"true":"false",h->normalClose==1?"false":"true",(long long)maxSequence,(long long)h->committed,(long long)h->overwritten,(long long)h->droppedBusy,(long long)h->droppedStale,rows.size(),(unsigned long long)empty,(unsigned long long)torn,(unsigned long long)badCrc,(unsigned long long)future,(unsigned long long)sequenceGaps,(unsigned long long)orphanEnds,open.size(),hasCrash?"true":"false",crashBad,modulesStable?"true":"false",h->moduleTruncated?"true":"false",incomplete?"true":"false");
  for(LONG i=0;i<moduleCount;++i){modules[i].name[95]=0;fprintf(out,"{\"type\":\"module\",\"base\":%llu,\"size\":%llu,\"basename\":%s}\n",(unsigned long long)modules[i].base,(unsigned long long)modules[i].size,quoted(modules[i].name).c_str());}
  auto emit=[&](const char*type,int64_t sequence,const Payload&p){
   const char*module="";uint64_t moduleBase=0,offset=0;if(p.stage==SMX_DIAG_UNHANDLED994)for(LONG i=0;i<moduleCount;++i)if(p.resource>=modules[i].base&&p.resource-modules[i].base<modules[i].size){module=modules[i].name;moduleBase=modules[i].base;offset=p.resource-moduleBase;break;}
   fprintf(out,"{\"type\":\"%s\",\"sequence\":%lld,\"qpc\":%lld,\"utc\":%s,\"pid\":%u,\"session_sequence\":%u,\"tid\":%u,\"stage\":%u,\"stage_name\":\"%s\",\"action\":\"%s\",\"call_id\":%llu,\"domain\":\"%s\",\"result\":%lld,\"result_hex\":\"0x%016llx\",\"frame\":%llu,\"job\":%llu,\"resource\":%llu,\"fence\":%llu,\"source_qpc\":%lld,\"value0\":%lld,\"value1\":%lld,\"module\":%s,\"module_base\":%llu,\"module_offset\":%llu}\n",type,(long long)sequence,(long long)p.qpc,quoted(utc(h->utcFileTime,p.qpc,h->startQpc,h->frequency).c_str()).c_str(),h->pid,p.captureSession,p.tid,p.stage,smx_diag_stage_name994(p.stage),actionName(p.action),(unsigned long long)p.callId,domainName(p.domain),(long long)p.result,(unsigned long long)p.result,(unsigned long long)p.frame,(unsigned long long)p.job,(unsigned long long)p.resource,(unsigned long long)p.fence,(long long)p.sourceQpc,(long long)p.value0,(long long)p.value1,quoted(module).c_str(),(unsigned long long)moduleBase,(unsigned long long)offset);
  };
  for(const auto&r:rows)emit("event",r.sequence,r.p);
  if(hasCrash)emit("crash",crashSequence,crash);
  for(const auto&[id,r]:open){(void)id;emit("open_begin",r.sequence,r.p);}
  if(fflush(out)||ferror(out)){cleanup();return ERROR_WRITE_FAULT;}cleanup();return 0;
 }catch(...){cleanup();return ERROR_NOT_ENOUGH_MEMORY;}
}
extern "C" uint32_t smx_diag_selftest994(const wchar_t*directory) noexcept {
 try{
  if(!smx_diag_open994(directory,"cpu-selftest","diag994-test"))return 1;uint32_t checks=0;auto test=[&](bool b){++checks;if(!b)throw checks;};
  smx_diag_session994(7);smxdiag994::Scope priorSession(SMX_DIAG_DISPLAY_STATS994);smx_diag_session994(8);
  SmxDiagFields994 f{};f.frame=11;f.job=22;f.resource=33;f.fence=44;f.sourceQpc=55;auto call=smx_diag_begin994(SMX_DIAG_PRESENT994,&f);test(call>0);
  smx_diag_event994(SMX_DIAG_PRESENT994,SMX_DIAG_END994,call,SMX_DIAG_HRESULT994,int64_t(int32_t(0x887a0005u)),&f);
  {smxdiag994::Scope scope(SMX_DIAG_CAPTURE_COPY994,f);}
  std::vector<std::thread>workers;for(unsigned t=0;t<4;++t)workers.emplace_back([t]{SmxDiagFields994 v{};for(unsigned n=0;n<20000;++n){v.job=t;v.frame=n;smx_diag_event994(SMX_DIAG_MODEL_SUBMIT994,SMX_DIAG_INSTANT994,0,SMX_DIAG_SEMANTIC994,0,&v);}});
  for(auto&t:workers)t.join();test(g.header->nextSequence>Capacity);test(g.header->overwritten>0);
  // A held slot never blocks: exactly one attempted claim is omitted.
  int64_t next=g.header->nextSequence+1;Record&r=g.records[(uint64_t(next)-1)%Capacity];InterlockedExchange(&r.busy,1);auto old=g.header->droppedBusy;smx_diag_event994(SMX_DIAG_MODEL_QUERY994,SMX_DIAG_BEGIN994,0,SMX_DIAG_NONE994,0,nullptr);test(g.header->droppedBusy==old+1);InterlockedExchange(&r.busy,0);
  priorSession.end();
  auto unreturned=smx_diag_begin994(SMX_DIAG_MODEL_FENCE994,&f);test(unreturned>0);
  EXCEPTION_RECORD er{};er.ExceptionCode=0xc0000005u;er.ExceptionAddress=reinterpret_cast<void*>(&smx_diag_selftest994);EXCEPTION_POINTERS ep{&er,nullptr};recordCrash(&ep);Payload cp{};int64_t cs=0;uint32_t bad=0;test(readRecord(g.header->crash,cs,cp,bad));test(cp.result==0xc0000005u&&cp.resource!=0&&cp.job!=0);
  // Corrupted/torn slots are rejected, never interpreted as successful API ends.
  Record local{};Payload p{};p.qpc=123;p.stage=SMX_DIAG_PRESENT994;
  test(commitRecord(local,123,p,false));Payload read{};int64_t seq=0;test(readRecord(local,seq,read,bad)&&seq==123);bool wasStale=false;test(!commitRecord(local,122,p,false,&wasStale)&&wasStale);test(readRecord(local,seq,read,bad)&&seq==123);local.payload.result^=1;test(!readRecord(local,seq,read,bad)&&bad==2);local.begin=-123;test(!readRecord(local,seq,read,bad)&&bad==1);
  wchar_t input[1024],output[1024],report[1024];test(pathJoin(input,1024,directory,L"app-flight994.bin"));test(pathJoin(output,1024,directory,L"flight994-selftest.jsonl"));test(smx_diag_export994(input,output)==0);
  smx_diag_close994();test(smx_diag_export994(input,output)==0);
  pathJoin(report,1024,directory,L"diagnostic994-selftest.json");FILE*out=nullptr;_wfopen_s(&out,report,L"wb");if(!out)return 3;fprintf(out,"{\"pass\":true,\"checks\":%u,\"events_requested\":80000,\"concurrent_writers\":4,\"gpu_used\":false,\"windows_created\":false,\"exception_is_synthetic\":true,\"mapped_bytes\":%u}\n",checks,FileBytes);fclose(out);return 0;
 }catch(...){smx_diag_close994();return 2;}
}
