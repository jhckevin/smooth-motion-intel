using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using SmoothMotionDesktop;
class EnginePreset988Tests {
 static int n;static void Check(bool x,string name){n++;if(!x)throw new Exception(name);}
 static void Main(){
  string w=Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"UserData","runtime-work"),c=Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"UserData","pipeline-cache"),s=Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"..","candidate969","shaders");
  for(int bits=0;bits<4;bits++){
   var p=new Profile{CaptureCursor=(bits&1)!=0,OpaqueContent=(bits&2)!=0};
   var e=EnginePreset988.Create(w,c,s,w,p);
   Check(e["SMX_HOST_WORK988"]==Path.GetFullPath(w)&&e["SMX_MODEL_DIR"]==e["SMX_BENCH_DIR"]&&e["SMX_MODEL_DIR"]==e["SMX_HOST_WORK988"],"work equality");
   Check(e.ContainsKey("SMX_CAPTURE_CURSOR")==p.CaptureCursor,"cursor absence false");
   Check(e.ContainsKey("SMX_OPAQUE_CONTENT988")==p.OpaqueContent,"alpha toggle");
   Check(e["SMX_MANIFEST"]=="graph922.bin"&&e["SMX_WARP_FLAVOR"]=="native-flow"&&e["SMX_NATIVE_SHADERS969"]==Path.GetFullPath(s),"frozen graph");
   Check(e["SMX_PRESENT_DEPTH"]=="3"&&e["SMX_PRESENT_SYNC"]=="1"&&e["SMX_PRESENT_BACKEND"]=="dxgi"&&e["SMX_WINDOW_MODE976"]=="layered","composition");
   Check(e["SMX_DISPLAY_PREPARE_MS988"]=="32"&&e["SMX_RECOVERY_OUTPUT_HZ988"]=="60"&&e["SMX_CLAIMED_ORIGINAL_RECOVERY988"]=="1"&&e["SMX_SUSTAINED_RECOVERY988"]=="1","recovery");
   Check(e["SMX_TIMELINE_DELAY_MS"]=="80"&&e["SMX_UI_SECONDS964"]=="0","profile timing");
   Check(!e.Keys.Any(k=>k.EndsWith("987")||k.Contains("MARKER")||k.Contains("PROBE")||k.Contains("VISUAL_PROOF")||k.Contains("BENCH_SECONDS")||k.Contains("BENCH_KEEP")),"no legacy or diagnostics");
   Check(e["SMX_FUSE842"]=="1"&&e["SMX_READY_INPUT809"]=="0"&&e["SMX_PHASE_LOCK971"]=="1"&&!e.ContainsKey("SMX_QUEUE_FAMILY")&&!e.ContainsKey("SMX_QUEUE_INDEX")&&!e.ContainsKey("SMX_QUEUE_PRIORITY")&&e["SMX_DISPLAY_BOOST988"]=="1","measured config");
  }
  var custom=new Profile{TimelineMs=120,SessionSeconds=900};var dict=EnginePreset988.Create(w,c,s,w,custom);
  Check(dict["SMX_TIMELINE_DELAY_MS"]=="120"&&dict["SMX_UI_SECONDS964"]=="900","custom timing");
  bool bad=false;try{EnginePreset988.Create("relative",c,s,w,custom);}catch(ArgumentException){bad=true;}Check(bad,"no relative work");
  custom.SessionSeconds=0;custom.FrameMultiplier=8;custom.CaptureBackend="dxgi";custom.PresentSync=0;custom.PresentDepth=4;
  dict=EnginePreset988.Create(w,c,s,w,custom);Check(dict["SMX_UI_SECONDS964"]=="0"&&dict["SMX_FRAME_MULTIPLIER989"]=="8"&&dict["SMX_CAPTURE_BACKEND989"]=="dxgi"&&dict["SMX_PRESENT_SYNC"]=="0"&&dict["SMX_PRESENT_DEPTH"]=="4","real control wiring and continuous mode");
  custom.SessionSeconds=14401;bad=false;try{EnginePreset988.Create(w,c,s,w,custom);}catch(ArgumentException){bad=true;}Check(bad,"invalid duration");
  custom.SessionSeconds=0;custom.ProcessingSize="fit1080";dict=EnginePreset988.Create(w,c,s,w,custom);Check(dict["SMX_PROCESSING_SIZE992"]=="fit1080","1080 cap passed to real child environment");
  custom.ProcessingSize="fit1440";Check(EnginePreset988.Create(w,c,s,w,custom)["SMX_PROCESSING_SIZE992"]=="fit1440","1440 cap passed");
  Check(EnginePreset988.Create(w,c,s,w,new Profile())["SMX_PROCESSING_SIZE992"]=="native","native remains default");
  Check(!dict.ContainsKey("SMX_PREPARE_SECONDS862"),"consumer has no fatal foreground wait deadline");
  custom.ProcessingSize="nonsense";bad=false;try{EnginePreset988.Create(w,c,s,w,custom);}catch(ArgumentException){bad=true;}Check(bad,"invalid processing mode rejected");
  custom=new Profile();Check(custom.OutputMode=="follow","legacy default follows game");
  foreach(string mode in new[]{"follow","fullscreen","window1080","processing","small"}){custom.OutputMode=mode;Check(EnginePreset988.Create(w,c,s,w,custom)["SMX_OUTPUT_MODE993"]==mode,"output mode passed: "+mode);}
  custom.CaptureBackend="dxgi";bad=false;try{EnginePreset988.Create(w,c,s,w,custom);}catch(ArgumentException){bad=true;}Check(bad,"preview rejects desktop feedback capture");
  custom.OutputMode="follow";Check(EnginePreset988.Create(w,c,s,w,custom)["SMX_CAPTURE_BACKEND989"]=="dxgi","aligned desktop capture still available");
  custom.OutputMode="invalid";bad=false;try{EnginePreset988.Create(w,c,s,w,custom);}catch(ArgumentException){bad=true;}Check(bad,"invalid output rejected");
  Console.WriteLine("{\"pass\":true,\"checks\":"+n+",\"gpu_used\":false,\"process_started\":false,\"settings_changed\":false}");
 }
}