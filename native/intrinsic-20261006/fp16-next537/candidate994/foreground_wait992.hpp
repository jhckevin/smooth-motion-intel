#pragma once
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <initializer_list>
namespace smx992 {
enum class WaitResult { Waiting, Ready, Cancelled, TimedOut };
inline double foregroundTimeout(bool hosted,const char* raw){
 if(!raw||!*raw)return hosted?0.0:30.0;
 char* end=nullptr;double value=std::strtod(raw,&end);
 if(!end||*end||!std::isfinite(value)||value<0||value>3600)throw std::runtime_error("SMX_PREPARE_SECONDS862 must be 0..3600 seconds; zero means hosted unlimited or CLI default 30s");
 if(value==0)return hosted?0.0:30.0;
 return value;
}
inline WaitResult poll(double elapsed,double timeout,bool stop,bool targetAlive,bool foregroundReady){
 if(stop||!targetAlive)return WaitResult::Cancelled;
 if(foregroundReady)return WaitResult::Ready;
 if(timeout>0&&elapsed>=timeout)return WaitResult::TimedOut;
 return WaitResult::Waiting;
}
inline unsigned selfTest(){unsigned n=0;auto check=[&](bool b){++n;if(!b)throw std::runtime_error("foreground wait selftest");};
 check(foregroundTimeout(true,nullptr)==0);check(foregroundTimeout(false,nullptr)==30);
 check(foregroundTimeout(true,"0")==0);check(foregroundTimeout(false,"0")==30);check(foregroundTimeout(true,"2.5")==2.5);check(foregroundTimeout(false,"60")==60);
 for(const char*bad:{"-1","nan","inf","3601","30x"}){bool rejected=false;try{foregroundTimeout(true,bad);}catch(...){rejected=true;}check(rejected);}
 check(poll(100000,0,false,true,false)==WaitResult::Waiting);
 check(poll(100000,0,false,true,true)==WaitResult::Ready);
 check(poll(10,30,false,true,false)==WaitResult::Waiting);
 check(poll(30,30,false,true,false)==WaitResult::TimedOut);
 check(poll(30,30,false,true,true)==WaitResult::Ready);
 check(poll(100,30,true,true,true)==WaitResult::Cancelled);
 check(poll(100,30,false,false,true)==WaitResult::Cancelled);
 for(int e=0;e<=86400;e+=30)check(poll(e,0,false,true,false)==WaitResult::Waiting);
 // Focus loss after preparation is a wait; each return-to-focus ends the current wait, no retry/reinitialization.
 check(poll(42,0,false,true,false)==WaitResult::Waiting);check(poll(70,0,false,true,true)==WaitResult::Ready);
 check(poll(0,0,true,true,false)==WaitResult::Cancelled);check(poll(0,0,false,false,false)==WaitResult::Cancelled);
 return n;
}
}
