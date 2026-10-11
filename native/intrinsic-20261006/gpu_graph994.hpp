#include "../adapter_identity995.h"
#include "fp16-next537/candidate994/bounded_stdio989.h"
#include "fp16-next537/candidate994/resolution_budget989.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <windows.h>
#include "fp16-next537/candidate994/integration994.hpp"
#include <d3d11.h>
#include <d3d11_4.h>
#include <memory>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <deque>
#include <string>
extern "C" int smx_pixel_flow967(unsigned,VkBuffer*,uint64_t*,uint64_t*);
inline bool smxDirectEnabled(){const char*s=getenv("SMX_DIRECT_ORIGINALS");return s&&strcmp(s,"1")==0;}
inline bool smxDirectOutputEnabled(){const char*s=getenv("SMX_DIRECT_OUTPUT");return smxDirectEnabled()&&s&&strcmp(s,"1")==0;}
extern "C" VkQueryPool smx_graph_queries905(unsigned);
extern "C" VkCommandBuffer smx_graph_core839(unsigned,unsigned);
extern "C" int smx_graph_update_images(unsigned,const VkImageView*);
#include "graph_interop_rotate.h"
#include "fp16-next537/candidate994/vulkan_present994.hpp"
extern "C" VkQueue smx_wsi_queue961(void);
extern "C" VkInstance smx_wsi_instance958(void);
extern "C" VkPhysicalDevice smx_wsi_physical958(void);
extern "C" PFN_vkGetInstanceProcAddr smx_wsi_get958(void);
#include "capture_gpu_direct.hpp"
using Microsoft::WRL::ComPtr;
inline void smxBridgeFailure555(const char* phase,long code){
 smxdiag994::error(SMX_DIAG_FATAL994,code,SMX_DIAG_SEMANTIC994);LARGE_INTEGER q{},f{};QueryPerformanceCounter(&q);QueryPerformanceFrequency(&f);
 smx_log_fprintf989(stderr,"BRIDGE555_FATAL phase=%s code=%ld qpc=%.9f; bounded fail-stop, no wait-idle/retry\n",phase,code,double(q.QuadPart)/f.QuadPart);fflush(stderr);fflush(stdout);TerminateProcess(GetCurrentProcess(),9);
}
#define CK(x) do { VkResult vr=(x); if(vr!=VK_SUCCESS){smxdiag994::error(SMX_DIAG_FATAL994,vr,SMX_DIAG_VKRESULT994,smxdiag994::fields(0,0,(uintptr_t)device,0,0,__LINE__,2));smxBridgeFailure555(#x,long(vr));} } while(0)
#define HR(x) do { HRESULT hr=(x); if(FAILED(hr)){smxdiag994::error(SMX_DIAG_FATAL994,hr,SMX_DIAG_HRESULT994,smxdiag994::fields(0,0,0,0,0,__LINE__,2));smxBridgeFailure555(#x,long(hr));} } while(0)
struct BridgeHandle {HANDLE value=nullptr;~BridgeHandle(){if(value)CloseHandle(value);} };
struct BridgeLock {
 IDXGIKeyedMutex* mutex=nullptr;
 explicit BridgeLock(IDXGIKeyedMutex*m):mutex(m){HRESULT lockResult=SMX_DIAG_CALL994(SMX_DIAG_KEYED_ACQUIRE994,smxdiag994::fields(0,0,(uintptr_t)mutex),SMX_DIAG_HRESULT994,mutex->AcquireSync(0,5000));if(lockResult!=S_OK)smxBridgeFailure555("D3D AcquireSync",long(lockResult));}
 ~BridgeLock(){if(mutex)SMX_DIAG_CALL994(SMX_DIAG_KEYED_RELEASE994,smxdiag994::fields(0,0,(uintptr_t)mutex),SMX_DIAG_HRESULT994,mutex->ReleaseSync(0));}
 BridgeLock(const BridgeLock&)=delete;BridgeLock&operator=(const BridgeLock&)=delete;
};

// R948: every CPU transaction queues a GPU wait, access, signal, and flush while
// holding one resource lock. Vulkan locks all used resources in address order.
// Counter updates describe submitted signals, never GPU-completed work.
struct NativeSync948 {
 std::mutex cpu;
 ComPtr<ID3D11Fence> capture,display;
 VkSemaphore semaphore{};
 uint64_t value=0;
};
struct NativeD3DAccess948 {
 std::shared_ptr<NativeSync948> state;
 std::unique_lock<std::mutex> lock;
 ComPtr<ID3D11DeviceContext4> context;
 ID3D11Fence* fence=nullptr;
 bool committed=false;uint32_t diagnosticSignalStage994=SMX_DIAG_CAPTURE_SIGNAL994;
 NativeD3DAccess948(std::shared_ptr<NativeSync948> s,ID3D11DeviceContext*c,bool display)
  :state(std::move(s)),lock(state->cpu){
  diagnosticSignalStage994=display?SMX_DIAG_DISPLAY_SIGNAL994:SMX_DIAG_CAPTURE_SIGNAL994;
  HR(c->QueryInterface(IID_PPV_ARGS(&context)));
  fence=display?state->display.Get():state->capture.Get();
  if(!fence)throw std::runtime_error("R948 missing matching-device fence");
  HR(SMX_DIAG_CALL994(SMX_DIAG_D3D_WAIT994,smxdiag994::fields(0,0,(uintptr_t)fence,state->value),SMX_DIAG_HRESULT994,context->Wait(fence,state->value)));
 }
 void signal(){
  if(committed)throw std::runtime_error("R948 duplicate transaction commit");
  HR(SMX_DIAG_CALL994(diagnosticSignalStage994,smxdiag994::fields(0,0,(uintptr_t)fence,state->value+1),SMX_DIAG_HRESULT994,context->Signal(fence,state->value+1)));++state->value;committed=true;
 }
 ~NativeD3DAccess948(){
  // A queued wait cannot be cancelled safely after an exceptional partial access.
  if(!committed)smxBridgeFailure555("R948 abandoned D3D transaction",E_FAIL);
 }
};

class GpuGraphBridge {
#define BRIDGE_FUNCTIONS(X) X(vkDestroyDevice) X(vkGetDeviceQueue) X(vkCreateImage) X(vkDestroyImage) X(vkCreateImageView) X(vkDestroyImageView) X(vkGetImageMemoryRequirements) X(vkGetMemoryWin32HandlePropertiesKHR) X(vkAllocateMemory) X(vkFreeMemory) X(vkBindImageMemory) X(vkCreateBuffer) X(vkDestroyBuffer) X(vkGetBufferMemoryRequirements) X(vkBindBufferMemory) X(vkMapMemory) X(vkUnmapMemory) X(vkCreateCommandPool) X(vkDestroyCommandPool) X(vkAllocateCommandBuffers) X(vkBeginCommandBuffer) X(vkEndCommandBuffer) X(vkCmdPipelineBarrier) X(vkCmdCopyBuffer) X(vkCmdCopyImageToBuffer) X(vkCmdCopyBufferToImage) X(vkCreateFence) X(vkDestroyFence) X(vkQueueSubmit) X(vkGetFenceStatus) X(vkWaitForFences) X(vkResetCommandBuffer) X(vkResetFences) X(vkGetQueryPoolResults) X(vkDeviceWaitIdle) X(vkCmdCopyImage) X(vkCreateQueryPool) X(vkDestroyQueryPool) X(vkCmdResetQueryPool) X(vkCmdWriteTimestamp) X(vkCreateSemaphore) X(vkDestroySemaphore) X(vkImportSemaphoreWin32HandleKHR)
#define DECL(n) PFN_##n n=nullptr;
 BRIDGE_FUNCTIONS(DECL)
#undef DECL


  bool probe846=false,packet847=true;VkFence isolatedFence848{};double isolatedWait848=0;VkCommandBuffer probeCommand846{};VkQueryPool probeQueries846{};FILE*probeTrace846=nullptr;FILE*clockTrace846=nullptr;unsigned probeId846=0;
  PFN_vkGetCalibratedTimestampsEXT calibration846=nullptr;uint64_t anchor846[2]{},anchorDeviation846=0;double qpcFrequency846=0;
  void clock846(const char*phase){
   VkCalibratedTimestampInfoEXT ci[2]={{VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT},{VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT}};ci[0].timeDomain=VK_TIME_DOMAIN_DEVICE_EXT;ci[1].timeDomain=VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT;
   uint64_t values[2]{},dev=0;LARGE_INTEGER a{},z{};QueryPerformanceCounter(&a);VkResult result=calibration846(device,2,ci,values,&dev);QueryPerformanceCounter(&z);
   if(!clockTrace846){clockTrace846=smx_log_fopen989("app-clock846.csv","wb");if(clockTrace846)smx_log_fprintf989(clockTrace846,"phase,result,gpu_tick,qpc_tick,max_deviation_ns,call_ms,timestamp_period_ns,qpc_frequency\n");}
   if(clockTrace846)smx_log_fprintf989(clockTrace846,"%s,%d,%llu,%llu,%llu,%.6f,%.9f,%.0f\n",phase,int(result),(unsigned long long)values[0],(unsigned long long)values[1],(unsigned long long)dev,1000.0*(z.QuadPart-a.QuadPart)/qpcFrequency846,double(g->timestampPeriod),qpcFrequency846);
   if(result==VK_SUCCESS&&!anchor846[0]){anchor846[0]=values[0];anchor846[1]=values[1];anchorDeviation846=dev;}
  }
  double gpuQpc846(uint64_t tick)const{return double(anchor846[1])/qpcFrequency846+double(int64_t(tick-anchor846[0]))*g->timestampPeriod/1e9;}
 VkResult queryDiag994(VkDevice d,VkQueryPool p,uint32_t first,uint32_t count,size_t bytes,void*out,VkDeviceSize stride,VkQueryResultFlags flags){return SMX_DIAG_CALL994(SMX_DIAG_MODEL_QUERY994,smxdiag994::fields(0,0,(uintptr_t)p,0,0,first,count),SMX_DIAG_VKRESULT994,vkGetQueryPoolResults(d,p,first,count,bytes,out,stride,flags));}
 std::mutex queueMutex958,presentationMutex961;std::unique_ptr<VulkanPresenter958> presenter958;
 VkResult submitQueue958(VkQueue q,uint32_t count,const VkSubmitInfo*si,VkFence fence){std::lock_guard lock(queueMutex958);return SMX_DIAG_CALL994(SMX_DIAG_MODEL_SUBMIT994,smxdiag994::fields(0,0,(uintptr_t)q,(uintptr_t)fence,0,count,2),SMX_DIAG_VKRESULT994,vkQueueSubmit(q,count,si,fence));}
 const SmxGraphInterop*g;
 VkDevice device;VkQueue queue;VkCommandPool pool{};VkCommandBuffer transfer[2]{};VkFence fence{};
 struct DisplayOnly729 {ComPtr<ID3D11Texture2D>texture,alias;};std::vector<DisplayOnly729>displayOnly729;
 ComPtr<ID3D11Device>d3d;ComPtr<ID3D11DeviceContext>ctx;ComPtr<ID3D11Device1>displayDevice700;
 bool native948=false;uint64_t nativeResources948=0,nativeSubmits948=0;
  struct ImportedImage {std::shared_ptr<NativeSync948> sync948;ComPtr<ID3D11Texture2D> texture;ComPtr<ID3D11Texture2D> displayTexture700;ComPtr<IDXGIKeyedMutex> mutex;ComPtr<ID3D11UnorderedAccessView>uav;VkImage image{};VkImageView view{};VkDeviceMemory memory{};bool output=false;};
 struct PrivateInput651{VkImage image{};VkImageView view{};VkDeviceMemory memory{};}privateInputs651[2];
 VkCommandBuffer inputStage651{};VkFence inputFence651{};VkQueryPool inputQueries651{};bool inputInitialized651=false;
 PFN_vkGetCalibratedTimestampsEXT calibrated737=nullptr;FILE*calTrace737=nullptr;unsigned calId737=0;
 FILE*blockTrace851=nullptr;unsigned blockId851=0;FILE*abTrace839=nullptr;unsigned abId839=0;FILE*interopTrace651=nullptr;unsigned interopId651=0;
 struct Snapshot929 {PrivateInput651 images[2]{};VkCommandBuffer command{};VkQueryPool timing931{};bool initialized[2]{false,false};};Snapshot929 snapshots929[2];
 uint64_t pairFirst943=0,pairSecond943=0,previousSecond943=0,reuseHits943=0,snapshotImages943=0;ImportedImage*previousImage943=nullptr;unsigned previousSlot943=0;bool havePrevious943=false,reuseFirst943=false;
 bool snapshot651()const{const char*s=getenv("SMX_SNAPSHOT_INPUTS");return directPair&&s&&strcmp(s,"1")==0;}
 
