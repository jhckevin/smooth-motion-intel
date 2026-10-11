#pragma once
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <string_view>
namespace smx990 {
struct Rect {int64_t left=0,top=0,right=0,bottom=0;};
struct WindowInfo {bool valid=false,fullscreen=false,foreground=false;double coverage=0;int clientWidth=0,clientHeight=0,monitorWidth=0,monitorHeight=0;Rect client{},monitor{};};
inline WindowInfo geometry(Rect c,Rect m,bool borderless,bool foreground){
 WindowInfo s;s.client=c;s.monitor=m;s.foreground=foreground;
 const int64_t cw=c.right-c.left,ch=c.bottom-c.top,mw=m.right-m.left,mh=m.bottom-m.top;
 if(cw<=0||ch<=0||mw<=0||mh<=0||cw>16384||ch>16384||mw>131072||mh>131072)return s;
 s.valid=true;s.clientWidth=int(cw);s.clientHeight=int(ch);s.monitorWidth=int(mw);s.monitorHeight=int(mh);
 const int64_t w=std::max<int64_t>(0,std::min(c.right,m.right)-std::max(c.left,m.left));
 const int64_t h=std::max<int64_t>(0,std::min(c.bottom,m.bottom)-std::max(c.top,m.top));
 s.coverage=double(w)*double(h)/(double(mw)*double(mh));
 s.fullscreen=borderless&&std::abs(c.left-m.left)<=1&&std::abs(c.top-m.top)<=1&&std::abs(c.right-m.right)<=1&&std::abs(c.bottom-m.bottom)<=1;
 return s;
}
inline const char* reason(bool closed,bool minimized,bool visible,bool foreground,bool obscured,bool ready){
 if(closed)return "closed";if(minimized)return "minimized";if(!visible)return "hidden";if(!foreground)return "focus";if(obscured)return "obscured";if(!ready)return "preparing";return "none";
}
inline bool blocked(bool exists,bool identityMatches,bool minimized,bool visible,bool foreground){return !exists||!identityMatches||minimized||!visible||!foreground;}
inline unsigned selfTest(){unsigned n=0;auto check=[&](bool b){++n;if(!b)throw n;};
 auto exact=geometry({0,0,1920,1080},{0,0,1920,1080},true,true);check(exact.valid&&exact.fullscreen&&exact.coverage==1);
 check(!geometry({0,0,1920,1080},{0,0,1920,1080},false,true).fullscreen);
 check(geometry({-1920,0,0,1080},{-1920,0,0,1080},true,false).fullscreen);
 check(geometry({1,1,1919,1079},{0,0,1920,1080},true,true).fullscreen);
 check(!geometry({2,0,1920,1080},{0,0,1920,1080},true,true).fullscreen);
 check(!geometry({0,0,0,0},{0,0,1920,1080},true,true).valid);
 check(!geometry({0,0,20000,1080},{0,0,1920,1080},true,true).valid);
 check(geometry({1920,0,3840,1080},{0,0,1920,1080},true,true).coverage==0);
 check(geometry({0,0,960,1080},{0,0,1920,1080},true,true).coverage==.5);
 check(!geometry({-10,-10,1930,1090},{0,0,1920,1080},true,true).fullscreen);
 check(geometry({-10,-10,1930,1090},{0,0,1920,1080},true,true).coverage==1);
 check(reason(true,true,false,false,true,false)==std::string_view("closed"));
 check(reason(false,true,false,false,true,false)==std::string_view("minimized"));
 check(reason(false,false,false,false,true,false)==std::string_view("hidden"));
 check(reason(false,false,true,false,true,false)==std::string_view("focus"));
 check(reason(false,false,true,true,true,false)==std::string_view("obscured"));
 check(reason(false,false,true,true,false,false)==std::string_view("preparing"));
 check(reason(false,false,true,true,false,true)==std::string_view("none"));
 for(unsigned mask=0;mask<32;++mask){bool e=mask&1,i=mask&2,m=mask&4,v=mask&8,f=mask&16;check(blocked(e,i,m,v,f)==(!e||!i||m||!v||!f));}
 return n;
}
}
