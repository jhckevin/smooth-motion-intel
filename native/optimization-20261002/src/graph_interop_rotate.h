#pragma once
#include <vulkan/vulkan.h>
typedef struct SmxGraphInterop {
 VkDevice device;VkQueue queue;VkCommandBuffer core;
 VkQueryPool queries;uint32_t queryCount;float timestampPeriod;
 VkPhysicalDeviceMemoryProperties memory;
 VkBuffer input,output;uint32_t queueFamily,width,height;
 PFN_vkGetDeviceProcAddr getProc;uint8_t luid[8];int (*bindImages)(const VkImageView*);VkCommandBuffer alternates[2];
} SmxGraphInterop;
#ifdef __cplusplus
extern "C" {
#endif
const SmxGraphInterop* smx_graph_interop_context(void);
#ifdef __cplusplus
}
#endif
