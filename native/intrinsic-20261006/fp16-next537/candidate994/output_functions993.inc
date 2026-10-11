
static smx993::OutputMode outputMode993=smx993::OutputMode::Follow;
static smx993::OutputState outputState993;
static std::atomic<bool> outputGeometryFailed993{false};
static std::atomic<unsigned>outputProcessingWidth993{0},outputProcessingHeight993{0},previewClickStops993{0};
static bool outputPreview993(){return smx993::preview(outputMode993);}
static std::string outputFields993(){if(!output||!IsWindowVisible(output))outputState993.hide();return outputState993.fields();}
static DWORD outputStyle993(bool overlay){if(outputPreview993())return smx993::windowed(outputMode993)?(WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU):WS_POPUP;return overlay?WS_POPUP:WS_OVERLAPPEDWINDOW;}
static DWORD outputExStyle993(bool overlay,bool noActivate){if(outputPreview993())return WS_EX_LAYERED|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW;return outputExStyle976(overlay,noActivate);}
static smx993::OutputRect outputRectValue993(const RECT&r){return {r.left,r.top,r.right,r.bottom};}
static RECT outputWindowsRect993(const smx993::OutputRect&r){return {LONG(r.left),LONG(r.top),LONG(r.right),LONG(r.bottom)};}
static smx993::OutputPlan plannedOutput993(const RECT&source){
 MONITORINFO mi{sizeof(mi)};if(!GetMonitorInfoW(MonitorFromWindow(selected,MONITOR_DEFAULTTONEAREST),&mi))throw std::runtime_error("Cannot read output monitor");
 smx993::Insets frame{};
 if(smx993::windowed(outputMode993)){RECT r{0,0,100,100};if(!AdjustWindowRectExForDpi(&r,outputStyle993(true),FALSE,outputExStyle993(true,false),GetDpiForWindow(selected)))throw std::runtime_error("Cannot size preview frame");frame={unsigned(-r.left),unsigned(-r.top),unsigned(r.right-100),unsigned(r.bottom-100)};}
 const unsigned pw=outputProcessingWidth993.load(),ph=outputProcessingHeight993.load();
 return smx993::outputPlan(outputMode993,outputRectValue993(source),outputRectValue993(mi.rcMonitor),outputRectValue993(mi.rcWork),pw?pw:unsigned(source.right-source.left),ph?ph:unsigned(source.bottom-source.top),frame);
}
static void stopPreviewFromClick993(HWND h){
 if(!outputPreview993()||!running)return;
 if(!stopFlag.exchange(true)){++previewClickStops993;if(!overlayPaused.exchange(true))++overlayEpoch;}
 ShowWindow(h,SW_HIDE);overlayVisible=false;outputState993.hide();hideHud964();
 status(L"已停止输出预览；正在安全结束当前 GPU 工作。此点击没有转发为游戏菜单坐标。");
 if(control)PostMessageW(control,WM_APP+10,0,0);
}
