#include "processing_size992.hpp"
#include <cassert>
#include <iostream>
#include <cmath>
#include <thread>
#include <atomic>
int main(){
 unsigned checks=0;auto check=[&](bool b){++checks;if(!b)throw std::runtime_error("check failed "+std::to_string(checks));};
 auto test=[&](unsigned w,unsigned h,const char*m,unsigned ew,unsigned eh){auto e=smx992::processingExtent(w,h,m);check(e.width==ew&&e.height==eh);check(e.resampled==(w!=ew||h!=eh));};
 test(2880,1620,"fit1080",1920,1080);test(2880,1620,"fit1440",2560,1440);test(2880,1620,"native",2880,1620);
 test(1600,900,"fit1080",1600,900);test(1080,1920,"fit1080",607,1080);test(3440,1440,"fit1080",1920,803);
 test(10001,7777,"fit1440",1851,1440);test(1,16384,"fit1080",1,1080);test(16384,1,"fit1080",1920,1);
 test(1919,1079,"fit1080",1919,1079);test(1921,1080,"fit1080",1920,1079);test(1,1,"fit1440",1,1);
 for(const auto*mode:{"native","fit1080","fit1440"})for(unsigned w=1;w<16385;w+=127)for(unsigned h=1;h<16385;h+=131){
 auto e=smx992::processingExtent(w,h,mode);check(e.width>=1&&e.height>=1&&e.width<=w&&e.height<=h);
 if(e.mode==smx992::ProcessingMode::Native)check(e.width==w&&e.height==h);
 else{unsigned bw=e.mode==smx992::ProcessingMode::Fit1080?1920:2560,bh=e.mode==smx992::ProcessingMode::Fit1080?1080:1440;check(e.width<=bw&&e.height<=bh);
 double scale=std::min(1.0,std::min(double(bw)/w,double(bh)/h));
 check(std::abs(double(e.width)-w*scale)<1.000001&&std::abs(double(e.height)-h*scale)<1.000001);}
 }
 for(auto a: {0u,16385u,0xffffffffu}){bool threw=false;try{smx992::processingExtent(a,1080,"native");}catch(...){threw=true;}check(threw);}
 bool invalid=false;try{smx992::processingExtent(100,100,"1080");}catch(...){invalid=true;}check(invalid);
 auto e=smx992::processingExtent(2880,1620,"fit1080");auto c=smx992::scalingCost(e,2880,1650,6);
 check(c.captureDeltaAligned==6*(smx992::imageAlignedBytes(2880,1650)-smx992::imageAlignedBytes(1920,1080)));
 check(c.fallbackAligned==smx992::imageAlignedBytes(2880,1620));check(c.extraEstimate>c.captureDeltaAligned+c.fallbackAligned);
 check(smx992::scalingCost(smx992::processingExtent(2880,1620,"native"),2880,1650,6).extraEstimate==0);
 smx992::ProcessingState state;check(state.fields().find("\"processingInfoValid992\":false")!=std::string::npos);state.set(e);check(state.fields().find("\"processingWidth992\":1920")!=std::string::npos);state.clear();check(state.fields().find("\"sourceWidth992\":0")!=std::string::npos);
 std::atomic<bool>done{false};std::thread writer([&](){for(int i=0;i<5000;i++){state.set(e);state.clear();}done=true;});while(!done){auto j=state.fields();check(j.find("\"processingWidth992\":")!=std::string::npos);}writer.join();
 std::cout<<"{\"pass\":true,\"checks\":"<<checks<<",\"scope\":\"CPU extent, bounded cost, concurrent state; no GPU or image quality evidence\"}\n";
}
