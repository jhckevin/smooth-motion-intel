static void updateSourceOverlay(){
 recordWindowState990();
 if(!sourceOverlay||!running||!output){outputState993.hide();hideHud964();return;}
 if(!targetIdentity990()){targetClosed990=true;stopFlag=true;if(!overlayPaused.exchange(true))++overlayEpoch;ShowWindow(output,SW_HIDE);overlayVisible=false;outputState993.hide();hideHud964();recordWindowState990();return;}
 HWND fg=GetForegroundWindow();
 bool active=overlayReady.load()&&!stopFlag&&!IsIconic(selected)&&IsWindowVisible(selected)&&
  ((benchmarkSource&&getenv("SMX_BENCH_KEEP_CAPTURE")&&strcmp(getenv("SMX_BENCH_KEEP_CAPTURE"),"1")==0)||(fg&&GetAncestor(fg,GA_ROOT)==selected));
 if(!active){hideHud964();outputState993.hide();lastHud964=0;hzCapture964=hzGen964=hzPresent964=0;
  if(!overlayPaused.exchange(true)){overlayEpoch++;overlayEvent("pause_foreground_or_visibility",overlayRect);}
  if(overlayVisible){ShowWindow(output,SW_HIDE);overlayVisible=false;overlayHides++;overlayEvent("hide",overlayRect);}return;
 }
 RECT source{};POINT origin{};
 if(!GetClientRect(selected,&source)||!ClientToScreen(selected,&origin)||!validExtent795(source.right,source.bottom)){
  if(!overlayPaused.exchange(true))overlayEpoch++;ShowWindow(output,SW_HIDE);overlayVisible=false;outputState993.hide();return;
 }
 if(captureBackendPaused989){hideHud964();outputState993.hide();if(overlayVisible){ShowWindow(output,SW_HIDE);overlayVisible=false;}return;}
 if(overlayBypass){hideHud964();outputState993.hide();overlayPaused=false;if(overlayVisible){ShowWindow(output,SW_HIDE);overlayVisible=false;overlayHides++;overlayEvent("passthrough",overlayRect);}return;}
 OffsetRect(&source,origin.x,origin.y);
 try{
 auto plan=plannedOutput993(source);RECT desiredWindow=outputWindowsRect993(plan.window),actualWindow{};
 bool show=presented.load()!=0;GetWindowRect(output,&actualWindow);bool moved=!EqualRect(&desiredWindow,&actualWindow);
 if(moved||(!overlayVisible&&show)){
  if(!SetWindowPos(output,HWND_TOPMOST,desiredWindow.left,desiredWindow.top,desiredWindow.right-desiredWindow.left,desiredWindow.bottom-desiredWindow.top,
    SWP_NOACTIVATE|SWP_FRAMECHANGED|(show?SWP_SHOWWINDOW:0)))throw std::runtime_error("Cannot position own output window");
 }
 // Read back the actual physical client rectangle, including DPI-aware nonclient frame.
 RECT actualClient{};POINT actualOrigin{};if(!GetClientRect(output,&actualClient)||!ClientToScreen(output,&actualOrigin)||actualClient.right<=0||actualClient.bottom<=0)throw std::runtime_error("Cannot read output client rectangle");
 OffsetRect(&actualClient,actualOrigin.x,actualOrigin.y);GetWindowRect(output,&actualWindow);
 plan.window=outputRectValue993(actualWindow);plan.client=outputRectValue993(actualClient);
 plan.content=outputPreview993()?smx993::fitContent(plan.client,unsigned(source.right-source.left),unsigned(source.bottom-source.top)):plan.client;
 outputState993.set(plan,show);overlayRect=actualClient;
 if(!show){overlayPaused=false;hideHud964();return;}
 bool entering=!overlayVisible;overlayVisible=true;if(entering){overlayEntries++;overlayEvent("show",actualClient);}else if(moved)overlayEvent("move",actualClient);
 if((entering||moved)&&!outputPreview993()){
  unsigned pass=0;for(int y=1;y<=3;y++)for(int x=1;x<=3;x++){POINT pt{actualClient.left+(actualClient.right-actualClient.left)*x/4,actualClient.top+(actualClient.bottom-actualClient.top)*y/4};if(GetAncestor(WindowFromPoint(pt),GA_ROOT)==selected)pass++;}
  overlayHitPasses+=pass;overlayHitFailures+=9-pass;overlayEvent(pass==9?"hit_test_pass":"hit_test_failure",actualClient);
 }
 overlayPaused=false;
 if(!hostConfig988.hosted&&!benchmarkSource&&!controlMinimized964){ShowWindow(control,SW_SHOWMINNOACTIVE);controlMinimized964=true;}
 updateHud964();
 }catch(const std::exception&e){lastCaptureError=L"Output geometry failed: "+std::wstring(winrt::to_hstring(e.what()).c_str());outputGeometryFailed993=true;stopFlag=true;ShowWindow(output,SW_HIDE);overlayVisible=false;outputState993.hide();hideHud964();}
}
