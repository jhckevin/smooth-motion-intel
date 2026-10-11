#pragma once
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <mutex>
#include <cstdio>
#include <cstring>
namespace smx992 {
enum class ProcessingMode { Native,Fit1080,Fit1440 };
inline ProcessingMode processingMode(const char*text){if(!text||!*text||!strcmp(text,"native"))return ProcessingMode::Native;if(!strcmp(text,"fit1080"))return ProcessingMode::Fit1080;if(!strcmp(text,"fit1440"))return ProcessingMode::Fit1440;throw std::runtime_error("Unsupported SMX_PROCESSING_SIZE992; expected native, fit1080, or fit1440");}
inline const char* processingName(ProcessingMode mode){return mode==ProcessingMode::Fit1080?"fit1080":mode==ProcessingMode::Fit1440?"fit1440":"native";}
struct ProcessingExtent {uint32_t sourceWidth=0,sourceHeight=0,width=0,height=0;ProcessingMode mode=ProcessingMode::Native;bool resampled=false;};
inline ProcessingExtent processingExtent(uint32_t sw,uint32_t sh,const char*value){
 if(!sw||!sh||sw>16384||sh>16384)throw std::runtime_error("Processing source dimensions must be 1..16384; actual GPU/capture limits are checked separately");
 ProcessingExtent out{sw,sh,sw,sh,processingMode(value),false};
 if(out.mode==ProcessingMode::Native)return out;
 const uint32_t bw=out.mode==ProcessingMode::Fit1080?1920:2560,bh=out.mode==ProcessingMode::Fit1080?1080:1440;
 if(sw<=bw&&sh<=bh)return out;
 // Exact integer floor of min(1,bw/sw,bh/sh). Odd dimensions are supported.
 if(uint64_t(sw)*bh>=uint64_t(sh)*bw){out.width=bw;out.height=uint32_t(std::max<uint64_t>(1,uint64_t(sh)*bw/sw));}
 else{out.height=bh;out.width=uint32_t(std::max<uint64_t>(1,uint64_t(sw)*bh/sh));}
 out.resampled=out.width!=sw||out.height!=sh;return out;
}
inline uint64_t rgbaBytes(uint32_t w,uint32_t h){if(!w||!h||w>16384||h>16384)throw std::runtime_error("Invalid capture allocation dimensions");return uint64_t(w)*h*4;}
inline uint64_t imageAlignedBytes(uint32_t w,uint32_t h){return (rgbaBytes(w,h)+65535u)&~65535ULL;}
struct ScalingCost {uint64_t sourceClientRgba=0,processRgba=0,captureImageRgba=0,captureDeltaAligned=0,fallbackAligned=0,extraEstimate=0;};
inline ScalingCost scalingCost(const ProcessingExtent&p,uint32_t captureW,uint32_t captureH,unsigned capturePoolCount){
 if(capturePoolCount>6)throw std::runtime_error("Unexpected capture pool size");
 ScalingCost c;c.sourceClientRgba=rgbaBytes(p.sourceWidth,p.sourceHeight);c.processRgba=rgbaBytes(p.width,p.height);c.captureImageRgba=rgbaBytes(captureW,captureH);
 if(!p.resampled)return c;
 const uint64_t source=imageAlignedBytes(captureW,captureH),process=imageAlignedBytes(p.width,p.height);
 c.captureDeltaAligned=(source>process?source-process:0)*capturePoolCount;
 // Reserve one optional source-sized shader-readable copy, even if direct SRV succeeds.
 c.fallbackAligned=imageAlignedBytes(p.sourceWidth,p.sourceHeight);
 const uint64_t base=c.captureDeltaAligned+c.fallbackAligned;c.extraEstimate=base+(base+3)/4;return c;
}
class ProcessingState {
 std::mutex mutex;bool valid=false;ProcessingExtent extent{};
public:
 void clear(){std::lock_guard<std::mutex>lock(mutex);valid=false;extent={};}
 void set(const ProcessingExtent&v){std::lock_guard<std::mutex>lock(mutex);extent=v;valid=true;}
 std::string fields(){std::lock_guard<std::mutex>lock(mutex);return std::string("\"processingInfoValid992\":")+(valid?"true":"false")+",\"sourceWidth992\":"+std::to_string(extent.sourceWidth)+",\"sourceHeight992\":"+std::to_string(extent.sourceHeight)+",\"processingWidth992\":"+std::to_string(extent.width)+",\"processingHeight992\":"+std::to_string(extent.height)+",\"processingResampled992\":"+(valid&&extent.resampled?"true":"false")+",\"processingSize992\":\""+processingName(extent.mode)+"\"";}
};
}
#ifdef _WIN32
#include "../resolution_budget989.hpp"
#include "../bounded_stdio989.h"
namespace smx992 {
inline void preflightScaling(IDXGIAdapter*adapter,const ProcessingExtent&p,uint32_t captureW,uint32_t captureH,unsigned capturePoolCount,unsigned multiplier){
 if(!p.resampled)return;
 SmxResolutionBudget989 info{};char why[512]{};IDXGIAdapter3*b=nullptr;
 if(!adapter||FAILED(adapter->QueryInterface(__uuidof(IDXGIAdapter3),(void**)&b)))throw std::runtime_error("Scaling requires readable DXGI local memory budget");
 DXGI_QUERY_VIDEO_MEMORY_INFO mem{};HRESULT hr=b->QueryVideoMemoryInfo(0,DXGI_MEMORY_SEGMENT_GROUP_LOCAL,&mem);b->Release();if(FAILED(hr))throw std::runtime_error("Scaling memory budget query failed");
 if(!smx_extent_check989(p.width,p.height,16384,UINT32_MAX,&info,why,sizeof why)||!smx_estimate_memory989(&info,multiplier,0,why,sizeof why))throw std::runtime_error(why);
 auto extra=scalingCost(p,captureW,captureH,capturePoolCount);
 if(info.requiredBytes>UINT64_MAX-extra.extraEstimate)throw std::runtime_error("Scaling budget overflow");
 info.requiredBytes+=extra.extraEstimate;info.textureBytes+=extra.extraEstimate;
 if(!smx_budget_check989(&info,mem.Budget,mem.CurrentUsage,why,sizeof why))throw std::runtime_error(std::string("Processing-size capture/fallback budget: ")+why);
 smx_log_fprintf989(stderr,"PROCESSING992 budget mode=%s source=%ux%u processing=%ux%u capture_surface=%ux%u capture_pool=%u source_client_rgba=%llu process_rgba=%llu capture_delta_aligned=%llu fallback_reserve_aligned=%llu total_texture_estimate=%llu available=%llu headroom=%llu estimate_only=1 model_core_unchanged=1\n",processingName(p.mode),p.sourceWidth,p.sourceHeight,p.width,p.height,captureW,captureH,capturePoolCount,(unsigned long long)extra.sourceClientRgba,(unsigned long long)extra.processRgba,(unsigned long long)extra.captureDeltaAligned,(unsigned long long)extra.fallbackAligned,(unsigned long long)info.textureBytes,(unsigned long long)info.availableBytes,(unsigned long long)info.headroomBytes);
}
}
#endif
