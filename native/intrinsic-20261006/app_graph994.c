#define WIN32_LEAN_AND_MEAN
#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include <vulkan/vulkan.h>
#include "graph_interop_rotate.h"
static SmxGraphInterop interopContext;
static VkQueue wsiQueue961;VkQueue smx_wsi_queue961(void){return wsiQueue961;}
static VkInstance wsiInstance958;static VkPhysicalDevice wsiPhysical958;static PFN_vkGetInstanceProcAddr wsiGet958;
VkInstance smx_wsi_instance958(void){return wsiInstance958;}
VkPhysicalDevice smx_wsi_physical958(void){return wsiPhysical958;}
PFN_vkGetInstanceProcAddr smx_wsi_get958(void){return wsiGet958;}
const SmxGraphInterop* smx_graph_interop_context(void){return &interopContext;}

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../adapter_identity995.h"
#include "../device_probe995.h"
static SRWLOCK selectedLock995=SRWLOCK_INIT;static SmxAdapterIdentity995 selected995;static uint32_t wsiFamily995=UINT32_MAX;
int smx_selected_adapter995(SmxAdapterIdentity995*out){if(!out)return 0;AcquireSRWLockShared(&selectedLock995);*out=selected995;ReleaseSRWLockShared(&selectedLock995);return out->valid;}
uint32_t smx_wsi_family995(void){return wsiFamily995;}

#include "fp16-next537/candidate994/diagnostic994_c.h"
#include "fp16-next537/candidate994/bounded_stdio989.h"
#define fprintf smx_log_fprintf989
#define fopen smx_log_fopen989
#define fopen_s smx_log_fopen_s989
#define freopen_s smx_log_freopen_s989
#define fclose smx_log_fclose989
#include "fp16-next537/candidate994/resolution_budget989.hpp"
static uint32_t extentImageLimit989=16384u,extentDispatchLimit989=UINT32_MAX;
int smx_graph_extent830(uint32_t w,uint32_t h){
 SmxResolutionBudget989 checked989;char why989[384];
 if(!smx_extent_check989(w,h,extentImageLimit989,extentDispatchLimit989,&checked989,why989,sizeof(why989))){fprintf(stderr,"CAPACITY989 extent_rejected width=%u height=%u reason=%s\n",w,h,why989);return -1;}
 interopContext.width=w;interopContext.height=h;return 0;
}
#define CHECK(e) do{VkResult r=(e);if(r!=VK_SUCCESS){SmxDiagFields994 df={0};df.resource=(uint64_t)(uintptr_t)device;df.value0=__LINE__;df.value1=3;smx_diag_event994(SMX_DIAG_FATAL994,SMX_DIAG_FAIL994,0,SMX_DIAG_VKRESULT994,r,&df);fprintf(stderr,"%s failed: %d at %d\n",#e,r,__LINE__);exit(2);}}while(0)
#define DEVICE_FUNCS(X) \
 X(vkDestroyDevice) X(vkGetDeviceQueue) X(vkCreateBuffer) X(vkDestroyBuffer) \
 X(vkGetBufferMemoryRequirements) X(vkAllocateMemory) X(vkFreeMemory) X(vkBindBufferMemory) X(vkMapMemory) X(vkUnmapMemory) \
 X(vkCreateShaderModule) X(vkDestroyShaderModule) X(vkCreateDescriptorSetLayout) X(vkDestroyDescriptorSetLayout) \
 X(vkCreatePipelineLayout) X(vkDestroyPipelineLayout) X(vkCreateComputePipelines) X(vkDestroyPipeline) \
 X(vkCreateDescriptorPool) X(vkDestroyDescriptorPool) X(vkAllocateDescriptorSets) X(vkUpdateDescriptorSets) \
 X(vkCreateCommandPool) X(vkDestroyCommandPool) X(vkAllocateCommandBuffers) X(vkFreeCommandBuffers) \
 X(vkBeginCommandBuffer) X(vkEndCommandBuffer) X(vkCmdPipelineBarrier) X(vkCmdBindPipeline) X(vkCmdBindDescriptorSets) \
 X(vkCmdPushConstants) X(vkCmdDispatch) X(vkCmdCopyBuffer) X(vkCreateFence) X(vkDestroyFence) X(vkQueueSubmit) X(vkWaitForFences) X(vkResetFences) X(vkCreateQueryPool) X(vkDestroyQueryPool) X(vkCmdResetQueryPool) X(vkCmdWriteTimestamp) X(vkGetQueryPoolResults) X(vkCreateSampler) X(vkDestroySampler) X(vkCreatePipelineCache) X(vkDestroyPipelineCache) X(vkGetPipelineCacheData)
#define DECL(N) static PFN_##N N;
DEVICE_FUNCS(DECL)

static VkResult diagQueueSubmit994(VkQueue q,uint32_t n,const VkSubmitInfo*si,VkFence f){
 SmxDiagFields994 d={0};d.captureSession=smx_diag_current_session994();if(!d.captureSession)d.captureSession=UINT64_MAX;d.resource=(uint64_t)(uintptr_t)q;d.fence=(uint64_t)(uintptr_t)f;d.value0=n;d.value1=1;
 uint64_t id=smx_diag_begin994(SMX_DIAG_MODEL_SUBMIT994,&d);VkResult r=vkQueueSubmit(q,n,si,f);
 smx_diag_event994(SMX_DIAG_MODEL_SUBMIT994,r<0?SMX_DIAG_FAIL994:SMX_DIAG_END994,id,SMX_DIAG_VKRESULT994,r,&d);return r;
}
static VkResult diagFence994(VkDevice dev,uint32_t n,const VkFence*f,VkBool32 all,uint64_t ns){
 SmxDiagFields994 d={0};d.captureSession=smx_diag_current_session994();if(!d.captureSession)d.captureSession=UINT64_MAX;d.resource=(uint64_t)(uintptr_t)dev;d.fence=n?(uint64_t)(uintptr_t)f[0]:0;d.value0=ns;d.value1=n;
 uint64_t id=smx_diag_begin994(SMX_DIAG_MODEL_FENCE994,&d);VkResult r=vkWaitForFences(dev,n,f,all,ns);
 smx_diag_event994(SMX_DIAG_MODEL_FENCE994,r<0?SMX_DIAG_FAIL994:SMX_DIAG_END994,id,SMX_DIAG_VKRESULT994,r,&d);return r;
}
static VkResult diagQuery994(VkDevice dev,VkQueryPool pool,uint32_t first,uint32_t count,size_t size,void*data,VkDeviceSize stride,VkQueryResultFlags flags){
 SmxDiagFields994 d={0};d.captureSession=smx_diag_current_session994();if(!d.captureSession)d.captureSession=UINT64_MAX;d.resource=(uint64_t)(uintptr_t)pool;d.value0=first;d.value1=count;
 uint64_t id=smx_diag_begin994(SMX_DIAG_MODEL_QUERY994,&d);VkResult r=vkGetQueryPoolResults(dev,pool,first,count,size,data,stride,flags);
 smx_diag_event994(SMX_DIAG_MODEL_QUERY994,r<0?SMX_DIAG_FAIL994:SMX_DIAG_END994,id,SMX_DIAG_VKRESULT994,r,&d);return r;
}
static VkResult diagPipelines994(VkDevice d,VkPipelineCache cache,uint32_t n,const VkComputePipelineCreateInfo*ci,const VkAllocationCallbacks*a,VkPipeline*out){
 SmxDiagFields994 f={0};f.captureSession=smx_diag_current_session994();if(!f.captureSession)f.captureSession=UINT64_MAX;f.resource=(uint64_t)(uintptr_t)cache;f.value0=n;f.value1=4;
 uint64_t id=smx_diag_begin994(SMX_DIAG_MODEL_INIT994,&f);VkResult r=vkCreateComputePipelines(d,cache,n,ci,a,out);
 smx_diag_event994(SMX_DIAG_MODEL_INIT994,r<0?SMX_DIAG_FAIL994:SMX_DIAG_END994,id,SMX_DIAG_VKRESULT994,r,&f);return r;
}
#define vkCreateComputePipelines(d,p,n,c,a,o) diagPipelines994(d,p,n,c,a,o)
#define vkQueueSubmit(q,n,si,f) diagQueueSubmit994(q,n,si,f)
#define vkWaitForFences(d,n,f,a,t) diagFence994(d,n,f,a,t)
#define vkGetQueryPoolResults(d,p,f,c,s,o,t,g) diagQuery994(d,p,f,c,s,o,t,g)

