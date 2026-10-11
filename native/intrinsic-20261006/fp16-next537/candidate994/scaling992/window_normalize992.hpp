#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <array>
#include <stdexcept>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <exception>
#include "window_normalize795_vs.h"
#include "window_normalize795_ps.h"
class WindowNormalize992 {
 template<class T>using Ptr=Microsoft::WRL::ComPtr<T>;
 Ptr<ID3D11Device>d;Ptr<ID3D11DeviceContext>c;Ptr<ID3D11VertexShader>vs;Ptr<ID3D11PixelShader>ps;Ptr<ID3D11SamplerState>sampler;Ptr<ID3D11Buffer>constants;Ptr<ID3D11RasterizerState>raster;
 struct Read{Ptr<ID3D11Texture2D>t;Ptr<ID3D11ShaderResourceView>v;};std::array<Read,16>reads;unsigned nextRead=0;
 struct Write{Ptr<ID3D11Texture2D>t;Ptr<ID3D11RenderTargetView>v;};std::array<Write,16>writes;unsigned nextWrite=0;
 Ptr<ID3D11Texture2D>copy;Ptr<ID3D11ShaderResourceView>copyView;unsigned copyW=0,copyH=0;uint64_t directFrames992=0,fallbackFrames992=0,scaledFrames992=0; ID3D11Texture2D* preparedSource=nullptr,*preparedDest=nullptr;ID3D11ShaderResourceView*preparedInput=nullptr;ID3D11RenderTargetView*preparedTarget=nullptr;D3D11_BOX preparedCrop{};D3D11_VIEWPORT preparedViewport{};float preparedConstants[8]{};bool preparedCopy=false,preparedFallback=false,preparedValid=false;
 static void check(HRESULT h){if(FAILED(h)){char msg[100];sprintf_s(msg,"Window normalization D3D11 failure HRESULT=0x%08lx",static_cast<unsigned long>(h));throw std::runtime_error(msg);}}
 void initialize(){if(vs)return;check(d->CreateVertexShader(windowNormalizeVS795,sizeof(windowNormalizeVS795),nullptr,&vs));check(d->CreatePixelShader(windowNormalizePS795,sizeof(windowNormalizePS795),nullptr,&ps));D3D11_SAMPLER_DESC s{};s.Filter=D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;s.AddressU=s.AddressV=s.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;s.MaxLOD=0;check(d->CreateSamplerState(&s,&sampler));D3D11_BUFFER_DESC b{};b.ByteWidth=32;b.Usage=D3D11_USAGE_DEFAULT;b.BindFlags=D3D11_BIND_CONSTANT_BUFFER;check(d->CreateBuffer(&b,nullptr,&constants));D3D11_RASTERIZER_DESC r{};r.FillMode=D3D11_FILL_SOLID;r.CullMode=D3D11_CULL_NONE;r.DepthClipEnable=TRUE;check(d->CreateRasterizerState(&r,&raster));}
public:
 WindowNormalize992(ID3D11Device*device,ID3D11DeviceContext*context):d(device),c(context){initialize();}
 uint64_t directFrames()const{return directFrames992;}uint64_t fallbackFrames()const{return fallbackFrames992;}uint64_t scaledFrames()const{return scaledFrames992;}uint64_t fallbackBytes()const{return uint64_t(copyW)*copyH*4;}
 void resetSourceViews(){for(auto&r:reads){r.v.Reset();r.t.Reset();}copyView.Reset();copy.Reset();copyW=copyH=0;nextRead=0;}
 // Precompiled DXBC only: no D3DCompile. Caller holds NativeD3DAccess/keyed mutex.
 // Keep the capture owner until the existing Signal+Flush copy-ready fence completes.
 // Immediate-context draw uses current capture context; caller serializes callbacks.
 // Caches are bounded and destroyed only after capture GPU drain; no mid-session reset.
 void prepareClient(ID3D11Texture2D*source,unsigned ox,unsigned oy,unsigned w,unsigned h,ID3D11Texture2D*dest){
  preparedValid=false;preparedFallback=false;preparedSource=source;preparedDest=dest;
  D3D11_TEXTURE2D_DESC s{},out{};source->GetDesc(&s);dest->GetDesc(&out);
  if(!w||!h||uint64_t(ox)+w>s.Width||uint64_t(oy)+h>s.Height||s.SampleDesc.Count!=1||out.SampleDesc.Count!=1||s.Format!=DXGI_FORMAT_B8G8R8A8_UNORM||out.Format!=s.Format||s.Width>16384||s.Height>16384||!out.Width||!out.Height||out.Width>16384||out.Height>16384||s.ArraySize!=1||out.ArraySize!=1)throw std::runtime_error("Window client crop exceeds valid capture extent");
  D3D11_BOX box{ox,oy,0,ox+w,oy+h,1};preparedCrop=box;
  preparedCopy=w==out.Width&&h==out.Height;if(preparedCopy){preparedValid=true;return;}
  if(!(out.BindFlags&D3D11_BIND_RENDER_TARGET))throw std::runtime_error("Normalized output requires render-target capability");
  initialize();ID3D11ShaderResourceView*input=nullptr;unsigned tw=s.Width,th=s.Height;
  if((s.BindFlags&D3D11_BIND_SHADER_RESOURCE)&&s.ArraySize==1&&s.MipLevels==1){
   for(auto&r:reads)if(r.t.Get()==source){input=r.v.Get();break;}

   if(!input){auto&r=reads[nextRead++%reads.size()];r.v.Reset();r.t=source;check(d->CreateShaderResourceView(source,nullptr,&r.v));input=r.v.Get();}
  }else{
   preparedFallback=true;
   if(!copy||copyW!=w||copyH!=h){copyView.Reset();copy.Reset();D3D11_TEXTURE2D_DESC t{};t.Width=w;t.Height=h;t.MipLevels=t.ArraySize=t.SampleDesc.Count=1;t.Format=s.Format;t.Usage=D3D11_USAGE_DEFAULT;t.BindFlags=D3D11_BIND_SHADER_RESOURCE;check(d->CreateTexture2D(&t,nullptr,&copy));check(d->CreateShaderResourceView(copy.Get(),nullptr,&copyView));copyW=w;copyH=h;}
   input=copyView.Get();ox=oy=0;tw=w;th=h;
  }
  ID3D11RenderTargetView*target=nullptr;for(auto&r:writes)if(r.t.Get()==dest){target=r.v.Get();break;}
  if(!target){auto&r=writes[nextWrite++%writes.size()];r.v.Reset();r.t=dest;check(d->CreateRenderTargetView(dest,nullptr,&r.v));target=r.v.Get();}

  float cb[8]={float(ox)/tw,float(oy)/th,float(w)/tw,float(h)/th,(ox+.5f)/tw,(oy+.5f)/th,(ox+w-.5f)/tw,(oy+h-.5f)/th};
  std::copy(cb,cb+8,preparedConstants);preparedViewport={0,0,float(out.Width),float(out.Height),0,1};preparedInput=input;preparedTarget=target;preparedValid=true;
 }
 // No allocation, validation, or exceptions after ownership begins. prepareClient is
 // called with the same live source/destination before access948 / BeginCopy989.
 bool copyPrepared() noexcept {
  if(!preparedValid)std::terminate();preparedValid=false;
  if(preparedCopy){c->CopySubresourceRegion(preparedDest,0,0,0,0,preparedSource,0,&preparedCrop);return false;}
  if(preparedFallback){c->CopySubresourceRegion(copy.Get(),0,0,0,0,preparedSource,0,&preparedCrop);++fallbackFrames992;}else ++directFrames992;
  c->UpdateSubresource(constants.Get(),0,nullptr,preparedConstants,0,0);
  ID3D11Buffer*buf=constants.Get();ID3D11SamplerState*ss=sampler.Get();auto input=preparedInput;auto target=preparedTarget;
  c->IASetInputLayout(nullptr);c->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);c->VSSetShader(vs.Get(),nullptr,0);c->GSSetShader(nullptr,nullptr,0);c->PSSetShader(ps.Get(),nullptr,0);c->PSSetConstantBuffers(0,1,&buf);c->PSSetSamplers(0,1,&ss);c->PSSetShaderResources(0,1,&input);c->RSSetState(raster.Get());c->RSSetViewports(1,&preparedViewport);c->OMSetDepthStencilState(nullptr,0);c->OMSetBlendState(nullptr,nullptr,0xffffffff);c->OMSetRenderTargets(1,&target,nullptr);c->Draw(3,0);
  input=nullptr;buf=nullptr;ss=nullptr;c->PSSetShaderResources(0,1,&input);c->OMSetRenderTargets(0,nullptr,nullptr);c->VSSetShader(nullptr,nullptr,0);c->PSSetShader(nullptr,nullptr,0);c->PSSetConstantBuffers(0,1,&buf);c->PSSetSamplers(0,1,&ss);c->RSSetState(nullptr);
  ++scaledFrames992;return true;
 }
 bool copyClient(ID3D11Texture2D*source,unsigned ox,unsigned oy,unsigned w,unsigned h,ID3D11Texture2D*dest){prepareClient(source,ox,oy,w,h,dest);return copyPrepared();}
};
