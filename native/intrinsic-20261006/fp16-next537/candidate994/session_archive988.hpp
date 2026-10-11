#include "bounded_stdio989.h"
#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
namespace smx988 {
inline bool sessionArchivePrefix988(unsigned session,std::string& prefix,DWORD& error){
 FILETIME creation{},exit{},kernel{},user{};
 if(!GetProcessTimes(GetCurrentProcess(),&creation,&exit,&kernel,&user)){error=GetLastError();return false;}
 const uint64_t stamp=(uint64_t(creation.dwHighDateTime)<<32)|creation.dwLowDateTime;
 prefix="session783-p"+std::to_string(GetCurrentProcessId())+"-c"+std::to_string(stamp)+"-s"+std::to_string(session)+"-";
 error=ERROR_SUCCESS;return true;
}
struct SessionArchiveCopy988 {bool copied=false;DWORD error=ERROR_SUCCESS;unsigned collisions=0;std::string destination;};
inline SessionArchiveCopy988 copySessionArchive988(const std::string& source,const std::string& directory,const std::string& prefix,const char* name){
 SessionArchiveCopy988 result;
 // Never overwrite old evidence. Repeated calls even with identical identity
 // have a small bounded collision fallback; unrelated failures are not retried.
 for(unsigned i=0;i<32;++i){
  result.destination=directory+"\\"+prefix+(i?"copy"+std::to_string(i)+"-":"")+name;
  if(CopyFileA(source.c_str(),result.destination.c_str(),TRUE)){result.copied=true;result.error=ERROR_SUCCESS;return result;}
  result.error=GetLastError();
  if(result.error!=ERROR_FILE_EXISTS&&result.error!=ERROR_ALREADY_EXISTS)return result;
  ++result.collisions;
 }
 return result;
}
}