static VkDevice device;
static VkPhysicalDeviceMemoryProperties memory;
typedef struct {VkBuffer buffer;VkDeviceMemory memory;void* mapped;} Buffer;
static Buffer make_buffer(size_t n) {
 Buffer b={0};VkBufferCreateInfo ci={VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};ci.size=n;ci.usage=VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_SRC_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT;ci.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
 CHECK(vkCreateBuffer(device,&ci,NULL,&b.buffer));
 VkMemoryRequirements req;vkGetBufferMemoryRequirements(device,b.buffer,&req);
 uint32_t type=UINT32_MAX;
 for(uint32_t i=0;i<memory.memoryTypeCount;i++)if((req.memoryTypeBits&(1u<<i))&&
  ((memory.memoryTypes[i].propertyFlags&(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))==(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))){type=i;break;}
 const char* mt=getenv("SMX_MEMORY_TYPE");if(mt){uint32_t candidate=(uint32_t)atoi(mt);if(candidate>=memory.memoryTypeCount||!(req.memoryTypeBits&(1u<<candidate))||(memory.memoryTypes[candidate].propertyFlags&6u)!=6u){fprintf(stderr,"Invalid mapped memory type\n");exit(3);}type=candidate;}
 if(type==UINT32_MAX){fprintf(stderr,"No coherent host-visible memory\n");exit(3);}
 VkMemoryAllocateInfo ai={VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.allocationSize=req.size;ai.memoryTypeIndex=type;
 CHECK(vkAllocateMemory(device,&ai,NULL,&b.memory));CHECK(vkBindBufferMemory(device,b.buffer,b.memory,0));CHECK(vkMapMemory(device,b.memory,0,n,0,&b.mapped));return b;
}
static void drop_buffer(Buffer* b){if(b->mapped)vkUnmapMemory(device,b->memory);vkDestroyBuffer(device,b->buffer,NULL);vkFreeMemory(device,b->memory,NULL);}
static uint16_t to_half(float f){uint32_t u;memcpy(&u,&f,4);uint32_t s=(u>>16)&32768;int e=(int)((u>>23)&255)-127+15;if(e<=0)return(uint16_t)s;if(e>=31)return(uint16_t)(s|31744);uint32_t t=(u&8388607)+4095+((u>>13)&1);if(t&8388608){t=0;if(++e>=31)return(uint16_t)(s|31744);}return(uint16_t)(s|((uint32_t)e<<10)|(t>>13));}
static float from_half(uint16_t h){uint32_t e=(h>>10)&31,m=h&1023,s=(uint32_t)(h&32768)<<16,u;if(!e){if(!m)u=s;else {int ee=-14;while(!(m&1024)){m<<=1;ee--;}u=s|((uint32_t)(ee+127)<<23)|((m&1023)<<13);}}else u=s|((e==31?255:e+112)<<23)|(m<<13);float f;memcpy(&f,&u,4);return f;}
#define INST(T,N) T N=(T)gipa(instance,#N);if(!N){fprintf(stderr,"Missing %s\n",#N);return 2;}

typedef struct {Buffer gpu;uint64_t size;char init[128],reference[128],name[32];} GraphBuffer;
typedef struct {char shader[64],name[32];uint32_t bi[3],cfg[16],dispatch[3];uint64_t off[3],range[3];VkDescriptorSet set;uint32_t pipeline;} GraphNode;
typedef struct {char path[64];VkPipeline pipeline;VkShaderModule module;uint32_t readonlyMask;} GraphPipeline;
static void read_exact(FILE*f,void*p,size_t n){if(fread(p,1,n,f)!=n){fprintf(stderr,"Truncated graph manifest\n");exit(1);}}
static size_t read_file(const char*path,void**out){FILE*f=fopen(path,"rb");if(!f){fprintf(stderr,"Cannot read %s\n",path);exit(1);}fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);if(n<=0||n>1073741824){fprintf(stderr,"Invalid asset size\n");exit(1);}*out=malloc(n);if(!*out)exit(1);read_exact(f,*out,n);fclose(f);return(size_t)n;}
static void check_path(char*s,size_t n){if(!memchr(s,0,n)||strstr(s,"..")||strchr(s,':')||s[0]=='/'||s[0]=='\\'){fprintf(stderr,"Invalid relative asset path\n");exit(1);}}

static int independent(const GraphNode*a,const GraphNode*b){
 if(getenv("SMX_STRICT_BARRIERS"))return 0;
 if(strstr(a->shader,"_sg32_o")&&strstr(b->shader,"_sg32_o")&&!memcmp(a->bi,b->bi,sizeof(a->bi))&&!memcmp(a->cfg,b->cfg,sizeof(a->cfg))&&!memcmp(a->off,b->off,sizeof(a->off)))return 1;
 if(strstr(a->name,"fused-")&&strstr(b->name,"fused-")&&a->bi[2]==b->bi[2]&&a->bi[2]!=b->bi[0]&&a->bi[2]!=b->bi[1]&&a->cfg[0]==b->cfg[0]&&a->cfg[1]==b->cfg[1]&&a->cfg[10]==b->cfg[10]&&a->cfg[12]==b->cfg[12]&&a->cfg[15]==b->cfg[15]&&(a->cfg[13]+a->cfg[5]<=b->cfg[13]||b->cfg[13]+b->cfg[5]<=a->cfg[13]))return 1;
 return 0;
}


/* R395: independently implemented conservative whole-buffer dependency plan.
   Unknown SPIR-V access is read/write; no buffer aliasing is introduced. */
static uint32_t spirv_readonly(const uint32_t*w,size_t bytes){
 if(bytes<20||w[0]!=0x07230203u||w[3]>65536||w[3]<1)return 0;
 uint32_t bound=w[3],mask=0;
 uint32_t*binding=malloc(bound*4),*ptr=calloc(bound,4),*var=calloc(bound,4);
 unsigned char*ro=calloc(bound,1),*member=calloc(bound,1),*single=calloc(bound,1);
 if(!binding||!ptr||!var||!ro||!member||!single){free(binding);free(ptr);free(var);free(ro);free(member);free(single);return 0;}
 for(uint32_t j=0;j<bound;j++)binding[j]=UINT32_MAX;
 size_t len=bytes/4;int good=1;
 for(size_t k=5;k<len;){
  uint32_t wc=w[k]>>16,op=w[k]&65535;if(!wc||k+wc>len){good=0;break;}
  if(op==71&&wc>=3&&w[k+1]<bound){
   uint32_t id=w[k+1],dec=w[k+2];if(dec==24)ro[id]=1;
   if(dec==33&&wc>=4)binding[id]=w[k+3];
  }
  if(op==72&&wc>=4&&w[k+1]<bound&&w[k+2]==0&&w[k+3]==24)member[w[k+1]]=1;
  if(op==30&&wc==3&&w[k+1]<bound)single[w[k+1]]=1;
  if(op==32&&wc==4&&w[k+1]<bound&&w[k+3]<bound)ptr[w[k+1]]=w[k+3];
  if(op==59&&wc>=4&&w[k+1]<bound&&w[k+2]<bound)var[w[k+2]]=w[k+1];
  k+=wc;
 }
 if(good)for(uint32_t j=1;j<bound;j++)if(binding[j]<2&&var[j]){
  uint32_t p=ptr[var[j]];if(ro[j]||(p<bound&&single[p]&&member[p]))mask|=1u<<binding[j];
 }
 free(binding);free(ptr);free(var);free(ro);free(member);free(single);return mask;
}
static int forceBarriers839=0;
typedef struct {unsigned char read[128],write[128];uint32_t calls,dependencies,skipped837;const GraphNode*previous837;} Dependency395;
static void dependency395(VkCommandBuffer cmd,Dependency395*d,const GraphNode*n,const GraphPipeline*p,const GraphBuffer*bs){
 unsigned char cr[128]={0},cw[128]={0};
 for(unsigned j=0;j<3;j++){uint32_t b=n->bi[j];cr[b]=1;if(j==2||!(p->readonlyMask&(1u<<j)))cw[b]=1;}
 unsigned group837=0,part837=0,oldGroup837=0,oldPart837=0;
 int peer837=!forceBarriers839&&d->previous837&&sscanf(n->name,"parallel837-%u-%u",&group837,&part837)==2&&sscanf(d->previous837->name,"parallel837-%u-%u",&oldGroup837,&oldPart837)==2&&group837==oldGroup837&&part837==oldPart837+1&&!memcmp(n->bi,d->previous837->bi,sizeof(n->bi))&&!memcmp(n->off,d->previous837->off,sizeof(n->off))&&!memcmp(n->range,d->previous837->range,sizeof(n->range))&&!memcmp(n->cfg,d->previous837->cfg,sizeof(n->cfg));
 if(peer837)d->skipped837++;
 VkBufferMemoryBarrier barriers[3];uint32_t count=0;
 for(uint32_t b=0;b<128&&!peer837;b++)if((cr[b]&&d->write[b])||(cw[b]&&(d->read[b]||d->write[b]))){
  VkBufferMemoryBarrier*v=&barriers[count++];memset(v,0,sizeof(*v));v->sType=VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  v->srcAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;v->dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;
  v->srcQueueFamilyIndex=v->dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;v->buffer=bs[b].gpu.buffer;v->offset=0;v->size=bs[b].size;
  d->read[b]=d->write[b]=0;
 }
 if(count){vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,0,NULL,count,barriers,0,NULL);d->calls++;d->dependencies+=count;}
 for(uint32_t b=0;b<128;b++){d->read[b]|=cr[b];d->write[b]|=cw[b];}d->previous837=n;
}


static Buffer make_private467(size_t n){
 Buffer b={0};VkBufferCreateInfo ci={VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};ci.size=n;ci.usage=VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_SRC_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT;ci.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
 CHECK(vkCreateBuffer(device,&ci,NULL,&b.buffer));VkMemoryRequirements req;vkGetBufferMemoryRequirements(device,b.buffer,&req);uint32_t type=UINT32_MAX;
 for(uint32_t i=0;i<memory.memoryTypeCount;i++)if((req.memoryTypeBits&(1u<<i))&&(memory.memoryTypes[i].propertyFlags&VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)&&!(memory.memoryTypes[i].propertyFlags&VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)){type=i;break;}
 if(type==UINT32_MAX){fprintf(stderr,"PRIVATE467 unsupported memory; no silent mapped fallback\n");exit(3);}
 VkMemoryAllocateInfo ai={VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.allocationSize=req.size;ai.memoryTypeIndex=type;CHECK(vkAllocateMemory(device,&ai,NULL,&b.memory));CHECK(vkBindBufferMemory(device,b.buffer,b.memory,0));return b;
}
static void upload_private467(GraphBuffer*bs,Buffer*src,uint32_t nb,VkQueue queue,uint32_t qi){
 uint32_t count=0;uint64_t bytes=0;for(uint32_t i=0;i<nb;i++)if(src[i].buffer){count++;bytes+=bs[i].size;}if(!count)return;
 VkCommandPoolCreateInfo pi={VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};pi.queueFamilyIndex=qi;VkCommandPool pool;CHECK(vkCreateCommandPool(device,&pi,NULL,&pool));VkCommandBufferAllocateInfo ca={VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ca.commandPool=pool;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=1;VkCommandBuffer cmd;CHECK(vkAllocateCommandBuffers(device,&ca,&cmd));
 VkCommandBufferBeginInfo bi={VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};CHECK(vkBeginCommandBuffer(cmd,&bi));VkMemoryBarrier mb={VK_STRUCTURE_TYPE_MEMORY_BARRIER};mb.srcAccessMask=VK_ACCESS_HOST_WRITE_BIT;mb.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_HOST_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,1,&mb,0,NULL,0,NULL);
 for(uint32_t i=0;i<nb;i++)if(src[i].buffer){VkBufferCopy copy={0,0,bs[i].size};vkCmdCopyBuffer(cmd,src[i].buffer,bs[i].gpu.buffer,1,&copy);}
 mb.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;mb.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&mb,0,NULL,0,NULL);CHECK(vkEndCommandBuffer(cmd));
 VkFenceCreateInfo fi={VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};VkFence fence;CHECK(vkCreateFence(device,&fi,NULL,&fence));VkSubmitInfo si={VK_STRUCTURE_TYPE_SUBMIT_INFO};si.commandBufferCount=1;si.pCommandBuffers=&cmd;CHECK(vkQueueSubmit(queue,1,&si,fence));CHECK(vkWaitForFences(device,1,&fence,VK_TRUE,10000000000ull));vkDestroyFence(device,fence,NULL);vkDestroyCommandPool(device,pool,NULL);
 for(uint32_t i=0;i<nb;i++)if(src[i].buffer)drop_buffer(&src[i]);
 fprintf(stderr,"PRIVATE467 buffers=%u bytes=%llu startup upload only; frames/final/references mapped; zero per-frame extra copies\n",count,(unsigned long long)bytes);
}

