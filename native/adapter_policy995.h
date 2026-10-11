#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef struct SmxDeviceCaps995 {
 uint32_t vendor,device,api; int luidValid,extensions,features,bgra,shape,subgroups,externalImage,externalSemaphore,computeQueue,floatControls,workgroupLimits;
 unsigned char luid[8];
} SmxDeviceCaps995;
static const char* smx_capability_reject995(const SmxDeviceCaps995*c){
 if(c->vendor!=0x8086)return "backend currently supports Intel devices only";
 if(c->api<0x00403000)return "Vulkan 1.3 required";
 if(!c->luidValid)return "valid Windows adapter LUID required for D3D/Vulkan identity";
 if(!c->extensions)return "required cooperative-matrix/float8/Win32 external interop extension missing";
 if(!c->features)return "required FP16/scalar-FP8/int8/int64/storage16/timeline/memory-model features missing";
 if(!c->bgra)return "BGRA8 storage write without format unsupported";
 if(!c->shape)return "subgroup FP16 A/B/C/result 8x16x16 non-saturating cooperative matrix unsupported";
 if(!c->subgroups)return "required compute subgroup sizes 16 and 32 unavailable";
 if(!c->externalImage)return "BGRA D3D11 shared texture import unsupported for required usage";
 if(!c->externalSemaphore)return "D3D12 fence handle import unsupported";
 if(!c->floatControls)return "required FP16 denorm/signed-zero-inf-NaN preservation and RTE controls unavailable";
 if(!c->workgroupLimits)return "required 256-thread compute workgroup or 64-byte push constants unsupported";
 if(!c->computeQueue)return "timestamp-capable compute queue unavailable";
 return 0;
}
static int smx_luid_equal995(const unsigned char*a,const unsigned char*b){return a&&b&&!memcmp(a,b,8);}
static int smx_choose_queue995(const uint32_t*flags,const uint32_t*counts,const uint32_t*timestamps,uint32_t n){
 for(uint32_t i=0;i<n;i++)if(counts[i]&&(flags[i]&2)&&!(flags[i]&1)&&timestamps[i])return (int)i;
 for(uint32_t i=0;i<n;i++)if(counts[i]&&(flags[i]&2)&&timestamps[i])return (int)i;
 return -1;
}

static int smx_priority_retry995(int64_t result,int requested){return requested&&result==-1000174001LL;} /* VK_ERROR_NOT_PERMITTED_KHR */