 struct Job905 {VkCommandBuffer transfer[2]{};VkFence fence{};bool busy=false;LARGE_INTEGER submitQpc{},freq{};std::chrono::steady_clock::time_point start{},submitted{};double submitMs=0;};
 uint64_t diagnosticJob994[2]{},diagnosticNextJob994=0;
 Job905 jobs905[2];std::mutex completeMutex906;std::condition_variable completeCv906;std::deque<unsigned> completeQueue906;std::thread completeThread906;bool completeStop906=false;std::atomic<bool> done906[2]{};VkResult result906[2]{};LARGE_INTEGER fenceQpc906[2]{};LARGE_INTEGER completionRequested988{},completionEntered988{},completionEnqueued988[2]{},completionDequeued988[2]{},fenceWaitBegin988[2]{};bool completionReady988=false;std::atomic<HANDLE> wake906{nullptr};FILE*stageTrace931=nullptr;FILE*asyncTrace905=nullptr;unsigned asyncId905=0;
 std::vector<uint64_t> queryTimes;
 std::deque<ImportedImage> retained;ImportedImage*directFirst=nullptr,*directSecond=nullptr,*directOutput=nullptr;bool directPair=false;
 ImportedImage images[3];bool hasPrevious=false;unsigned seen=0,newest=0;
 static uint32_t typeFor(const VkPhysicalDeviceMemoryProperties&m,uint32_t bits,VkMemoryPropertyFlags flags){for(uint32_t i=0;i<m.memoryTypeCount;i++)if((bits&(1u<<i))&&(m.memoryTypes[i].propertyFlags&flags)==flags)return i;throw std::runtime_error("No compatible imported memory");}
 #include "fp16-next537/candidate969/pixel_audit967.inc"
 void cleanup() noexcept {
  if(completeThread906.joinable()){{std::lock_guard lock(completeMutex906);completeStop906=true;}completeCv906.notify_all();completeThread906.join();}
  presenter958.reset();if(vkDeviceWaitIdle&&device)SMX_DIAG_CALL994(SMX_DIAG_MODEL_IDLE994,smxdiag994::fields(0,0,(uintptr_t)device),SMX_DIAG_VKRESULT994,vkDeviceWaitIdle(device));pixelCleanup967();
 smx_log_fprintf989(stderr,"NATIVE948 enabled=%d resources=%llu vulkan_submits=%llu external_ownership_barriers=retained\n",native948,(unsigned long long)nativeResources948,(unsigned long long)nativeSubmits948);
  smx_log_fprintf989(stderr,"SNAPSHOT943 reuse_first_hits=%llu copied_images=%llu shared_input_mutexes_avoided=%llu bounded_private_images=4\n",(unsigned long long)reuseHits943,(unsigned long long)snapshotImages943,(unsigned long long)reuseHits943);
 for(auto&ss:snapshots929)for(auto&v:ss.images){if(v.view)vkDestroyImageView(device,v.view,nullptr);if(v.image)vkDestroyImage(device,v.image,nullptr);if(v.memory)vkFreeMemory(device,v.memory,nullptr);}
 
 if(stageTrace931)smx_log_fclose989(stageTrace931);for(auto&ss:snapshots929)if(ss.timing931)vkDestroyQueryPool(device,ss.timing931,nullptr);if(asyncTrace905)smx_log_fclose989(asyncTrace905);for(auto&j:jobs905)if(j.fence&&vkDestroyFence)vkDestroyFence(device,j.fence,nullptr);
   if(probe846&&calibration846&&device)clock846("end");if(clockTrace846){smx_log_fclose989(clockTrace846);clockTrace846=nullptr;}if(probeTrace846){smx_log_fclose989(probeTrace846);probeTrace846=nullptr;}if(isolatedFence848&&vkDestroyFence)vkDestroyFence(device,isolatedFence848,nullptr);if(probeQueries846&&vkDestroyQueryPool)vkDestroyQueryPool(device,probeQueries846,nullptr);
  if(blockTrace851){smx_log_fclose989(blockTrace851);blockTrace851=nullptr;}if(abTrace839){smx_log_fclose989(abTrace839);abTrace839=nullptr;}if(calTrace737){smx_log_fclose989(calTrace737);calTrace737=nullptr;}if(interopTrace651){smx_log_fclose989(interopTrace651);interopTrace651=nullptr;}
 if(inputFence651&&vkDestroyFence)vkDestroyFence(device,inputFence651,nullptr);if(inputQueries651&&vkDestroyQueryPool)vkDestroyQueryPool(device,inputQueries651,nullptr);
 for(auto&v:privateInputs651){if(v.view)vkDestroyImageView(device,v.view,nullptr);if(v.image)vkDestroyImage(device,v.image,nullptr);if(v.memory)vkFreeMemory(device,v.memory,nullptr);}
 if(fence&&vkDestroyFence)vkDestroyFence(device,fence,nullptr);fence={};
  if(pool&&vkDestroyCommandPool)vkDestroyCommandPool(device,pool,nullptr);pool={};
  for(auto&v:retained){if(v.sync948&&v.sync948->semaphore)vkDestroySemaphore(device,v.sync948->semaphore,nullptr);if(v.view)vkDestroyImageView(device,v.view,nullptr);if(v.image)vkDestroyImage(device,v.image,nullptr);if(v.memory)vkFreeMemory(device,v.memory,nullptr);}retained.clear();
  for(auto&v:images){if(v.view&&vkDestroyImageView)vkDestroyImageView(device,v.view,nullptr);v.view={};if(v.image&&vkDestroyImage)vkDestroyImage(device,v.image,nullptr);v.image={};if(v.memory&&vkFreeMemory)vkFreeMemory(device,v.memory,nullptr);v.memory={};}
 }
 void initialize(){SmxResolutionBudget989 extent989{};char why989[384]{};if(!smx_extent_check989(g->width,g->height,16384,UINT32_MAX,&extent989,why989,sizeof why989))throw std::runtime_error(why989);
  queryTimes.resize(g->queryCount);
  const uint32_t W=g->width,H=g->height,qi=g->queueFamily;const size_t bytes=size_t(W)*H*4;auto memory=g->memory;
#define LOAD(n) n=(PFN_##n)g->getProc(device,#n);if(!n)throw std::runtime_error(#n);
  BRIDGE_FUNCTIONS(LOAD)
#undef LOAD
 if(getenv("SMX_CALIBRATED_TIMING")){calibrated737=(PFN_vkGetCalibratedTimestampsEXT)g->getProc(device,"vkGetCalibratedTimestampsEXT");if(!calibrated737)throw std::runtime_error("Calibrated timestamp function absent");}
  ComPtr<IDXGIDevice>dx;HR(d3d.As(&dx));ComPtr<IDXGIAdapter>adapter;HR(dx->GetAdapter(&adapter));DXGI_ADAPTER_DESC ad{};HR(adapter->GetDesc(&ad));if(FILE*md=smx_log_fopen989("app-bridge-adapter994.json","wb")){smx_log_fprintf989(md,"{\"vendor_id\":%u,\"device_id\":%u,\"subsystem_id\":%u,\"revision\":%u,\"luid_high\":%ld,\"luid_low\":%lu,\"d3d_device\":%llu,\"vk_device\":%llu,\"compute_queue\":%llu,\"compute_queue_family\":%u,\"descriptor_query\":\"existing bridge adapter check reused; no new driver query\",\"validation\":\"following existing same-adapter LUID check determines success\"}\n",ad.VendorId,ad.DeviceId,ad.SubSysId,ad.Revision,ad.AdapterLuid.HighPart,ad.AdapterLuid.LowPart,(unsigned long long)(uintptr_t)d3d.Get(),(unsigned long long)(uintptr_t)device,(unsigned long long)(uintptr_t)queue,g->queueFamily);smx_log_fclose989(md);}if(ad.VendorId!=0x8086||memcmp(&ad.AdapterLuid,g->luid,8))throw std::runtime_error("GPU bridge requires the selected Intel Vulkan adapter LUID");
 D3D11_TEXTURE2D_DESC td{};td.Width=W;td.Height=H;td.MipLevels=1;td.ArraySize=1;td.Format=DXGI_FORMAT_R8G8B8A8_UNORM;td.BindFlags=D3D11_BIND_SHADER_RESOURCE;td.SampleDesc.Count=1;td.Usage=D3D11_USAGE_DEFAULT;td.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET|D3D11_BIND_UNORDERED_ACCESS;td.MiscFlags=D3D11_RESOURCE_MISC_SHARED_NTHANDLE|D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
 for(uint32_t j=0;j<3;j++){
  td.Format=DXGI_FORMAT_B8G8R8A8_UNORM;td.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;auto&v=images[j];HR(d3d->CreateTexture2D(&td,nullptr,&v.texture));HR(v.texture.As(&v.mutex));
  ComPtr<IDXGIResource1>resource;HR(v.texture.As(&resource));BridgeHandle ownedHandle;HANDLE&handle=ownedHandle.value;HR(resource->CreateSharedHandle(nullptr,DXGI_SHARED_RESOURCE_READ|DXGI_SHARED_RESOURCE_WRITE,nullptr,&handle));
  VkExternalMemoryImageCreateInfo ei{VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};ei.handleTypes=VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
  VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};ii.pNext=&ei;ii.imageType=VK_IMAGE_TYPE_2D;ii.format=VK_FORMAT_B8G8R8A8_UNORM;ii.extent={W,H,1};ii.mipLevels=1;ii.arrayLayers=1;ii.samples=VK_SAMPLE_COUNT_1_BIT;ii.tiling=VK_IMAGE_TILING_OPTIMAL;ii.usage=VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT|(j==2?VK_IMAGE_USAGE_STORAGE_BIT:0);ii.sharingMode=VK_SHARING_MODE_EXCLUSIVE;ii.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
  CK(vkCreateImage(device,&ii,nullptr,&v.image));VkMemoryRequirements req;vkGetImageMemoryRequirements(device,v.image,&req);
  VkMemoryWin32HandlePropertiesKHR hp{VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR};CK(vkGetMemoryWin32HandlePropertiesKHR(device,VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT,handle,&hp));
  VkMemoryDedicatedAllocateInfo dedicated{VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO};dedicated.image=v.image;VkImportMemoryWin32HandleInfoKHR imp{VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR};imp.pNext=&dedicated;imp.handleType=VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;imp.handle=handle;
  VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.pNext=&imp;ai.allocationSize=req.size;ai.memoryTypeIndex=typeFor(memory,req.memoryTypeBits&hp.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VkResult allocated=vkAllocateMemory(device,&ai,nullptr,&v.memory);CK(allocated);CK(vkBindImageMemory(device,v.image,v.memory,0));
  VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};vi.image=v.image;vi.viewType=VK_IMAGE_VIEW_TYPE_2D;vi.format=VK_FORMAT_B8G8R8A8_UNORM;vi.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};CK(vkCreateImageView(device,&vi,nullptr,&v.view));
 }
 VkImageView views[3]={images[0].view,images[1].view,images[2].view};if(!g->bindImages||g->bindImages(views))throw std::runtime_error("Direct image graph initialization failed");std::swap(views[0],views[1]);if(g->bindImages(views))throw std::runtime_error("Reverse image graph initialization failed");
 VkCommandPoolCreateInfo pi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};pi.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;pi.queueFamilyIndex=qi;CK(vkCreateCommandPool(device,&pi,nullptr,&pool));VkCommandBufferAllocateInfo ca{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ca.commandPool=pool;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=2;CK(vkAllocateCommandBuffers(device,&ca,transfer));VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
   probe846=getenv("SMX_PROBE846")!=nullptr;if(probe846){
   calibration846=(PFN_vkGetCalibratedTimestampsEXT)g->getProc(device,"vkGetCalibratedTimestampsEXT");if(!calibration846)throw std::runtime_error("Probe calibration function missing");LARGE_INTEGER q{};QueryPerformanceFrequency(&q);qpcFrequency846=double(q.QuadPart);clock846("begin");if(getenv("SMX_PROBE_ISOLATE848")){VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};CK(vkCreateFence(device,&fi,nullptr,&isolatedFence848));}if(!anchor846[0])throw std::runtime_error("Probe calibration failed");
   VkQueryPoolCreateInfo pq{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};pq.queryType=VK_QUERY_TYPE_TIMESTAMP;pq.queryCount=5;CK(vkCreateQueryPool(device,&pq,nullptr,&probeQueries846));VkCommandBufferAllocateInfo pc=ca;pc.commandBufferCount=1;CK(vkAllocateCommandBuffers(device,&pc,&probeCommand846));CK(vkBeginCommandBuffer(probeCommand846,&begin));vkCmdResetQueryPool(probeCommand846,probeQueries846,0,5);vkCmdWriteTimestamp(probeCommand846,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,probeQueries846,0);CK(vkEndCommandBuffer(probeCommand846));
  }
