#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11_4.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <atomic>
#include "window_normalize992.hpp"
using Microsoft::WRL::ComPtr;
static std::atomic<bool> unsafeGpu{false};
static std::string reportPath992;
[[noreturn]] static void gpuFailStop992(const std::string&message){std::string clean=message;for(auto&v:clean)if(v==char(34)||v==char(92)||v==char(10)||v==char(13))v=char(32);std::string json=std::string("{\"pass\":false,\"gpu_fail_stop\":true,\"error\":\"")+clean+"\"}\n";if(!reportPath992.empty()){std::ofstream f(reportPath992,std::ios::binary);if(f)f<<json;}std::cerr<<json<<std::flush;TerminateProcess(GetCurrentProcess(),125);std::abort();}

static void checked(HRESULT h,const char*what){if(FAILED(h)){std::ostringstream s;s<<what<<" HRESULT=0x"<<std::hex<<unsigned(h);if(unsafeGpu)gpuFailStop992(s.str());throw std::runtime_error(s.str());}}
static uint64_t hashBytes(const std::vector<unsigned char>&v){uint64_t h=1469598103934665603ULL;for(auto x:v){h^=x;h*=1099511628211ULL;}return h;}
static std::string runCase(ID3D11Device*d,ID3D11DeviceContext*c,unsigned sw,unsigned sh,unsigned ox,unsigned oy,unsigned cw,unsigned ch,unsigned dw,unsigned dh,bool fallback){
 std::vector<unsigned char>data(size_t(sw)*sh*4),output(size_t(dw)*dh*4);
 for(unsigned y=0;y<sh;y++)for(unsigned x=0;x<sw;x++){auto*p=&data[(size_t(y)*sw+x)*4];p[0]=unsigned char(x%256);p[1]=unsigned char(y%256);p[2]=unsigned char((x/8+y/8)%256);p[3]=255;}
 D3D11_TEXTURE2D_DESC desc{};desc.Width=sw;desc.Height=sh;desc.MipLevels=desc.ArraySize=desc.SampleDesc.Count=1;desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=fallback?0:D3D11_BIND_SHADER_RESOURCE;
 D3D11_SUBRESOURCE_DATA init{data.data(),sw*4,0};ComPtr<ID3D11Texture2D>src,dst,read;checked(d->CreateTexture2D(&desc,&init,&src),"Create source");
 desc.Width=dw;desc.Height=dh;desc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;checked(d->CreateTexture2D(&desc,nullptr,&dst),"Create dest");desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;checked(d->CreateTexture2D(&desc,nullptr,&read),"Create readback");
 ComPtr<ID3D11Device5>d5;ComPtr<ID3D11DeviceContext4>c4;checked(d->QueryInterface(IID_PPV_ARGS(&d5)),"Device5");checked(c->QueryInterface(IID_PPV_ARGS(&c4)),"Context4");
 ComPtr<ID3D11Fence>f;checked(d5->CreateFence(0,D3D11_FENCE_FLAG_NONE,IID_PPV_ARGS(&f)),"CreateFence");
 HANDLE event=CreateEventW(nullptr,FALSE,FALSE,nullptr);if(!event)throw std::runtime_error("CreateEvent");
 WindowNormalize992 normalize(d,c);normalize.prepareClient(src.Get(),ox,oy,cw,ch,dst.Get());
 LARGE_INTEGER frequency{},begin{},end{};QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&begin);
 unsafeGpu=true;bool draw=normalize.copyPrepared();c->CopyResource(read.Get(),dst.Get());checked(c4->Signal(f.Get(),1),"Signal");checked(f->SetEventOnCompletion(1,event),"SetEventOnCompletion");c->Flush();
 DWORD waited=WaitForSingleObject(event,5000);CloseHandle(event);if(waited!=WAIT_OBJECT_0)gpuFailStop992("GPU completion timeout/wait failure");if(f->GetCompletedValue()==UINT64_MAX)gpuFailStop992("Device removed");
 unsafeGpu=false;QueryPerformanceCounter(&end);
 D3D11_MAPPED_SUBRESOURCE m{};checked(c->Map(read.Get(),0,D3D11_MAP_READ,D3D11_MAP_FLAG_DO_NOT_WAIT,&m),"Nonblocking readback");
 for(unsigned y=0;y<dh;y++)memcpy(output.data()+size_t(y)*dw*4,(unsigned char*)m.pData+size_t(y)*m.RowPitch,dw*4);c->Unmap(read.Get(),0);
 int maxError=0;uint64_t sumError=0,over2=0,alphaBad=0;
 for(unsigned y=0;y<dh;y++)for(unsigned x=0;x<dw;x++){
 double fx=std::clamp(ox+(x+.5)*cw/dw-.5,double(ox),double(ox+cw-1));double fy=std::clamp(oy+(y+.5)*ch/dh-.5,double(oy),double(oy+ch-1));
 unsigned x0=unsigned(std::floor(fx)),y0=unsigned(std::floor(fy)),x1=std::min(x0+1,ox+cw-1),y1=std::min(y0+1,oy+ch-1);double tx=fx-x0,ty=fy-y0;
 for(unsigned k=0;k<3;k++){auto get=[&](unsigned xx,unsigned yy){return double(data[(size_t(yy)*sw+xx)*4+k]);};double expected=(get(x0,y0)*(1-tx)+get(x1,y0)*tx)*(1-ty)+(get(x0,y1)*(1-tx)+get(x1,y1)*tx)*ty;int delta=abs(int(output[(size_t(y)*dw+x)*4+k])-int(std::floor(expected+.5)));maxError=std::max(maxError,delta);sumError+=delta;over2+=delta>2;}
 alphaBad+=output[(size_t(y)*dw+x)*4+3]!=255;
 }
 bool pass=!alphaBad&&!over2;std::ostringstream s;s<<std::fixed<<std::setprecision(6)<<"{\"pass\":"<<(pass?"true":"false")<<",\"source\":["<<sw<<","<<sh<<"],\"crop\":["<<ox<<","<<oy<<","<<cw<<","<<ch<<"],\"processing\":["<<dw<<","<<dh<<"],\"requested_fallback\":"<<(fallback?"true":"false")<<",\"gpu_draws\":"<<normalize.scaledFrames()<<",\"native_copy\":"<<(!draw?"true":"false")<<",\"direct_draws\":"<<normalize.directFrames()<<",\"fallback_draws\":"<<normalize.fallbackFrames()<<",\"max_rgb_error_255\":"<<maxError<<",\"mae_rgb_255\":"<<double(sumError)/(double(dw)*dh*3)<<",\"error_over2\":"<<over2<<",\"alpha_non255\":"<<alphaBad<<",\"bgra_fnv64\":\""<<std::hex<<hashBytes(output)<<std::dec<<"\",\"submit_to_fence_wall_ms\":"<<double(end.QuadPart-begin.QuadPart)/frequency.QuadPart*1000<<"}";
 if(!pass)throw std::runtime_error(std::string("Pixel reference validation failed: ")+s.str());return s.str();
}
int main(int argc,char**argv){
 std::string mode="--small",json;for(int i=1;i<argc;i++){std::string a=argv[i];if(a=="--small"||a=="--large"||a=="--large-fallback")mode=a;else if(a=="--json"&&i+1<argc)json=argv[++i];else{std::cerr<<"usage: normalization-probe992.exe [--small|--large|--large-fallback] [--json report-path]\n";return 2;}}
 reportPath992=json;
 HANDLE done=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!done)return 3;
 std::thread watchdog([&](){if(WaitForSingleObject(done,15000)!=WAIT_OBJECT_0){const char*m="{\"pass\":false,\"error\":\"15s process watchdog\"}\n";DWORD n;WriteFile(GetStdHandle(STD_ERROR_HANDLE),m,DWORD(strlen(m)),&n,nullptr);TerminateProcess(GetCurrentProcess(),124);}});
 int code=0;std::string result;
 try{
 ComPtr<ID3D11Device>d;ComPtr<ID3D11DeviceContext>c;D3D_FEATURE_LEVEL fl{};checked(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,nullptr,0,D3D11_SDK_VERSION,&d,&fl,&c),"D3D11CreateDevice");
 ComPtr<IDXGIDevice>dx;ComPtr<IDXGIAdapter>a;checked(d.As(&dx),"DXGI device");checked(dx->GetAdapter(&a),"Adapter");DXGI_ADAPTER_DESC ad{};checked(a->GetDesc(&ad),"GetDesc");
 if(ad.VendorId!=0x8086)throw std::runtime_error("Probe requires Intel hardware adapter; no GPU draw submitted");
 std::vector<std::string>cases;
 if(mode=="--small"){cases.push_back(runCase(d.Get(),c.Get(),80,54,7,9,64,36,32,18,false));cases.push_back(runCase(d.Get(),c.Get(),80,54,7,9,64,36,32,18,true));}
 else cases.push_back(runCase(d.Get(),c.Get(),2880,1620,0,0,2880,1620,1920,1080,mode=="--large-fallback"));
 result="{\"pass\":true,\"vendor_id\":"+std::to_string(ad.VendorId)+",\"device_id\":"+std::to_string(ad.DeviceId)+",\"cases\":[";for(size_t i=0;i<cases.size();i++){if(i)result+=",";result+=cases[i];}result+="],\"scope\":\"Standalone precompiled-DXBC bilinear correctness only; no capture, model, visible window, or performance acceptance\"}";
 }catch(const std::exception&e){std::string err=e.what();for(auto&x:err)if(x=='\"'||x=='\\'||x=='\n'||x=='\r')x=' ';result="{\"pass\":false,\"error\":\""+err+"\"}";code=5;}
 if(!json.empty()){std::ofstream f(json,std::ios::binary);if(f)f<<result<<"\n";else{result="{\"pass\":false,\"error\":\"Report file creation failed\"}";code=6;}}
 std::cout<<result<<std::endl;if(unsafeGpu){std::cout.flush();TerminateProcess(GetCurrentProcess(),code?code:125);}
 SetEvent(done);watchdog.join();CloseHandle(done);return code;
}