typedef struct {VkQueue queue;VkCommandBuffer cmd;VkFence fence;VkQueryPool queries;uint32_t nodes;float period;} LiveGraph;
static double run_live(void* opaque){
 LiveGraph*g=(LiveGraph*)opaque;VkSubmitInfo si={VK_STRUCTURE_TYPE_SUBMIT_INFO};si.commandBufferCount=1;si.pCommandBuffers=&g->cmd;
 CHECK(vkQueueSubmit(g->queue,1,&si,g->fence));CHECK(vkWaitForFences(device,1,&g->fence,VK_TRUE,10000000000ull));
 uint64_t t[1024];CHECK(vkGetQueryPoolResults(device,g->queries,0,g->nodes*2,g->nodes*2*sizeof(uint64_t),t,sizeof(uint64_t),VK_QUERY_RESULT_64_BIT|VK_QUERY_RESULT_WAIT_BIT));
 CHECK(vkResetFences(device,1,&g->fence));return (t[g->nodes*2-1]-t[0])*g->period/1e6;
}
extern int smx_app_session(void*,const void*,uint32_t,uint32_t,void*,double(*)(void*),const char*);

/* R969: redirect only native texture adapters, never graph/model assets.
   Feature and exact BGRA format guards execute before device creation.
   Math instructions are unchanged; repaired shader interface is Unknown-format storage write. */
