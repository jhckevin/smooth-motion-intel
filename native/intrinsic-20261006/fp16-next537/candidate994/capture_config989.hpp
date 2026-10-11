#pragma once
#include <windows.h>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <stdexcept>
namespace smx989 {
inline unsigned sessionSeconds(const char*s){if(!s||!*s)return 0;char*end=nullptr;unsigned long n=strtoul(s,&end,10);if(!end||*end||*s=='-'||(n!=0&&(n<10||n>14400)))throw std::runtime_error("Session duration must be 0 (continuous), or 10..14400 seconds");return unsigned(n);}
inline bool desktopCapture(const char*s){if(!s||!strcmp(s,"wgc"))return false;if(!strcmp(s,"dxgi"))return true;throw std::runtime_error("Unknown capture backend; choose wgc or dxgi");}
inline bool clientInsideMonitor(const RECT&monitor,POINT origin,LONG width,LONG height){return width>0&&height>0&&origin.x>=monitor.left&&origin.y>=monitor.top&&int64_t(origin.x)+width<=monitor.right&&int64_t(origin.y)+height<=monitor.bottom;}
}
