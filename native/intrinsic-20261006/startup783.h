/* R783: optional workspace cache, compatibility/integrity checks and phase timing. */
#include <wchar.h>
static FILE* phase783;
static double epoch783,last783;
static double clock783(void){LARGE_INTEGER t,f;QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);return 1000.0*(double)t.QuadPart/(double)f.QuadPart;}
static void begin783(void){char path[4096];const char*d=getenv("SMX_BENCH_DIR");epoch783=last783=clock783();if(d&&snprintf(path,sizeof(path),"%s\\startup783.csv",d)>0)phase783=fopen(path,"wb");if(phase783){fprintf(phase783,"event,item,elapsed_ms,duration_ms,feedback_valid,application_cache_hit\n");fflush(phase783);}}
static void sample783(const char*event,const char*item,double elapsed,double duration,int valid,int hit){if(phase783){fprintf(phase783,"%s,%s,%.6f,%.6f,%d,%d\n",event,item,elapsed-epoch783,duration,valid,hit);fflush(phase783);}}
static void mark783(const char*name){double t=clock783();sample783("phase",name,t,t-last783,0,0);last783=t;}
static uint64_t hash783(const void*data,size_t n){uint64_t h=14695981039346656037ull;const unsigned char*p=data;for(size_t i=0;i<n;i++){h^=p[i];h*=1099511628211ull;}return h;}
typedef struct {char magic[8];uint32_t size,version,vendor,device,driver,flags;unsigned char uuid[16];uint64_t bytes,hash;} CacheHeader783;
static wchar_t cachePath783[4096];static int cacheWritable783;
static int cacheHeader783(const void*d,size_t n,const VkPhysicalDeviceProperties*p){if(n<32)return 0;uint32_t h[4];memcpy(h,d,sizeof(h));return h[0]>=32&&h[0]<=n&&h[1]==VK_PIPELINE_CACHE_HEADER_VERSION_ONE&&h[2]==p->vendorID&&h[3]==p->deviceID&&!memcmp((const char*)d+16,p->pipelineCacheUUID,16);}
static void* cacheLoad783(const VkPhysicalDeviceProperties*p,size_t*bytes){
 *bytes=0;wchar_t dir[3500],uuid[33];DWORD n=GetEnvironmentVariableW(L"SMX_PIPELINE_CACHE_DIR",dir,3500);cachePath783[0]=0;cacheWritable783=0;
 if(!n||n>=3500){fprintf(stderr,"CACHE783 disabled: explicit workspace directory absent\n");return NULL;}
 if(!((dir[0]&&dir[1]==L':'&&(dir[2]==L'\\'||dir[2]==L'/'))||(dir[0]==L'\\'&&dir[1]==L'\\'))){fprintf(stderr,"CACHE783 disabled: absolute directory required\n");return NULL;}
 DWORD attr=GetFileAttributesW(dir);if(attr==INVALID_FILE_ATTRIBUTES||!(attr&FILE_ATTRIBUTE_DIRECTORY)||attr&FILE_ATTRIBUTE_REPARSE_POINT){fprintf(stderr,"CACHE783 disabled: existing non-reparse directory required\n");return NULL;}
 for(unsigned i=0;i<16;i++)swprintf(uuid+i*2,3,L"%02x",p->pipelineCacheUUID[i]);
 unsigned flags=(getenv("SMX_DISABLE_PIPELINE_OPT")?1u:0u)|(getenv("SMX_COLLECT_STATISTICS")?2u:0u);
 if(swprintf(cachePath783,4096,L"%ls\\vk783-%04x-%04x-%08x-%ls-%u.bin",dir,p->vendorID,p->deviceID,p->driverVersion,uuid,flags)<0){cachePath783[0]=0;return NULL;}
 cacheWritable783=!getenv("SMX_NO_CACHE_WRITE");FILE*f=_wfopen(cachePath783,L"rb");if(!f){fprintf(stderr,"CACHE783 cold: no application cache\n");return NULL;}
 CacheHeader783 h={0};void*d=NULL;int good=0;
 if(fread(&h,1,sizeof(h),f)==sizeof(h)&&!memcmp(h.magic,"SMXPC783",8)&&h.size==sizeof(h)&&h.version==1&&h.vendor==p->vendorID&&h.device==p->deviceID&&h.driver==p->driverVersion&&h.flags==flags&&!memcmp(h.uuid,p->pipelineCacheUUID,16)&&h.bytes>=32&&h.bytes<=268435456){
  d=malloc((size_t)h.bytes);if(d&&fread(d,1,(size_t)h.bytes,f)==h.bytes&&fgetc(f)==EOF&&hash783(d,(size_t)h.bytes)==h.hash&&cacheHeader783(d,(size_t)h.bytes,p))good=1;
 }
 fclose(f);if(!good){free(d);fprintf(stderr,"CACHE783 ignored: incompatible, truncated, or corrupt cache\n");return NULL;}
 *bytes=(size_t)h.bytes;fprintf(stderr,"CACHE783 compatible bytes=%llu; hits require creation feedback\n",(unsigned long long)*bytes);return d;
}
static void cacheSave783(VkPipelineCache cache,const VkPhysicalDeviceProperties*p){
 if(!cacheWritable783||!cachePath783[0])return;size_t bytes=0;VkResult r=vkGetPipelineCacheData(device,cache,&bytes,NULL);
 if(r!=VK_SUCCESS||bytes<=32||bytes>268435456){fprintf(stderr,"CACHE783 save skipped result=%d bytes=%llu\n",r,(unsigned long long)bytes);return;}
 void*d=malloc(bytes);if(!d)return;r=vkGetPipelineCacheData(device,cache,&bytes,d);if(r!=VK_SUCCESS||!cacheHeader783(d,bytes,p)){free(d);fprintf(stderr,"CACHE783 save skipped invalid driver payload\n");return;}
 CacheHeader783 h={0};memcpy(h.magic,"SMXPC783",8);h.size=sizeof(h);h.version=1;h.vendor=p->vendorID;h.device=p->deviceID;h.driver=p->driverVersion;h.flags=(getenv("SMX_DISABLE_PIPELINE_OPT")?1u:0u)|(getenv("SMX_COLLECT_STATISTICS")?2u:0u);memcpy(h.uuid,p->pipelineCacheUUID,16);h.bytes=bytes;h.hash=hash783(d,bytes);
 wchar_t tmp[4096];if(swprintf(tmp,4096,L"%ls.%lu.tmp",cachePath783,GetCurrentProcessId())<0){free(d);return;}
 HANDLE f=CreateFileW(tmp,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);int good=0;DWORD written=0;
 if(f!=INVALID_HANDLE_VALUE){good=WriteFile(f,&h,sizeof(h),&written,NULL)&&written==sizeof(h);if(good)good=WriteFile(f,d,(DWORD)bytes,&written,NULL)&&written==bytes;if(good)good=FlushFileBuffers(f)!=0;CloseHandle(f);if(good)good=MoveFileExW(tmp,cachePath783,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;if(!good)DeleteFileW(tmp);}
 fprintf(stderr,"CACHE783 save_success=%d bytes=%llu\n",good,(unsigned long long)bytes);free(d);
}
