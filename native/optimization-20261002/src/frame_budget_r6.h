#ifndef SMX_FRAME_BUDGET_H
#define SMX_FRAME_BUDGET_H
#include <math.h>
#include <string.h>
#include <stdlib.h>
typedef struct {
 double maxDuty,duty,minDuty,meanMs,varMs,nextMs,lastProbeMs,probeEveryMs;
 unsigned samples,executed,skippedBudget,skippedDeadline,probes,late;
} SmxFrameBudget;
static inline double smx_clamp(double x,double lo,double hi){return x<lo?lo:(x>hi?hi:x);}
static inline void smx_budget_init(SmxFrameBudget*b,double duty){
 memset(b,0,sizeof(*b));b->maxDuty=smx_clamp(duty,.1,1.0);b->duty=b->maxDuty;b->minDuty=fmin(.15,b->maxDuty);
 b->meanMs=10.0;b->probeEveryMs=300.0;b->lastProbeMs=-100000.0;
}
static inline double smx_budget_predict(const SmxFrameBudget*b){
 return smx_clamp(b->meanMs+sqrt(fmax(0.0,b->varMs))*.75,1.0,500.0);
}
// Return 0=skip, 1=regular admission, 2=periodic recovery probe.
// No queue of stale inference work is created; original presentation is independent.
static inline int smx_budget_admit(SmxFrameBudget*b,double nowMs,double availableMs){
 if(nowMs<b->nextMs){b->skippedBudget++;return 0;}
 double predicted=smx_budget_predict(b);
 if(b->samples>=3&&predicted+2.0>availableMs){
  if(nowMs-b->lastProbeMs<b->probeEveryMs){b->skippedDeadline++;return 0;}
  b->lastProbeMs=nowMs;b->probes++;return 2;
 }
 return 1;
}
static inline void smx_budget_complete(SmxFrameBudget*b,double endMs,double costMs,int onTime){
 costMs=smx_clamp(costMs,.01,2000.0);
 double delta=costMs-b->meanMs;
 double a=b->samples<4?.5:.15;
 b->meanMs+=a*delta;b->varMs=(1-a)*(b->varMs+a*delta*delta);
 b->samples++;b->executed++;
 if(!onTime){b->late++;b->duty=fmax(b->minDuty,b->duty*.85);}
 else b->duty=fmin(b->maxDuty,b->duty+.015);
 const char* deadlineOnly=getenv("SMX_DEADLINE_ONLY");
 if(deadlineOnly&&strcmp(deadlineOnly,"1")==0)b->duty=b->maxDuty;
 double reserve=fmax(costMs,smx_budget_predict(b));
 b->nextMs=endMs+reserve*(1.0/b->duty-1.0);
}
#endif
