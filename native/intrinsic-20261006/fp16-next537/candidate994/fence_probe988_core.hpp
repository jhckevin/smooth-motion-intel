#include "bounded_stdio989.h"
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>
namespace smx988 {
struct FenceRecord988 {
 uint64_t sequence=0,presentId=0,fenceValue=0,lastValue=0,fenceTag=0,queries=0;
 bool generated=false,complete=false,deviceError=false; double source=0,due=0,copyBegin=0,submit=0,signalReturn=0,flushReturn=0,published=0;
 double lastNegativeBegin=-1,lastNegativeEnd=-1,firstPositiveBegin=-1,firstPositiveEnd=-1,queryCostMs=0,maxQueryMs=0;
 double lowerBound()const{return std::max(submit,lastNegativeBegin);}
};
// Caller serializes this bookkeeping. No GPU, timestamps synthesized, or resource ownership.
class FenceCore988 {
public:
 static constexpr size_t Capacity=64,ReadBudget=16,LogLimit=72000;
 std::array<uint64_t,Capacity> slots{};std::vector<FenceRecord988> records;
 uint64_t attempted=0,ringFull=0,logFull=0,missingFence=0,invalidClock=0,bindErrors=0;size_t cursor=0;
 FenceCore988(){records.reserve(LogLimit);}
 size_t pending()const{size_t n=0;for(auto id:slots)n+=id!=0;return n;}
 uint64_t enqueue(FenceRecord988 r,size_t&slot){++attempted;if(!r.fenceTag||!r.fenceValue){++missingFence;return 0;}if(records.size()>=LogLimit){++logFull;return 0;}slot=0;while(slot<Capacity&&slots[slot])++slot;if(slot==Capacity){++ringFull;return 0;}r.sequence=records.size()+1;records.push_back(r);slots[slot]=r.sequence;return r.sequence;}
 bool bind(uint64_t seq,uint64_t id){if(!seq)return false;if(seq>records.size()||!id||records[seq-1].presentId){++bindErrors;return false;}records[seq-1].presentId=id;return true;}
 struct Selection{std::array<size_t,ReadBudget> indices{};size_t count=0;};
 Selection select(){Selection out;for(size_t n=0;n<Capacity&&out.count<ReadBudget;++n){size_t i=cursor;cursor=(cursor+1)%Capacity;if(slots[i])out.indices[out.count++]=i;}return out;}
 // Returns true on terminal sample. UINT64_MAX is DEVICE_REMOVED, never completion.
 bool observe(size_t slot,uint64_t seq,uint64_t value,double begin,double end){
  if(slot>=Capacity||!seq||slots[slot]!=seq)return false;
  auto&r=records[seq-1];r.lastValue=value;
  if(value==std::numeric_limits<uint64_t>::max()){r.deviceError=true;slots[slot]=0;return true;}
  if(!std::isfinite(begin)||!std::isfinite(end)||begin<r.submit||end<begin){++invalidClock;return false;}
  ++r.queries;double cost=(end-begin)*1000;r.queryCostMs+=cost;r.maxQueryMs=std::max(r.maxQueryMs,cost);
  if(value>=r.fenceValue){r.complete=true;r.firstPositiveBegin=begin;r.firstPositiveEnd=end;slots[slot]=0;return true;}
  r.lastNegativeBegin=begin;r.lastNegativeEnd=end;return false;
 }
};
}
