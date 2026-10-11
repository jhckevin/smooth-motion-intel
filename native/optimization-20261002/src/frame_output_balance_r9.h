#ifndef SMX_OUTPUT_BALANCE_H
#define SMX_OUTPUT_BALANCE_H
#include <stdint.h>
#include <string.h>
typedef enum {SMX_BASELINE=0,SMX_PROBE=1,SMX_ACTIVE=2,SMX_BACKOFF=3} SmxOutputState;
typedef struct {
 SmxOutputState state;
 double phaseMs,sampleMs,referenceBaseHz,lastBaseHz,referenceHz,lastOriginalHz,lastGeneratedHz;
 uint64_t sampleBase,sampleOriginal,sampleGenerated;
 unsigned transitions,accepted,rejected,baseFloorRejects;
 int sampling;
} SmxOutputBalance;
static inline void smx_output_begin(SmxOutputBalance*b,SmxOutputState state,double now){
 b->state=state;b->phaseMs=now;b->sampling=0;b->transitions++;
}
static inline void smx_output_init(SmxOutputBalance*b,double now){
 memset(b,0,sizeof(*b));smx_output_begin(b,SMX_BASELINE,now);
}
static inline int smx_output_admit(SmxOutputBalance*b,double now,uint64_t base,uint64_t original,uint64_t generated){
 if(b->state==SMX_BACKOFF){
  if(now-b->phaseMs>=20000)smx_output_begin(b,SMX_BASELINE,now);
  return 0;
 }
 if(!b->sampling){
  if(now-b->phaseMs<300)return b->state!=SMX_BASELINE;
  b->sampleMs=now;b->sampleBase=base;b->sampleOriginal=original;b->sampleGenerated=generated;b->sampling=1;
  return b->state!=SMX_BASELINE;
 }
 double span=now-b->sampleMs;
 if(span<2000)return b->state!=SMX_BASELINE;
 double baseHz=(base-b->sampleBase)*1000.0/span;
 double originalHz=(original-b->sampleOriginal)*1000.0/span;
 double generatedHz=(generated-b->sampleGenerated)*1000.0/span;
 b->lastBaseHz=baseHz;b->lastOriginalHz=originalHz;b->lastGeneratedHz=generatedHz;
 if(b->state==SMX_BASELINE){
  if(originalHz<1||baseHz<1){smx_output_begin(b,SMX_BASELINE,now);return 0;}
  b->referenceBaseHz=baseHz;b->referenceHz=originalHz+generatedHz;smx_output_begin(b,SMX_PROBE,now);return 1;
 }
 int baseSafe=baseHz>=b->referenceBaseHz*.90;
 int benefit=originalHz+generatedHz>=b->referenceHz*1.05&&generatedHz>=2;
 if(baseSafe&&benefit){
  b->accepted++;smx_output_begin(b,SMX_ACTIVE,now);return 1;
 }
 b->rejected++;if(!baseSafe)b->baseFloorRejects++;
 smx_output_begin(b,b->state==SMX_ACTIVE?SMX_BASELINE:SMX_BACKOFF,now);
 return 0;
}
#endif
