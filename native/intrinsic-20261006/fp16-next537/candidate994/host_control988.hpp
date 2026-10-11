#include "bounded_stdio989.h"
#pragma once
#include <windows.h>
#include <sddl.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <cstdint>
#include <limits>
#include <functional>
namespace host988 {
constexpr UINT CommandMessage=WM_APP+40, ShutdownMessage=WM_APP+41;
inline uint64_t filetime(FILETIME f){return (uint64_t(f.dwHighDateTime)<<32)|f.dwLowDateTime;}
inline bool decimal(const std::string&s,uint64_t&v){if(s.empty())return false;v=0;for(char c:s){if(c<'0'||c>'9'||v>(UINT64_MAX-uint64_t(c-'0'))/10)return false;v=v*10+uint64_t(c-'0');}return true;}
inline std::string utf8(const std::wstring&s){if(s.empty())return {};int n=WideCharToMultiByte(CP_UTF8,0,s.data(),int(s.size()),nullptr,0,nullptr,nullptr);std::string o(size_t(n),'\0');WideCharToMultiByte(CP_UTF8,0,s.data(),int(s.size()),o.data(),n,nullptr,nullptr);return o;}
inline std::string quote(const std::string&s){std::string o="\"";for(unsigned char c:s){if(c=='"'||c=='\\'){o+='\\';o+=char(c);}else if(c<32){char x[7];sprintf_s(x,"\\u%04x",unsigned(c));o+=x;}else o+=char(c);}return o+'"';}
struct Command {enum Kind{Ping,Status,Start,Stop,Hud,Detail,Quit,Invalid}kind=Invalid;uint64_t hwnd=0,pid=0,creation=0;bool enabled=false;};
inline Command parse(const std::string&line){Command c;std::vector<std::string>v;size_t pos=0;while(pos<line.size()){while(pos<line.size()&&(line[pos]==' '||line[pos]=='\r'))++pos;size_t begin=pos;while(pos<line.size()&&line[pos]!=' '&&line[pos]!='\r')++pos;if(begin<pos)v.push_back(line.substr(begin,pos-begin));}if(v.empty())return c;
 if(v.size()==1){if(v[0]=="PING")c.kind=Command::Ping;else if(v[0]=="STATUS")c.kind=Command::Status;else if(v[0]=="STOP")c.kind=Command::Stop;else if(v[0]=="QUIT")c.kind=Command::Quit;}
 else if(v.size()==2&&(v[0]=="HUD"||v[0]=="DETAIL")&&(v[1]=="0"||v[1]=="1")){c.kind=v[0]=="HUD"?Command::Hud:Command::Detail;c.enabled=v[1]=="1";}
 else if(v.size()==4&&v[0]=="START"&&decimal(v[1],c.hwnd)&&decimal(v[2],c.pid)&&decimal(v[3],c.creation)&&c.hwnd&&c.pid&&c.pid<=MAXDWORD&&c.creation)c.kind=Command::Start;return c;}
struct Config {bool hosted=false;std::wstring name;DWORD pid=0;uint64_t creation=0;};
inline Config config(){Config o;int argc=0;LPWSTR*argv=CommandLineToArgvW(GetCommandLineW(),&argc);if(!argv)throw std::runtime_error("Command line parse");try{for(int i=1;i<argc;i++)if(!wcscmp(argv[i],L"--hosted"))o.hosted=true;if(o.hosted){for(int i=1;i<argc;i++){std::wstring k=argv[i];if(k==L"--hosted")continue;if(i+1>=argc)throw std::runtime_error("Hosted argument missing");std::wstring v=argv[++i];uint64_t n=0;if(k==L"--pipe")o.name=v;else if(k==L"--host-pid"){if(!decimal(utf8(v),n)||!n||n>MAXDWORD)throw std::runtime_error("Host PID invalid");o.pid=DWORD(n);}else if(k==L"--host-start"){if(!decimal(utf8(v),o.creation)||!o.creation)throw std::runtime_error("Host creation time invalid");}else throw std::runtime_error("Unknown hosted argument");}
 if(o.name.size()<8||o.name.size()>100||!o.pid||!o.creation)throw std::runtime_error("Hosted identity incomplete");for(wchar_t c:o.name)if(!((c>=L'a'&&c<=L'z')||(c>=L'A'&&c<=L'Z')||(c>=L'0'&&c<=L'9')||c==L'-'||c==L'_'||c==L'.'))throw std::runtime_error("Pipe name invalid");}}catch(...){LocalFree(argv);throw;}LocalFree(argv);return o;}
inline bool processMatches(HANDLE process,uint64_t expected){FILETIME c,e,k,u;return process&&GetProcessTimes(process,&c,&e,&k,&u)&&filetime(c)==expected&&WaitForSingleObject(process,0)==WAIT_TIMEOUT;}
inline std::wstring fullPath(const wchar_t*p){wchar_t b[32768];DWORD n=GetFullPathNameW(p,32768,b,nullptr);if(!n||n>=32768)throw std::runtime_error("Workspace path invalid");std::wstring v(b);while(v.size()>3&&(v.back()==L'\\'||v.back()==L'/'))v.pop_back();return v;}
struct Request {Command command;HANDLE done=CreateEventW(nullptr,TRUE,FALSE,nullptr);std::atomic<bool> cancelled{false};std::string response;~Request(){if(done)CloseHandle(done);}};
class Server {
 Config cfg_;HWND window_;HANDLE parent_=nullptr,stop_=nullptr,pipe_=INVALID_HANDLE_VALUE,lock_=INVALID_HANDLE_VALUE;std::thread thread_,watchdog_;HANDLE finished_=nullptr;std::mutex mutex_;std::shared_ptr<Request> request_;ULONGLONG lastRequest_=0;bool announced_=false;
 void announce(){if(!announced_){announced_=true;watchdog_=std::thread([this]{if(WaitForSingleObject(finished_,15000)!=WAIT_OBJECT_0)TerminateProcess(GetCurrentProcess(),89);});PostMessageW(window_,ShutdownMessage,0,0);}}
 bool alive(){if(WaitForSingleObject(stop_,0)==WAIT_OBJECT_0)return false;if(WaitForSingleObject(parent_,0)!=WAIT_TIMEOUT||GetTickCount64()-lastRequest_>20000){announce();return false;}return true;}
 bool completeIo(OVERLAPPED&o,BOOL immediate,DWORD error,DWORD&bytes,DWORD timeout){if(immediate)return true;if(error!=ERROR_IO_PENDING)return false;HANDLE handles[]={stop_,parent_,o.hEvent};DWORD result=WaitForMultipleObjects(3,handles,FALSE,timeout);if(result==WAIT_OBJECT_0+2)return GetOverlappedResult(pipe_,&o,&bytes,FALSE)!=FALSE;CancelIoEx(pipe_,&o);if(WaitForSingleObject(o.hEvent,1000)!=WAIT_OBJECT_0)TerminateProcess(GetCurrentProcess(),88);if(result==WAIT_OBJECT_0+1)announce();return false;}
 bool transfer(bool write,void*data,DWORD count,DWORD&bytes,DWORD timeout=1500){OVERLAPPED o{};o.hEvent=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!o.hEvent)return false;BOOL ok=write?WriteFile(pipe_,data,count,&bytes,&o):ReadFile(pipe_,data,count,&bytes,&o);DWORD err=ok?0:GetLastError();bool result=completeIo(o,ok,err,bytes,timeout);CloseHandle(o.hEvent);return result;}
 bool connect(){OVERLAPPED o{};o.hEvent=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!o.hEvent)return false;BOOL ok=ConnectNamedPipe(pipe_,&o);DWORD err=ok?0:GetLastError(),bytes=0;if(err==ERROR_PIPE_CONNECTED)ok=TRUE;bool result=completeIo(o,ok,err,bytes,500);CloseHandle(o.hEvent);return result;}
 void loop(){lastRequest_=GetTickCount64();while(alive()){if(!connect()){DisconnectNamedPipe(pipe_);continue;}ULONG client=0;if(!GetNamedPipeClientProcessId(pipe_,&client)||client!=cfg_.pid||!processMatches(parent_,cfg_.creation)){DisconnectNamedPipe(pipe_);continue;}
 std::string line;bool readOk=false;while(line.size()<1024&&alive()){char buf[256];DWORD n=0;if(!transfer(false,buf,DWORD(sizeof buf),n)||!n)break;line.append(buf,buf+n);if(line.size()>1024)break;auto end=line.find('\n');if(end!=std::string::npos){if(end+1!=line.size())break;line.resize(end);readOk=true;break;}}
 if(!readOk){DisconnectNamedPipe(pipe_);announce();break;}lastRequest_=GetTickCount64();auto request=std::make_shared<Request>();request->command=parse(line);if(!request->done){DisconnectNamedPipe(pipe_);continue;}{std::lock_guard<std::mutex>l(mutex_);request_=request;}
 if(!PostMessageW(window_,CommandMessage,0,0)){request->cancelled=true;DisconnectNamedPipe(pipe_);announce();break;}HANDLE waits[]={stop_,parent_,request->done};DWORD w=WaitForMultipleObjects(3,waits,FALSE,1500);if(w!=WAIT_OBJECT_0+2){request->cancelled=true;announce();DisconnectNamedPipe(pipe_);break;}
 std::string out=request->response+'\n';DWORD sent=0;if(!transfer(true,out.data(),DWORD(out.size()),sent)||sent!=out.size()){DisconnectNamedPipe(pipe_);announce();break;}else{
 // Client closes after ReadLine. Wait boundedly instead of blocking FlushFileBuffers.
 ULONGLONG until=GetTickCount64()+250;while(alive()&&GetTickCount64()<until){DWORD available=0;if(!PeekNamedPipe(pipe_,nullptr,0,nullptr,&available,nullptr))break;WaitForSingleObject(stop_,2);}}
 DisconnectNamedPipe(pipe_);if(request->command.kind==Command::Quit){announce();break;}
 }}
 public:
 Server(Config cfg,HWND h):cfg_(std::move(cfg)),window_(h){try{
 parent_=OpenProcess(SYNCHRONIZE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,cfg_.pid);if(!processMatches(parent_,cfg_.creation))throw std::runtime_error("Host identity mismatch");
 const wchar_t*work=_wgetenv(L"SMX_HOST_WORK988"),*model=_wgetenv(L"SMX_MODEL_DIR");if(!work||!model||_wcsicmp(fullPath(work).c_str(),fullPath(model).c_str()))throw std::runtime_error("Private runtime-work required");auto dir=fullPath(work);if(!SetCurrentDirectoryW(dir.c_str()))throw std::runtime_error("Runtime directory unavailable");lock_=CreateFileW((dir+L"\\engine988.lock").c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(lock_==INVALID_HANDLE_VALUE)throw std::runtime_error("Runtime directory already owned");
 HANDLE token=nullptr;if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))throw std::runtime_error("Process token unavailable");DWORD bytes=0;GetTokenInformation(token,TokenUser,nullptr,0,&bytes);std::vector<unsigned char>data(bytes);if(!bytes||!GetTokenInformation(token,TokenUser,data.data(),bytes,&bytes)){CloseHandle(token);throw std::runtime_error("Current user unavailable");}CloseHandle(token);LPWSTR sid=nullptr;if(!ConvertSidToStringSidW(((TOKEN_USER*)data.data())->User.Sid,&sid))throw std::runtime_error("SID conversion failed");std::wstring sddl=L"D:P(A;;GA;;;"+std::wstring(sid)+L")";LocalFree(sid);PSECURITY_DESCRIPTOR descriptor=nullptr;if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl.c_str(),SDDL_REVISION_1,&descriptor,nullptr))throw std::runtime_error("Pipe DACL failed");SECURITY_ATTRIBUTES attributes{sizeof(attributes),descriptor,FALSE};
 finished_=CreateEventW(nullptr,TRUE,FALSE,nullptr);stop_=CreateEventW(nullptr,TRUE,FALSE,nullptr);std::wstring path=L"\\\\.\\pipe\\"+cfg_.name;pipe_=CreateNamedPipeW(path.c_str(),PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED|FILE_FLAG_FIRST_PIPE_INSTANCE,PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT|PIPE_REJECT_REMOTE_CLIENTS,1,4096,4096,1500,&attributes);LocalFree(descriptor);if(!stop_||!finished_||pipe_==INVALID_HANDLE_VALUE)throw std::runtime_error("Private pipe create failed");thread_=std::thread([this]{loop();});
 }catch(...){cleanup();throw;}}
 std::shared_ptr<Request> take(){std::lock_guard<std::mutex>l(mutex_);auto r=std::move(request_);request_.reset();return r;}
 void stop(){if(stop_)SetEvent(stop_);if(pipe_!=INVALID_HANDLE_VALUE)CancelIoEx(pipe_,nullptr);if(thread_.joinable())thread_.join();}
 void cleanup(){stop();if(finished_)SetEvent(finished_);if(watchdog_.joinable())watchdog_.join();if(finished_){CloseHandle(finished_);finished_=nullptr;}if(pipe_!=INVALID_HANDLE_VALUE){CloseHandle(pipe_);pipe_=INVALID_HANDLE_VALUE;}if(stop_){CloseHandle(stop_);stop_=nullptr;}if(parent_){CloseHandle(parent_);parent_=nullptr;}if(lock_!=INVALID_HANDLE_VALUE){CloseHandle(lock_);lock_=INVALID_HANDLE_VALUE;}}
 ~Server(){cleanup();}
};
inline bool selfTest(){uint64_t n=0;return decimal("18446744073709551615",n)&&n==UINT64_MAX&&!decimal("18446744073709551616",n)&&!decimal("-1",n)&&parse("PING").kind==Command::Ping&&parse("START 123 7 456").kind==Command::Start&&parse("START 0 7 456").kind==Command::Invalid&&parse("HUD 0").kind==Command::Hud&&!parse("HUD 0").enabled&&parse("HUD 2").kind==Command::Invalid&&parse("STOP x").kind==Command::Invalid&&parse("START 1 4294967296 2").kind==Command::Invalid&&quote("a\n\"\\")=="\"a\\u000a\\\"\\\\\"";}
}
