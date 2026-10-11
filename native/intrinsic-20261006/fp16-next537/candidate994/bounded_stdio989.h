#pragma once
#include <stdio.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int smx_log_fprintf989(FILE*,const char*,...);
FILE* smx_log_fopen989(const char*,const char*);
int smx_log_fopen_s989(FILE**,const char*,const char*);
FILE* smx_log_fsopen989(const char*,const char*,int);
int smx_log_freopen_s989(FILE**,const char*,const char*,FILE*);
int smx_log_fclose989(FILE*);
void smx_log_reset989(FILE*,const char*);
uint64_t smx_log_limit989(void);
void smx_log_stream989(FILE*,uint64_t*,uint64_t*);
void smx_log_totals989(uint64_t*,uint64_t*,uint64_t*);
#ifdef __cplusplus
}
#endif
