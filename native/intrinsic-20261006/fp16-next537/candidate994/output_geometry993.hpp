#pragma once
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <mutex>
namespace smx993 {
enum class OutputMode { Follow,Fullscreen,Window1080,Processing,Small };
inline OutputMode outputMode(const char*s){if(!s||!*s||!strcmp(s,"follow"))return OutputMode::Follow;if(!strcmp(s,"fullscreen"))return OutputMode::Fullscreen;if(!strcmp(s,"window1080"))return OutputMode::Window1080;if(!strcmp(s,"processing"))return OutputMode::Processing;if(!strcmp(s,"small"))return OutputMode::Small;throw std::runtime_error("SMX_OUTPUT_MODE993 must be follow, fullscreen, window1080, processing or small");}
inline const char* outputModeName(OutputMode m){switch(m){case OutputMode::Follow:return "follow";case OutputMode::Fullscreen:return "fullscreen";case OutputMode::Window1080:return "window1080";case OutputMode::Processing:return "processing";case OutputMode::Small:return "small";}return "follow";}
inline bool preview(OutputMode m){return m!=OutputMode::Follow;}
inline bool windowed(OutputMode m){return m==OutputMode::Window1080||m==OutputMode::Processing||m==OutputMode::Small;}
struct OutputRect {int64_t left=0,top=0,right=0,bottom=0;int64_t width()const{return right-left;}int64_t height()const{return bottom-top;}bool valid()const{return right>left&&bottom>top&&left>=INT32_MIN&&top>=INT32_MIN&&right<=INT32_MAX&&bottom<=INT32_MAX&&width()<=32768&&height()<=32768;}};
struct Insets {unsigned left=0,top=0,right=0,bottom=0;};
struct OutputPlan {OutputMode mode=OutputMode::Follow;OutputRect window,client,content;bool constrained=false;};
inline OutputRect fitContent(OutputRect box,unsigned sourceW,unsigned sourceH){
 if(!box.valid()||!sourceW||!sourceH||sourceW>16384||sourceH>16384)throw std::runtime_error("Invalid output geometry");
 uint64_t w=uint64_t(box.width()),h=uint64_t(box.height());if(w*sourceH<=h*sourceW)h=std::max<uint64_t>(1,w*sourceH/sourceW);else w=std::max<uint64_t>(1,h*sourceW/sourceH);
 const int64_t x=box.left+(box.width()-int64_t(w))/2,y=box.top+(box.height()-int64_t(h))/2;return {x,y,x+int64_t(w),y+int64_t(h)};
}
inline OutputPlan outputPlan(OutputMode m,OutputRect source,OutputRect monitor,OutputRect work,unsigned pw,unsigned ph,Insets frame={}){
 if(!source.valid()||source.width()>16384||source.height()>16384||!monitor.valid()||!work.valid()||!pw||!ph||pw>16384||ph>16384)throw std::runtime_error("Invalid source/processing/monitor geometry");
 OutputPlan p;p.mode=m;
 if(m==OutputMode::Follow){p.window=p.client=p.content=source;return p;}
 if(m==OutputMode::Fullscreen){p.window=p.client=monitor;p.content=fitContent(p.client,unsigned(source.width()),unsigned(source.height()));return p;}
 unsigned wantW=m==OutputMode::Window1080?1920:m==OutputMode::Small?960:pw,wantH=m==OutputMode::Window1080?1080:m==OutputMode::Small?540:ph;
 constexpr int64_t margin=16;
 const int64_t availW=work.width()-frame.left-frame.right-margin*2,availH=work.height()-frame.top-frame.bottom-margin*2;
 if(availW<1||availH<1)throw std::runtime_error("Monitor work area cannot contain preview window");
 uint64_t w=wantW,h=wantH;if(w>uint64_t(availW)||h>uint64_t(availH)){
  if(uint64_t(availW)*h<=uint64_t(availH)*w){h=std::max<uint64_t>(1,h*uint64_t(availW)/w);w=uint64_t(availW);}
  else {w=std::max<uint64_t>(1,w*uint64_t(availH)/h);h=uint64_t(availH);}p.constrained=true;
 }
 const int64_t ow=int64_t(w)+frame.left+frame.right,oh=int64_t(h)+frame.top+frame.bottom;
 const int64_t x=work.left+(work.width()-ow)/2,y=work.top+(work.height()-oh)/2;p.window={x,y,x+ow,y+oh};p.client={x+frame.left,y+frame.top,x+frame.left+int64_t(w),y+frame.top+int64_t(h)};p.content=fitContent(p.client,unsigned(source.width()),unsigned(source.height()));return p;
}
inline bool sameRect(const OutputRect&a,const OutputRect&b){return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;}
inline std::string rectJson(OutputRect r){return "["+std::to_string(r.left)+","+std::to_string(r.top)+","+std::to_string(r.right)+","+std::to_string(r.bottom)+"]";}
class OutputState {
 std::mutex mutex;bool valid=false,visible=false;OutputPlan plan{};
public:
 void clear(OutputMode mode){std::lock_guard<std::mutex>lock(mutex);valid=false;visible=false;plan={};plan.mode=mode;}
 void set(const OutputPlan&p,bool show){std::lock_guard<std::mutex>lock(mutex);plan=p;valid=true;visible=show;}
 void hide(){std::lock_guard<std::mutex>lock(mutex);visible=false;}
 std::string fields(){std::lock_guard<std::mutex>lock(mutex);return std::string("\"outputInfoValid993\":")+(valid?"true":"false")+",\"outputMode993\":\""+outputModeName(plan.mode)+"\",\"outputPreview993\":"+(preview(plan.mode)?"true":"false")+",\"outputRect993\":"+rectJson(plan.client)+",\"outputWindowRect993\":"+rectJson(plan.window)+",\"outputContentRect993\":"+rectJson(plan.content)+",\"outputVisible993\":"+(visible?"true":"false")+",\"outputConstrained993\":"+(plan.constrained?"true":"false")+",\"outputInputPolicy993\":\""+(preview(plan.mode)?"click_stops_preview_no_mapping":"aligned_passthrough")+"\"";}
};
}
