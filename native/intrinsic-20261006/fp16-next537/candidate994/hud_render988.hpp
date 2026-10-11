#include "bounded_stdio989.h"
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cmath>
#include <cstdint>
#include <cwchar>
#include <string>
#include <vector>
#include <algorithm>
#include "hud_metrics988.hpp"
namespace smx988 {
struct HudExtra {
 bool demo=false,details=false;uint32_t nativeWidth=0,nativeHeight=0;
 std::wstring backendName=L"",status=L"";
 uint64_t captureCount=0,inferCount=0,submitCount=0,lateCompleted=0,lateSubmitted=0;
 uint32_t inflight=0;bool countersValid=false;
 bool cpuPackageValid=false;double cpuPackageWatts=-1;
 bool powerValid=false;double powerWatts=0,powerAgeSeconds=-1;std::wstring powerDomain=L"Unknown";
 bool powerRangeValid=false;double powerMinWatts=0,powerMaxWatts=0;
 bool frequencyValid=false;double gpuMHz=0;bool throttleFlagsValid=false;uint32_t throttleFlags=0;
 bool gpuTimingValid=false,serviceTimingValid=false,queueTimingValid=false;
 double gpuMs=-1,serviceMs=-1,queueWaitMs=-1;
};
// Physical client pixels; deliberately independent of desktop DPI scaling.
inline RECT preferredHudRect(const RECT&client,bool details=false){
 const int cw=std::max(1,int(client.right-client.left)),ch=std::max(1,int(client.bottom-client.top));
 const double baseHeight=details?634.:364.;
 double scale=std::min({1.,double(cw)*.40/438.,double(ch)*.75/baseHeight});
 int w=std::max(1,int(std::floor(438*scale))),h=std::max(1,int(std::floor(baseHeight*scale)));
 int pad=std::max(2,std::min(12,cw/80)),x=std::max(int(client.left),int(client.right)-pad-w),y=int(client.top)+pad;
 return RECT{x,y,x+w,std::min(int(client.bottom),y+h)};
}
class HudPainter {
 HDC dc_=nullptr;HBITMAP bitmap_=nullptr,oldBitmap_=nullptr;void*bits_=nullptr;
 int width_=0,height_=0;double scale_=1;bool details_=false;ULONGLONG lastPaint_=0;bool hasPaint_=false;
 HFONT textFont_=nullptr,smallFont_=nullptr;
 HPEN border_=nullptr,grid_=nullptr,blue_=nullptr;HBRUSH background_=nullptr;
 static constexpr COLORREF Bg=RGB(8,10,12),White=RGB(242,244,246),Gray=RGB(169,173,179),Blue=RGB(55,181,244),Green=RGB(105,221,137),Amber=RGB(237,205,97);
 int px(double v)const{return int(std::lround(v*scale_));}
 RECT box(double x,double y,double w,double h)const{return RECT{px(x),px(y),px(x+w),px(y+h)};}
 static std::wstring num(double v,int digits=1){if(!std::isfinite(v)||v<0)return L"—";wchar_t b[64]{};swprintf_s(b,L"%.*f",digits,v);return b;}
 static std::wstring count(uint64_t v){wchar_t b[40]{};swprintf_s(b,L"%llu",static_cast<unsigned long long>(v));return b;}
 void text(const std::wstring&s,double x,double y,double w,double h,COLORREF color=White,UINT align=DT_LEFT,bool smallText=false){
  auto old=SelectObject(dc_,smallText?smallFont_:textFont_);SetTextColor(dc_,color);RECT r=box(x,y,w,h);
  DrawTextW(dc_,s.c_str(),int(s.size()),&r,align|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);SelectObject(dc_,old);
 }
 void line(double x,double y,double ex,double ey,HPEN pen){auto old=SelectObject(dc_,pen);MoveToEx(dc_,px(x),px(y),nullptr);LineTo(dc_,px(ex),px(ey));SelectObject(dc_,old);}
 void row(const std::wstring&label,const std::wstring&value,double y,COLORREF color=White){text(label,16,y,273,24);text(value,263,y,159,24,color,DT_RIGHT);}
 HFONT font(double size){return CreateFontW(-std::max(10,px(size)),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,FIXED_PITCH|FF_MODERN,L"Consolas");}
 bool ensure(int w,int h,bool details){
  if(dc_&&width_==w&&height_==h&&details_==details)return true;reset();if(w<1||h<1||w>2048||h>2048)return false;
  width_=w;height_=h;details_=details;scale_=std::min(double(w)/438,double(h)/(details?634:364));dc_=CreateCompatibleDC(nullptr);if(!dc_){reset();return false;}
  BITMAPINFO bi{};bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=w;bi.bmiHeader.biHeight=-h;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;
  bitmap_=CreateDIBSection(dc_,&bi,DIB_RGB_COLORS,&bits_,nullptr,0);if(!bitmap_){reset();return false;}oldBitmap_=(HBITMAP)SelectObject(dc_,bitmap_);
  textFont_=font(20);smallFont_=font(12);border_=CreatePen(PS_SOLID,1,RGB(154,159,165));grid_=CreatePen(PS_SOLID,1,RGB(113,120,128));blue_=CreatePen(PS_SOLID,std::max(1,px(1.3)),Blue);background_=CreateSolidBrush(Bg);
  if(!textFont_||!smallFont_||!border_||!grid_||!blue_||!background_){reset();return false;}return true;
 }
 static std::wstring powerDomain(const std::wstring&s){
  if(s.find(L"Package")!=std::wstring::npos)return L"PKG";if(s.find(L"Card")!=std::wstring::npos)return L"Card";if(s.find(L"Stack")!=std::wstring::npos)return L"Stack";if(s.find(L"Memory")!=std::wstring::npos)return L"Memory";if(s.find(L"GPU")!=std::wstring::npos)return L"GPU";return L"?";
 }
 static std::wstring backend(const std::wstring&s){if(s.find(L"DComp")!=std::wstring::npos||s==L"dxgi")return L"DComp";if(s.find(L"Vulkan")!=std::wstring::npos)return L"Vulkan";return L"—";}
 static std::wstring intervalMean(const HudMetrics::Snapshot&s){
  if(s.graph.empty())return L"—";size_t start=s.graph.size()>120?s.graph.size()-120:0;double sum=0;size_t n=0;
  for(size_t i=start;i<s.graph.size();++i){const auto&p=s.graph[i];if(p.gap||!p.timed||!std::isfinite(p.intervalMs)||p.intervalMs<=0)return L"—";sum+=p.intervalMs;++n;}
  return n?num(sum/n,2)+L" ms":L"—";
 }
 void graph(const HudMetrics::Snapshot&s,double top,double bottom){
  const double left=16,right=422;for(unsigned k=0;k<=12;k++){double x=left+(right-left)*k/12;line(x,top,x,bottom,grid_);}line(left,top,right,top,border_);line(left,bottom,right,bottom,border_);
  double maxMs=33.3334;size_t start=s.graph.size()>120?s.graph.size()-120:0;
  for(size_t i=start;i<s.graph.size();++i)if(s.graph[i].timed&&std::isfinite(s.graph[i].intervalMs))maxMs=std::max(maxMs,std::min(200.,s.graph[i].intervalMs*1.08));
  auto yy=[&](double ms){return bottom-2-std::min(maxMs,std::max(0.,ms))/maxMs*(bottom-top-4);};
  size_t n=s.graph.size()-start;double oldX=0,oldY=0;bool previous=false;
  for(size_t j=0;j<n;++j){const auto&p=s.graph[start+j];double x=left+(120-n+j)*(right-left)/119.;bool good=p.timed&&std::isfinite(p.intervalMs)&&p.intervalMs>0;
   if(!good||p.gap){previous=false;continue;}double y=yy(p.intervalMs);if(previous)line(oldX,oldY,x,y,blue_);else SetPixelV(dc_,px(x),px(y),Blue);oldX=x;oldY=y;previous=true;}
 }
 void paint(const HudMetrics::Snapshot&s,const HudExtra&e){
  RECT r{0,0,width_,height_};FillRect(dc_,&r,background_);SetBkMode(dc_,TRANSPARENT);
  auto oldP=SelectObject(dc_,border_);auto oldB=SelectObject(dc_,GetStockObject(NULL_BRUSH));RoundRect(dc_,0,0,width_,height_,std::max(2,px(20)),std::max(2,px(20)));SelectObject(dc_,oldB);SelectObject(dc_,oldP);
  text(L"Intel GPU",16,12,300,24);if(e.demo)text(L"DEMO",342,12,80,24,Amber,DT_RIGHT,true);else if(e.details)text(L"DETAILS",324,12,98,24,Gray,DT_RIGHT,true);
  std::wstring geometry=e.nativeWidth&&e.nativeHeight?count(e.nativeWidth)+L"x"+count(e.nativeHeight):L"—";row(geometry,backend(e.backendName),36,Green);
  bool showRate=!s.warmingUp&&!s.stalled&&s.coverage>0;
  bool pOk=e.powerValid&&std::isfinite(e.powerWatts)&&e.powerWatts>=0;
  uint64_t lc=e.countersValid?e.lateCompleted:s.lateCompleted,ls=e.countersValid?e.lateSubmitted:s.lateSubmitted;
  double gpu=e.gpuTimingValid?e.gpuMs:s.modelGpuMs,service=e.serviceTimingValid?e.serviceMs:s.serviceMs,queue=e.queueTimingValid?e.queueWaitMs:s.queueWaitMs;
  bool gv=e.gpuTimingValid||s.modelTimingValid,sv=e.serviceTimingValid||s.modelTimingValid,qv=e.queueTimingValid||s.queueWaitValid;
  if(!e.details){
   row(L"FPS",showRate?(s.displayValid?L"":L"≥")+num(s.displayHz):L"—",72);
   if(!s.displayValid&&showRate)text(L"Cov "+num(std::clamp(s.coverage,0.,1.)*100,0)+L"%",207,72,94,24,Gray,DT_RIGHT,true);
   row(L"Native / FG",showRate?num(s.originalHz)+L" / "+num(s.generatedHz):L"— / —",96);
   row(L"1% Low",s.low1PercentValid?num(s.low1PercentHz):L"—",120);
   row(L"GPU",gv?num(gpu,2)+L" ms":L"—",144);
   row(L"GPU driver ("+powerDomain(e.powerDomain)+L")",pOk?num(e.powerWatts)+L" W":L"—",168);
   row(L"CPU Package",e.cpuPackageValid?num(e.cpuPackageWatts)+L" W":L"—",192);
   row(L"Present / Late",count(e.countersValid?e.submitCount:s.submittedCount)+L" / "+count(ls),216,ls?Amber:White);
   row(L"Frame Interval",intervalMean(s),240);graph(s,276,348);
  }else{
   row(L"Capture",e.countersValid?count(e.captureCount):L"—",72);
   row(L"Infer",count(e.countersValid?e.inferCount:s.generationCompleteCount),96);
   row(L"Present",count(e.countersValid?e.submitCount:s.submittedCount),120);
   row(L"Display (epoch)",count(s.observedDisplayCount),144);
   row(L"Unconfirmed (epoch)",count(s.epochSubmittedCount>s.observedDisplayCount?s.epochSubmittedCount-s.observedDisplayCount:0),168);
   row(L"Late Complete",count(lc),192,lc?Amber:White);row(L"Late Present",count(ls),216,ls?Amber:White);
   row(L"Inflight",count(e.countersValid?e.inflight:s.inflight),240);
   row(L"Content to Display",s.latencyValid?num(s.contentToDisplayMs)+L" ms":L"—",264);
   row(L"Present to Display",s.latencyValid?num(s.presentToDisplayMs)+L" ms":L"—",288);
   row(L"Submit to Model",qv?num(queue)+L" ms":L"—",312);
   row(L"GPU / Service",(gv?num(gpu):L"—")+L" / "+(sv?num(service):L"—")+L" ms",336);
   row(L"P95 Interval",s.validIntervals?num(s.displayIntervalP95Ms)+L" ms":L"—",360);
   row(L"Coverage",num(std::clamp(s.coverage,0.,1.)*100,0)+L"%",384);
   row(L"Missing / Unknown",count(s.missingIds)+L" / "+count(s.unmappedCount+s.untimedCount),408);
   row(L"GPU driver ("+powerDomain(e.powerDomain)+L")",pOk?num(e.powerWatts)+L" W":L"—",432);
   row(L"CPU Package",e.cpuPackageValid?num(e.cpuPackageWatts)+L" W":L"—",456);
   row(L"GPU Clock",e.frequencyValid?num(e.gpuMHz,0)+L" MHz":L"—",480);
   graph(s,520,590);
   text(L"GPU driver scope unverified",16,598,406,16,Gray,DT_LEFT,true);
   text(e.cpuPackageValid?L"CPU package: Windows RAPL PKG":L"CPU package: unavailable / warming",16,614,406,16,Gray,DT_LEFT,true);
  }
 }
public:
 HudPainter()=default;HudPainter(const HudPainter&)=delete;HudPainter&operator=(const HudPainter&)=delete;~HudPainter(){reset();}
 void reset(){if(dc_&&oldBitmap_)SelectObject(dc_,oldBitmap_);oldBitmap_=nullptr;if(bitmap_)DeleteObject(bitmap_);bitmap_=nullptr;bits_=nullptr;
  for(HFONT*f:{&textFont_,&smallFont_})if(*f){DeleteObject(*f);*f=nullptr;}for(HPEN*p:{&border_,&grid_,&blue_})if(*p){DeleteObject(*p);*p=nullptr;}
  if(background_)DeleteObject(background_);background_=nullptr;if(dc_)DeleteDC(dc_);dc_=nullptr;width_=height_=0;hasPaint_=false;lastPaint_=0;}
 bool renderToBacking(int w,int h,const HudMetrics::Snapshot&s,const HudExtra&e,bool force=false){if(!ensure(w,h,e.details))return false;ULONGLONG now=GetTickCount64();if(force||!hasPaint_||now-lastPaint_>=250){paint(s,e);GdiFlush();lastPaint_=now;hasPaint_=true;}return true;}
 bool draw(HDC target,const RECT&r,const HudMetrics::Snapshot&s,const HudExtra&e){int w=int(r.right-r.left),h=int(r.bottom-r.top);if(!renderToBacking(w,h,s,e))return false;return BitBlt(target,r.left,r.top,w,h,dc_,0,0,SRCCOPY)!=FALSE;}
 // Own cached DIB only: neither desktop capture nor composition/physical-screen proof.
 bool dumpBackingBmp(const wchar_t*path)const{if(!path||!hasPaint_||!bits_||width_<1||height_<1)return false;HANDLE f=CreateFileW(path,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(f==INVALID_HANDLE_VALUE)return false;
  BITMAPFILEHEADER fh{};BITMAPINFOHEADER ih{};DWORD bytes=DWORD(width_)*DWORD(height_)*4;fh.bfType=0x4d42;fh.bfOffBits=sizeof(fh)+sizeof(ih);fh.bfSize=fh.bfOffBits+bytes;
  ih.biSize=sizeof(ih);ih.biWidth=width_;ih.biHeight=-height_;ih.biPlanes=1;ih.biBitCount=32;ih.biCompression=BI_RGB;ih.biSizeImage=bytes;DWORD n=0;
  bool ok=WriteFile(f,&fh,sizeof(fh),&n,nullptr)&&n==sizeof(fh);ok=ok&&WriteFile(f,&ih,sizeof(ih),&n,nullptr)&&n==sizeof(ih);ok=ok&&WriteFile(f,bits_,bytes,&n,nullptr)&&n==bytes;CloseHandle(f);return ok;}
};
inline HudPainter& hudPainter988(){static thread_local HudPainter p;return p;}
inline bool drawHud(HDC dc,const RECT&r,const HudMetrics::Snapshot&s,const HudExtra&e){return hudPainter988().draw(dc,r,s,e);}
inline void releaseHud(){hudPainter988().reset();}
inline bool dumpHudBackingBmp(const wchar_t*path){return hudPainter988().dumpBackingBmp(path);}
} // namespace smx988