auto imageBarrier=[&](VkCommandBuffer cmd,VkImage image,VkImageLayout oldLayout,VkImageLayout newLayout,uint32_t source,uint32_t dest,VkAccessFlags srcAccess,VkAccessFlags dstAccess,VkPipelineStageFlags srcStage,VkPipelineStageFlags dstStage){
  VkImageMemoryBarrier im{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};im.oldLayout=oldLayout;im.newLayout=newLayout;im.srcQueueFamilyIndex=source;im.dstQueueFamilyIndex=dest;im.srcAccessMask=srcAccess;im.dstAccessMask=dstAccess;im.image=image;im.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};vkCmdPipelineBarrier(cmd,srcStage,dstStage,0,0,nullptr,0,nullptr,1,&im);
 };

 CK(vkBeginCommandBuffer(transfer[0],&begin));
 for(uint32_t j=0;j<3;j++)imageBarrier(transfer[0],images[j].image,VK_IMAGE_LAYOUT_GENERAL,VK_IMAGE_LAYOUT_GENERAL,VK_QUEUE_FAMILY_EXTERNAL,qi,0,j==2?VK_ACCESS_SHADER_WRITE_BIT:VK_ACCESS_SHADER_READ_BIT,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
 CK(vkEndCommandBuffer(transfer[0]));
 CK(vkBeginCommandBuffer(transfer[1],&begin));
 for(uint32_t j=0;j<3;j++)imageBarrier(transfer[1],images[j].image,VK_IMAGE_LAYOUT_GENERAL,VK_IMAGE_LAYOUT_GENERAL,qi,VK_QUEUE_FAMILY_EXTERNAL,j==2?VK_ACCESS_SHADER_WRITE_BIT:VK_ACCESS_SHADER_READ_BIT,0,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
 CK(vkEndCommandBuffer(transfer[1]));
  VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};CK(vkCreateFence(device,&fi,nullptr,&fence));
 }
