using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Threading;
using SmoothMotionDesktop;
class DesktopLifecycle988Tests {
 static int checks;static readonly BindingFlags Flags=BindingFlags.Instance|BindingFlags.NonPublic;
 static object Get(object w,string n){return w.GetType().GetField(n,Flags).GetValue(w);}
 static void Set(object w,string n,object v){w.GetType().GetField(n,Flags).SetValue(w,v);}
 static object Call(object w,string n,params object[] v){return w.GetType().GetMethod(n,Flags).Invoke(w,v);}
 static void Check(bool b,string msg){checks++;if(!b)throw new Exception(msg);}
 [STAThread] static int Main(){
  RenderOptions.ProcessRenderMode=System.Windows.Interop.RenderMode.SoftwareOnly;
  var app=new Application();app.ShutdownMode=ShutdownMode.OnExplicitShutdown;
  using(var input=Assembly.GetExecutingAssembly().GetManifestResourceStream("Theme988.xaml"))app.Resources.MergedDictionaries.Add((ResourceDictionary)XamlReader.Load(input));
  DesktopWindow w=null;
  try{
   w=new DesktopWindow();((DispatcherTimer)Get(w,"timer")).Stop();
   var ready992=EngineReply.Parse("{\"ok\":true,\"state\":\"wait_foreground\",\"running\":true,\"safeIdle\":false,\"preparationStage992\":\"wait_foreground\",\"modelReady992\":true,\"foregroundWaitSeconds992\":45.5,\"sourceWidth992\":2880,\"sourceHeight992\":1620,\"processingWidth992\":1920,\"processingHeight992\":1080,\"processingResampled992\":true,\"targetClientWidth990\":2880,\"targetClientHeight990\":1620}");
   Check(ready992.ModelReady992&&ready992.ForegroundWaitSeconds992==45.5,"prepared model remains ready after thirty seconds");
   Check(((string)Call(w,"StateText",ready992)).Contains("请切回游戏"),"foreground wait is actionable status not initialization error");
   Check(!EngineReply.Parse("{\"ok\":true}").ModelReady992,"missing stage never invents ready");
   ((EngineHost)Get(w,"engine")).Last=ready992;Set(w,"running",true);Set(w,"lastEngineState","wait_foreground");Call(w,"UpdateTelemetry");
   Check(((System.Windows.Controls.TextBlock)Get(w,"splitText")).Text.Contains("请切回游戏"),"wait state removes stale FPS and gives instruction");
   Check(((System.Windows.Controls.TextBlock)Get(w,"geometryText992")).Text.Contains("处理 1920 × 1080"),"actual processing dimensions separated");
   var out993=EngineReply.Parse("{\"outputInfoValid993\":true,\"outputVisible993\":true,\"outputMode993\":\"fullscreen\",\"outputPreview993\":true,\"outputRect993\":[0,0,2880,1800],\"outputContentRect993\":[0,90,2880,1710],\"outputWindowRect993\":[0,0,2880,1800]}");
   Check(out993.OutputInfoValid993&&out993.OutputPreview993&&out993.OutputContentRect993[1]==90,"real output rectangles parsed");
   ((EngineHost)Get(w,"engine")).Last=out993;Call(w,"UpdateOutputInfo993");Check(((System.Windows.Controls.TextBlock)Get(w,"outputInfo993")).Text.Contains("输出 2880 × 1800")&&((System.Windows.Controls.TextBlock)Get(w,"outputInfo993")).Text.Contains("画面 2880 × 1620"),"actual output and content shown without conflating source");
   Check(!EngineReply.Parse("{\"outputInfoValid993\":true,\"outputRect993\":[0,0,0,0]}").OutputInfoValid993,"invalid geometry not shown as valid");
   ((EngineHost)Get(w,"engine")).Last=ready992;
   Call(w,"UpdateControls");Check(((System.Windows.Controls.Button)Get(w,"start")).IsEnabled,"wait can be stopped");
   var sig=typeof(EngineHost).GetMethod("Signature",Flags); // actual retained-engine key
   var current=(EngineHost)Get(w,"engine");
   Check(!Object.Equals(sig.Invoke(current,new object[]{new Profile()}),sig.Invoke(current,new object[]{new Profile{ProcessingSize="fit1080"}})),"processing mode switch invalidates retained child configuration");
   Check(!Object.Equals(sig.Invoke(current,new object[]{new Profile()}),sig.Invoke(current,new object[]{new Profile{OutputMode="fullscreen"}})),"output changes rebuild child config");
   Set(w,"running",false);Set(w,"lastEngineState","idle");
   var bounds=new DesktopWindow.MinMaxInfo988{MinTrackSize=new DesktopWindow.NativePoint988{X=1750,Y=1120},MaxTrackSize=new DesktopWindow.NativePoint988{X=3000,Y=2000}};
   var monitor=new DesktopWindow.NativeRect988{Left=0,Top=0,Right=2880,Bottom=1800};var work=new DesktopWindow.NativeRect988{Left=0,Top=0,Right=2880,Bottom=1720};
   Check(DesktopWindow.ConstrainMaximizedBounds988(ref bounds,monitor,work)&&bounds.MaxSize.X==2880&&bounds.MaxSize.Y==1720,"175 percent desktop work area remains physical, taskbar excluded");
   Check(bounds.MinTrackSize.X==1750&&bounds.MinTrackSize.Y==1120&&bounds.MaxTrackSize.X==3000,"normal minimum/tracking constraints preserved");
   monitor=new DesktopWindow.NativeRect988{Left=-1920,Top=-300,Right=0,Bottom=780};work=new DesktopWindow.NativeRect988{Left=-1880,Top=-276,Right=0,Bottom=780};
   Check(DesktopWindow.ConstrainMaximizedBounds988(ref bounds,monitor,work)&&bounds.MaxPosition.X==40&&bounds.MaxPosition.Y==24&&bounds.MaxSize.X==1880&&bounds.MaxSize.Y==1056,"secondary negative-origin monitor and left/top taskbar offsets");
   var before=bounds;work.Right=work.Left;Check(!DesktopWindow.ConstrainMaximizedBounds988(ref bounds,monitor,work)&&bounds.MaxSize.X==before.MaxSize.X,"invalid work area leaves native bounds unchanged");
   Check(w.MinWidth==920&&w.MinHeight==600,"R989 compact minimum remains usable");

   var status990=EngineReply.Parse("{\"ok\":true,\"state\":\"paused\",\"running\":true,\"safeIdle\":false,\"targetInfoValid990\":true,\"targetFullscreen990\":true,\"targetForeground990\":false,\"coverage990\":1,\"pauseReason990\":\"focus\",\"targetClientWidth990\":1920,\"targetClientHeight990\":1080,\"targetMonitorWidth990\":1920,\"targetMonitorHeight990\":1080}");
   Check(status990.TargetInfoValid990&&status990.TargetFullscreen990&&status990.Coverage990==1,"R990 reads explicit fullscreen geometry");
   Check(!EngineReply.Parse("{\"ok\":true}").TargetInfoValid990,"old protocol does not invent geometry");
   Check(!EngineReply.Parse("{\"targetInfoValid990\":true,\"targetFullscreen990\":true,\"coverage990\":2}").TargetFullscreen990,"invalid geometry never claims fullscreen");
   Check(((string)Call(w,"StateText",status990)).Contains("游戏失焦"),"focus pause reason visible");
   status990.PauseReason990="minimized";Check(((string)Call(w,"StateText",status990)).Contains("最小化"),"minimize pause reason visible");
   status990.PauseReason990="obscured";Check(((string)Call(w,"StateText",status990)).Contains("遮挡"),"obscured pause reason visible");
   status990.PauseReason990="closed";status990.SafeIdle=true;Check(((string)Call(w,"StateText",status990)).Contains("游戏已退出"),"closed is distinct safe stop");
   status990.PauseReason990="focus";status990.SafeIdle=false;
   ((EngineHost)Get(w,"engine")).Last=status990;Set(w,"running",true);Set(w,"lastEngineState","paused");
   ((System.Windows.Controls.TextBlock)Get(w,"fpsText")).Text="60.0 FPS";Call(w,"UpdateTelemetry");
   Check(((System.Windows.Controls.TextBlock)Get(w,"fpsText")).Text=="— FPS","paused clears stale FPS");
   Check(((System.Windows.Controls.TextBlock)Get(w,"splitText")).Text.Contains("失焦"),"paused reason replaces telemetry counts");
   Set(w,"sessionStartQpc",10.0);Call(w,"AcceptEngineState990",new EngineReply{State="running",SafeIdle=false},20.0);
   Check((double)Get(w,"sessionStartQpc")==20,"resume advances freshness cutoff");
   Call(w,"AcceptEngineState990",new EngineReply{State="running",SafeIdle=false},21.0);
   Check((double)Get(w,"sessionStartQpc")==20,"steady running does not starve fresh telemetry");
   Call(w,"AcceptEngineState990",status990,22.0);Call(w,"AcceptEngineState990",new EngineReply{State="running",SafeIdle=false},23.0);
   Check((double)Get(w,"sessionStartQpc")==23,"second resume requires its own new telemetry");
   Set(w,"running",false);Set(w,"lastEngineState","idle");
   var retained=new Profile();Check(retained.FrameMultiplier==2&&retained.CaptureBackend=="wgc"&&retained.PresentSync==1&&retained.PresentDepth==3&&retained.SessionSeconds==0,"R989 defaults retained");

   Call(w,"LatchFault","injected CPU test");
   Call(w,"ToggleStart",false);
   Check((bool)Get(w,"faulted")&&!(bool)Get(w,"counting"),"global toggle cannot clear fault or countdown");
   Check(!((EngineHost)Get(w,"engine")).HasProcess,"fault test never launched engine");
   Call(w,"ToggleStart",true);
   Check(!(bool)Get(w,"faulted")&&!(bool)Get(w,"counting")&&!(bool)Get(w,"running"),"explicit acknowledgement clears only fault");
   ((AppSettings)Get(w,"settings")).CountdownSeconds=5;
   Call(w,"ToggleStart",false);
   Check((bool)Get(w,"counting"),"new gesture starts countdown after acknowledgement");
   Call(w,"ToggleStart",false);
   Check(!(bool)Get(w,"counting")&&!(bool)Get(w,"running"),"second gesture cancels countdown");
   Set(w,"counting",true);Call(w,"LatchFault","second injected CPU test");
   Check(!(bool)Get(w,"counting"),"fault cancels pending countdown");
   Set(w,"quitting",true);Set(w,"faulted",false);
   Call(w,"ToggleStart",true);
   Check(!(bool)Get(w,"counting"),"quitting blocks start");
   Console.WriteLine("{\"pass\":true,\"checks\":"+checks+",\"window_shown\":false,\"engine_started\":false,\"render_mode\":\"SoftwareOnly\",\"ui_source\":\"actual Desktop988.cs\"}");
   return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}
  finally{if(w!=null){((DispatcherTimer)Get(w,"timer")).Stop();Set(w,"allowClose",true);w.Close();}app.Shutdown();}
 }
}