static size_t read_native_shader969(const char*path,void**out){
 const char*leaf=NULL;
 if(!strcmp(path,"shaders/native830-down.spv"))leaf="native969-down-compiled.spv";
 else if(!strcmp(path,"shaders/native830-warp.spv"))leaf="native969-warp-compiled.spv";
 if(!leaf)return read_file(path,out);
 const char*dir=getenv("SMX_NATIVE_SHADERS969");char resolved[4096];
 if(!dir||!*dir){fprintf(stderr,"NATIVE969_FATAL missing SMX_NATIVE_SHADERS969; original model shaders are not overwritten or used as fallback\n");exit(3);}
 if(!(dir[0]=='/'||dir[0]=='\\'||(strlen(dir)>=3&&dir[1]==':'&&(dir[2]=='\\'||dir[2]=='/')))){fprintf(stderr,"NATIVE969_FATAL shader directory must be absolute\n");exit(3);}
 int count=snprintf(resolved,sizeof(resolved),"%s/%s",dir,leaf);
 if(count<0||(size_t)count>=sizeof(resolved)){fprintf(stderr,"NATIVE969_FATAL shader path too long\n");exit(3);}
 size_t bytes=read_file(resolved,out);const uint32_t*w=(const uint32_t*)*out;
 if(bytes%4||bytes<20||w[0]!=0x07230203u){fprintf(stderr,"NATIVE969_FATAL invalid SPIR-V\n");exit(3);}
 size_t words=bytes/4,i=5;unsigned images=0,writes=0,writeCapability=0;
 while(i<words){uint32_t n=w[i]>>16,op=w[i]&65535u;if(!n||n>words-i){fprintf(stderr,"NATIVE969_FATAL invalid SPIR-V instruction span\n");exit(3);}
  if(op==17&&n>=2&&w[i+1]==56u)writeCapability=1;
  if(op==25&&n>=9&&w[i+7]==2u){++images;if(w[i+8]!=0u){fprintf(stderr,"NATIVE969_FATAL typed storage image format=%u incompatible with BGRA view; no fallback\n",w[i+8]);exit(3);}}
  if(op==99u)++writes;i+=n;
 }
 if(!images||(writes&&!writeCapability)){fprintf(stderr,"NATIVE969_FATAL storage-image contract missing images=%u writes=%u capability56=%u\n",images,writes,writeCapability);exit(3);}
 fprintf(stderr,"NATIVE969_SHADER requested=%s resolved=%s bytes=%llu storage_images=%u image_writes=%u storage_format=Unknown write_without_format_capability=%u original_model_assets_unchanged=1\n",path,resolved,(unsigned long long)bytes,images,writes,writeCapability);fflush(stderr);return bytes;
}
#define read_file read_native_shader969
#include "external_core905.h"
#undef read_file
#include "startup783.h"
extern int smx_app_wait_restart783(int);
int smx_graph_main(int argc,char**argv){
 SmxDiagFields994 initFields994={0};initFields994.captureSession=smx_diag_current_session994();if(!initFields994.captureSession)initFields994.captureSession=UINT64_MAX;uint64_t initDiagnostic994=smx_diag_begin994(SMX_DIAG_MODEL_INIT994,&initFields994);
 begin783();
 if(argc<2||argc>3){fprintf(stderr,"usage: graph_gpu.exe graph.bin [repeats]\n");return 1;}
 uint32_t repeats=argc==3?(uint32_t)strtoul(argv[2],NULL,10):3;if(!repeats||repeats>100000)return 1;
 FILE*f=fopen(argv[1],"rb");if(!f)return 1;char magic[8];read_exact(f,magic,8);if(memcmp(magic,"SMXG0001",8))return 1;
 uint32_t nb,nn,final,width,height;read_exact(f,&nb,4);read_exact(f,&nn,4);read_exact(f,&final,4);read_exact(f,&width,4);read_exact(f,&height,4);
 if(!nb||nb>128||!nn||nn>512||final>=nb||!width||!height||width>8192||height>8192)return 1;
 {
 const char*base994=argv[1];for(const char*k=argv[1];*k;k++)if(*k=='/'||*k=='\\')base994=k+1;char name994[128];size_t j994=0;for(;base994[j994]&&j994<sizeof(name994)-1;++j994){char c=base994[j994];name994[j994]=((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-'||c=='.')?c:'_';}name994[j994]=0;
 FILE*meta994=fopen("app-model994.json","wb");if(meta994){fprintf(meta994,"{\"graph_basename_sanitized\":\"%s\",\"graph_buffers\":%u,\"graph_nodes\":%u,\"graph_width\":%u,\"graph_height\":%u,\"native_build\":\"R994-diagnostics-v1\",\"shader_mapping\":\"app-node-map830.csv and existing model logs\",\"asset_hashes\":\"host package asset manifest; this record does not independently rehash assets\",\"power_and_frequency\":\"existing app-hud988.jsonl validity and domain fields; no new sampling\",\"scope\":\"default observes CPU API boundaries; no per-node trace or hardware occupancy inference\"}\n",name994,nb,nn,width,height);fclose(meta994);}
 }
 GraphBuffer bs[128]={0};GraphNode ns[512]={0};
 for(uint32_t i=0;i<nb;i++){
  read_exact(f,&bs[i].size,8);read_exact(f,bs[i].init,128);read_exact(f,bs[i].reference,128);read_exact(f,bs[i].name,32);
  if(!bs[i].size||bs[i].size>1073741824||bs[i].size%4||!memchr(bs[i].name,0,32))return 1;check_path(bs[i].init,128);check_path(bs[i].reference,128);
 }
 for(uint32_t i=0;i<nn;i++){
  GraphNode*n=&ns[i];read_exact(f,n->shader,64);check_path(n->shader,64);
  for(int j=0;j<3;j++){read_exact(f,&n->bi[j],4);read_exact(f,&n->off[j],8);read_exact(f,&n->range[j],8);if(n->bi[j]>=nb||n->off[j]>=bs[n->bi[j]].size||!n->range[j]||n->range[j]>bs[n->bi[j]].size-n->off[j])return 1;}
  read_exact(f,n->cfg,64);read_exact(f,n->dispatch,12);read_exact(f,n->name,32);if(!memchr(n->name,0,32))return 1;
  for(int j=0;j<3;j++)if(!n->dispatch[j]||n->dispatch[j]>65535)return 1;
 }if(fgetc(f)!=EOF)return 1;fclose(f);if(bs[final].size<(uint64_t)width*height*4)return 1;
 mark783("manifest_read");
 HMODULE lib=LoadLibraryExW(L"vulkan-1.dll",NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!lib)return 1;
 PFN_vkGetInstanceProcAddr gipa=(PFN_vkGetInstanceProcAddr)GetProcAddress(lib,"vkGetInstanceProcAddr");if(!gipa)return 1;
 PFN_vkCreateInstance vkCreateInstance=(PFN_vkCreateInstance)gipa(NULL,"vkCreateInstance");
 VkApplicationInfo app={VK_STRUCTURE_TYPE_APPLICATION_INFO};app.pApplicationName="Smooth Motion port: matrix backend validation";app.apiVersion=VK_API_VERSION_1_3;
 VkInstanceCreateInfo ici={VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};ici.pApplicationInfo=&app;const char* wsiExtensions958[]={VK_KHR_SURFACE_EXTENSION_NAME,VK_KHR_WIN32_SURFACE_EXTENSION_NAME};ici.enabledExtensionCount=2;ici.ppEnabledExtensionNames=wsiExtensions958;VkInstance instance;CHECK(vkCreateInstance(&ici,NULL,&instance));wsiInstance958=instance;wsiGet958=gipa;
 INST(PFN_vkDestroyInstance,vkDestroyInstance)
 INST(PFN_vkEnumeratePhysicalDevices,vkEnumeratePhysicalDevices)
 INST(PFN_vkGetPhysicalDeviceProperties,vkGetPhysicalDeviceProperties)
 INST(PFN_vkGetPhysicalDeviceFeatures2,vkGetPhysicalDeviceFeatures2)
 INST(PFN_vkGetPhysicalDeviceProperties2,vkGetPhysicalDeviceProperties2)
 INST(PFN_vkGetPhysicalDeviceFormatProperties2,vkGetPhysicalDeviceFormatProperties2)
 INST(PFN_vkGetPhysicalDeviceQueueFamilyProperties,vkGetPhysicalDeviceQueueFamilyProperties)
 INST(PFN_vkGetPhysicalDeviceMemoryProperties,vkGetPhysicalDeviceMemoryProperties)
 INST(PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR,vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)
 INST(PFN_vkEnumerateDeviceExtensionProperties,vkEnumerateDeviceExtensionProperties)
 INST(PFN_vkCreateDevice,vkCreateDevice)
 INST(PFN_vkGetDeviceProcAddr,vkGetDeviceProcAddr)
 uint32_t nd=0;CHECK(vkEnumeratePhysicalDevices(instance,&nd,NULL));VkPhysicalDevice* ds=calloc(nd,sizeof(*ds));CHECK(vkEnumeratePhysicalDevices(instance,&nd,ds));VkPhysicalDevice phys=NULL;VkPhysicalDeviceProperties props;
 SmxDeviceCaps995 selectedCaps995={0};
 for(uint32_t i=0;i<nd;i++){VkPhysicalDeviceProperties candidate;vkGetPhysicalDeviceProperties(ds[i],&candidate);SmxDeviceCaps995 caps;char why[512];int ok=smx_probe_device995(gipa,instance,ds[i],&caps,why,sizeof(why));fprintf(stderr,"ADAPTER995 index=%u name=%s vendor=%04x device=%04x eligible=%d reason=%s\n",i,candidate.deviceName,candidate.vendorID,candidate.deviceID,ok,why);if(ok&&!phys){phys=ds[i];props=candidate;selectedCaps995=caps;}}
 free(ds);if(!phys){fprintf(stderr,"No Intel adapter satisfies this shader package and Win32 interoperability requirements; see ADAPTER995 rejection reasons. Device names are not an allowlist.\n");vkDestroyInstance(instance,NULL);FreeLibrary(lib);return 3;}
 AcquireSRWLockExclusive(&selectedLock995);memset(&selected995,0,sizeof(selected995));selected995.vendor=props.vendorID;selected995.device=props.deviceID;memcpy(selected995.luid,selectedCaps995.luid,8);memcpy(selected995.name,props.deviceName,sizeof(selected995.name)-1);selected995.valid=1;ReleaseSRWLockExclusive(&selectedLock995);
 extentImageLimit989=props.limits.maxImageDimension2D;extentDispatchLimit989=props.limits.maxComputeWorkGroupCount[0];
 const uint32_t nativeW989=interopContext.width?interopContext.width:width,nativeH989=interopContext.height?interopContext.height:height;
 SmxResolutionBudget989 nativeCapacity989;char capacityWhy989[384];
 if(!smx_extent_check989(nativeW989,nativeH989,extentImageLimit989,extentDispatchLimit989,&nativeCapacity989,capacityWhy989,sizeof(capacityWhy989))){fprintf(stderr,"CAPACITY989 Vulkan extent rejected: %s\n",capacityWhy989);vkDestroyInstance(instance,NULL);wsiInstance958=VK_NULL_HANDLE;wsiGet958=NULL;FreeLibrary(lib);return 3;}
 uint64_t modelManifestBytes989=0;for(uint32_t n989=0;n989<nb;n989++){if(bs[n989].size>UINT64_MAX-modelManifestBytes989){vkDestroyInstance(instance,NULL);wsiInstance958=VK_NULL_HANDLE;wsiGet958=NULL;FreeLibrary(lib);return 3;}modelManifestBytes989+=bs[n989].size;}
 VkPhysicalDeviceIDProperties capacityId989={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};VkPhysicalDeviceProperties2 capacityProps989={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};capacityProps989.pNext=&capacityId989;vkGetPhysicalDeviceProperties2(phys,&capacityProps989);
 if(!capacityId989.deviceLUIDValid){fprintf(stderr,"CAPACITY989 Vulkan adapter has no valid DXGI LUID; no memory budget evidence\n");vkDestroyInstance(instance,NULL);wsiInstance958=VK_NULL_HANDLE;wsiGet958=NULL;FreeLibrary(lib);return 3;}
 const char*factorText989=getenv("SMX_FRAME_MULTIPLIER989");unsigned factor989=factorText989?(unsigned)strtoul(factorText989,NULL,10):2u;
 if(!smx_preflight_model_budget989(capacityId989.deviceLUID,nativeW989,nativeH989,factor989,modelManifestBytes989,&nativeCapacity989,capacityWhy989,sizeof(capacityWhy989))){fprintf(stderr,"CAPACITY989 cold model allocation rejected: %s\n",capacityWhy989);vkDestroyInstance(instance,NULL);wsiInstance958=VK_NULL_HANDLE;wsiGet958=NULL;FreeLibrary(lib);return 3;}
 fprintf(stderr,"CAPACITY989 stage=before_model_allocation width=%u height=%u multiplier=%u vk_max_image_2d=%u vk_max_dispatch_x=%u native_dispatch_x=%u texture_count_estimate=%llu texture_bytes_estimate=%llu model_manifest_bytes=%llu model_reserve_bytes=%llu budget=%llu current_usage=%llu available=%llu safety_headroom=%llu estimate_only=1\n",nativeW989,nativeH989,factor989,extentImageLimit989,extentDispatchLimit989,nativeCapacity989.dispatchX,(unsigned long long)nativeCapacity989.textureCount,(unsigned long long)nativeCapacity989.textureBytes,(unsigned long long)modelManifestBytes989,(unsigned long long)nativeCapacity989.modelBytes,(unsigned long long)nativeCapacity989.budgetBytes,(unsigned long long)nativeCapacity989.currentUsageBytes,(unsigned long long)nativeCapacity989.availableBytes,(unsigned long long)nativeCapacity989.headroomBytes);
 uint32_t extCount=0;CHECK(vkEnumerateDeviceExtensionProperties(phys,NULL,&extCount,NULL));VkExtensionProperties* exts=calloc(extCount,sizeof(*exts));CHECK(vkEnumerateDeviceExtensionProperties(phys,NULL,&extCount,exts));int hasFaultExt994=0,hasFaultKhr994=0;for(uint32_t e=0;e<extCount;e++){if(!strcmp(exts[e].extensionName,"VK_EXT_device_fault"))hasFaultExt994=1;if(!strcmp(exts[e].extensionName,"VK_KHR_device_fault"))hasFaultKhr994=1;}int hasCal737=0;for(uint32_t e=0;e<extCount;e++)if(!strcmp(exts[e].extensionName,VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME))hasCal737=1;int hasGlobal=0;for(uint32_t e=0;e<extCount;e++)if(!strcmp(exts[e].extensionName,VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME))hasGlobal=1;int hasExe=0;for(uint32_t i=0;i<extCount;i++)if(!strcmp(exts[i].extensionName,VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME))hasExe=1;free(exts);hasExe=hasExe&&getenv("SMX_COLLECT_STATISTICS")!=NULL;
 VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pe={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR};
 VkPhysicalDeviceShaderFloat8FeaturesEXT f8={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT8_FEATURES_EXT};
 VkPhysicalDeviceCooperativeMatrixFeaturesKHR cm={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR};cm.pNext=&f8;if(hasExe)f8.pNext=&pe;
 VkPhysicalDeviceVulkan13Features f13={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};f13.pNext=&cm;
 VkPhysicalDeviceVulkan12Features f12={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};f12.pNext=&f13;
 VkPhysicalDeviceVulkan11Features f11={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};f11.pNext=&f12;
 VkPhysicalDeviceFeatures2 ft={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};ft.pNext=&f11;vkGetPhysicalDeviceFeatures2(phys,&ft);
 /* Exact per-format capability is distinct from enabling the device feature. */
 VkFormatProperties3 bgra3_969={VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3};
 VkFormatProperties2 bgra2_969={VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};bgra2_969.pNext=&bgra3_969;
 vkGetPhysicalDeviceFormatProperties2(phys,VK_FORMAT_B8G8R8A8_UNORM,&bgra2_969);
 const VkFormatFeatureFlags2 requiredBgra969=VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT|VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
 const int bgraWrite969=ft.features.shaderStorageImageWriteWithoutFormat&&((bgra3_969.optimalTilingFeatures&requiredBgra969)==requiredBgra969);
 fprintf(stderr,"NATIVE969_FORMAT format=VK_FORMAT_B8G8R8A8_UNORM core_shaderStorageImageWriteWithoutFormat=%u optimal_features2=0x%llx required=0x%llx storage_image=%u storage_write_without_format=%u pass=%u\n",ft.features.shaderStorageImageWriteWithoutFormat,(unsigned long long)bgra3_969.optimalTilingFeatures,(unsigned long long)requiredBgra969,!!(bgra3_969.optimalTilingFeatures&VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT),!!(bgra3_969.optimalTilingFeatures&VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT),bgraWrite969);
 if(!bgraWrite969){fprintf(stderr,"NATIVE969_FATAL BGRA storage write without format unsupported; stop before device/pipeline creation, no fallback\n");return 3;}
 const char*flavor969=getenv("SMX_WARP_FLAVOR");
 if(!getenv("SMX_NATIVE_DOWNSCALE")||!flavor969||strcmp(flavor969,"native-flow")||getenv("SMX_EXP_WARP860")||getenv("SMX_AB_WARP860")){
  fprintf(stderr,"NATIVE969_FATAL this candidate requires runtime-native downscale and native-flow warp; unverified alternate warp is disabled\n");return 3;
 }

 fprintf(stderr,"FP8 features: scalar=%u matrix=%u\n",f8.shaderFloat8,f8.shaderFloat8CooperativeMatrix);if(!f8.shaderFloat8||!f12.shaderInt8)return 3;f8.shaderFloat8CooperativeMatrix=VK_FALSE;
 if(!ft.features.shaderInt64||!f12.vulkanMemoryModel||!f13.maintenance4||!cm.cooperativeMatrix||!f12.shaderFloat16||!f11.storageBuffer16BitAccess||!f13.subgroupSizeControl||!f13.computeFullSubgroups){fprintf(stderr,"Required matrix features absent\n");return 3;}
 VkPhysicalDeviceSubgroupSizeControlProperties sg={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES};
 VkPhysicalDeviceProperties2 p2={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};p2.pNext=&sg;vkGetPhysicalDeviceProperties2(phys,&p2);
 if(sg.minSubgroupSize>16||sg.maxSubgroupSize<32||!(sg.requiredSubgroupSizeStages&VK_SHADER_STAGE_COMPUTE_BIT)){fprintf(stderr,"Required subgroup16/32 unavailable\n");return 3;}
 uint32_t nc=0;CHECK(vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(phys,&nc,NULL));VkCooperativeMatrixPropertiesKHR* cp=calloc(nc,sizeof(*cp));for(uint32_t i=0;i<nc;i++)cp[i].sType=VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR;CHECK(vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(phys,&nc,cp));int shape=0;for(uint32_t i=0;i<nc;i++)if(cp[i].MSize==8&&cp[i].NSize==16&&cp[i].KSize==16&&cp[i].AType==VK_COMPONENT_TYPE_FLOAT16_KHR&&cp[i].BType==VK_COMPONENT_TYPE_FLOAT16_KHR&&cp[i].CType==VK_COMPONENT_TYPE_FLOAT16_KHR&&cp[i].ResultType==VK_COMPONENT_TYPE_FLOAT16_KHR&&cp[i].scope==VK_SCOPE_SUBGROUP_KHR&&!cp[i].saturatingAccumulation)shape=1;free(cp);if(!shape){fprintf(stderr,"FP16/FP16 8x16x16 shape absent\n");return 3;}
 uint32_t nq=0;vkGetPhysicalDeviceQueueFamilyProperties(phys,&nq,NULL);VkQueueFamilyProperties* qp=calloc(nq,sizeof(*qp));vkGetPhysicalDeviceQueueFamilyProperties(phys,&nq,qp);uint32_t qi=UINT32_MAX;
 if(nq>1024){fprintf(stderr,"Queue family count exceeds bounded capability storage\n");return 3;}uint32_t qFlags995[1024],qCounts995[1024],qTime995[1024];for(uint32_t i=0;i<nq;i++){qFlags995[i]=qp[i].queueFlags;qCounts995[i]=qp[i].queueCount;qTime995[i]=qp[i].timestampValidBits;fprintf(stderr,"QUEUE family=%u count=%u flags=%u timestamp_bits=%u\n",i,qCounts995[i],qFlags995[i],qTime995[i]);}int picked995=smx_choose_queue995(qFlags995,qCounts995,qTime995,nq);if(picked995>=0)qi=(uint32_t)picked995;
 const char* requested=getenv("SMX_QUEUE_FAMILY");if(requested){uint32_t pick=(uint32_t)atoi(requested);if(pick>=nq||!qp[pick].queueCount||!(qp[pick].queueFlags&VK_QUEUE_COMPUTE_BIT)||!qp[pick].timestampValidBits)return 3;qi=pick;}
 uint32_t queueIndex=getenv("SMX_QUEUE_INDEX")?(uint32_t)atoi(getenv("SMX_QUEUE_INDEX")):0;if(qi==UINT32_MAX||queueIndex>=qp[qi].queueCount)return 3;wsiFamily995=UINT32_MAX;
 for(uint32_t q=0;q<nq;q++)if(q!=qi&&qp[q].queueCount&&(qp[q].queueFlags&VK_QUEUE_TRANSFER_BIT)&&!(qp[q].queueFlags&(VK_QUEUE_GRAPHICS_BIT|VK_QUEUE_COMPUTE_BIT))&&qp[q].timestampValidBits){wsiFamily995=q;break;}
 if(wsiFamily995==UINT32_MAX)for(uint32_t q=0;q<nq;q++)if(q!=qi&&qp[q].queueCount&&(qp[q].queueFlags&(VK_QUEUE_TRANSFER_BIT|VK_QUEUE_GRAPHICS_BIT|VK_QUEUE_COMPUTE_BIT))&&qp[q].timestampValidBits){wsiFamily995=q;break;}
 fprintf(stderr,"PRESENT995 separate_wsi_family=%u available=%u; DComp does not require an extra Vulkan queue\n",wsiFamily995,wsiFamily995!=UINT32_MAX);free(qp);
 fprintf(stderr,"QUEUE selected=%u shared_memory=%u\n",qi,props.limits.maxComputeSharedMemorySize);

 memset(&f11,0,sizeof(f11));f11.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;f11.pNext=&f12;f11.storageBuffer16BitAccess=VK_TRUE;
 if(!f12.timelineSemaphore){fprintf(stderr,"R948 timeline semaphores unsupported\n");return 3;}
 memset(&f12,0,sizeof(f12));f12.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;f12.pNext=&f13;f12.timelineSemaphore=VK_TRUE;f12.shaderFloat16=VK_TRUE;f12.shaderInt8=VK_TRUE;f12.vulkanMemoryModel=VK_TRUE;
 memset(&f13,0,sizeof(f13));f13.sType=VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;f13.pNext=&cm;f13.subgroupSizeControl=VK_TRUE;f13.computeFullSubgroups=VK_TRUE;f13.maintenance4=VK_TRUE;
 cm.cooperativeMatrix=VK_TRUE;cm.cooperativeMatrixRobustBufferAccess=VK_FALSE;
 float priorities[4]={1,1,1,1};if(queueIndex>=4)return 3;VkDeviceQueueCreateInfo qci={VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};qci.queueFamilyIndex=qi;qci.queueCount=queueIndex+1;qci.pQueuePriorities=priorities;
 VkDeviceQueueGlobalPriorityCreateInfoEXT priority={VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT};
 const char* requestedPriority=getenv("SMX_QUEUE_PRIORITY");
 if(requestedPriority){
  
  if(!strcmp(requestedPriority,"low"))priority.globalPriority=VK_QUEUE_GLOBAL_PRIORITY_LOW_EXT;
  else if(!strcmp(requestedPriority,"medium"))priority.globalPriority=VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT;
  else if(!strcmp(requestedPriority,"high"))priority.globalPriority=VK_QUEUE_GLOBAL_PRIORITY_HIGH_EXT;
  else {fprintf(stderr,"Invalid queue priority; realtime prohibited\n");return 3;}
  if(hasGlobal)qci.pNext=&priority;else fprintf(stderr,"QUEUE995 global priority extension absent; using default ordinary priority\n");fprintf(stderr,"QUEUE requested_priority=%s value=%d; only this device queue\n",requestedPriority,priority.globalPriority);
 }
 const char* ext[9]={VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME,VK_EXT_SHADER_FLOAT8_EXTENSION_NAME,VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,VK_KHR_WIN32_KEYED_MUTEX_EXTENSION_NAME,VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME};uint32_t enabledCount=5;ext[enabledCount++]=VK_KHR_SWAPCHAIN_EXTENSION_NAME;wsiPhysical958=phys;
 hasExe=hasExe&&pe.pipelineExecutableInfo;if(hasExe)ext[enabledCount++]=VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME;
 if(requestedPriority&&hasGlobal)ext[enabledCount++]=VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME;if(getenv("SMX_CALIBRATED_TIMING")||getenv("SMX_PROBE846")){if(!hasCal737){fprintf(stderr,"CAL737 unsupported device extension\n");return 3;}PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT domains737=(PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)gipa(instance,"vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");if(!domains737)return 3;uint32_t nc737=0;CHECK(domains737(phys,&nc737,NULL));if(nc737>16)return 3;VkTimeDomainEXT ds737[16];CHECK(domains737(phys,&nc737,ds737));int device737=0,host737=0;for(uint32_t i=0;i<nc737;i++){device737|=ds737[i]==VK_TIME_DOMAIN_DEVICE_EXT;host737|=ds737[i]==VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT;}fprintf(stderr,"CAL737 supported_DEVICE=%d supported_QPC=%d\n",device737,host737);if(!device737||!host737)return 3;ext[enabledCount++]=VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME;}
 VkDeviceQueueCreateInfo queues961[2]={qci,{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO}};float copyPriority961=.5f;queues961[1].queueFamilyIndex=wsiFamily995;queues961[1].queueCount=1;queues961[1].pQueuePriorities=&copyPriority961;
 VkPhysicalDeviceFeatures core={0};core.shaderInt64=VK_TRUE;core.shaderStorageImageWriteWithoutFormat=VK_TRUE;VkDeviceCreateInfo dci={VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};dci.pEnabledFeatures=&core;dci.pNext=&f11;dci.queueCreateInfoCount=wsiFamily995==UINT32_MAX?1:2;dci.pQueueCreateInfos=queues961;dci.enabledExtensionCount=enabledCount;dci.ppEnabledExtensionNames=ext;VkResult deviceResult995=vkCreateDevice(phys,&dci,NULL,&device);if(smx_priority_retry995(deviceResult995,queues961[0].pNext!=NULL)){fprintf(stderr,"QUEUE995 requested priority denied VkResult=%d; retry ordinary priority for this device only\n",deviceResult995);queues961[0].pNext=NULL;deviceResult995=vkCreateDevice(phys,&dci,NULL,&device);}CHECK(deviceResult995);
 {FILE*md=fopen("app-device994.json","wb");if(md){fprintf(md,"{\"api\":\"Vulkan\",\"vendor_id\":%u,\"device_id\":%u,\"api_version_raw\":%u,\"driver_version_raw\":%u,\"vk_ext_device_fault_enumerated\":%s,\"vk_khr_device_fault_enumerated\":%s,\"device_fault_enabled\":false,\"device_fault_query\":\"unavailable: extension feature not enabled by existing device configuration\",\"d3d_api\":\"D3D11\",\"dred\":\"unavailable: D3D11 path\",\"removed_reason_query\":\"not added: existing bounded fail-stop must not block on diagnostic driver calls\",\"per_node_trace\":false,\"extra_gpu_synchronization\":false}\n",props.vendorID,props.deviceID,props.apiVersion,props.driverVersion,hasFaultExt994?"true":"false",hasFaultKhr994?"true":"false");fclose(md);}}

#define LOAD(N) N=(PFN_##N)vkGetDeviceProcAddr(device,#N);if(!N){fprintf(stderr,"Missing %s\n",#N);return 2;}
 DEVICE_FUNCS(LOAD)
 VkQueue queue;vkGetDeviceQueue(device,qi,queueIndex,&queue);wsiQueue961=VK_NULL_HANDLE;if(wsiFamily995!=UINT32_MAX)vkGetDeviceQueue(device,wsiFamily995,0,&wsiQueue961);
 {FILE*qm=fopen("app-vulkan-queues994.json","wb");if(qm){fprintf(qm,"{\"device\":%llu,\"compute_queue\":%llu,\"compute_family\":%u,\"compute_index\":%u,\"wsi_copy_queue\":%llu,\"wsi_family\":%u,\"wsi_index\":0,\"driver_version_raw\":%u,\"api_version_raw\":%u,\"hardware_counters_added\":false,\"timestamp_interpretation\":\"GPU elapsed spans may include preemption or memory waits; not XMX occupancy or power attribution\"}\n",(unsigned long long)(uintptr_t)device,(unsigned long long)(uintptr_t)queue,qi,queueIndex,(unsigned long long)(uintptr_t)wsiQueue961,wsiFamily995,props.driverVersion,props.apiVersion);fclose(qm);}}
vkGetPhysicalDeviceMemoryProperties(phys,&memory);for(uint32_t mi=0;mi<memory.memoryTypeCount;mi++)fprintf(stderr,"MEMORY type=%u flags=%u heap=%u\n",mi,memory.memoryTypes[mi].propertyFlags,memory.memoryTypes[mi].heapIndex);
 VkDescriptorSetLayoutBinding bindings[3]={0};for(uint32_t i=0;i<3;i++){bindings[i].binding=i;bindings[i].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;bindings[i].descriptorCount=1;bindings[i].stageFlags=VK_SHADER_STAGE_COMPUTE_BIT;}
 VkDescriptorSetLayoutCreateInfo dlci={VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};dlci.bindingCount=3;dlci.pBindings=bindings;VkDescriptorSetLayout layout;CHECK(vkCreateDescriptorSetLayout(device,&dlci,NULL,&layout));
 VkPushConstantRange push={VK_SHADER_STAGE_COMPUTE_BIT,0,64};VkPipelineLayoutCreateInfo plci={VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};plci.setLayoutCount=1;plci.pSetLayouts=&layout;plci.pushConstantRangeCount=1;plci.pPushConstantRanges=&push;VkPipelineLayout pl;CHECK(vkCreatePipelineLayout(device,&plci,NULL,&pl));


 mark783("device_and_layouts");
 size_t cacheBytes=0;void*cacheData=cacheLoad783(&props,&cacheBytes);
 VkPipelineCacheCreateInfo cacheInfo={VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};cacheInfo.initialDataSize=cacheBytes;cacheInfo.pInitialData=cacheBytes?cacheData:NULL;VkPipelineCache cache;CHECK(vkCreatePipelineCache(device,&cacheInfo,NULL,&cache));free(cacheData);mark783("cache_load");
 VkDescriptorPoolSize ps={VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,nn*3};VkDescriptorPoolCreateInfo dpci={VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};dpci.maxSets=nn;dpci.poolSizeCount=1;dpci.pPoolSizes=&ps;VkDescriptorPool pool;CHECK(vkCreateDescriptorPool(device,&dpci,NULL,&pool));
 GraphPipeline pipelines[192]={0};uint32_t np=0;
 Buffer uploads467[128]={0};int private467=getenv("SMX_PRIVATE_GRAPH")!=NULL;uint64_t total467=0;for(uint32_t i=0;i<nb;i++)total467+=bs[i].size;
 if(private467&&total467>268435456ull){fprintf(stderr,"PRIVATE467 graph exceeds 256MiB experiment bound\n");return 3;}
 for(uint32_t i=0;i<nb;i++){
  int usePrivate=private467&&i!=final&&strcmp(bs[i].name,"frames")&&(!strcmp(bs[i].reference,"-"));
  if(usePrivate){bs[i].gpu=make_private467((size_t)bs[i].size);uploads467[i]=make_buffer((size_t)bs[i].size);}else bs[i].gpu=make_buffer((size_t)bs[i].size);
  void*mapped=usePrivate?uploads467[i].mapped:bs[i].gpu.mapped;memset(mapped,0,(size_t)bs[i].size);
  if(strcmp(bs[i].init,"-")){void*data;size_t n=read_file(bs[i].init,&data);if(n>bs[i].size){fprintf(stderr,"Oversized buffer initializer\n");return 1;}memcpy(mapped,data,n);free(data);}
 }
 upload_private467(bs,uploads467,nb,queue,qi);mark783("buffer_allocate_upload");

 for(uint32_t i=0;i<nn;i++){
  GraphNode*n=&ns[i];uint32_t pi=0;while(pi<np&&strcmp(pipelines[pi].path,n->shader))pi++;
  if(pi==np){
   if(np==192){fprintf(stderr,"Pipeline capacity exceeded\n");return 1;};strcpy(pipelines[np].path,n->shader);void*code;size_t len=read_file(n->shader,&code);if(len%4)return 1;
   fprintf(stderr,"Preparing pipeline %s\n",n->shader);
   VkShaderModuleCreateInfo sci={VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};pipelines[np].readonlyMask=spirv_readonly(code,len);sci.codeSize=len;sci.pCode=code;CHECK(vkCreateShaderModule(device,&sci,NULL,&pipelines[np].module));free(code);
   VkPipelineShaderStageRequiredSubgroupSizeCreateInfo sgi={VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO};sgi.requiredSubgroupSize=strstr(n->shader,"sg32")?32:16;
   VkComputePipelineCreateInfo pci={VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};pci.stage.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;pci.stage.pNext=&sgi;pci.stage.flags=VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;pci.stage.stage=VK_SHADER_STAGE_COMPUTE_BIT;pci.stage.module=pipelines[np].module;pci.stage.pName="main";pci.layout=pl;
   
   if(hasExe)pci.flags|=VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR|VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
   if(getenv("SMX_DISABLE_PIPELINE_OPT"))pci.flags|=VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
   VkPipelineCreationFeedback fb783={0},stage783={0};VkPipelineCreationFeedbackCreateInfo fbi783={VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO};fbi783.pPipelineCreationFeedback=&fb783;fbi783.pipelineStageCreationFeedbackCount=1;fbi783.pPipelineStageCreationFeedbacks=&stage783;pci.pNext=&fbi783;
   double pipeStart783=clock783();CHECK(vkCreateComputePipelines(device,cache,1,&pci,NULL,&pipelines[np].pipeline));double pipeEnd783=clock783();sample783("pipeline",n->shader,pipeEnd783,pipeEnd783-pipeStart783,!!(fb783.flags&VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT),!!(fb783.flags&VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT));np++;
   if(hasExe){
    PFN_vkGetPipelineExecutablePropertiesKHR getProps=(PFN_vkGetPipelineExecutablePropertiesKHR)vkGetDeviceProcAddr(device,"vkGetPipelineExecutablePropertiesKHR");
    PFN_vkGetPipelineExecutableStatisticsKHR getStats=(PFN_vkGetPipelineExecutableStatisticsKHR)vkGetDeviceProcAddr(device,"vkGetPipelineExecutableStatisticsKHR");
    PFN_vkGetPipelineExecutableInternalRepresentationsKHR getIR=(PFN_vkGetPipelineExecutableInternalRepresentationsKHR)vkGetDeviceProcAddr(device,"vkGetPipelineExecutableInternalRepresentationsKHR");
    VkPipelineInfoKHR info={VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR};info.pipeline=pipelines[np-1].pipeline;
    uint32_t count=0;if(getProps&&getStats&&getProps(device,&info,&count,NULL)==VK_SUCCESS&&count<32){
     VkPipelineExecutablePropertiesKHR ep[32]={0};for(uint32_t k=0;k<count;k++)ep[k].sType=VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR;
     CHECK(getProps(device,&info,&count,ep));
     for(uint32_t k=0;k<count;k++){
      VkPipelineExecutableInfoKHR ei={VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR};ei.pipeline=info.pipeline;ei.executableIndex=k;
      fprintf(stderr,"EXEC %s %s subgroup=%u\n",n->shader,ep[k].name,ep[k].subgroupSize);
      uint32_t sc=0;if(getStats(device,&ei,&sc,NULL)==VK_SUCCESS&&sc<128){VkPipelineExecutableStatisticKHR st[128]={0};for(uint32_t z=0;z<sc;z++)st[z].sType=VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR;CHECK(getStats(device,&ei,&sc,st));for(uint32_t z=0;z<sc;z++){
       fprintf(stderr,"STAT %s %s=",n->shader,st[z].name);
       if(st[z].format==VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR)fprintf(stderr,"%.9g\n",st[z].value.f64);
       else fprintf(stderr,"%llu\n",(unsigned long long)st[z].value.u64);
      }}
      uint32_t ic=0;if(getIR&&getIR(device,&ei,&ic,NULL)==VK_SUCCESS&&ic<16){VkPipelineExecutableInternalRepresentationKHR ir[16]={0};for(uint32_t z=0;z<ic;z++)ir[z].sType=VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR;
       if(getIR(device,&ei,&ic,ir)==VK_SUCCESS){for(uint32_t z=0;z<ic;z++)if(ir[z].dataSize&&ir[z].dataSize<16777216)ir[z].pData=malloc(ir[z].dataSize);
        if(getIR(device,&ei,&ic,ir)==VK_SUCCESS)for(uint32_t z=0;z<ic;z++)if(ir[z].pData){char path[256];const char*base=strrchr(n->shader,'/');base=base?base+1:n->shader;snprintf(path,sizeof(path),"pipeline-%s-%u-%u.txt",base,k,z);FILE*out=fopen(path,"wb");if(out){fwrite(ir[z].pData,1,ir[z].dataSize,out);fclose(out);}fprintf(stderr,"IR %s %s %zu bytes\n",n->shader,ir[z].name,ir[z].dataSize);}
        for(uint32_t z=0;z<ic;z++)free(ir[z].pData);
       }
      }
     }
    }
   }


  }n->pipeline=pi;
  int sharedSet=0;
  for(uint32_t prior=0;prior<i;prior++)if(!memcmp(n->bi,ns[prior].bi,sizeof(n->bi))&&!memcmp(n->off,ns[prior].off,sizeof(n->off))&&!memcmp(n->range,ns[prior].range,sizeof(n->range))){n->set=ns[prior].set;sharedSet=1;break;}
  if(sharedSet)continue;
  VkDescriptorSetAllocateInfo dai={VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};dai.descriptorPool=pool;dai.descriptorSetCount=1;dai.pSetLayouts=&layout;CHECK(vkAllocateDescriptorSets(device,&dai,&n->set));
  VkDescriptorBufferInfo bis[3];VkWriteDescriptorSet writes[3]={0};
  for(uint32_t j=0;j<3;j++){
   if(n->off[j]%props.limits.minStorageBufferOffsetAlignment){fprintf(stderr,"Misaligned binding\n");return 1;}
   bis[j].buffer=bs[n->bi[j]].gpu.buffer;bis[j].offset=n->off[j];bis[j].range=n->range[j];
   writes[j].sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;writes[j].dstSet=n->set;writes[j].dstBinding=j;writes[j].descriptorCount=1;writes[j].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;writes[j].pBufferInfo=&bis[j];
  }vkUpdateDescriptorSets(device,3,writes,0,NULL);
 }
 mark783("pipeline_modules_descriptors");
 {
 uint32_t initCount=0;for(uint32_t i=0;i<nn;i++)if(!strcmp(ns[i].name,"cache-attention-weights"))initCount++;
 if(initCount){
 VkCommandPoolCreateInfo ip={VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};ip.queueFamilyIndex=qi;VkCommandPool poolInit;CHECK(vkCreateCommandPool(device,&ip,NULL,&poolInit));
 VkCommandBufferAllocateInfo ia={VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ia.commandPool=poolInit;ia.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ia.commandBufferCount=1;VkCommandBuffer ic;CHECK(vkAllocateCommandBuffers(device,&ia,&ic));
 VkCommandBufferBeginInfo ib={VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};CHECK(vkBeginCommandBuffer(ic,&ib));
 VkMemoryBarrier im={VK_STRUCTURE_TYPE_MEMORY_BARRIER};im.srcAccessMask=VK_ACCESS_HOST_WRITE_BIT;im.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;vkCmdPipelineBarrier(ic,VK_PIPELINE_STAGE_HOST_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&im,0,NULL,0,NULL);
 for(uint32_t i=0;i<nn;i++){GraphNode*n=&ns[i];if(strcmp(n->name,"cache-attention-weights"))continue;
 vkCmdBindPipeline(ic,VK_PIPELINE_BIND_POINT_COMPUTE,pipelines[n->pipeline].pipeline);vkCmdBindDescriptorSets(ic,VK_PIPELINE_BIND_POINT_COMPUTE,pl,0,1,&n->set,0,NULL);vkCmdPushConstants(ic,pl,VK_SHADER_STAGE_COMPUTE_BIT,0,64,n->cfg);vkCmdDispatch(ic,n->dispatch[0],n->dispatch[1],n->dispatch[2]);im.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;im.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;vkCmdPipelineBarrier(ic,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&im,0,NULL,0,NULL);}
 im.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;im.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;vkCmdPipelineBarrier(ic,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&im,0,NULL,0,NULL);CHECK(vkEndCommandBuffer(ic));
 VkFenceCreateInfo fi={VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};VkFence ff;CHECK(vkCreateFence(device,&fi,NULL,&ff));VkSubmitInfo si={VK_STRUCTURE_TYPE_SUBMIT_INFO};si.commandBufferCount=1;si.pCommandBuffers=&ic;CHECK(vkQueueSubmit(queue,1,&si,ff));CHECK(vkWaitForFences(device,1,&ff,VK_TRUE,5000000000ull));vkDestroyFence(device,ff,NULL);vkDestroyCommandPool(device,poolInit,NULL);
 uint32_t kept=0;for(uint32_t i=0;i<nn;i++)if(strcmp(ns[i].name,"cache-attention-weights"))ns[kept++]=ns[i];nn=kept;
 fprintf(stderr,"Prepared %u static attention blocks once; %u live dispatches\n",initCount,nn);
 }
 }

 mark783("static_attention_precompute");cacheSave783(cache,&props);mark783("cache_save");
 uint32_t removed=0;for(uint32_t i=1;i<nn;i++)removed+=independent(&ns[i-1],&ns[i]);fprintf(stderr,"BARRIERS omitted=%u queue_index=%u\n",removed,queueIndex);
 uint32_t resourceSets=0,resourceBinds=0;for(uint32_t i=0;i<nn;i++){int seen=0;for(uint32_t j=0;j<i;j++)if(ns[j].set==ns[i].set){seen=1;break;}resourceSets+=!seen;resourceBinds+=i==0||ns[i-1].set!=ns[i].set;}
 fprintf(stderr,"RESOURCE SETS unique=%u nodes=%u adjacent_binds=%u; R86 identical storage bindings reused\n",resourceSets,nn,resourceBinds);
 int coarse851=getenv("SMX_BLOCK851")!=NULL;int fineTiming=getenv("SMX_FINE_TIMING")!=NULL;if(coarse851&&(fineTiming||nn!=300)){fprintf(stderr,"Block profiling requires the 300-node current graph and aggregate mode\n");return 71;}uint32_t queryCount=coarse851?14:fineTiming?nn*2:2;
 FILE*actual653=fopen("app-actual-config653.json","wb");if(actual653){fprintf(actual653,"{\"query_count\":%u,\"fine_timing\":%s,\"private_graph\":%s,\"pipeline_statistics\":%s,\"dependency_barriers\":%s,\"snapshot_inputs\":%s}\n",queryCount,fineTiming?"true":"false",private467?"true":"false",hasExe?"true":"false",(getenv("SMX_DEPENDENCY_BARRIERS")&&!getenv("SMX_STRICT_BARRIERS"))?"true":"false",getenv("SMX_SNAPSHOT_INPUTS")&&strcmp(getenv("SMX_SNAPSHOT_INPUTS"),"1")==0?"true":"false");fclose(actual653);}

 VkQueryPoolCreateInfo qpi={VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};qpi.queryType=VK_QUERY_TYPE_TIMESTAMP;qpi.queryCount=queryCount;VkQueryPool queries;CHECK(vkCreateQueryPool(device,&qpi,NULL,&queries));
 VkCommandPoolCreateInfo cpi={VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};cpi.queueFamilyIndex=qi;VkCommandPool cpool;CHECK(vkCreateCommandPool(device,&cpi,NULL,&cpool));
 VkCommandBufferAllocateInfo cai={VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};cai.commandPool=cpool;cai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;cai.commandBufferCount=1;VkCommandBuffer cmd;CHECK(vkAllocateCommandBuffers(device,&cai,&cmd));
 VkCommandBufferBeginInfo begin={VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};CHECK(vkBeginCommandBuffer(cmd,&begin));vkCmdResetQueryPool(cmd,queries,0,queryCount);
 VkMemoryBarrier mb={VK_STRUCTURE_TYPE_MEMORY_BARRIER};mb.srcAccessMask=VK_ACCESS_HOST_WRITE_BIT;mb.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_HOST_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&mb,0,NULL,0,NULL);
 Dependency395 dep395={0};int useDependency395=getenv("SMX_DEPENDENCY_BARRIERS")&&!getenv("SMX_STRICT_BARRIERS");
 for(uint32_t i=0;i<nn;i++){
  GraphNode*n=&ns[i];
  if(useDependency395)dependency395(cmd,&dep395,n,&pipelines[n->pipeline],bs);
  else if(i&&!independent(&ns[i-1],n)){mb.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;mb.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&mb,0,NULL,0,NULL);}
  if(fineTiming||i==0)vkCmdWriteTimestamp(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,queries,fineTiming?i*2:0);
  vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pipelines[n->pipeline].pipeline);if(i==0||ns[i-1].set!=n->set)vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pl,0,1,&n->set,0,NULL);vkCmdPushConstants(cmd,pl,VK_SHADER_STAGE_COMPUTE_BIT,0,64,n->cfg);vkCmdDispatch(cmd,n->dispatch[0],n->dispatch[1],n->dispatch[2]);
  if(coarse851){int e=phaseEnd851(i);if(e>=0)vkCmdWriteTimestamp(cmd,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries,e+1);}else if(fineTiming||i+1==nn)vkCmdWriteTimestamp(cmd,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries,fineTiming?i*2+1:1);
 }
 mb.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;mb.dstAccessMask=VK_ACCESS_HOST_READ_BIT;vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_HOST_BIT,0,1,&mb,0,NULL,0,NULL);fprintf(stderr,"DEPENDENCY837 enabled=%d calls=%u buffer_dependencies=%u proven_skips=%u\n",useDependency395,dep395.calls,dep395.dependencies,dep395.skipped837);CHECK(vkEndCommandBuffer(cmd));
 VkFenceCreateInfo fci={VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};VkFence fence;CHECK(vkCreateFence(device,&fci,NULL,&fence));
 LiveGraph live={queue,cmd,fence,queries,queryCount/2,props.limits.timestampPeriod};int frameIndex=-1;
 for(uint32_t i=0;i<nb;i++)if(!strcmp(bs[i].name,"frames"))frameIndex=(int)i;
 mark783("command_record");
 FILE*map830=fopen("app-node-map830.csv","wb");if(map830){fprintf(map830,"index,name,shader,dispatch_x,dispatch_y,dispatch_z\n");for(uint32_t i=0;i<nn;i++)fprintf(map830,"%u,%s,%s,%u,%u,%u\n",i,ns[i].name,ns[i].shader,ns[i].dispatch[0],ns[i].dispatch[1],ns[i].dispatch[2]);fclose(map830);}
 int pass=frameIndex>=0;
 if(pass){
  double checkMs=run_live(&live);uint64_t allErrors=0;int qualityPass=0;double qualityMae=0,qualityMse=0;uint64_t qualityAlpha=0;FILE*record=fopen("startup-verification.json","wb");
  if(record)fprintf(record,"{\"gpu_ms\":%.6f,\"comparisons\":[",checkMs);unsigned compared=0;
  for(uint32_t i=0;i<nb;i++)if(strcmp(bs[i].reference,"-")){
   void*expected=NULL;size_t bytes=read_file(bs[i].reference,&expected);uint64_t errors=0;
   if(bytes>bs[i].size){free(expected);pass=0;break;}
   for(size_t j=0;j<bytes;j++)errors+=((unsigned char*)expected)[j]!=((unsigned char*)bs[i].gpu.mapped)[j];
   if(i==final){
    for(size_t j=0;j<bytes;j++){int delta=(int)((unsigned char*)expected)[j]-(int)((unsigned char*)bs[i].gpu.mapped)[j];if((j&3)==3)qualityAlpha+=delta!=0;else{qualityMae+=abs(delta);qualityMse+=delta*delta;}}
    qualityMae/=width*(double)height*3;qualityMse/=width*(double)height*3;qualityPass=qualityMae<=0.02&&qualityMse<=0.065025&&qualityAlpha==0;
   }
   free(expected);allErrors+=errors;
   if(record)fprintf(record,"%s{\"buffer\":\"%s\",\"bytes\":%llu,\"mismatches\":%llu}",compared++?",":"",bs[i].name,(unsigned long long)bytes,(unsigned long long)errors);
  }
  pass=pass&&(allErrors==0||(getenv("SMX_ALLOW_NUMERIC_DIFF")&&qualityPass));if(record){fprintf(record,"],\"byte_exact\":%s,\"rgb_mae_255\":%.9f,\"rgb_mse_255\":%.9f,\"quality_gate\":\"MAE <= 0.02 and MSE <= 0.065025 and alpha exact\",\"pass\":%s}\n",allErrors==0?"true":"false",qualityMae,qualityMse,pass?"true":"false");fclose(record);}
  mark783("startup_numerical_verify");
  interopContext=(SmxGraphInterop){device,queue,cmd,queries,queryCount,props.limits.timestampPeriod,memory,bs[frameIndex].gpu.buffer,bs[final].gpu.buffer,qi,width,height,vkGetDeviceProcAddr};
  VkPhysicalDeviceIDProperties bridgeId={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};VkPhysicalDeviceProperties2 bridgeProps={VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};bridgeProps.pNext=&bridgeId;vkGetPhysicalDeviceProperties2(phys,&bridgeProps);if(!bridgeId.deviceLUIDValid)return 3;memcpy(interopContext.luid,bridgeId.deviceLUID,8);
  smx_diag_event994(SMX_DIAG_MODEL_INIT994,SMX_DIAG_END994,initDiagnostic994,SMX_DIAG_SEMANTIC994,pass?0:1,&initFields994);
  if(pass){unsigned sessions783=0;for(;;){
  directCoreIndex=0;directCoreState=(ExternalCoreState){0};directCoreState.graphBuffers=bs;directCoreState.nodes=ns;directCoreState.pipelines=pipelines;directCoreState.nn=nn;directCoreState.qi=qi;directCoreState.fine=fineTiming;directCoreState.buffers=layout;directCoreState.cache=cache;directCoreStates[1]=directCoreStates[0];interopContext.bindImages=bind_external_images;
   interopContext.core=cmd;interopContext.alternates[0]=interopContext.alternates[1]=VK_NULL_HANDLE;
   char sessionName783[48];snprintf(sessionName783,sizeof(sessionName783),"session_%u_begin",++sessions783);mark783(sessionName783);
   int sessionCode783=smx_app_session(bs[frameIndex].gpu.mapped,bs[final].gpu.mapped,width,height,&live,run_live,bs[final].reference);
   snprintf(sessionName783,sizeof(sessionName783),"session_%u_capture_cleanup",sessions783);mark783(sessionName783);
   cleanup_external();directCoreIndex=0;memset(directCoreStates,0,sizeof(directCoreStates));interopContext.core=cmd;interopContext.alternates[0]=interopContext.alternates[1]=VK_NULL_HANDLE;
   cacheSave783(cache,&props);mark783("session_cache_save");pass=sessionCode783==0;
   if(!smx_app_wait_restart783(sessionCode783))break;
   mark783("retained_engine_resume");
  }}
  else fprintf(stderr,"Startup numerical verification failed. Capture has not started.\n");
 }
 cleanup_external();vkDestroyFence(device,fence,NULL);vkDestroyQueryPool(device,queries,NULL);vkDestroyCommandPool(device,cpool,NULL);vkDestroyDescriptorPool(device,pool,NULL);
 for(uint32_t i=0;i<np;i++){vkDestroyPipeline(device,pipelines[i].pipeline,NULL);vkDestroyShaderModule(device,pipelines[i].module,NULL);}
 for(uint32_t i=0;i<nb;i++)drop_buffer(&bs[i].gpu);
 vkDestroyPipelineCache(device,cache,NULL);vkDestroyPipelineLayout(device,pl,NULL);vkDestroyDescriptorSetLayout(device,layout,NULL);vkDestroyDevice(device,NULL);vkDestroyInstance(instance,NULL);FreeLibrary(lib);mark783("engine_destroy");if(phase783){fclose(phase783);phase783=NULL;}return pass?0:4;
}

