#pragma once
#include "adapter_identity995.h"
#include "deps/level-zero/zes_api.h"
#include <windows.h>
#include <cstring>
/* Exact LUID via Level Zero core -> same core UUID via Sysman. No device-name or device-ID-only fallback. */
inline zes_device_handle_t smxSysmanAdapter995(HMODULE lib){
 SmxAdapterIdentity995 selected{};if(!smx_selected_adapter995(&selected))return nullptr;
 auto init=(decltype(&zeInit))GetProcAddress(lib,"zeInit");auto drivers=(decltype(&zeDriverGet))GetProcAddress(lib,"zeDriverGet");auto devices=(decltype(&zeDeviceGet))GetProcAddress(lib,"zeDeviceGet");auto props=(decltype(&zeDeviceGetProperties))GetProcAddress(lib,"zeDeviceGetProperties");auto sysDrivers=(decltype(&zesDriverGet))GetProcAddress(lib,"zesDriverGet");auto byUuid=(decltype(&zesDriverGetDeviceByUuidExp))GetProcAddress(lib,"zesDriverGetDeviceByUuidExp");
 if(!init||!drivers||!devices||!props||!sysDrivers||!byUuid||init(ZE_INIT_FLAG_GPU_ONLY)!=ZE_RESULT_SUCCESS)return nullptr;
 ze_driver_handle_t dd[16]{};uint32_t dn=16;if(drivers(&dn,dd)!=ZE_RESULT_SUCCESS||dn>16)return nullptr;
 zes_uuid_t uuid{};bool found=false;
 for(uint32_t i=0;i<dn&&!found;i++){ze_device_handle_t vv[32]{};uint32_t vn=32;if(devices(dd[i],&vn,vv)!=ZE_RESULT_SUCCESS||vn>32)continue;for(uint32_t j=0;j<vn;j++){ze_device_luid_ext_properties_t luid{ZE_STRUCTURE_TYPE_DEVICE_LUID_EXT_PROPERTIES};ze_device_properties_t p{ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES};p.pNext=&luid;if(props(vv[j],&p)!=ZE_RESULT_SUCCESS||p.vendorId!=selected.vendor||p.deviceId!=selected.device||!luid.nodeMask||memcmp(luid.luid.id,selected.luid,8))continue;static_assert(sizeof(uuid.id)==sizeof(p.uuid.id));memcpy(uuid.id,p.uuid.id,sizeof(uuid.id));found=true;break;}}
 if(!found)return nullptr;zes_driver_handle_t sd[16]{};uint32_t sn=16;if(sysDrivers(&sn,sd)!=ZE_RESULT_SUCCESS||sn>16)return nullptr;
 for(uint32_t i=0;i<sn;i++){zes_device_handle_t device=nullptr;ze_bool_t sub=0;uint32_t index=0;if(byUuid(sd[i],uuid,&device,&sub,&index)==ZE_RESULT_SUCCESS&&!sub)return device;}
 return nullptr; // Telemetry unavailable is safer than sampling another GPU.
}
