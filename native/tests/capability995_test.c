#include "../adapter_policy995.h"
#include <stdio.h>
#include <stdlib.h>
static unsigned checks=0;
#define T(x) do{checks++;if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 SmxDeviceCaps995 b={0};b.vendor=0x8086;b.device=0xb080;b.api=0x00403000;b.luidValid=b.extensions=b.features=b.bgra=b.shape=b.subgroups=b.externalImage=b.externalSemaphore=b.computeQueue=b.floatControls=b.workgroupLimits=1;b.luid[0]=0x44;
 T(!smx_capability_reject995(&b));b.device=0xe20b;T(!smx_capability_reject995(&b)); // Labelled B580 mock: capability contract only, no hardware validation.
 b.device=0x1234;T(!smx_capability_reject995(&b)); // Future device ID is not rejected solely by its name/ID.
 int*flags[]={&b.luidValid,&b.extensions,&b.features,&b.bgra,&b.shape,&b.subgroups,&b.externalImage,&b.externalSemaphore,&b.computeQueue,&b.floatControls,&b.workgroupLimits};
 for(unsigned i=0;i<sizeof(flags)/sizeof(*flags);i++){*flags[i]=0;T(smx_capability_reject995(&b)!=0);*flags[i]=1;}
 b.vendor=0x10de;T(smx_capability_reject995(&b)!=0);b.vendor=0x8086;b.api=0x00402000;T(smx_capability_reject995(&b)!=0);b.api=0x00403000;
 unsigned char a0[8]={1,2,3,4,5,6,7,8},a1[8]={1,2,3,4,5,6,7,9},selected[8]={1,2,3,4,5,6,7,9};T(!smx_luid_equal995(a0,selected));T(smx_luid_equal995(a1,selected));T(!smx_luid_equal995(0,selected)); // same vendor/device, different actual LUID
 uint32_t f[]={7,4,2,2},c[]={1,1,1,0},t[]={64,64,64,64};T(smx_choose_queue995(f,c,t,4)==2);f[2]=7;T(smx_choose_queue995(f,c,t,4)==0);f[2]=2;t[2]=0;T(smx_choose_queue995(f,c,t,4)==0);t[0]=0;T(smx_choose_queue995(f,c,t,4)==-1);
 T(smx_priority_retry995(-1000174001LL,1));T(!smx_priority_retry995(-4,1));T(!smx_priority_retry995(-1000174001LL,0));
 printf("{\"pass\":true,\"checks\":%u,\"gpu_used\":false,\"B390\":\"capability mock\",\"B580\":\"capability mock only; not physically tested\",\"multi_gpu\":\"actual-LUID matching policy\"}\n",checks);return 0;
}