#include "fp16-next537/candidate969/flow_access967.c.inc"

int smx_device_capabilities_cli995(void){
 FILE*out=fopen("app-capabilities995.jsonl","wb");if(!out)return 2;
 HMODULE lib=LoadLibraryExW(L"vulkan-1.dll",NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!lib){fprintf(out,"{\"error\":\"vulkan_loader_unavailable\",\"win32\":%lu}\n",GetLastError());fclose(out);return 3;}
 PFN_vkGetInstanceProcAddr get=(PFN_vkGetInstanceProcAddr)GetProcAddress(lib,"vkGetInstanceProcAddr");PFN_vkCreateInstance create=get?(PFN_vkCreateInstance)get(NULL,"vkCreateInstance"):NULL;if(!create){fclose(out);FreeLibrary(lib);return 3;}
 VkApplicationInfo ai={VK_STRUCTURE_TYPE_APPLICATION_INFO};ai.apiVersion=VK_API_VERSION_1_3;VkInstanceCreateInfo ci={VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};ci.pApplicationInfo=&ai;VkInstance in;VkResult r=create(&ci,NULL,&in);if(r){fprintf(out,"{\"error\":\"create_instance\",\"vk_result\":%d}\n",r);fclose(out);FreeLibrary(lib);return 3;}
 PFN_vkDestroyInstance destroy=(PFN_vkDestroyInstance)get(in,"vkDestroyInstance");PFN_vkEnumeratePhysicalDevices enumerate=(PFN_vkEnumeratePhysicalDevices)get(in,"vkEnumeratePhysicalDevices");PFN_vkGetPhysicalDeviceProperties props=(PFN_vkGetPhysicalDeviceProperties)get(in,"vkGetPhysicalDeviceProperties");uint32_t n=0;int eligible=0;r=enumerate(in,&n,NULL);if(r==VK_SUCCESS&&n<=128){VkPhysicalDevice ds[128];r=enumerate(in,&n,ds);if(r==VK_SUCCESS)for(uint32_t i=0;i<n;i++){SmxDeviceCaps995 c;char why[512];VkPhysicalDeviceProperties p;props(ds[i],&p);int ok=smx_probe_device995(get,in,ds[i],&c,why,sizeof(why));eligible+=ok;for(char*t=p.deviceName;*t;t++)if(*t=='"'||*t=='\\'||(unsigned char)*t<32)*t=' ';for(char*t=why;*t;t++)if(*t=='"'||*t=='\\'||(unsigned char)*t<32)*t=' ';fprintf(out,"{\"index\":%u,\"name\":\"%s\",\"vendor_id\":%u,\"device_id\":%u,\"eligible\":%s,\"reason\":\"%s\",\"features\":%d,\"shape_fp16_8x16x16\":%d,\"subgroups_16_32\":%d,\"external_image\":%d,\"external_fence\":%d,\"gpu_work_submitted\":false,\"memory_budget_checked\":false,\"b580_physically_tested\":false}\n",i,p.deviceName,p.vendorID,p.deviceID,ok?"true":"false",why,c.features,c.shape,c.subgroups,c.externalImage,c.externalSemaphore);}}
 destroy(in,NULL);FreeLibrary(lib);fclose(out);return eligible?0:3;
}