public:

 void startWSI958(HWND hwnd,bool verify=false){if(!native948)throw std::runtime_error("WSI requires native sync");if(!smx_wsi_queue961()||smx_wsi_family995()==UINT32_MAX)throw std::runtime_error("Optional Vulkan WSI backend requires a separate timestamp-capable queue family; use DXGI/DComp");presenter958=std::make_unique<VulkanPresenter958>(smx_wsi_instance958(),smx_wsi_physical958(),device,smx_wsi_queue961(),smx_wsi_family995(),g->timestampPeriod,smx_wsi_get958(),g->getProc,presentationMutex961,hwnd,g->width,g->height,verify||pixelAudit967());}
 void prepareWSI958(ID3D11Texture2D*t,std::shared_ptr<void>lease,bool generated967=false){if(!presenter958)throw std::runtime_error("WSI not initialized");for(auto&v:retained)if(v.texture.Get()==t&&v.sync948){auto&s=*v.sync948;uint64_t auditId967=pixelWsiJob967(v.image);presenter958->setDiagnosticReadback967(!pixelAudit967()||auditId967!=0);presenter958->prepare({v.image,s.semaphore,&s.value,&s.cpu,true,generated967},std::move(lease));pixelWsiFinish967(auditId967);return;}throw std::runtime_error("WSI unknown synchronized texture");}
 UINT presentWSI958(){return UINT(presenter958->present());}
 void pollWSI968(){if(presenter958)presenter958->poll();}
 void drainWSI958(){if(presenter958)presenter958->drain();}
 void stopWSI958(){presenter958.reset();}
 const unsigned char* pixelsWSI958(){return presenter958->verifiedPixels();}
 void enableNativeSync948(){
  if(!vkCreateSemaphore||!vkDestroySemaphore||!vkImportSemaphoreWin32HandleKHR)
   throw std::runtime_error("R948 external timeline semaphore entrypoints unavailable");
  native948=true;
 }
 uint64_t nativeResourceCount948()const{return nativeResources948;}
 uint64_t nativeSubmitCount948()const{return nativeSubmits948;}
 std::unique_ptr<NativeD3DAccess948> access948(ID3D11Texture2D*t,ID3D11DeviceContext*c){
  for(auto&v:retained){
   if(v.texture.Get()==t){if(v.sync948)return std::make_unique<NativeD3DAccess948>(v.sync948,c,false);return {};}
   if(v.displayTexture700.Get()==t){if(v.sync948)return std::make_unique<NativeD3DAccess948>(v.sync948,c,true);return {};}
  }
  return {};
 }
 void initializeSync948(ImportedImage&v){
  v.sync948=std::make_shared<NativeSync948>();auto&s=*v.sync948;
  ComPtr<ID3D11Device5> cd;HR(d3d.As(&cd));
  HR(cd->CreateFence(0,D3D11_FENCE_FLAG_SHARED,IID_PPV_ARGS(&s.capture)));
  BridgeHandle h;HR(s.capture->CreateSharedHandle(nullptr,GENERIC_ALL,nullptr,&h.value));
  if(displayDevice700){ComPtr<ID3D11Device5> dd;HR(displayDevice700.As(&dd));HR(dd->OpenSharedFence(h.value,IID_PPV_ARGS(&s.display)));}
  VkSemaphoreTypeCreateInfo type{VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO};type.semaphoreType=VK_SEMAPHORE_TYPE_TIMELINE;type.initialValue=0;
  VkSemaphoreCreateInfo ci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};ci.pNext=&type;
  CK(vkCreateSemaphore(device,&ci,nullptr,&s.semaphore));
  VkImportSemaphoreWin32HandleInfoKHR imp{VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR};
  imp.semaphore=s.semaphore;imp.handleType=VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;imp.handle=h.value;
  CK(vkImportSemaphoreWin32HandleKHR(device,&imp));++nativeResources948;
 }

 double queueWaitMs988=-1;
 double inputLockMs=0,inputSubmitMs=0,vkSubmitMs=0,vkFenceMs=0,queryMs=0,outputLockMs=0,outputSubmitMs=0;
 using Clock=std::chrono::steady_clock;
 static double elapsed(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
 GpuGraphBridge(ID3D11Device*d,ID3D11DeviceContext*c):g(smx_graph_interop_context()),device(g->device),queue(g->queue),d3d(d),ctx(c){try{initialize();}catch(...){cleanup();throw;}}
 ~GpuGraphBridge(){cleanup();}
 GpuGraphBridge(const GpuGraphBridge&)=delete;GpuGraphBridge&operator=(const GpuGraphBridge&)=delete;
 ComPtr<ID3D11Texture2D> makeDisplayOriginal729(){
 if(!displayDevice700)throw std::runtime_error("Display clone requires separate display device");
 D3D11_TEXTURE2D_DESC td{};td.Width=g->width;td.Height=g->height;td.MipLevels=td.ArraySize=1;td.Format=DXGI_FORMAT_B8G8R8A8_UNORM;td.SampleDesc.Count=1;td.Usage=D3D11_USAGE_DEFAULT;td.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;td.MiscFlags=D3D11_RESOURCE_MISC_SHARED_NTHANDLE|D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
 DisplayOnly729 v;HR(d3d->CreateTexture2D(&td,nullptr,&v.texture));ComPtr<IDXGIResource1>r;HR(v.texture.As(&r));BridgeHandle handle;HR(r->CreateSharedHandle(nullptr,DXGI_SHARED_RESOURCE_READ|DXGI_SHARED_RESOURCE_WRITE,nullptr,&handle.value));HR(displayDevice700->OpenSharedResource1(handle.value,IID_PPV_ARGS(&v.alias)));displayOnly729.push_back(v);return v.texture;
 }
 void setDisplayDevice700(ID3D11Device*d){HR(d->QueryInterface(IID_PPV_ARGS(&displayDevice700)));}
 ID3D11Texture2D* displayTexture700(ID3D11Texture2D*t){if(!displayDevice700)return t;for(auto&v:retained)if(v.texture.Get()==t){if(!v.displayTexture700)throw std::runtime_error("Missing pre-opened display resource");return v.displayTexture700.Get();}for(auto&v:displayOnly729)if(v.texture.Get()==t)return v.alias.Get();throw std::runtime_error("Split display requires retained direct textures");}
 ComPtr<ID3D11Texture2D> makeRetainedTexture(bool output=false){
  const uint32_t W=g->width,H=g->height;auto memory=g->memory;
  D3D11_TEXTURE2D_DESC td{};td.Width=W;td.Height=H;td.MipLevels=1;td.ArraySize=1;td.SampleDesc.Count=1;td.Usage=D3D11_USAGE_DEFAULT;
  td.MiscFlags=D3D11_RESOURCE_MISC_SHARED_NTHANDLE|(native948?D3D11_RESOURCE_MISC_SHARED:D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX);
  retained.emplace_back();
  td.Format=DXGI_FORMAT_B8G8R8A8_UNORM;td.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;auto&v=retained.back();v.output=output;HR(d3d->CreateTexture2D(&td,nullptr,&v.texture));if(!native948)HR(v.texture.As(&v.mutex));
  ComPtr<IDXGIResource1>resource;HR(v.texture.As(&resource));BridgeHandle ownedHandle;HANDLE&handle=ownedHandle.value;HR(resource->CreateSharedHandle(nullptr,DXGI_SHARED_RESOURCE_READ|DXGI_SHARED_RESOURCE_WRITE,nullptr,&handle));if(displayDevice700)HR(displayDevice700->OpenSharedResource1(handle,IID_PPV_ARGS(&v.displayTexture700)));
  VkExternalMemoryImageCreateInfo ei{VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};ei.handleTypes=VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
  VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};ii.pNext=&ei;ii.imageType=VK_IMAGE_TYPE_2D;ii.format=VK_FORMAT_B8G8R8A8_UNORM;ii.extent={W,H,1};ii.mipLevels=1;ii.arrayLayers=1;ii.samples=VK_SAMPLE_COUNT_1_BIT;ii.tiling=VK_IMAGE_TILING_OPTIMAL;ii.usage=VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT|(output?VK_IMAGE_USAGE_STORAGE_BIT:0);ii.sharingMode=VK_SHARING_MODE_EXCLUSIVE;ii.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
  CK(vkCreateImage(device,&ii,nullptr,&v.image));VkMemoryRequirements req;vkGetImageMemoryRequirements(device,v.image,&req);
  VkMemoryWin32HandlePropertiesKHR hp{VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR};CK(vkGetMemoryWin32HandlePropertiesKHR(device,VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT,handle,&hp));
  VkMemoryDedicatedAllocateInfo dedicated{VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO};dedicated.image=v.image;VkImportMemoryWin32HandleInfoKHR imp{VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR};imp.pNext=&dedicated;imp.handleType=VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;imp.handle=handle;
  VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.pNext=&imp;ai.allocationSize=req.size;ai.memoryTypeIndex=typeFor(memory,req.memoryTypeBits&hp.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VkResult allocated=vkAllocateMemory(device,&ai,nullptr,&v.memory);CK(allocated);CK(vkBindImageMemory(device,v.image,v.memory,0));
  VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};vi.image=v.image;vi.viewType=VK_IMAGE_VIEW_TYPE_2D;vi.format=VK_FORMAT_B8G8R8A8_UNORM;vi.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};CK(vkCreateImageView(device,&vi,nullptr,&v.view));
  if(native948)initializeSync948(v);
  return retained.back().texture;
 }
 void setDirectPair(ID3D11Texture2D*first,ID3D11Texture2D*second){
  directFirst=directSecond=nullptr;
  for(auto&v:retained){if(v.texture.Get()==first)directFirst=&v;if(v.texture.Get()==second)directSecond=&v;}
  if(!directFirst||!directSecond||directFirst==directSecond)throw std::runtime_error("Invalid retained input pair");
  directPair=true;seen=2;
 }
 ComPtr<ID3D11Texture2D> makeOutputTexture(){return smxDirectOutputEnabled()?makeRetainedTexture(true):makePresentationTexture();}
 void setDirectOutput(ID3D11Texture2D*t){
  directOutput=nullptr;for(auto&v:retained)if(v.output&&v.texture.Get()==t)directOutput=&v;
  if(!directOutput||!directPair)throw std::runtime_error("Invalid retained output target");
 }
 ImportedImage& outputImage(){return directOutput?*directOutput:images[2];}
 void initializeSnapshot651(){
 if(inputStage651)return;
 for(auto&v:privateInputs651){
  VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};ii.imageType=VK_IMAGE_TYPE_2D;ii.format=VK_FORMAT_B8G8R8A8_UNORM;ii.extent={g->width,g->height,1};ii.mipLevels=ii.arrayLayers=1;ii.samples=VK_SAMPLE_COUNT_1_BIT;ii.tiling=VK_IMAGE_TILING_OPTIMAL;ii.usage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT|(pixelAudit967()?VK_IMAGE_USAGE_TRANSFER_SRC_BIT:0);ii.sharingMode=VK_SHARING_MODE_EXCLUSIVE;ii.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
  CK(vkCreateImage(device,&ii,nullptr,&v.image));VkMemoryRequirements req{};vkGetImageMemoryRequirements(device,v.image,&req);VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.allocationSize=req.size;ai.memoryTypeIndex=typeFor(g->memory,req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);CK(vkAllocateMemory(device,&ai,nullptr,&v.memory));CK(vkBindImageMemory(device,v.image,v.memory,0));
  VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};vi.image=v.image;vi.viewType=VK_IMAGE_VIEW_TYPE_2D;vi.format=VK_FORMAT_B8G8R8A8_UNORM;vi.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};CK(vkCreateImageView(device,&vi,nullptr,&v.view));
 }
 VkCommandBufferAllocateInfo ca{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ca.commandPool=pool;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=1;CK(vkAllocateCommandBuffers(device,&ca,&inputStage651));VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};CK(vkCreateFence(device,&fi,nullptr,&inputFence651));VkQueryPoolCreateInfo qi{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};qi.queryType=VK_QUERY_TYPE_TIMESTAMP;qi.queryCount=2;CK(vkCreateQueryPool(device,&qi,nullptr,&inputQueries651));
 smx_log_fprintf989(stderr,"SNAPSHOT651 extent=%ux%u private_input_logical_bytes=%llu; GPU copies; no resolution/math changes\n",g->width,g->height,(unsigned long long)g->width*g->height*8);
}
void recordInputSnapshot651(){
 initializeSnapshot651();CK(vkResetCommandBuffer(inputStage651,0));VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};CK(vkBeginCommandBuffer(inputStage651,&begin));vkCmdResetQueryPool(inputStage651,inputQueries651,0,2);vkCmdWriteTimestamp(inputStage651,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,inputQueries651,0);ImportedImage*src[2]={directFirst,directSecond};
 for(unsigned j=0;j<2;j++){
  VkImageMemoryBarrier im{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};im.oldLayout=im.newLayout=VK_IMAGE_LAYOUT_GENERAL;im.srcQueueFamilyIndex=VK_QUEUE_FAMILY_EXTERNAL;im.dstQueueFamilyIndex=g->queueFamily;im.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;im.image=src[j]->image;im.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};vkCmdPipelineBarrier(inputStage651,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&im);
  im.oldLayout=inputInitialized651?VK_IMAGE_LAYOUT_GENERAL:VK_IMAGE_LAYOUT_UNDEFINED;im.newLayout=VK_IMAGE_LAYOUT_GENERAL;im.srcQueueFamilyIndex=im.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;im.srcAccessMask=inputInitialized651?VK_ACCESS_SHADER_READ_BIT:0;im.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;im.image=privateInputs651[j].image;vkCmdPipelineBarrier(inputStage651,inputInitialized651?VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT:VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&im);
  VkImageCopy cp{};cp.srcSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};cp.dstSubresource=cp.srcSubresource;cp.extent={g->width,g->height,1};vkCmdCopyImage(inputStage651,src[j]->image,VK_IMAGE_LAYOUT_GENERAL,privateInputs651[j].image,VK_IMAGE_LAYOUT_GENERAL,1,&cp);
  im.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;im.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;vkCmdPipelineBarrier(inputStage651,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,0,nullptr,0,nullptr,1,&im);
  im.image=src[j]->image;im.srcQueueFamilyIndex=g->queueFamily;im.dstQueueFamilyIndex=VK_QUEUE_FAMILY_EXTERNAL;im.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;im.dstAccessMask=0;vkCmdPipelineBarrier(inputStage651,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,0,0,nullptr,0,nullptr,1,&im);
 }
 vkCmdWriteTimestamp(inputStage651,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,inputQueries651,1);CK(vkEndCommandBuffer(inputStage651));inputInitialized651=true;
}

 void recordDirectTransfers(){
 if(snapshot651())recordInputSnapshot651();
  VkImageView views[3]={snapshot651()?privateInputs651[0].view:directFirst->view,snapshot651()?privateInputs651[1].view:directSecond->view,outputImage().view};
  if(smx_graph_update_images(0,views))throw std::runtime_error("Direct descriptor update failed");
  ImportedImage*items[3]={directFirst,directSecond,&outputImage()};
  for(unsigned side=0;side<2;side++){
   CK(vkResetCommandBuffer(transfer[side],0));VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      CK(vkBeginCommandBuffer(transfer[side],&begin));
    if(probe846&&side==0&&!packet847){vkCmdResetQueryPool(transfer[side],probeQueries846,0,5);vkCmdWriteTimestamp(transfer[side],VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,probeQueries846,0);}
    if(probe846)vkCmdWriteTimestamp(transfer[side],VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,probeQueries846,side?3:1);
    VkImageMemoryBarrier grouped927[3]{};uint32_t count927=0;
    for(unsigned j=0;j<3;j++){
     if(snapshot651()&&j<2)continue; // Private copies are sampled by the core; shared inputs were released by inputStage651.

    VkImageMemoryBarrier im{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};im.oldLayout=im.newLayout=VK_IMAGE_LAYOUT_GENERAL;
    im.srcQueueFamilyIndex=side?g->queueFamily:VK_QUEUE_FAMILY_EXTERNAL;im.dstQueueFamilyIndex=side?VK_QUEUE_FAMILY_EXTERNAL:g->queueFamily;
    VkAccessFlags access=j==2?VK_ACCESS_SHADER_WRITE_BIT:VK_ACCESS_SHADER_READ_BIT;im.srcAccessMask=side?access:0;im.dstAccessMask=side?0:access;
    im.image=items[j]->image;im.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
    grouped927[count927++]=im;
       }
    vkCmdPipelineBarrier(transfer[side],side?VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT:VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,side?VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT:VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,0,nullptr,0,nullptr,count927,grouped927);
    if(probe846)vkCmdWriteTimestamp(transfer[side],VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,probeQueries846,side?4:2);CK(vkEndCommandBuffer(transfer[side]));
  }
 }

 bool pushFrame(ID3D11Texture2D*source,uint32_t cw,uint32_t ch){
  auto p0=Clock::now(); newest^=1u;BridgeLock current(images[newest].mutex.Get());auto p1=Clock::now();
    D3D11_TEXTURE2D_DESC input{};source->GetDesc(&input);
  if(cw!=g->width||ch!=g->height||input.Width!=cw||input.Height!=ch||input.Format!=DXGI_FORMAT_B8G8R8A8_UNORM)throw std::runtime_error("Native-copy backend requires native BGRA 1080p input");
  ctx->CopyResource(images[newest].texture.Get(),source);ctx->Flush();auto p2=Clock::now();inputLockMs=elapsed(p0,p1);inputSubmitMs=elapsed(p1,p2);
  bool ready=hasPrevious;hasPrevious=true;seen=std::min(seen+1,2u);return ready;
 }

 // Max two in-flight jobs on ONE queue; no descriptor/command/query mutation until each fence is signalled.

 void setWake906(HANDLE h){wake906=h;}
 void startCompletion906(){
  if(completeThread906.joinable())return;
  QueryPerformanceCounter(&completionRequested988);
  completeThread906=std::thread([this]{
   {std::lock_guard lock(completeMutex906);QueryPerformanceCounter(&completionEntered988);completionReady988=true;}completeCv906.notify_all();
   for(;;){unsigned s;
    {std::unique_lock lock(completeMutex906);completeCv906.wait(lock,[&]{return completeStop906||!completeQueue906.empty();});if(completeQueue906.empty()){if(completeStop906)return;continue;}s=completeQueue906.front();completeQueue906.pop_front();QueryPerformanceCounter(&completionDequeued988[s]);}
    QueryPerformanceCounter(&fenceWaitBegin988[s]);result906[s]=SMX_DIAG_CALL994(SMX_DIAG_MODEL_FENCE994,smxdiag994::fields(0,diagnosticJob994[s],(uintptr_t)device,(uintptr_t)jobs905[s].fence,0,s,5000000000ll),SMX_DIAG_VKRESULT994,vkWaitForFences(device,1,&jobs905[s].fence,VK_TRUE,5000000000ull));QueryPerformanceCounter(&fenceQpc906[s]);{std::lock_guard lock(completeMutex906);done906[s].store(true,std::memory_order_release);}
    completeCv906.notify_all();if(wake906)SetEvent(wake906);
    if(result906[s]!=VK_SUCCESS)return;
   }
  });
  {std::unique_lock lock(completeMutex906);if(!completeCv906.wait_for(lock,std::chrono::seconds(2),[&]{return completionReady988;}))smxBridgeFailure555("Completion worker initialization timeout",WAIT_TIMEOUT);}
  LARGE_INTEGER frequency988{};QueryPerformanceFrequency(&frequency988);FILE*startup988=smx_log_fopen989("app-completion-start988.json","wb");if(startup988){smx_log_fprintf989(startup988,"{\"thread_requested_qpc\":%.9f,\"thread_entered_qpc\":%.9f,\"startup_ms\":%.6f,\"ready_before_first_submit\":true,\"gpu_commands_submitted_by_startup\":false}\n",double(completionRequested988.QuadPart)/frequency988.QuadPart,double(completionEntered988.QuadPart)/frequency988.QuadPart,1000.*double(completionEntered988.QuadPart-completionRequested988.QuadPart)/frequency988.QuadPart);smx_log_fclose989(startup988);}
 }
