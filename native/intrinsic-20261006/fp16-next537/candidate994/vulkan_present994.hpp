#pragma once
#include <vulkan/vulkan.h>
#include "integration994.hpp"
#include <windows.h>
#include <vector>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
class VulkanPresenter958 {
#define WP_DEV958(X) X(vkCreateSwapchainKHR) X(vkDestroySwapchainKHR) X(vkGetSwapchainImagesKHR) X(vkAcquireNextImageKHR) X(vkQueuePresentKHR) X(vkCreateCommandPool) X(vkDestroyCommandPool) X(vkAllocateCommandBuffers) X(vkResetCommandBuffer) X(vkBeginCommandBuffer) X(vkEndCommandBuffer) X(vkCmdPipelineBarrier) X(vkCmdCopyImage) X(vkCmdCopyImageToBuffer) X(vkCmdCopyBufferToImage) X(vkCreateFence) X(vkDestroyFence) X(vkResetFences) X(vkWaitForFences) X(vkGetFenceStatus) X(vkCreateSemaphore) X(vkDestroySemaphore) X(vkQueueSubmit) X(vkDeviceWaitIdle) X(vkCreateBuffer) X(vkDestroyBuffer) X(vkGetBufferMemoryRequirements) X(vkAllocateMemory) X(vkFreeMemory) X(vkBindBufferMemory) X(vkMapMemory) X(vkUnmapMemory) X(vkCreateQueryPool) X(vkDestroyQueryPool) X(vkCmdResetQueryPool) X(vkCmdWriteTimestamp) X(vkGetQueryPoolResults)
#define WP_INST958(X) X(vkCreateWin32SurfaceKHR) X(vkDestroySurfaceKHR) X(vkGetPhysicalDeviceSurfaceSupportKHR) X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) X(vkGetPhysicalDeviceSurfaceFormatsKHR) X(vkGetPhysicalDeviceMemoryProperties)
#define DECL958(n) PFN_##n n{};
 WP_DEV958(DECL958) WP_INST958(DECL958)
#undef DECL958
 VkInstance instance{};VkPhysicalDevice physical{};VkDevice device{};VkQueue queue{};uint32_t family=0,width=0,height=0;float timestampPeriod=0;
 std::mutex&queueMutex;VkSurfaceKHR surface{};VkSwapchainKHR swapchain{};VkCommandPool pool{};FILE*trace=nullptr;
 struct Slot{VkCommandBuffer command{};VkFence fence{};VkSemaphore acquire{};std::shared_ptr<void>lease;bool busy=false;uint64_t sequence=0;double submitQpc=0;uint32_t probeIndex976=UINT32_MAX;VkQueryPool queries{};VkBuffer marker{};VkDeviceMemory markerMemory{};unsigned char*markerMap=nullptr;};
 Slot slots[3]{};std::vector<VkImage>images;std::vector<VkSemaphore>complete;uint32_t cursor=0,preparedImage=0;bool prepared=false;uint64_t sequence=0;bool marker967=false;
 bool diagnosticReadback967=true;
 VkBuffer readback{};VkDeviceMemory readbackMemory{};unsigned char*mapped=nullptr;
 #include "pixel_probe976.inc"
 static void ck(VkResult r,const char*s){if(r!=VK_SUCCESS){smxdiag994::error(SMX_DIAG_EXCEPTION_CAUGHT994,r,SMX_DIAG_VKRESULT994);throw std::runtime_error(std::string(s)+" VkResult="+std::to_string(int(r)));}}
 static double now(){LARGE_INTEGER q{},f{};QueryPerformanceCounter(&q);QueryPerformanceFrequency(&f);return double(q.QuadPart)/f.QuadPart;}
 void retire(Slot&s){
  if(!s.busy)return;ck(SMX_DIAG_CALL994(SMX_DIAG_MODEL_FENCE994,smxdiag994::fields(s.sequence,0,(uintptr_t)device,(uintptr_t)s.fence,0,2000000000ll,3),SMX_DIAG_VKRESULT994,vkWaitForFences(device,1,&s.fence,VK_TRUE,2000000000ull)),"WSI copy fence");
  uint64_t ticks[2]{};VkResult qr=vkGetQueryPoolResults(device,s.queries,0,2,sizeof(ticks),ticks,sizeof(uint64_t),VK_QUERY_RESULT_64_BIT);
  if(trace)fprintf(trace,"copy_complete,%llu,%.9f,%.9f,%.6f,%d,0\n",(unsigned long long)s.sequence,s.submitQpc,now(),qr==VK_SUCCESS?double(ticks[1]-ticks[0])*timestampPeriod/1e6:-1,int(qr));
  if(s.probeIndex976!=UINT32_MAX)probe976.frames[s.probeIndex976].completed=true;
  s.busy=false;s.lease.reset();
 }
public:
 struct Source{VkImage image{};VkSemaphore semaphore{};uint64_t*value=nullptr;std::mutex*mutex=nullptr;bool external=true;bool generated=false;};
 VulkanPresenter958(VkInstance in,VkPhysicalDevice ph,VkDevice dev,VkQueue q,uint32_t fam,float period,PFN_vkGetInstanceProcAddr ip,PFN_vkGetDeviceProcAddr dp,std::mutex&qm,HWND hwnd,uint32_t w,uint32_t h,bool verify=false):instance(in),physical(ph),device(dev),queue(q),family(fam),width(w),height(h),timestampPeriod(period),queueMutex(qm){
  try{
  configureProbe976();
#define LOAD_D958(n) n=(PFN_##n)dp(device,#n);if(!n)throw std::runtime_error(#n);
#define LOAD_I958(n) n=(PFN_##n)ip(instance,#n);if(!n)throw std::runtime_error(#n);
  WP_DEV958(LOAD_D958) WP_INST958(LOAD_I958)
#undef LOAD_D958
#undef LOAD_I958
  VkWin32SurfaceCreateInfoKHR sc{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};sc.hinstance=GetModuleHandleW(nullptr);sc.hwnd=hwnd;ck(vkCreateWin32SurfaceKHR(instance,&sc,nullptr,&surface),"WSI surface");
  VkBool32 supported=0;ck(vkGetPhysicalDeviceSurfaceSupportKHR(physical,family,surface,&supported),"WSI support");if(!supported)throw std::runtime_error("Model queue cannot present");
  VkSurfaceCapabilitiesKHR caps{};ck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical,surface,&caps),"WSI capabilities");
  if(caps.currentExtent.width!=width||caps.currentExtent.height!=height)throw std::runtime_error("Output window differs from native image extent");
  if(!(caps.supportedUsageFlags&VK_IMAGE_USAGE_TRANSFER_DST_BIT)||((verify||probe976.enabled)&&!(caps.supportedUsageFlags&VK_IMAGE_USAGE_TRANSFER_SRC_BIT)))throw std::runtime_error("WSI transfer usage unavailable");
  uint32_t nf=0;ck(vkGetPhysicalDeviceSurfaceFormatsKHR(physical,surface,&nf,nullptr),"WSI formats");std::vector<VkSurfaceFormatKHR>formats(nf);ck(vkGetPhysicalDeviceSurfaceFormatsKHR(physical,surface,&nf,formats.data()),"WSI formats");
  bool format=false;for(auto&f:formats)if(f.format==VK_FORMAT_B8G8R8A8_UNORM&&f.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)format=true;if(!format)throw std::runtime_error("WSI exact BGRA UNORM unavailable");
  VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};ci.surface=surface;ci.minImageCount=std::max(3u,caps.minImageCount);if(caps.maxImageCount)ci.minImageCount=std::min(ci.minImageCount,caps.maxImageCount);ci.imageFormat=VK_FORMAT_B8G8R8A8_UNORM;ci.imageColorSpace=VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;ci.imageExtent={width,height};ci.imageArrayLayers=1;ci.imageUsage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|((verify||probe976.enabled)?VK_IMAGE_USAGE_TRANSFER_SRC_BIT:0);ci.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE;ci.preTransform=caps.currentTransform;ci.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;if(!(caps.supportedCompositeAlpha&ci.compositeAlpha))throw std::runtime_error("Opaque WSI unsupported");ci.presentMode=VK_PRESENT_MODE_FIFO_KHR;ci.clipped=VK_TRUE;
  ck(vkCreateSwapchainKHR(device,&ci,nullptr,&swapchain),"WSI swapchain");
  uint32_t count=0;ck(vkGetSwapchainImagesKHR(device,swapchain,&count,nullptr),"WSI images");images.resize(count);complete.resize(count);ck(vkGetSwapchainImagesKHR(device,swapchain,&count,images.data()),"WSI images");
  {FILE*j=fopen("app-wsi-config976.json","wb");if(j){fprintf(j,"{\n\"revision\":976,\"scope\":\"requested VkSwapchainCreateInfoKHR accepted by successful creation; not scanout/MPO proof\",\"createSucceeded\":true,\"imageFormat\":%u,\"imageColorSpace\":%u,\"compositeAlpha\":%u,\"supportedCompositeAlpha\":%u,\"presentMode\":%u,\"preTransform\":%u,\"imageUsage\":%u,\"supportedUsageFlags\":%u,\"width\":%u,\"height\":%u,\"requestedMinImageCount\":%u,\"returnedImageCount\":%u,\"surfaceMinImageCount\":%u,\"surfaceMaxImageCount\":%u,\"queueFamily\":%u,\"pixelProbeEnabled\":%s,\"verifyReadbackEnabled\":%s\n}\n",uint32_t(ci.imageFormat),uint32_t(ci.imageColorSpace),uint32_t(ci.compositeAlpha),uint32_t(caps.supportedCompositeAlpha),uint32_t(ci.presentMode),uint32_t(ci.preTransform),uint32_t(ci.imageUsage),uint32_t(caps.supportedUsageFlags),width,height,ci.minImageCount,count,caps.minImageCount,caps.maxImageCount,family,probe976.enabled?"true":"false",verify?"true":"false");fclose(j);}fprintf(stderr,"WSI_CONFIG976 create_success=1 format=%u colorspace=%u composite_alpha=%u supported_alpha=%u present_mode=%u transform=%u usage=%u extent=%ux%u images=%u probe=%u not_scanout_proof=1\n",uint32_t(ci.imageFormat),uint32_t(ci.imageColorSpace),uint32_t(ci.compositeAlpha),uint32_t(caps.supportedCompositeAlpha),uint32_t(ci.presentMode),uint32_t(ci.preTransform),uint32_t(ci.imageUsage),width,height,count,unsigned(probe976.enabled));}
  allocateProbe976();
  VkCommandPoolCreateInfo pc{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};pc.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;pc.queueFamilyIndex=family;ck(vkCreateCommandPool(device,&pc,nullptr,&pool),"WSI command pool");
  VkSemaphoreCreateInfo sem{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};for(auto&s:complete)ck(vkCreateSemaphore(device,&sem,nullptr,&s),"WSI present semaphore");
  for(auto&s:slots){VkCommandBufferAllocateInfo ca{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ca.commandPool=pool;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=1;ck(vkAllocateCommandBuffers(device,&ca,&s.command),"WSI command");VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};ck(vkCreateFence(device,&fi,nullptr,&s.fence),"WSI fence");ck(vkCreateSemaphore(device,&sem,nullptr,&s.acquire),"WSI acquire semaphore");VkQueryPoolCreateInfo qi{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};qi.queryType=VK_QUERY_TYPE_TIMESTAMP;qi.queryCount=2;ck(vkCreateQueryPool(device,&qi,nullptr,&s.queries),"WSI queries");}
  marker967=getenv("SMX_MARKER967")&&strcmp(getenv("SMX_MARKER967"),"1")==0;
  if(marker967){
   if(width<144||height<16)throw std::runtime_error("Marker extent too small");
   VkPhysicalDeviceMemoryProperties mp{};vkGetPhysicalDeviceMemoryProperties(physical,&mp);
   for(auto&slot:slots){
    VkBufferCreateInfo bc{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};bc.size=128*4*4;bc.usage=VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    ck(vkCreateBuffer(device,&bc,nullptr,&slot.marker),"Marker buffer");VkMemoryRequirements rq{};vkGetBufferMemoryRequirements(device,slot.marker,&rq);
    uint32_t mt=UINT32_MAX;for(uint32_t k=0;k<mp.memoryTypeCount;k++)if((rq.memoryTypeBits&(1u<<k))&&(mp.memoryTypes[k].propertyFlags&(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))==(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)){mt=k;break;}
    if(mt==UINT32_MAX)throw std::runtime_error("Marker coherent memory unavailable");
    VkMemoryAllocateInfo ma{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ma.allocationSize=rq.size;ma.memoryTypeIndex=mt;ck(vkAllocateMemory(device,&ma,nullptr,&slot.markerMemory),"Marker allocation");ck(vkBindBufferMemory(device,slot.marker,slot.markerMemory,0),"Marker bind");
    void*p=nullptr;ck(vkMapMemory(device,slot.markerMemory,0,VK_WHOLE_SIZE,0,&p),"Marker map");slot.markerMap=(unsigned char*)p;
   }
   fprintf(stderr,"MARKER967 enabled=1 diagnostic_only=1 x=8 y=%u width=128 height=4 sync=D3AC id_little_endian=1 crc8_poly=07 readback_affinity_must_be_NONE=1\n",height-8);
  }
  if(verify){VkBufferCreateInfo bc{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};bc.size=uint64_t(width)*height*4;bc.usage=VK_BUFFER_USAGE_TRANSFER_DST_BIT;ck(vkCreateBuffer(device,&bc,nullptr,&readback),"WSI readback");VkMemoryRequirements req{};vkGetBufferMemoryRequirements(device,readback,&req);VkPhysicalDeviceMemoryProperties mem{};vkGetPhysicalDeviceMemoryProperties(physical,&mem);uint32_t type=UINT32_MAX;for(uint32_t i=0;i<mem.memoryTypeCount;i++)if((req.memoryTypeBits&(1u<<i))&&(mem.memoryTypes[i].propertyFlags&(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))==(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)){type=i;break;}if(type==UINT32_MAX)throw std::runtime_error("WSI readback memory");VkMemoryAllocateInfo ma{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ma.allocationSize=req.size;ma.memoryTypeIndex=type;ck(vkAllocateMemory(device,&ma,nullptr,&readbackMemory),"WSI readback memory");ck(vkBindBufferMemory(device,readback,readbackMemory,0),"WSI readback bind");void*p=nullptr;ck(vkMapMemory(device,readbackMemory,0,VK_WHOLE_SIZE,0,&p),"WSI readback map");mapped=(unsigned char*)p;}
  trace=fopen("app-wsi958.csv","wb");if(trace)fprintf(trace,"event,id,submit_qpc,observed_qpc,gpu_ms,result,image\n");
  fprintf(stderr,"WSI958 queue_family=%u extent=%ux%u images=%u fifo=1 external_barriers=retained per_image_present_semaphores=1 readback=%u physical_display_unverified=1 external_wait_ALL_COMMANDS=1\n",family,width,height,count,unsigned(verify));
  }catch(...){cleanup();throw;}
 }
 ~VulkanPresenter958(){cleanup();}
 VulkanPresenter958(const VulkanPresenter958&)=delete;
 void cleanup()noexcept{
  VkResult idle976=VK_ERROR_INITIALIZATION_FAILED;if(device&&vkDeviceWaitIdle){std::lock_guard lock(queueMutex);idle976=SMX_DIAG_CALL994(SMX_DIAG_MODEL_IDLE994,smxdiag994::fields(0,0,(uintptr_t)device,0,0,3),SMX_DIAG_VKRESULT994,vkDeviceWaitIdle(device));}
  flushProbe976(idle976);destroyProbe976();
  for(auto&s:slots){s.lease.reset();if(s.markerMap)vkUnmapMemory(device,s.markerMemory);if(s.marker)vkDestroyBuffer(device,s.marker,nullptr);if(s.markerMemory)vkFreeMemory(device,s.markerMemory,nullptr);if(s.queries)vkDestroyQueryPool(device,s.queries,nullptr);if(s.fence)vkDestroyFence(device,s.fence,nullptr);if(s.acquire)vkDestroySemaphore(device,s.acquire,nullptr);s={};}
  if(mapped){vkUnmapMemory(device,readbackMemory);mapped=nullptr;}if(readback){vkDestroyBuffer(device,readback,nullptr);readback={};}if(readbackMemory){vkFreeMemory(device,readbackMemory,nullptr);readbackMemory={};}
  if(pool){vkDestroyCommandPool(device,pool,nullptr);pool={};}for(auto&s:complete)if(s)vkDestroySemaphore(device,s,nullptr);complete.clear();
  if(swapchain){vkDestroySwapchainKHR(device,swapchain,nullptr);swapchain={};}if(surface){vkDestroySurfaceKHR(instance,surface,nullptr);surface={};}if(trace){fclose(trace);trace=nullptr;}
 }
 void poll(){
  // Called only by the presentation owner thread. Release CPU leases promptly,
  // but never before the corresponding GPU copy fence has signaled.
  for(auto&slot:slots)if(slot.busy){VkResult r=vkGetFenceStatus(device,slot.fence);if(r==VK_SUCCESS)retire(slot);else if(r!=VK_NOT_READY)ck(r,"WSI nonblocking retirement");}
 }
 void setDiagnosticReadback967(bool v){diagnosticReadback967=v;}
 void prepare(Source source,std::shared_ptr<void>lease){
  poll();
  if(prepared)throw std::runtime_error("WSI unpresented prepared frame");if(source.external&&(!source.mutex||!source.value||!source.semaphore))throw std::runtime_error("WSI requires synchronized external source");
  auto&s=slots[cursor];retire(s);double begin=now();
  if(marker967){
   unsigned char bytes[8]={0xD3,0xAC,0,0,0,0,(unsigned char)(source.generated?1:0),0};uint32_t id=uint32_t(sequence+1);
   for(unsigned k=0;k<4;k++)bytes[k+2]=(id>>(8*k))&255;
   unsigned char crc=0;for(unsigned k=0;k<7;k++){crc^=bytes[k];for(unsigned bit=0;bit<8;bit++)crc=(crc&0x80)?static_cast<unsigned char>((crc<<1)^0x07):static_cast<unsigned char>(crc<<1);}bytes[7]=crc;
   for(unsigned y=0;y<4;y++)for(unsigned x=0;x<128;x++){unsigned bit=x/2;unsigned char color=((bytes[bit/8]>>(bit%8))&1)?255:0;auto*p=s.markerMap+(y*128+x)*4;p[0]=p[1]=p[2]=color;p[3]=255;}
  }
  VkResult ar=SMX_DIAG_CALL994(SMX_DIAG_DISPLAY_CREDIT994,smxdiag994::fields(0,0,(uintptr_t)swapchain,0,0,500000000ll,3),SMX_DIAG_VKRESULT994,vkAcquireNextImageKHR(device,swapchain,500000000ull,s.acquire,VK_NULL_HANDLE,&preparedImage));if(ar!=VK_SUCCESS&&ar!=VK_SUBOPTIMAL_KHR)ck(ar,"WSI acquire");
  ck(vkResetFences(device,1,&s.fence),"WSI fence reset");ck(vkResetCommandBuffer(s.command,0),"WSI command reset");
  VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};bi.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;ck(vkBeginCommandBuffer(s.command,&bi),"WSI command begin");
  s.probeIndex976=beginProbe976(sequence+1,source.generated,begin);
  vkCmdResetQueryPool(s.command,s.queries,0,2);vkCmdWriteTimestamp(s.command,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,s.queries,0);
  VkImageMemoryBarrier src{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};src.oldLayout=src.newLayout=VK_IMAGE_LAYOUT_GENERAL;src.srcQueueFamilyIndex=source.external?VK_QUEUE_FAMILY_EXTERNAL:VK_QUEUE_FAMILY_IGNORED;src.dstQueueFamilyIndex=source.external?family:VK_QUEUE_FAMILY_IGNORED;src.srcAccessMask=source.external?0:VK_ACCESS_MEMORY_WRITE_BIT;src.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;src.image=source.image;src.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
  vkCmdPipelineBarrier(s.command,source.external?VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT:VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&src);
  recordProbe976(s.command,source.image,VK_IMAGE_LAYOUT_GENERAL,s.probeIndex976,false);
  VkImageMemoryBarrier dst{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};dst.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED;dst.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;dst.srcQueueFamilyIndex=dst.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;dst.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;dst.image=images[preparedImage];dst.subresourceRange=src.subresourceRange;
  vkCmdPipelineBarrier(s.command,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&dst);
  VkImageCopy cp{};cp.srcSubresource=cp.dstSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};cp.extent={width,height,1};vkCmdCopyImage(s.command,source.image,VK_IMAGE_LAYOUT_GENERAL,dst.image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&cp);
  if(marker967){
   VkImageMemoryBarrier order=dst;order.oldLayout=order.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;order.srcAccessMask=order.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
   vkCmdPipelineBarrier(s.command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&order);
   VkBufferImageCopy region{};region.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};region.imageOffset={8,int(height)-8,0};region.imageExtent={128,4,1};
   vkCmdCopyBufferToImage(s.command,s.marker,dst.image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);
  }
  if(source.external){src.srcQueueFamilyIndex=family;src.dstQueueFamilyIndex=VK_QUEUE_FAMILY_EXTERNAL;src.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;src.dstAccessMask=0;vkCmdPipelineBarrier(s.command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,0,0,nullptr,0,nullptr,1,&src);}
  dst.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;dst.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
  if((readback&&diagnosticReadback967)||s.probeIndex976!=UINT32_MAX){
   dst.newLayout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;dst.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;vkCmdPipelineBarrier(s.command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&dst);
   recordProbe976(s.command,dst.image,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,s.probeIndex976,true);
   if(readback&&diagnosticReadback967){VkBufferImageCopy bc{};bc.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};bc.imageExtent={width,height,1};vkCmdCopyImageToBuffer(s.command,dst.image,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,readback,1,&bc);}
   VkMemoryBarrier host{VK_STRUCTURE_TYPE_MEMORY_BARRIER};host.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;host.dstAccessMask=VK_ACCESS_HOST_READ_BIT;vkCmdPipelineBarrier(s.command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_HOST_BIT,0,1,&host,0,nullptr,0,nullptr);dst.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;dst.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;
  }
  dst.newLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;dst.dstAccessMask=0;vkCmdPipelineBarrier(s.command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,0,0,nullptr,0,nullptr,1,&dst);
  vkCmdWriteTimestamp(s.command,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,s.queries,1);ck(vkEndCommandBuffer(s.command),"WSI end");
  std::unique_lock<std::mutex>resourceLock;if(source.mutex)resourceLock=std::unique_lock<std::mutex>(*source.mutex);
  VkSemaphore waits[2]={s.acquire,source.semaphore},signals[2]={complete[preparedImage],source.semaphore};uint64_t wv[2]={0,source.value?*source.value:0},sv[2]={0,wv[1]+1};// R966: external ownership acquire is not confined to TRANSFER without
  // VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR.
  // Wait before ALL_COMMANDS so acquire/cache metadata cannot race the producer.
  VkPipelineStageFlags stages[2]={VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
  VkTimelineSemaphoreSubmitInfo times{VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO};times.waitSemaphoreValueCount=times.signalSemaphoreValueCount=source.semaphore?2:1;times.pWaitSemaphoreValues=wv;times.pSignalSemaphoreValues=sv;
  VkSubmitInfo sub{VK_STRUCTURE_TYPE_SUBMIT_INFO};sub.pNext=&times;sub.waitSemaphoreCount=sub.signalSemaphoreCount=source.semaphore?2:1;sub.pWaitSemaphores=waits;sub.pWaitDstStageMask=stages;sub.pSignalSemaphores=signals;sub.commandBufferCount=1;sub.pCommandBuffers=&s.command;
  s.sequence=++sequence;s.submitQpc=now();{std::lock_guard lock(queueMutex);ck(SMX_DIAG_CALL994(SMX_DIAG_DISPLAY_COPY994,smxdiag994::fields(s.sequence,0,(uintptr_t)queue,(uintptr_t)s.fence,0,3),SMX_DIAG_VKRESULT994,vkQueueSubmit(queue,1,&sub,s.fence)),"WSI submit");}if(s.probeIndex976!=UINT32_MAX)probe976.frames[s.probeIndex976].submitted=true;if(source.value)*source.value=sv[1];s.lease=std::move(lease);s.busy=true;prepared=true;
  if(trace)fprintf(trace,"copy_submit,%llu,%.9f,%.9f,0,0,%u\n",(unsigned long long)s.sequence,begin,s.submitQpc,preparedImage);
 }
 const unsigned char*verifiedPixels(){if(!readback||!prepared)throw std::runtime_error("Readback not enabled");retire(slots[cursor]);return mapped;}
 uint64_t present(){
  if(!prepared)throw std::runtime_error("WSI present without copy");VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};pi.waitSemaphoreCount=1;pi.pWaitSemaphores=&complete[preparedImage];pi.swapchainCount=1;pi.pSwapchains=&swapchain;pi.pImageIndices=&preparedImage;
  double begin=now();VkResult result;{std::lock_guard lock(queueMutex);result=SMX_DIAG_CALL994(SMX_DIAG_PRESENT994,smxdiag994::fields(slots[cursor].sequence,0,(uintptr_t)queue,0,0,preparedImage,3),SMX_DIAG_VKRESULT994,vkQueuePresentKHR(queue,&pi));}if(result!=VK_SUCCESS&&result!=VK_SUBOPTIMAL_KHR)ck(result,"WSI present");
  if(slots[cursor].probeIndex976!=UINT32_MAX)probe976.frames[slots[cursor].probeIndex976].presented=true;
  uint64_t id=slots[cursor].sequence;if(trace)fprintf(trace,"present_return,%llu,%.9f,%.9f,0,%d,%u\n",(unsigned long long)id,begin,now(),int(result),preparedImage);prepared=false;cursor=(cursor+1)%3;return id;
 }
 void drain(){for(auto&s:slots)retire(s);if(trace)fflush(trace);}
#undef WP_DEV958
#undef WP_INST958
};
