#pragma once
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include "bounded_stdio989.h"
static bool smxPixelProofEligible989(uint32_t width,uint32_t height)noexcept{return width&&height&&uint64_t(width)*height<=1048576;}
static void smxPixelProofSkipped989(const char*prefix,UINT width,UINT height){char path[128];sprintf_s(path,"%s-pixels.json",prefix);FILE*f=smx_log_fopen989(path,"wb");if(f){smx_log_fprintf989(f,"{\"width\":%u,\"height\":%u,\"skipped\":true,\"reason\":\"over_1mp_diagnostic_limit\",\"actual_texture_readback\":false,\"gpu_transaction_started_for_proof\":false}\n",width,height);smx_log_fclose989(f);}}
// Diagnostics only: caller holds every input/output lease and all GPU jobs have completed.
static void smxPixelProof989(ID3D11Device*device,ID3D11DeviceContext*context,const std::vector<ID3D11Texture2D*>& textures,const char*prefix="app-mfg989"){
 if(textures.empty()||textures.size()>9)throw std::runtime_error("Invalid pixel-proof count");
 D3D11_TEXTURE2D_DESC first{};textures.front()->GetDesc(&first);if(first.Width&&first.Height&&!smxPixelProofEligible989(first.Width,first.Height)){smxPixelProofSkipped989(prefix,first.Width,first.Height);return;}
 std::vector<std::vector<unsigned char>> images;std::vector<uint64_t> hashes;UINT width=0,height=0;
 for(size_t index=0;index<textures.size();++index){
  auto*texture=textures[index];D3D11_TEXTURE2D_DESC desc{};texture->GetDesc(&desc);
  if(uint64_t(desc.Width)*desc.Height>1048576||!desc.Width||!desc.Height)throw std::runtime_error("Pixel proof limited to 1 MP");
  if(index&&(desc.Width!=width||desc.Height!=height))throw std::runtime_error("Pixel proof dimensions mismatch");width=desc.Width;height=desc.Height;
  if(desc.Format!=DXGI_FORMAT_R8G8B8A8_UNORM&&desc.Format!=DXGI_FORMAT_B8G8R8A8_UNORM)throw std::runtime_error("Unsupported proof texture format");
  bool bgra=desc.Format==DXGI_FORMAT_B8G8R8A8_UNORM;desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.MiscFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;winrt::check_hresult(device->CreateTexture2D(&desc,nullptr,&staging));
  Microsoft::WRL::ComPtr<IDXGIKeyedMutex> mutex;std::unique_ptr<BridgeLock> guard;if(SUCCEEDED(texture->QueryInterface(IID_PPV_ARGS(&mutex))))guard=std::make_unique<BridgeLock>(mutex.Get());
  context->CopyResource(staging.Get(),texture);D3D11_MAPPED_SUBRESOURCE mapped{};winrt::check_hresult(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped));
  std::vector<unsigned char> bytes(size_t(width)*height*4);
  for(UINT y=0;y<height;++y){auto*dst=bytes.data()+size_t(y)*width*4;memcpy(dst,static_cast<unsigned char*>(mapped.pData)+size_t(y)*mapped.RowPitch,width*4);if(bgra)for(UINT x=0;x<width;++x)std::swap(dst[x*4],dst[x*4+2]);}
  context->Unmap(staging.Get(),0);guard.reset();uint64_t hash=1469598103934665603ull;for(auto value:bytes){hash^=value;hash*=1099511628211ull;}hashes.push_back(hash);
  char name[96];sprintf_s(name,"%s-%02u.bmp",prefix,unsigned(index));FILE*file=smx_log_fopen989(name,"wb");if(!file)throw std::runtime_error("Proof write failed");
  BITMAPFILEHEADER bh{};bh.bfType=0x4d42;bh.bfOffBits=sizeof(bh)+sizeof(BITMAPINFOHEADER);bh.bfSize=bh.bfOffBits+DWORD(bytes.size());BITMAPINFOHEADER info{};info.biSize=sizeof(info);info.biWidth=LONG(width);info.biHeight=-LONG(height);info.biPlanes=1;info.biBitCount=32;info.biCompression=BI_RGB;
  fwrite(&bh,1,sizeof bh,file);fwrite(&info,1,sizeof info,file);auto bmp=bytes;for(size_t i=0;i<bmp.size();i+=4)std::swap(bmp[i],bmp[i+2]);bool ok=fwrite(bmp.data(),1,bmp.size(),file)==bmp.size();smx_log_fclose989(file);if(!ok)throw std::runtime_error("Incomplete proof image");images.push_back(std::move(bytes));
 }
 char reportPath[128];sprintf_s(reportPath,"%s-pixels.json",prefix);FILE*report=smx_log_fopen989(reportPath,"wb");if(!report)throw std::runtime_error("Proof report failed");
 smx_log_fprintf989(report,"{\"width\":%u,\"height\":%u,\"actual_texture_readback\":true,\"ground_truth_quality_test\":false,\"frames\":[",width,height);
 for(size_t j=0;j<images.size();++j){double difference=0;unsigned badAlpha=0;for(size_t i=0;i<images[j].size();++i){if(i%4==3){badAlpha+=images[j][i]!=255;continue;}if(j)difference+=std::abs(int(images[j][i])-int(images[j-1][i]));}
  smx_log_fprintf989(report,"%s{\"phase\":%u,\"denominator\":%u,\"hash\":\"%016llx\",\"rgb_mae_previous\":%.9f,\"nonopaque_pixels\":%u}",j?",":"",unsigned(j),unsigned(images.size()-1),(unsigned long long)hashes[j],difference/(double(width)*height*3),badAlpha);}
 smx_log_fprintf989(report,"]}\n");smx_log_fclose989(report);
}

// One bounded source-client diagnostic before the production capture copy.
// For DDA this crops the acquired monitor; it never reads back the whole desktop.
static void smxCaptureRawProof989(ID3D11Device*device,ID3D11DeviceContext*context,ID3D11Texture2D*source,const D3D11_BOX&box){
 D3D11_TEXTURE2D_DESC input{};source->GetDesc(&input);const UINT width=box.right-box.left,height=box.bottom-box.top;
 if(!width||!height||uint64_t(width)*height>1048576||box.right>input.Width||box.bottom>input.Height)throw std::runtime_error("Raw capture proof limited to valid 1 MP client");
 D3D11_TEXTURE2D_DESC target=input;target.Width=width;target.Height=height;target.MipLevels=target.ArraySize=1;target.Usage=D3D11_USAGE_DEFAULT;target.BindFlags=target.MiscFlags=target.CPUAccessFlags=0;
 Microsoft::WRL::ComPtr<ID3D11Texture2D>copy;winrt::check_hresult(device->CreateTexture2D(&target,nullptr,&copy));context->CopySubresourceRegion(copy.Get(),0,0,0,0,source,0,&box);
 smxPixelProof989(device,context,{copy.Get()},"app-capture-raw989");
}