void initializeSnapshot929(unsigned slot){auto&ss=snapshots929[slot];
 if(ss.command)return;VkQueryPoolCreateInfo tq{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};tq.queryType=VK_QUERY_TYPE_TIMESTAMP;tq.queryCount=4;CK(vkCreateQueryPool(device,&tq,nullptr,&ss.timing931));
 for(auto&v:ss.images){
  VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};ii.imageType=VK_IMAGE_TYPE_2D;ii.format=VK_FORMAT_B8G8R8A8_UNORM;ii.extent={g->width,g->height,1};ii.mipLevels=ii.arrayLayers=1;ii.samples=VK_SAMPLE_COUNT_1_BIT;ii.tiling=VK_IMAGE_TILING_OPTIMAL;ii.usage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT|(pixelAudit967()?VK_IMAGE_USAGE_TRANSFER_SRC_BIT:0);ii.sharingMode=VK_SHARING_MODE_EXCLUSIVE;ii.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
  CK(vkCreateImage(device,&ii,nullptr,&v.image));VkMemoryRequirements req{};vkGetImageMemoryRequirements(device,v.image,&req);VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.allocationSize=req.size;ai.memoryTypeIndex=typeFor(g->memory,req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);CK(vkAllocateMemory(device,&ai,nullptr,&v.memory));CK(vkBindImageMemory(device,v.image,v.memory,0));
  VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};vi.image=v.image;vi.viewType=VK_IMAGE_VIEW_TYPE_2D;vi.format=VK_FORMAT_B8G8R8A8_UNORM;vi.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};CK(vkCreateImageView(device,&vi,nullptr,&v.view));
 }
 VkCommandBufferAllocateInfo ca{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ca.commandPool=pool;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=1;CK(vkAllocateCommandBuffers(device,&ca,&ss.command));
}
void recordSnapshot929(unsigned slot){auto&ss=snapshots929[slot];
 initializeSnapshot929(slot);CK(vkResetCommandBuffer(ss.command,0));VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};CK(vkBeginCommandBuffer(ss.command,&begin));vkCmdResetQueryPool(ss.command,ss.timing931,0,4);vkCmdWriteTimestamp(ss.command,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,ss.timing931,0);ImportedImage*src[2]={directFirst,directSecond};
 for(unsigned j=0;j<2;j++){
   if(j==0&&reuseFirst943)continue;
  VkImageMemoryBarrier im{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};im.oldLayout=im.newLayout=VK_IMAGE_LAYOUT_GENERAL;im.srcQueueFamilyIndex=VK_QUEUE_FAMILY_EXTERNAL;im.dstQueueFamilyIndex=g->queueFamily;im.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;im.image=src[j]->image;im.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};vkCmdPipelineBarrier(ss.command,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&im);
  im.oldLayout=ss.initialized[j]?VK_IMAGE_LAYOUT_GENERAL:VK_IMAGE_LAYOUT_UNDEFINED;im.newLayout=VK_IMAGE_LAYOUT_GENERAL;im.srcQueueFamilyIndex=im.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;im.srcAccessMask=ss.initialized[j]?VK_ACCESS_SHADER_READ_BIT:0;im.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;im.image=ss.images[j].image;vkCmdPipelineBarrier(ss.command,ss.initialized[j]?VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT:VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&im);
  VkImageCopy cp{};cp.srcSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};cp.dstSubresource=cp.srcSubresource;cp.extent={g->width,g->height,1};vkCmdCopyImage(ss.command,src[j]->image,VK_IMAGE_LAYOUT_GENERAL,ss.images[j].image,VK_IMAGE_LAYOUT_GENERAL,1,&cp);
  im.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;im.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;vkCmdPipelineBarrier(ss.command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,0,nullptr,0,nullptr,1,&im);
  im.image=src[j]->image;im.srcQueueFamilyIndex=g->queueFamily;im.dstQueueFamilyIndex=VK_QUEUE_FAMILY_EXTERNAL;im.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;im.dstAccessMask=0;vkCmdPipelineBarrier(ss.command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,0,0,nullptr,0,nullptr,1,&im);
  ss.initialized[j]=true;++snapshotImages943;
 }
 pixelSnapshot967(slot);vkCmdWriteTimestamp(ss.command,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,ss.timing931,1);CK(vkEndCommandBuffer(ss.command));
}



 void prepareAsync930(){
  for(unsigned slot=0;slot<2;slot++){
   initializeSnapshot929(slot);auto&j=jobs905[slot];
   if(!j.fence){VkFenceCreateInfo f{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};CK(vkCreateFence(device,&f,nullptr,&j.fence));VkCommandBufferAllocateInfo a{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};a.commandPool=pool;a.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;a.commandBufferCount=2;CK(vkAllocateCommandBuffers(device,&a,j.transfer));}
  }
  smx_log_fprintf989(stderr,"ASYNC_PREALLOC930 private_inputs=4 job_slots=2 before_capture=1 logical_bytes=%llu\n",(unsigned long long)g->width*g->height*16);
 }
 double submitTime930(unsigned slot)const{return double(jobs905[slot].submitQpc.QuadPart)/jobs905[slot].freq.QuadPart;}
 double fenceTime930(unsigned slot)const{return double(fenceQpc906[slot].QuadPart)/jobs905[slot].freq.QuadPart;}

 void setPairKeys943(uint64_t first,uint64_t second){pairFirst943=first;pairSecond943=second;}
 uint64_t reuseCount943()const{return reuseHits943;}
 uint64_t copyCount943()const{return snapshotImages943;}
 bool allSnapshotLayouts944()const{return snapshots929[0].initialized[0]&&snapshots929[0].initialized[1]&&snapshots929[1].initialized[0]&&snapshots929[1].initialized[1];}
 void submit905(unsigned slot){
  if(slot>1||jobs905[slot].busy||!directPair||!directOutput||snapshot651()||probe846||getenv("SMX_AB_FUSION842")||getenv("SMX_AB_BARRIERS839")||getenv("SMX_AB_WARP860"))throw std::runtime_error("Invalid bounded async configuration");
  auto&j=jobs905[slot];diagnosticJob994[slot]=++diagnosticNextJob994;
  smxdiag994::Scope submitScope994(SMX_DIAG_MODEL_SUBMIT994,smxdiag994::fields(pairSecond943,diagnosticJob994[slot],(uintptr_t)directOutput,(uintptr_t)j.fence,0,slot,pairFirst943));
  if(!j.fence){VkFenceCreateInfo f{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};CK(vkCreateFence(device,&f,nullptr,&j.fence));VkCommandBufferAllocateInfo a{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};a.commandPool=pool;a.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;a.commandBufferCount=2;CK(vkAllocateCommandBuffers(device,&a,j.transfer));}
  reuseFirst943=havePrevious943&&previousSlot943!=slot&&pairFirst943!=0&&pairSecond943!=0&&pairFirst943==previousSecond943&&directFirst==previousImage943;
  pixelBegin967(slot);recordSnapshot929(slot);VkImageView views[3]={reuseFirst943?snapshots929[previousSlot943].images[1].view:snapshots929[slot].images[0].view,snapshots929[slot].images[1].view,directOutput->view};
  if(reuseFirst943)++reuseHits943;
  if(smx_graph_update_images(slot,views))throw std::runtime_error("Async descriptor slot missing");
  ImportedImage*items[3]={directFirst,directSecond,directOutput};
  for(unsigned side=0;side<2;side++){
   CK(vkResetCommandBuffer(j.transfer[side],0));VkCommandBufferBeginInfo b{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};CK(vkBeginCommandBuffer(j.transfer[side],&b));if(side==0)vkCmdWriteTimestamp(j.transfer[side],VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,snapshots929[slot].timing931,2);
   if(side==1)pixelOutput967(slot,j.transfer[side]);
   for(unsigned k=2;k<3;k++){VkImageMemoryBarrier im{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};im.oldLayout=im.newLayout=VK_IMAGE_LAYOUT_GENERAL;im.srcQueueFamilyIndex=side?g->queueFamily:VK_QUEUE_FAMILY_EXTERNAL;im.dstQueueFamilyIndex=side?VK_QUEUE_FAMILY_EXTERNAL:g->queueFamily;VkAccessFlags a=k==2?VK_ACCESS_SHADER_WRITE_BIT:VK_ACCESS_SHADER_READ_BIT;im.srcAccessMask=side?(a|(pixelJobs967[slot].active?VK_ACCESS_TRANSFER_READ_BIT:0)):0;im.dstAccessMask=side?0:a;im.image=items[k]->image;im.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};vkCmdPipelineBarrier(j.transfer[side],side?(pixelJobs967[slot].active?VK_PIPELINE_STAGE_ALL_COMMANDS_BIT:VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT):VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,side?VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT:VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,0,nullptr,0,nullptr,1,&im);}
   if(side==1)vkCmdWriteTimestamp(j.transfer[side],VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,snapshots929[slot].timing931,3);CK(vkEndCommandBuffer(j.transfer[side]));
  }
  VkDeviceMemory syncs[3]={directFirst->memory,directSecond->memory,directOutput->memory};uint64_t keys[3]{};uint32_t timeouts[3]={5000,5000,5000};
  VkWin32KeyedMutexAcquireReleaseInfoKHR keyed{VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR};keyed.acquireCount=keyed.releaseCount=3;keyed.pAcquireSyncs=keyed.pReleaseSyncs=syncs;keyed.pAcquireKeys=keyed.pReleaseKeys=keys;keyed.pAcquireTimeouts=timeouts;
  // R942 return original textures immediately after private snapshot.
  // Both external ownership transfer barriers and keyed mutex synchronization remain.
  VkWin32KeyedMutexAcquireReleaseInfoKHR inputKeyed942=keyed;
  inputKeyed942.acquireCount=inputKeyed942.releaseCount=reuseFirst943?1:2;
  if(reuseFirst943){inputKeyed942.pAcquireSyncs=inputKeyed942.pReleaseSyncs=syncs+1;inputKeyed942.pAcquireKeys=inputKeyed942.pReleaseKeys=keys+1;inputKeyed942.pAcquireTimeouts=timeouts+1;}
  VkSubmitInfo inputSubmit942{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  inputSubmit942.pNext=&inputKeyed942;inputSubmit942.commandBufferCount=1;
  inputSubmit942.pCommandBuffers=&snapshots929[slot].command;
  keyed.acquireCount=keyed.releaseCount=1;
  keyed.pAcquireSyncs=keyed.pReleaseSyncs=syncs+2;
  keyed.pAcquireKeys=keyed.pReleaseKeys=keys+2;keyed.pAcquireTimeouts=timeouts+2;
  VkCommandBuffer compute942[3]={j.transfer[0],smx_graph_core839(slot,0),j.transfer[1]};
  VkSubmitInfo modelSubmit942{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  modelSubmit942.pNext=&keyed;modelSubmit942.commandBufferCount=3;modelSubmit942.pCommandBuffers=compute942;
  VkSubmitInfo submissions942[2]={inputSubmit942,modelSubmit942};

  const bool nativeJob948=bool(directOutput->sync948);
  std::vector<NativeSync948*> states948;
  std::vector<std::unique_lock<std::mutex>> locks948;
  VkSemaphore sems948[3]{};
  uint64_t waits948[3]{},signals948[3]{};
  VkPipelineStageFlags stages948[3]={VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
  VkTimelineSemaphoreSubmitInfo timeline948[2]={{VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO},{VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO}};
  unsigned first948=reuseFirst943?1:0;
  if(nativeJob948){
   for(unsigned k=first948;k<3;k++){
    if(!items[k]->sync948)throw std::runtime_error("R948 mixed synchronization backends");
    states948.push_back(items[k]->sync948.get());
   }
   std::sort(states948.begin(),states948.end(),std::less<NativeSync948*>{});
   if(std::adjacent_find(states948.begin(),states948.end())!=states948.end())throw std::runtime_error("R948 duplicate transaction resource");
   for(auto*p:states948)locks948.emplace_back(p->cpu);
   for(unsigned k=first948;k<3;k++){
    auto&v=*items[k]->sync948;sems948[k]=v.semaphore;waits948[k]=v.value;signals948[k]=v.value+1;
   }
   for(unsigned k=0;k<2;k++){
    unsigned start=k?2:first948,count=k?1:2-first948;
    auto&t=timeline948[k];t.waitSemaphoreValueCount=t.signalSemaphoreValueCount=count;
    t.pWaitSemaphoreValues=waits948+start;t.pSignalSemaphoreValues=signals948+start;
    auto&v=submissions942[k];v.pNext=&t;v.waitSemaphoreCount=v.signalSemaphoreCount=count;
    v.pWaitSemaphores=v.pSignalSemaphores=sems948+start;v.pWaitDstStageMask=stages948+start;
   }
  }else if(directFirst->sync948||directSecond->sync948)throw std::runtime_error("R948 mixed synchronization backends");

  startCompletion906();done906[slot]=false;QueryPerformanceFrequency(&j.freq);QueryPerformanceCounter(&j.submitQpc);j.start=Clock::now();CK(submitQueue958(queue,2,submissions942,j.fence));if(nativeJob948){for(unsigned k=first948;k<3;k++)items[k]->sync948->value=signals948[k];++nativeSubmits948;}locks948.clear();havePrevious943=pairSecond943!=0;previousSecond943=pairSecond943;previousImage943=directSecond;previousSlot943=slot;j.submitted=Clock::now();j.submitMs=elapsed(j.start,j.submitted);j.busy=true;{std::lock_guard lock(completeMutex906);QueryPerformanceCounter(&completionEnqueued988[slot]);completeQueue906.push_back(slot);}completeCv906.notify_all();submitScope994.end();
 }
 bool ready905(unsigned slot){if(slot>1||!jobs905[slot].busy)throw std::runtime_error("Missing async job");if(!done906[slot].load(std::memory_order_acquire))return false;CK(result906[slot]);return true;}
 double finish905(unsigned slot){
 smxdiag994::Scope finishScope994(SMX_DIAG_MODEL_COMPLETE994,smxdiag994::fields(0,slot<2?diagnosticJob994[slot]:0,(uintptr_t)device,0,0,slot));
  if(slot>1||!jobs905[slot].busy)throw std::runtime_error("Missing async completion");auto&j=jobs905[slot];
  
  {std::unique_lock lock(completeMutex906);if(!completeCv906.wait_for(lock,std::chrono::milliseconds(5100),[&]{return done906[slot].load(std::memory_order_acquire);}))throw std::runtime_error("Async completion notification timeout; no retry");}CK(result906[slot]);
  pixelFinish967(slot);auto returned=Clock::now();LARGE_INTEGER hostReturn{};QueryPerformanceCounter(&hostReturn);
  auto&times=queryTimes;CK(queryDiag994(device,smx_graph_queries905(slot),0,g->queryCount,times.size()*sizeof(uint64_t),times.data(),sizeof(uint64_t),VK_QUERY_RESULT_64_BIT));CK(vkResetFences(device,1,&j.fence));
  vkSubmitMs=j.submitMs;vkFenceMs=elapsed(j.submitted,returned);queryMs=elapsed(returned,Clock::now());outputLockMs=outputSubmitMs=0;
  double model=(times.back()-times.front())*g->timestampPeriod/1e6;
  double begin=0,end=0;uint64_t deviation=0;
  if(calibrated737){VkCalibratedTimestampInfoEXT ci[2]={{VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT},{VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT}};ci[0].timeDomain=VK_TIME_DOMAIN_DEVICE_EXT;ci[1].timeDomain=VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT;uint64_t cv[2]{};CK(calibrated737(device,2,ci,cv,&deviation));begin=double(cv[1])/j.freq.QuadPart+double(int64_t(times.front()-cv[0]))*g->timestampPeriod/1e9;end=double(cv[1])/j.freq.QuadPart+double(int64_t(times.back()-cv[0]))*g->timestampPeriod/1e9;}
  if(!asyncTrace905){asyncTrace905=smx_log_fopen989("app-async905.csv","wb");if(asyncTrace905)smx_log_fprintf989(asyncTrace905,"job,slot,submit_qpc,return_qpc,model_begin_qpc,model_end_qpc,model_ms,service_ms,submit_ms,query_ms,calibration_deviation_ns,fence_notify_qpc,notify_to_consume_ms,completion_worker_entered_qpc,completion_enqueued_qpc,completion_dequeued_qpc,fence_wait_begin_qpc,fence_wait_ms\n");}
  queueWaitMs988=(begin>0&&deviation<=1000000&&begin>=double(j.submitQpc.QuadPart)/j.freq.QuadPart&&begin-double(j.submitQpc.QuadPart)/j.freq.QuadPart<5)?1000*(begin-double(j.submitQpc.QuadPart)/j.freq.QuadPart):-1;
  if(asyncTrace905)smx_log_fprintf989(asyncTrace905,"%u,%u,%.9f,%.9f,%.9f,%.9f,%.6f,%.6f,%.6f,%.6f,%llu,%.9f,%.6f,%.9f,%.9f,%.9f,%.9f,%.6f\n",++asyncId905,slot,double(j.submitQpc.QuadPart)/j.freq.QuadPart,double(hostReturn.QuadPart)/j.freq.QuadPart,begin,end,model,elapsed(j.start,returned),vkSubmitMs,queryMs,(unsigned long long)deviation,double(fenceQpc906[slot].QuadPart)/j.freq.QuadPart,1000.*double(hostReturn.QuadPart-fenceQpc906[slot].QuadPart)/j.freq.QuadPart,double(completionEntered988.QuadPart)/j.freq.QuadPart,double(completionEnqueued988[slot].QuadPart)/j.freq.QuadPart,double(completionDequeued988[slot].QuadPart)/j.freq.QuadPart,double(fenceWaitBegin988[slot].QuadPart)/j.freq.QuadPart,1000.*double(fenceQpc906[slot].QuadPart-fenceWaitBegin988[slot].QuadPart)/j.freq.QuadPart);
  uint64_t stage931[4]{};CK(queryDiag994(device,snapshots929[slot].timing931,0,4,sizeof(stage931),stage931,sizeof(uint64_t),VK_QUERY_RESULT_64_BIT));
  if(!stageTrace931){stageTrace931=smx_log_fopen989("app-stages931.csv","wb");if(stageTrace931)smx_log_fprintf989(stageTrace931,"job,slot,submit_qpc,snapshot_begin_qpc,snapshot_end_qpc,output_acquire_begin_qpc,model_begin_qpc,model_end_qpc,release_end_qpc,fence_notify_qpc,deviation_ns\n");}
  auto stamp931=[&](uint64_t t){return begin?begin+double(int64_t(t-times.front()))*g->timestampPeriod/1e9:0;};
  if(stageTrace931)smx_log_fprintf989(stageTrace931,"%u,%u,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%llu\n",asyncId905,slot,double(j.submitQpc.QuadPart)/j.freq.QuadPart,stamp931(stage931[0]),stamp931(stage931[1]),stamp931(stage931[2]),begin,end,stamp931(stage931[3]),double(fenceQpc906[slot].QuadPart)/j.freq.QuadPart,(unsigned long long)deviation);
  if(getenv("SMX_BLOCK851")){if(times.size()!=14)throw std::runtime_error("Invalid coarse timestamp count");if(!blockTrace851){blockTrace851=smx_log_fopen989("app-block851.csv","wb");if(blockTrace851)smx_log_fprintf989(blockTrace851,"frame,qpc,preprocess,feature_convs,attention1_a,attention1_b,attention1_c,attention1_d,bridge_conv,attention2_a,attention2_b,attention2_c,attention2_d,decode_convs,warp,total_gpu_ms,fence_ms\n");}if(blockTrace851){smx_log_fprintf989(blockTrace851,"%u,%.9f",++blockId851,double(hostReturn.QuadPart)/j.freq.QuadPart);for(unsigned k=0;k<13;k++)smx_log_fprintf989(blockTrace851,",%.6f",(times[k+1]-times[k])*g->timestampPeriod/1e6);smx_log_fprintf989(blockTrace851,",%.6f,%.6f\n",model,vkFenceMs);}}
  j.busy=false;finishScope994.end();return model;
 }
 double infer(){
 queueWaitMs988=-1;
  if(seen<2)throw std::runtime_error("Capture frame missing");
  if(probe846)packet847=isolatedFence848||!getenv("SMX_PROBE_AB847")||((probeId846%4)==0||(probeId846%4)==3);
  if(directPair)recordDirectTransfers();
  VkDeviceMemory syncs[3]={directPair?directFirst->memory:images[0].memory,directPair?directSecond->memory:images[1].memory,outputImage().memory};uint64_t keys[3]={0,0,0};uint32_t timeouts[3]={5000,5000,5000};
  VkWin32KeyedMutexAcquireReleaseInfoKHR keyed{VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR};keyed.acquireCount=3;keyed.pAcquireSyncs=syncs;keyed.pAcquireKeys=keys;keyed.pAcquireTimeouts=timeouts;keyed.releaseCount=3;keyed.pReleaseSyncs=syncs;keyed.pReleaseKeys=keys;
  bool ab839=getenv("SMX_AB_WARP860")!=nullptr||getenv("SMX_AB_BARRIERS839")!=nullptr||getenv("SMX_AB_FUSION842")!=nullptr;unsigned phase839=(abId839++)%4,baseline839=ab839&&(phase839==0||phase839==3);
  VkCommandBuffer commands[3]={transfer[0],smx_graph_core839(directPair?0:(newest==1?0:1),baseline839),transfer[1]};VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};si.pNext=&keyed;si.commandBufferCount=3;si.pCommandBuffers=commands;
  LARGE_INTEGER hostSubmit737{},qpf737{};QueryPerformanceFrequency(&qpf737);QueryPerformanceCounter(&hostSubmit737);
 bool snap=snapshot651();double inputWait651=0,inputGpu651=0;auto i0=Clock::now();
   if(probe846&&packet847){if(!directPair)throw std::runtime_error("Probe requires direct retained pair");VkSubmitInfo ps{VK_STRUCTURE_TYPE_SUBMIT_INFO};ps.commandBufferCount=1;ps.pCommandBuffers=&probeCommand846;auto pw848=Clock::now();CK(submitQueue958(queue,1,&ps,isolatedFence848));if(isolatedFence848){CK(SMX_DIAG_CALL994(SMX_DIAG_MODEL_FENCE994,smxdiag994::fields(0,0,(uintptr_t)device,(uintptr_t)isolatedFence848,0,5000000000ll,2),SMX_DIAG_VKRESULT994,vkWaitForFences(device,1,&isolatedFence848,VK_TRUE,5000000000ull)));CK(vkResetFences(device,1,&isolatedFence848));}isolatedWait848=isolatedFence848?elapsed(pw848,Clock::now()):0;}
if(snap){VkSubmitInfo inputSubmit=si;inputSubmit.commandBufferCount=1;inputSubmit.pCommandBuffers=&inputStage651;VkWin32KeyedMutexAcquireReleaseInfoKHR inputKeyed=keyed;inputKeyed.acquireCount=inputKeyed.releaseCount=2;inputSubmit.pNext=&inputKeyed;CK(submitQueue958(queue,1,&inputSubmit,inputFence651));keyed.acquireCount=keyed.releaseCount=1;keyed.pAcquireSyncs=syncs+2;keyed.pReleaseSyncs=syncs+2;keyed.pAcquireKeys=keys+2;keyed.pReleaseKeys=keys+2;keyed.pAcquireTimeouts=timeouts+2;}
 CK(submitQueue958(queue,1,&si,fence));auto i1=Clock::now();if(snap){CK(SMX_DIAG_CALL994(SMX_DIAG_MODEL_FENCE994,smxdiag994::fields(0,0,(uintptr_t)device,(uintptr_t)inputFence651,0,5000000000ll,2),SMX_DIAG_VKRESULT994,vkWaitForFences(device,1,&inputFence651,VK_TRUE,5000000000ull)));inputWait651=elapsed(i1,Clock::now());}
 CK(SMX_DIAG_CALL994(SMX_DIAG_MODEL_FENCE994,smxdiag994::fields(0,0,(uintptr_t)device,(uintptr_t)fence,0,5000000000ll,2),SMX_DIAG_VKRESULT994,vkWaitForFences(device,1,&fence,VK_TRUE,5000000000ull)));auto i2=Clock::now();vkSubmitMs=elapsed(i0,i1);vkFenceMs=elapsed(i1,i2);
 if(snap){uint64_t q[2]{};CK(queryDiag994(device,inputQueries651,0,2,sizeof(q),q,sizeof(uint64_t),VK_QUERY_RESULT_64_BIT));inputGpu651=(q[1]-q[0])*g->timestampPeriod/1e6;CK(vkResetFences(device,1,&inputFence651));}
 auto&times=queryTimes;CK(queryDiag994(device,smx_graph_queries905(directPair?0:(newest==1?0:1)),0,g->queryCount,times.size()*sizeof(uint64_t),times.data(),sizeof(uint64_t),VK_QUERY_RESULT_64_BIT|VK_QUERY_RESULT_WAIT_BIT));CK(vkResetFences(device,1,&fence));queryMs=elapsed(i2,Clock::now());
  if(getenv("SMX_PROFILE_PHASES")){static bool emitted=false;if(!emitted&&g->queryCount>2){emitted=true;smx_log_fprintf989(stderr,"INTEROP_NODE_MS [");for(size_t i=0;i+1<times.size();i+=2)smx_log_fprintf989(stderr,"%s%.6f",i?",":"",(times[i+1]-times[i])*g->timestampPeriod/1e6);smx_log_fprintf989(stderr,"]\n");}}
    if(getenv("SMX_FINE_TIMING")){ static unsigned id=0;FILE*f=smx_log_fopen989("app-phases.csv",id?"ab":"wb");if(f){LARGE_INTEGER q,fr;QueryPerformanceCounter(&q);QueryPerformanceFrequency(&fr);smx_log_fprintf989(f,"%u,%.9f",++id,double(q.QuadPart)/fr.QuadPart);for(size_t n=0;n+1<times.size();n+=2)smx_log_fprintf989(f,",%.6f",(times[n+1]-times[n])*g->timestampPeriod/1e6);smx_log_fprintf989(f,"\n");smx_log_fclose989(f);} }
  double modelGpu651=(times.back()-times.front())*g->timestampPeriod/1e6;
 if(getenv("SMX_BLOCK851")){if(times.size()!=14)throw std::runtime_error("Invalid coarse timestamp count");if(!blockTrace851){blockTrace851=smx_log_fopen989("app-block851.csv","wb");if(blockTrace851)smx_log_fprintf989(blockTrace851,"frame,qpc,preprocess,feature_convs,attention1_a,attention1_b,attention1_c,attention1_d,bridge_conv,attention2_a,attention2_b,attention2_c,attention2_d,decode_convs,warp,total_gpu_ms,fence_ms\n");}if(blockTrace851){LARGE_INTEGER q,f;QueryPerformanceCounter(&q);QueryPerformanceFrequency(&f);smx_log_fprintf989(blockTrace851,"%u,%.9f",++blockId851,double(q.QuadPart)/f.QuadPart);for(unsigned j=0;j<13;j++)smx_log_fprintf989(blockTrace851,",%.6f",(times[j+1]-times[j])*g->timestampPeriod/1e6);smx_log_fprintf989(blockTrace851,",%.6f,%.6f\n",modelGpu651,vkFenceMs);}}

   if(probe846){uint64_t pq[5]{};CK(queryDiag994(device,probeQueries846,0,5,sizeof(pq),pq,sizeof(uint64_t),VK_QUERY_RESULT_64_BIT));LARGE_INTEGER h{};QueryPerformanceCounter(&h);
    if(!probeTrace846){probeTrace846=smx_log_fopen989("app-queue846.csv","wb");if(probeTrace846)smx_log_fprintf989(probeTrace846,"frame,snapshot,probe_packet,submit_qpc,queue_probe_qpc,acquire_begin_qpc,acquire_end_qpc,model_begin_qpc,model_end_qpc,release_begin_qpc,release_end_qpc,host_fence_qpc,anchor_deviation_ns,input_copy_gpu_ms,isolated_probe_wait_ms\n");}
    if(probeTrace846)smx_log_fprintf989(probeTrace846,"%u,%u,%u,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%llu,%.6f,%.6f\n",++probeId846,unsigned(snap),unsigned(packet847),double(hostSubmit737.QuadPart)/qpf737.QuadPart,gpuQpc846(pq[0]),gpuQpc846(pq[1]),gpuQpc846(pq[2]),gpuQpc846(times.front()),gpuQpc846(times.back()),gpuQpc846(pq[3]),gpuQpc846(pq[4]),double(h.QuadPart)/qpf737.QuadPart,(unsigned long long)anchorDeviation846,inputGpu651,isolatedWait848);
   }

  if(ab839){if(!abTrace839){abTrace839=smx_log_fopen989(getenv("SMX_AB_WARP860")?"app-ab860.csv":getenv("SMX_AB_FUSION842")?"app-ab842.csv":"app-ab839.csv","wb");if(abTrace839)smx_log_fprintf989(abTrace839,"frame,baseline_variant,model_gpu_ms,fence_ms,submit_ms,query_ms\n");}if(abTrace839)smx_log_fprintf989(abTrace839,"%u,%u,%.6f,%.6f,%.6f,%.6f\n",abId839,baseline839,modelGpu651,vkFenceMs,vkSubmitMs,queryMs);}
 if(calibrated737){VkCalibratedTimestampInfoEXT ci737[2]={{VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT},{VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT}};ci737[0].timeDomain=VK_TIME_DOMAIN_DEVICE_EXT;ci737[1].timeDomain=VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT;uint64_t calibratedValues737[2]{},deviation737=0;LARGE_INTEGER hostReturn737{};QueryPerformanceCounter(&hostReturn737);auto calBegin833=Clock::now();CK(calibrated737(device,2,ci737,calibratedValues737,&deviation737));double calibrationWall833=elapsed(calBegin833,Clock::now());double hostCalibration737=double(calibratedValues737[1])/qpf737.QuadPart;double begin737=hostCalibration737+double(int64_t(times.front()-calibratedValues737[0]))*g->timestampPeriod/1e9;double end737=hostCalibration737+double(int64_t(times.back()-calibratedValues737[0]))*g->timestampPeriod/1e9;if(!calTrace737){calTrace737=smx_log_fopen989("app-calibrated737.csv","wb");if(calTrace737)smx_log_fprintf989(calTrace737,"infer,submit_host_qpc,model_begin_est_qpc,model_end_est_qpc,host_return_qpc,pre_model_span_ms,post_model_span_ms,max_deviation_ns,timestamp_period_ns,calibration_call_wall_ms\n");}if(calTrace737)smx_log_fprintf989(calTrace737,"%u,%.9f,%.9f,%.9f,%.9f,%.6f,%.6f,%llu,%.9f,%.6f\n",++calId737,double(hostSubmit737.QuadPart)/qpf737.QuadPart,begin737,end737,double(hostReturn737.QuadPart)/qpf737.QuadPart,(begin737-double(hostSubmit737.QuadPart)/qpf737.QuadPart)*1000,(double(hostReturn737.QuadPart)/qpf737.QuadPart-end737)*1000,(unsigned long long)deviation737,double(g->timestampPeriod),calibrationWall833);}

 if(!interopTrace651){interopTrace651=smx_log_fopen989("app-interop651.csv","wb");if(interopTrace651)smx_log_fprintf989(interopTrace651,"infer,snapshot,submit_ms,total_fence_wait_ms,input_ready_wait_ms,input_copy_gpu_ms,model_gpu_span_ms,query_ms\n");}
 if(interopTrace651)smx_log_fprintf989(interopTrace651,"%u,%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",++interopId651,unsigned(snap),vkSubmitMs,vkFenceMs,inputWait651,inputGpu651,modelGpu651,queryMs);
 return modelGpu651;
 }
 ComPtr<ID3D11Texture2D> makeOriginalTexture(){if(smxDirectEnabled())return makeRetainedTexture();auto t=makePresentationTexture(true);return t;}
ComPtr<ID3D11Texture2D> makePresentationTexture(bool bgra=false){
  D3D11_TEXTURE2D_DESC td{};td.Width=g->width;td.Height=g->height;td.MipLevels=1;td.ArraySize=1;td.Format=DXGI_FORMAT_B8G8R8A8_UNORM;td.BindFlags=D3D11_BIND_SHADER_RESOURCE;td.SampleDesc.Count=1;td.Usage=D3D11_USAGE_DEFAULT;
  ComPtr<ID3D11Texture2D>t;HR(d3d->CreateTexture2D(&td,nullptr,&t));return t;
 }
 void copyMiddle(ID3D11Texture2D*middle){auto c0=Clock::now();auto&out=outputImage();if(middle==out.texture.Get()){outputLockMs=outputSubmitMs=0;return;}BridgeLock b(out.mutex.Get());auto c1=Clock::now();outputLockMs=elapsed(c0,c1);ctx->CopyResource(middle,outputImage().texture.Get());ctx->Flush();outputSubmitMs=elapsed(c1,Clock::now());}
 void copyPair(ID3D11Texture2D*first,ID3D11Texture2D*middle){
  auto c0=Clock::now();BridgeLock a(directPair?directFirst->mutex.Get():images[newest^1u].mutex.Get()),b(outputImage().mutex.Get());auto c1=Clock::now();outputLockMs=elapsed(c0,c1);
  ctx->CopyResource(first,directPair?directFirst->texture.Get():images[newest^1u].texture.Get());if(middle!=outputImage().texture.Get())ctx->CopyResource(middle,outputImage().texture.Get());ctx->Flush();outputSubmitMs=elapsed(c1,Clock::now());
 }
};
#undef BRIDGE_FUNCTIONS
#undef CK
#undef HR
