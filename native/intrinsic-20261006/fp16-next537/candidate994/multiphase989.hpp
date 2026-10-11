#include <cstdlib>
#pragma once
#include <vector>
#include <stdexcept>
#include <algorithm>
namespace smx989 {
struct MidpointStep { unsigned left,right,output,rank; };
inline void appendMidpoints(unsigned left,unsigned right,std::vector<MidpointStep>&out) {
 if(right-left<=1)return;
 unsigned mid=(left+right)/2;out.push_back({left,right,mid,unsigned(out.size()+1)});
 appendMidpoints(left,mid,out);appendMidpoints(mid,right,out);
}
inline std::vector<MidpointStep> midpointPlan(unsigned multiplier) {
 if(multiplier!=2&&multiplier!=4&&multiplier!=8)throw std::invalid_argument("Multiplier must be 2, 4 or 8");
 std::vector<MidpointStep> p;appendMidpoints(0,multiplier,p);return p;
}
inline unsigned frameMultiplier() {
 const char* value=std::getenv("SMX_FRAME_MULTIPLIER989");
 if(!value)return 2;
 char* end=nullptr;unsigned long parsed=std::strtoul(value,&end,10);
 if(!*value||!end||*end||(parsed!=2&&parsed!=4&&parsed!=8))throw std::invalid_argument("Invalid frame multiplier");
 return unsigned(parsed);
}
}
