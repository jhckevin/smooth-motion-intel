#ifndef NOMINMAX
#define NOMINMAX
#endif
#pragma once
#include "adapter_identity995.h"
#include "adapter_policy995.h"
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <stdexcept>
#include <string>
inline Microsoft::WRL::ComPtr<IDXGIAdapter> smxD3DAdapter995(){
 SmxAdapterIdentity995 id{};if(!smx_selected_adapter995(&id))throw std::runtime_error("Vulkan adapter identity not initialized");
 Microsoft::WRL::ComPtr<IDXGIFactory4> f;HRESULT hr=CreateDXGIFactory1(IID_PPV_ARGS(&f));if(FAILED(hr))throw std::runtime_error("DXGI factory cannot resolve selected Vulkan adapter; HRESULT="+std::to_string((uint32_t)hr));
 LUID luid{};memcpy(&luid,id.luid,sizeof(luid));Microsoft::WRL::ComPtr<IDXGIAdapter> a;hr=f->EnumAdapterByLuid(luid,IID_PPV_ARGS(&a));if(FAILED(hr))throw std::runtime_error("Selected Vulkan adapter LUID is unavailable in DXGI; HRESULT="+std::to_string((uint32_t)hr));DXGI_ADAPTER_DESC d{};hr=a->GetDesc(&d);if(FAILED(hr)||d.VendorId!=id.vendor||d.DeviceId!=id.device||!smx_luid_equal995((const unsigned char*)&d.AdapterLuid,id.luid))throw std::runtime_error("DXGI/Vulkan adapter identity mismatch; GetDesc HRESULT="+std::to_string((uint32_t)hr));return a;
}
