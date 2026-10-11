#pragma once
#include "diagnostic994_c.h"
#ifdef __cplusplus
namespace smxdiag994 {
class Scope {
 uint32_t stage_;SmxDiagFields994 fields_;uint64_t call_;bool finished_=false;
 static SmxDiagFields994 latch(SmxDiagFields994 f) noexcept{if(!f.captureSession){f.captureSession=smx_diag_current_session994();if(!f.captureSession)f.captureSession=UINT64_MAX;}return f;}
public:
 explicit Scope(uint32_t stage,SmxDiagFields994 fields={}) noexcept:stage_(stage),fields_(latch(fields)),call_(smx_diag_begin994(stage,&fields_)){}
 Scope(const Scope&)=delete;Scope&operator=(const Scope&)=delete;
 void end(uint16_t domain=SMX_DIAG_NONE994,int64_t result=0) noexcept{if(!finished_){smx_diag_event994(stage_,SMX_DIAG_END994,call_,domain,result,&fields_);finished_=true;}}
 void fail(uint16_t domain,int64_t result) noexcept{if(!finished_){smx_diag_event994(stage_,SMX_DIAG_FAIL994,call_,domain,result,&fields_);finished_=true;}}
 void update(SmxDiagFields994 fields) noexcept{fields.captureSession=fields_.captureSession;fields_=fields;}
 uint64_t callId()const noexcept{return call_;}
 ~Scope(){if(!finished_)smx_diag_event994(stage_,SMX_DIAG_ABORT994,call_,SMX_DIAG_NONE994,0,&fields_);}
};
}

#endif
