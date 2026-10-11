#pragma once
// Read-only evidence for the selected game process; a window close is NOT proof
// of process termination. A retained handle prevents PID reuse in this record.
class TargetProcessEvidence993 {
 std::mutex mutex;HANDLE process=nullptr;DWORD pid=0,openError=0;uint64_t creation=0;bool creationValid=false;std::string path;
public:
 ~TargetProcessEvidence993(){if(process)CloseHandle(process);}
 void begin(DWORD targetPid,const char*directory){
  std::lock_guard<std::mutex>lock(mutex);if(process)CloseHandle(process);process=nullptr;pid=targetPid;creation=0;creationValid=false;path=std::string(directory&&*directory?directory:".")+"\\app-target-process993.jsonl";
  SetLastError(0);process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid);openError=process?0:GetLastError();
  if(process){FILETIME c{},e{},k{},u{};creationValid=GetProcessTimes(process,&c,&e,&k,&u)!=FALSE;if(creationValid)creation=uint64_t(c.dwLowDateTime)|(uint64_t(c.dwHighDateTime)<<32);}
 }
 void record(const char*event){
  std::lock_guard<std::mutex>lock(mutex);if(path.empty())return;
  DWORD wait=process?WaitForSingleObject(process,0):WAIT_FAILED,waitError=wait==WAIT_FAILED?GetLastError():0,exitCode=0;
  bool exited=process&&wait==WAIT_OBJECT_0;bool exitKnown=exited&&GetExitCodeProcess(process,&exitCode);
  if(FILE*f=smx_log_fopen989(path.c_str(),"ab")){smx_log_fprintf989(f,"{\"event\":\"%s\",\"qpc_seconds\":%.9f,\"pid\":%lu,\"retained_handle\":%s,\"open_error\":%lu,\"creation_time_valid\":%s,\"creation_time_filetime\":%llu,\"process_signaled\":%s,\"wait_result\":%lu,\"wait_error\":%lu,\"exit_code_valid\":%s,\"exit_code\":%lu,\"scope\":\"Read-only selected-process identity and termination evidence; exit code does not establish the cause of game termination.\"}\n",event,smx988::qpcNow988(),pid,process?"true":"false",openError,creationValid?"true":"false",(unsigned long long)creation,exited?"true":"false",wait,waitError,exitKnown?"true":"false",exitCode);smx_log_fclose989(f);}
 }
};
