#pragma once
#include <windows.h>
#include <atomic>
#include <array>
#include <memory>
#include <thread>
#include <functional>
#include <stdexcept>
#include <cstring>
#include <cmath>
namespace smx993 {
// Process-lifetime registry survives any late return from a driver API.
// A single ticket bounds all isolated workers and COM resources to one group.
inline std::atomic<bool>& collectorGate(){static auto* value=new std::atomic<bool>(false);return *value;}
struct CollectorTicket {~CollectorTicket(){collectorGate().store(false,std::memory_order_release);}};
struct OwnedEvent {HANDLE value=nullptr;OwnedEvent(){value=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!value)throw std::runtime_error("telemetry event creation failed");}~OwnedEvent(){if(value)CloseHandle(value);}};
struct WorkerPhase {
 std::atomic<const char*> name{"not_started"};std::atomic<double> since{0};
 static double now(){LARGE_INTEGER q{},f{};QueryPerformanceCounter(&q);QueryPerformanceFrequency(&f);return double(q.QuadPart)/f.QuadPart;}
 void set(const char*s){since.store(now(),std::memory_order_relaxed);name.store(s,std::memory_order_release);}
};
inline bool statsQueryExpired(const char* phase,double since,double now){return phase&&std::strcmp(phase,"dxgi.GetFrameStatistics")==0&&std::isfinite(since)&&std::isfinite(now)&&since>0&&now>=since&&now-since>.500;}
struct ShutdownResult {bool stopped=true,isolated=false;DWORD waitResult=WAIT_OBJECT_0,error=0;unsigned pendingMask=0;double elapsedMs=0;};
class CollectorGroup {
 std::shared_ptr<CollectorTicket>ticket_;std::shared_ptr<OwnedEvent>stop_;
 std::array<std::thread,3>threads_;unsigned count_=0;bool closed_=false;
public:
 CollectorGroup(){bool expected=false;if(!collectorGate().compare_exchange_strong(expected,true,std::memory_order_acq_rel))return;
  try{ticket_=std::make_shared<CollectorTicket>();stop_=std::make_shared<OwnedEvent>();}catch(...){ticket_.reset();collectorGate().store(false);throw;}}
 bool available()const{return bool(ticket_);}
 HANDLE stopEvent()const{return stop_?stop_->value:nullptr;}
 static bool priorGroupActive(){return collectorGate().load(std::memory_order_acquire);}
 void signal(){if(stop_)SetEvent(stop_->value);}
 void start(std::function<void(HANDLE)>body){
  if(!ticket_||closed_||count_>=threads_.size())throw std::runtime_error("telemetry group not available");
  threads_[count_]=std::thread([ticket=ticket_,event=stop_,body=std::move(body)]()mutable{
   try{body(event->value);}catch(...){}
   // Destruct closure-owned COM resources on worker before reopening admission.
   // A blocked COM Release also retains this group's ticket and event.
   body={};event.reset();ticket.reset();
  });++count_;
 }
 ShutdownResult shutdown(unsigned deadlineMs=700){
  ShutdownResult result;if(closed_)return result;closed_=true;const double begin=WorkerPhase::now();signal();
  HANDLE handles[3]{};for(unsigned i=0;i<count_;++i)handles[i]=threads_[i].native_handle();
  if(count_){result.waitResult=WaitForMultipleObjects(count_,handles,TRUE,deadlineMs);if(result.waitResult==WAIT_FAILED)result.error=GetLastError();}
  for(unsigned i=0;i<count_;++i)if(threads_[i].joinable()){
   if(WaitForSingleObject(handles[i],0)==WAIT_OBJECT_0)threads_[i].join();
   else{result.pendingMask|=1u<<i;threads_[i].detach();}
  }
  result.isolated=result.pendingMask!=0;result.stopped=!result.isolated;result.elapsedMs=(WorkerPhase::now()-begin)*1000;
  ticket_.reset();stop_.reset();return result;
 }
 ~CollectorGroup(){shutdown();}
};
inline unsigned collectorLifetimeSelfTest(){
 unsigned checks=0;auto require=[&](bool v){++checks;if(!v)throw std::runtime_error("collector lifetime selftest");};
 auto until=[&](auto fn,unsigned ms=1000){auto end=GetTickCount64()+ms;while(!fn()&&GetTickCount64()<end)Sleep(1);return fn();};
 require(!CollectorGroup::priorGroupActive());
 require(!statsQueryExpired("stats.wait",10,12));require(!statsQueryExpired("dxgi.GetFrameStatistics",10,10.5));require(statsQueryExpired("dxgi.GetFrameStatistics",10,10.5001));require(!statsQueryExpired("dxgi.GetFrameStatistics",10,9));require(!statsQueryExpired("dxgi.GetFrameStatistics",0,100));
 {CollectorGroup g;require(g.available());g.start([](HANDLE stop){WaitForSingleObject(stop,INFINITE);});auto r=g.shutdown(100);require(r.stopped&&!r.isolated);}
 require(!CollectorGroup::priorGroupActive());
 struct Context {std::shared_ptr<OwnedEvent>release=std::make_shared<OwnedEvent>();std::atomic<bool>entered{false};std::shared_ptr<std::atomic<unsigned>>destroyed,published;~Context(){if(destroyed)++*destroyed;}};
 auto destroyed=std::make_shared<std::atomic<unsigned>>(0),published=std::make_shared<std::atomic<unsigned>>(0);
 auto c=std::make_shared<Context>();c->destroyed=destroyed;c->published=published;std::weak_ptr<Context>weak=c;auto release=c->release;
 {CollectorGroup g;require(g.available());g.start([c](HANDLE stop){c->entered=true;WaitForSingleObject(c->release->value,INFINITE);if(WaitForSingleObject(stop,0)!=WAIT_OBJECT_0)++*c->published;});
  require(until([&]{return c->entered.load();}));auto r=g.shutdown(10);require(r.isolated&&r.pendingMask==1);require(r.elapsedMs<500);
  {CollectorGroup rejected;require(!rejected.available());}c.reset();require(!weak.expired());require(destroyed->load()==0);SetEvent(release->value);}
 require(until([&]{return !CollectorGroup::priorGroupActive();}));require(weak.expired());require(destroyed->load()==1);require(published->load()==0);
 {CollectorGroup next;require(next.available());require(next.shutdown(5).stopped);}
 auto release2=std::make_shared<OwnedEvent>();auto entered=std::make_shared<std::atomic<unsigned>>(0);
 {CollectorGroup g;for(unsigned i=0;i<2;++i)g.start([release2,entered](HANDLE){++*entered;WaitForSingleObject(release2->value,INFINITE);});
  g.start([](HANDLE stop){WaitForSingleObject(stop,INFINITE);});require(until([&]{return entered->load()==2;}));auto r=g.shutdown(10);require(r.pendingMask==3&&r.isolated);
  for(unsigned i=0;i<100;++i){CollectorGroup denied;require(!denied.available());}SetEvent(release2->value);}
 require(until([&]{return !CollectorGroup::priorGroupActive();}));
 struct Cleanup {std::shared_ptr<OwnedEvent>release,entered;~Cleanup(){SetEvent(entered->value);WaitForSingleObject(release->value,INFINITE);}};
 auto cleanRelease=std::make_shared<OwnedEvent>(),cleanEntered=std::make_shared<OwnedEvent>();
 {CollectorGroup g;auto resource=std::make_shared<Cleanup>();resource->release=cleanRelease;resource->entered=cleanEntered;g.start([resource](HANDLE stop){WaitForSingleObject(stop,INFINITE);});resource.reset();g.signal();
  require(WaitForSingleObject(cleanEntered->value,1000)==WAIT_OBJECT_0);auto r=g.shutdown(10);require(r.isolated);require(CollectorGroup::priorGroupActive());SetEvent(cleanRelease->value);}
 require(until([&]{return !CollectorGroup::priorGroupActive();}));return checks;
}
}
