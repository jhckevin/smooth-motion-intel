#pragma once
#include "output_geometry993.hpp"
inline unsigned outputGeometrySelftest993(){
 using namespace smx993;unsigned checks=0;auto ck=[&](bool value){++checks;if(!value)throw std::runtime_error("geometry test "+std::to_string(checks));};
 OutputRect monitor{0,0,2880,1800},work{0,0,2880,1740},source{0,90,2880,1710};
 auto p=outputPlan(OutputMode::Fullscreen,source,monitor,work,1920,1080);ck(sameRect(p.client,monitor));ck(sameRect(p.content,source));
 p=outputPlan(OutputMode::Follow,source,monitor,work,1920,1080);ck(sameRect(p.content,source)&&sameRect(p.client,source));
 p=outputPlan(OutputMode::Window1080,source,monitor,work,1920,1080,{8,40,8,8});ck(p.client.width()==1920&&p.client.height()==1080&&!p.constrained);ck(p.window.width()==1936);
 p=outputPlan(OutputMode::Processing,source,monitor,work,1600,900);ck(p.client.width()==1600&&p.client.height()==900);
 p=outputPlan(OutputMode::Small,source,monitor,work,1920,1080);ck(p.client.width()==960&&p.client.height()==540);
 p=outputPlan(OutputMode::Window1080,{0,0,1080,1920},{-1920,-200,0,880},{-1920,-200,0,840},607,1080,{8,40,8,8});ck(p.constrained);ck(p.window.left>=-1920&&p.window.right<=0&&p.window.top>=-200&&p.window.bottom<=840);ck(p.content.width()<p.client.width());
 for(int m=0;m<5;m++)for(unsigned sw=1;sw<16385;sw+=257)for(unsigned sh=1;sh<16385;sh+=263){
  p=outputPlan(OutputMode(m),{20,-300,int64_t(sw)+20,int64_t(sh)-300},monitor,work,1920,1080,{8,32,8,8});ck(p.client.valid()&&p.content.valid());ck(p.content.left>=p.client.left&&p.content.top>=p.client.top&&p.content.right<=p.client.right&&p.content.bottom<=p.client.bottom);
  if(m>=2){ck(p.window.left>=work.left&&p.window.right<=work.right&&p.window.top>=work.top&&p.window.bottom<=work.bottom);}
 }
 bool bad=false;try{outputMode("bad");}catch(...){bad=true;}ck(bad);
 OutputState state;state.clear(OutputMode::Small);ck(state.fields().find("\"outputInfoValid993\":false")!=std::string::npos);state.set(p,true);ck(state.fields().find("\"outputVisible993\":true")!=std::string::npos);state.hide();ck(state.fields().find("\"outputVisible993\":false")!=std::string::npos);
 return checks;
}
