#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct SmxAdapterIdentity995{uint32_t vendor,device;unsigned char luid[8];char name[256];int valid;}SmxAdapterIdentity995;
int smx_selected_adapter995(SmxAdapterIdentity995*out);
uint32_t smx_wsi_family995(void);
int smx_device_capabilities_cli995(void);
#ifdef __cplusplus
}
#endif
