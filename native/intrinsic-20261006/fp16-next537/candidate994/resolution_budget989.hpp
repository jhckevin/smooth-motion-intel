#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#ifndef UINT64_MAX
#define UINT64_MAX 18446744073709551615ULL
#endif
typedef struct SmxResolutionBudget989 {
 uint64_t pixels,rgbaBytes,alignedImageBytes,textureCount,textureBytes,modelBytes,requiredBytes;
 uint64_t budgetBytes,currentUsageBytes,availableBytes,headroomBytes;
 uint32_t dispatchX;
} SmxResolutionBudget989;
static int smx_extent_error989(char*why,size_t cap,const char*message){if(why&&cap)snprintf(why,cap,"%s",message);return 0;}
static int smx_extent_check989(uint32_t w,uint32_t h,uint32_t maxImageDimension,uint32_t maxDispatchX,SmxResolutionBudget989*out,char*why,size_t cap){
 uint64_t pixels,groups;
 if(!out)return smx_extent_error989(why,cap,"Missing capacity result");
 memset(out,0,sizeof(*out));
 if(!w||!h)return smx_extent_error989(why,cap,"Capture dimensions must be nonzero");
 if(w>16384u||h>16384u)return smx_extent_error989(why,cap,"Capture exceeds D3D11 2D texture dimension limit (16384)");
 if(!maxImageDimension||!maxDispatchX)return smx_extent_error989(why,cap,"GPU image/dispatch limits are unavailable");
 if(w>maxImageDimension||h>maxImageDimension)return smx_extent_error989(why,cap,"Capture exceeds actual Vulkan maxImageDimension2D");
 pixels=(uint64_t)w*(uint64_t)h;
 if(pixels>UINT64_MAX/4u)return smx_extent_error989(why,cap,"RGBA byte count overflow");
 groups=pixels/256u+(pixels%256u!=0);
 if(groups>maxDispatchX)return smx_extent_error989(why,cap,"Native warp linear dispatch exceeds actual Vulkan maxComputeWorkGroupCount[0]");
 out->pixels=pixels;out->rgbaBytes=pixels*4u;out->dispatchX=(uint32_t)groups;
 if(why&&cap)why[0]=0;return 1;
}
static int smx_estimate_memory989(SmxResolutionBudget989*out,unsigned multiplier,uint64_t modelReserveBytes,char*why,size_t cap){
 uint64_t aligned,base,extra;
 if(!out||!out->pixels)return smx_extent_error989(why,cap,"Extent must be validated before memory estimation");
 if(multiplier!=2&&multiplier!=4&&multiplier!=8)return smx_extent_error989(why,cap,"Unsupported frame multiplier in memory preflight");
 if(out->rgbaBytes>UINT64_MAX-65535u)return smx_extent_error989(why,cap,"Image alignment overflow");
 aligned=(out->rgbaBytes+65535u)&~65535ULL;
 out->textureCount=multiplier==2?39u:45u;
 if(aligned>UINT64_MAX/out->textureCount)return smx_extent_error989(why,cap,"Texture pool byte count overflow");
 base=aligned*out->textureCount;
 extra=base/4u+(base%4u!=0); // 25% estimate for pitch/tile/allocation differences, not a driver allocation guarantee.
 if(base>UINT64_MAX-extra)return smx_extent_error989(why,cap,"Texture allocation margin overflow");
 out->alignedImageBytes=aligned;out->textureBytes=base+extra;out->modelBytes=modelReserveBytes;
 if(out->textureBytes>UINT64_MAX-modelReserveBytes)return smx_extent_error989(why,cap,"Model plus texture reservation overflow");
 out->requiredBytes=out->textureBytes+modelReserveBytes;
 if(why&&cap)why[0]=0;return 1;
}
static int smx_budget_check989(SmxResolutionBudget989*out,uint64_t budget,uint64_t usage,char*why,size_t cap){
 uint64_t reserve;
 if(!out||!out->requiredBytes)return smx_extent_error989(why,cap,"Memory estimate is unavailable");
 out->budgetBytes=budget;out->currentUsageBytes=usage;
 if(!budget)return smx_extent_error989(why,cap,"DXGI local memory budget is zero or unavailable");
 out->availableBytes=usage<budget?budget-usage:0;
 reserve=budget/10u;
 if(reserve<128ULL*1024u*1024u)reserve=128ULL*1024u*1024u;
 if(reserve>512ULL*1024u*1024u)reserve=512ULL*1024u*1024u;
 out->headroomBytes=reserve;
 if(out->requiredBytes>out->availableBytes||reserve>out->availableBytes-out->requiredBytes){
  if(why&&cap)snprintf(why,cap,"Insufficient DXGI local memory headroom: estimate=%llu MiB reserve=%llu MiB available=%llu MiB (budget=%llu MiB usage=%llu MiB)",
   (unsigned long long)(out->requiredBytes>>20),(unsigned long long)(reserve>>20),(unsigned long long)(out->availableBytes>>20),(unsigned long long)(budget>>20),(unsigned long long)(usage>>20));
  return 0;
 }
 if(why&&cap)why[0]=0;return 1;
}
#ifdef __cplusplus
extern "C" {
#endif
int smx_preflight_model_budget989(const unsigned char luid[8],uint32_t width,uint32_t height,unsigned multiplier,uint64_t modelManifestBytes,SmxResolutionBudget989*out,char*why,size_t capacity);
#ifdef __cplusplus
}
#ifdef _WIN32
#include <dxgi1_4.h>
#include <stdexcept>
static inline SmxResolutionBudget989 smx_preflight_dxgi989(IDXGIAdapter*adapter,uint32_t width,uint32_t height,unsigned multiplier,bool modelAlreadyResident,const char*stage){
 SmxResolutionBudget989 info={};char why[384];IDXGIAdapter3*budgetAdapter=nullptr;
 if(!adapter||FAILED(adapter->QueryInterface(__uuidof(IDXGIAdapter3),(void**)&budgetAdapter)))throw std::runtime_error("DXGI memory budget interface is unavailable; capture not started");
 DXGI_QUERY_VIDEO_MEMORY_INFO memory={};HRESULT hr=budgetAdapter->QueryVideoMemoryInfo(0,DXGI_MEMORY_SEGMENT_GROUP_LOCAL,&memory);budgetAdapter->Release();
 if(FAILED(hr)){snprintf(why,sizeof(why),"DXGI local memory budget query failed HRESULT=0x%08lx",(unsigned long)hr);throw std::runtime_error(why);}
 // App callbacks normally run after the graph is resident. Cold graph uses the
 // separate manifest-aware C entry point rather than double-counting its buffers.
 uint64_t modelReserve=modelAlreadyResident?0:384ULL*1024u*1024u;
 if(!smx_extent_check989(width,height,16384u,UINT32_MAX,&info,why,sizeof(why))||
    !smx_estimate_memory989(&info,multiplier,modelReserve,why,sizeof(why))||
    !smx_budget_check989(&info,memory.Budget,memory.CurrentUsage,why,sizeof(why)))throw std::runtime_error(why);
 fprintf(stderr,"CAPACITY989 stage=%s width=%u height=%u multiplier=%u rgba_textures_estimate=%llu texture_bytes_estimate=%llu model_extra_bytes=%llu budget=%llu current_usage=%llu available=%llu safety_headroom=%llu estimate_only=1 actual_vk_limits_checked_by_graph=1\n",
  stage?stage:"unknown",width,height,multiplier,(unsigned long long)info.textureCount,(unsigned long long)info.textureBytes,(unsigned long long)info.modelBytes,
  (unsigned long long)info.budgetBytes,(unsigned long long)info.currentUsageBytes,(unsigned long long)info.availableBytes,(unsigned long long)info.headroomBytes);
 return info;
}
#endif
#endif
