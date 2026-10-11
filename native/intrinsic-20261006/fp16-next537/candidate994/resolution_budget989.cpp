#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include "resolution_budget989.hpp"
extern "C" int smx_preflight_model_budget989(const unsigned char luidBytes[8],uint32_t width,uint32_t height,unsigned multiplier,uint64_t manifestBytes,SmxResolutionBudget989*out,char*why,size_t cap){
 if(!luidBytes||!out)return smx_extent_error989(why,cap,"Missing physical adapter LUID/capacity result");
 if(manifestBytes>(UINT64_MAX-128ULL*1024u*1024u)/2u)return smx_extent_error989(why,cap,"Model allocation estimate overflow");
 uint64_t modelReserve=manifestBytes*2u+128ULL*1024u*1024u;
 if(!smx_extent_check989(width,height,16384u,UINT32_MAX,out,why,cap)||!smx_estimate_memory989(out,multiplier,modelReserve,why,cap))return 0;
 Microsoft::WRL::ComPtr<IDXGIFactory4>factory;HRESULT hr=CreateDXGIFactory1(IID_PPV_ARGS(&factory));
 if(FAILED(hr))return smx_extent_error989(why,cap,"Cannot read DXGI factory for cold model memory preflight");
 LUID luid;memcpy(&luid,luidBytes,sizeof(luid));Microsoft::WRL::ComPtr<IDXGIAdapter3>adapter;
 hr=factory->EnumAdapterByLuid(luid,IID_PPV_ARGS(&adapter));
 if(FAILED(hr))return smx_extent_error989(why,cap,"Vulkan physical adapter LUID has no matching DXGI budget adapter");
 DXGI_QUERY_VIDEO_MEMORY_INFO memory={};hr=adapter->QueryVideoMemoryInfo(0,DXGI_MEMORY_SEGMENT_GROUP_LOCAL,&memory);
 if(FAILED(hr)){if(why&&cap)snprintf(why,cap,"Cold model DXGI budget query failed HRESULT=0x%08lx",(unsigned long)hr);return 0;}
 return smx_budget_check989(out,memory.Budget,memory.CurrentUsage,why,cap);
}
