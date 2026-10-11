#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include "../build/capture_gpu_shader.h"
class CaptureGpu {
 using MicrosoftComPtr=Microsoft::WRL::ComPtr<ID3D11Texture2D>;
 Microsoft::WRL::ComPtr<ID3D11Device> device;
 Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 Microsoft::WRL::ComPtr<ID3D11ComputeShader> shader;
 Microsoft::WRL::ComPtr<ID3D11Buffer> constants;
 MicrosoftComPtr copy;
 Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
 bool bound=false;uint32_t width=0,height=0,lastSizes[4]={};unsigned nextView=0;
 struct CachedView{MicrosoftComPtr source;Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;};CachedView sourceViews[16];
 static void check(HRESULT h){if(FAILED(h))throw std::runtime_error("GPU capture conversion resource failure");}
public:
 CaptureGpu(ID3D11Device*d,ID3D11DeviceContext*c):device(d),context(c){
  check(device->CreateComputeShader(captureGpuShader,sizeof(captureGpuShader),nullptr,&shader));
  D3D11_BUFFER_DESC b{};b.ByteWidth=16;b.Usage=D3D11_USAGE_DEFAULT;b.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
  check(device->CreateBuffer(&b,nullptr,&constants));
 }
 void convert(ID3D11Texture2D*source,uint32_t cw,uint32_t ch,ID3D11UnorderedAccessView*output,uint32_t ow,uint32_t oh){
  D3D11_TEXTURE2D_DESC s{};source->GetDesc(&s);
  if(!cw||!ch||!ow||!oh||cw>s.Width||ch>s.Height||s.SampleDesc.Count!=1||s.Format!=DXGI_FORMAT_B8G8R8A8_UNORM)throw std::runtime_error("Unsupported capture surface");
  if(cw>8192||ch>8192||ow>8192||oh>8192)throw std::runtime_error("Capture extent exceeds supported bound");
  ID3D11ShaderResourceView*input=nullptr;
  if((s.BindFlags&D3D11_BIND_SHADER_RESOURCE)&&s.MipLevels==1&&s.ArraySize==1){
   for(auto&v:sourceViews)if(v.source.Get()==source)input=v.view.Get();
   if(!input){auto&v=sourceViews[nextView++%16];v.view.Reset();v.source=source;check(device->CreateShaderResourceView(source,nullptr,&v.view));input=v.view.Get();}
  }else{
  if(width!=cw||height!=ch){
   view.Reset();copy.Reset();width=0;height=0;
   D3D11_TEXTURE2D_DESC t{};t.Width=cw;t.Height=ch;t.MipLevels=1;t.ArraySize=1;t.Format=s.Format;t.SampleDesc.Count=1;t.Usage=D3D11_USAGE_DEFAULT;t.BindFlags=D3D11_BIND_SHADER_RESOURCE;
   check(device->CreateTexture2D(&t,nullptr,&copy));check(device->CreateShaderResourceView(copy.Get(),nullptr,&view));width=cw;height=ch;
  }
  D3D11_BOX box{0,0,0,cw,ch,1};context->CopySubresourceRegion(copy.Get(),0,0,0,0,source,0,&box);
  input=view.Get();
  }
  uint32_t sizes[4]={cw,ch,ow,oh};if(memcmp(sizes,lastSizes,sizeof(sizes))){context->UpdateSubresource(constants.Get(),0,nullptr,sizes,0,0);memcpy(lastSizes,sizes,sizeof(sizes));}
  ID3D11Buffer*cb=constants.Get();
  context->CSSetShader(shader.Get(),nullptr,0);context->CSSetConstantBuffers(0,1,&cb);context->CSSetShaderResources(0,1,&input);context->CSSetUnorderedAccessViews(0,1,&output,nullptr);
  context->Dispatch((ow+15)/16,(oh+15)/16,1);
  input=nullptr;output=nullptr;cb=nullptr;context->CSSetShaderResources(0,1,&input);context->CSSetUnorderedAccessViews(0,1,&output,nullptr);
 }
};
