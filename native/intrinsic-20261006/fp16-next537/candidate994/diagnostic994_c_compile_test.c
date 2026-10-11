#include "diagnostic994_c.h"
void c_abi_compile_test994(void) {
 SmxDiagFields994 f={0};uint64_t c=smx_diag_begin994(SMX_DIAG_MODEL_SUBMIT994,&f);
 smx_diag_event994(SMX_DIAG_MODEL_SUBMIT994,SMX_DIAG_END994,c,SMX_DIAG_VKRESULT994,0,&f);
}
