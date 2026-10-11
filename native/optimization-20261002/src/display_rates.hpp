#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <cstdio>
static double displayHz(DISPLAYCONFIG_RATIONAL r){return r.Denominator?double(r.Numerator)/r.Denominator:0;}
static void logDisplayRates(){
 for(UINT32 flags:{UINT32(QDC_ONLY_ACTIVE_PATHS|QDC_VIRTUAL_MODE_AWARE|0x40)}){
  UINT32 np=0,nm=0;LONG e=GetDisplayConfigBufferSizes(flags,&np,&nm);if(e){fprintf(stderr,"flags=%u sizes_error=%ld\n",flags,e);continue;}
  std::vector<DISPLAYCONFIG_PATH_INFO>p(np);std::vector<DISPLAYCONFIG_MODE_INFO>m(nm);
  e=QueryDisplayConfig(flags,&np,p.data(),&nm,m.data(),nullptr);fprintf(stderr,"flags=%u query_error=%ld paths=%u modes=%u\n",flags,e,np,nm);if(e)continue;
  for(UINT32 i=0;i<np;i++){
   const auto&x=p[i];UINT32 idx=(flags&QDC_VIRTUAL_MODE_AWARE)?x.targetInfo.targetModeInfoIdx:x.targetInfo.modeInfoIdx;
   fprintf(stderr,"path=%u flags=0x%x source_id=%u target_id=%u path_hz=%.6f target_mode_index=%u output_technology=%d\n",i,x.flags,x.sourceInfo.id,x.targetInfo.id,displayHz(x.targetInfo.refreshRate),idx,int(x.targetInfo.outputTechnology));
   if(idx<nm&&m[idx].infoType==DISPLAYCONFIG_MODE_INFO_TYPE_TARGET){const auto&s=m[idx].targetMode.targetVideoSignalInfo;fprintf(stderr,"signal=%u x %u vsync_hz=%.6f pixel_rate=%llu scanline=%u\n",s.activeSize.cx,s.activeSize.cy,displayHz(s.vSyncFreq),s.pixelRate,unsigned(s.scanLineOrdering));}
  }
 }
}
