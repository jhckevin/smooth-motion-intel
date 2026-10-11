#pragma once
#include <dcomp.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <stdexcept>
#include <cstdio>
// A single opaque black pixel, composed underneath the image. No extra swapchain
// or per-frame clear/copy/Present. The surface may be atlas-backed: clear ONLY the
// updateOffset rectangle, never the whole returned render target.
class OutputBackground993 {
 template<class T>using Ptr=Microsoft::WRL::ComPtr<T>;
 Ptr<IDCompositionVisual>root,black;Ptr<IDCompositionSurface>surface;
 static void check(HRESULT h){if(FAILED(h)){char text[128];sprintf_s(text,"Output background DComp failure 0x%08lx",(unsigned long)h);throw std::runtime_error(text);}}
public:
 void initialize(IDCompositionDevice*composition,IDCompositionTarget*target,IDCompositionVisual*image,ID3D11Device*device,ID3D11DeviceContext*context){
  check(composition->CreateVisual(&root));check(composition->CreateVisual(&black));check(composition->CreateSurface(1,1,DXGI_FORMAT_B8G8R8A8_UNORM,DXGI_ALPHA_MODE_IGNORE,&surface));
  Ptr<ID3D11DeviceContext1>context1;check(context->QueryInterface(IID_PPV_ARGS(&context1)));
  Ptr<IDXGISurface>drawingSurface;POINT offset{};check(surface->BeginDraw(nullptr,IID_PPV_ARGS(&drawingSurface),&offset));
  bool drawing=true;try{
   Ptr<IDXGISurface2>drawingSurface2;check(drawingSurface.As(&drawingSurface2));Ptr<ID3D11Texture2D>texture;UINT subresource=0;check(drawingSurface2->GetResource(IID_PPV_ARGS(&texture),&subresource));
   D3D11_TEXTURE2D_DESC desc{};texture->GetDesc(&desc);if(!desc.MipLevels||!desc.ArraySize||desc.SampleDesc.Count!=1||uint64_t(subresource)>=uint64_t(desc.MipLevels)*desc.ArraySize)throw std::runtime_error("Invalid DComp surface resource");
   const UINT mip=subresource%desc.MipLevels,slice=subresource/desc.MipLevels;if(mip>=32||offset.x<0||offset.y<0||unsigned(offset.x)>=std::max(1u,desc.Width>>mip)||unsigned(offset.y)>=std::max(1u,desc.Height>>mip))throw std::runtime_error("DComp update offset exceeds selected subresource");
   D3D11_RENDER_TARGET_VIEW_DESC view{};view.Format=DXGI_FORMAT_B8G8R8A8_UNORM;if(desc.ArraySize>1){view.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2DARRAY;view.Texture2DArray.MipSlice=mip;view.Texture2DArray.FirstArraySlice=slice;view.Texture2DArray.ArraySize=1;}else{view.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;view.Texture2D.MipSlice=mip;}
   Ptr<ID3D11RenderTargetView>rtv;check(device->CreateRenderTargetView(texture.Get(),&view,&rtv));const float color[4]={0,0,0,1};const D3D11_RECT rect{offset.x,offset.y,offset.x+1,offset.y+1};context1->ClearView(rtv.Get(),color,&rect,1);context->Flush();HRESULT ended=surface->EndDraw();drawing=false;check(ended);}
  catch(...){if(drawing)surface->EndDraw();throw;}
  check(black->SetContent(surface.Get()));check(root->AddVisual(black.Get(),FALSE,nullptr));check(root->AddVisual(image,TRUE,black.Get()));check(target->SetRoot(root.Get()));
 }
 void resize(unsigned w,unsigned h){if(!black)return;D2D_MATRIX_3X2_F matrix{float(w),0,0,float(h),0,0};check(black->SetTransform(matrix));}
};
