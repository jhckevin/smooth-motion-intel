using System;
using System.IO;
using System.Linq;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using SmoothMotionDesktop;

sealed class FakeChild988:IEngineProcess988 {
 public int Id{get;set;} public long CreationTimeUtcFileTime{get;set;}
 public bool Dead,KillThrows,WaitNever;public int Code,KillCalls,DisposeCalls;
 public bool HasExited{get{return Dead;}}public int ExitCode{get{if(!Dead)throw new InvalidOperationException();return Code;}}
 public bool WaitForExit(int ms){return !WaitNever&&Dead;}
 public void Kill(){KillCalls++;if(KillThrows)throw new IOException("injected Kill denied");if(!WaitNever){Dead=true;Code=99;}}
 public void Dispose(){DisposeCalls++;}
}
class EngineHost988Tests {
 static int checks,cases;static string root;static int serial;
 static void Check(bool ok,string label){checks++;if(!ok)throw new Exception(label);}
 static async Task Case(Func<Task> action){await action();cases++;}
 static string NewRoot(){string p=Path.Combine(root,"case-"+(++serial));Directory.CreateDirectory(p);return p;}
 static EngineReply Idle(){return new EngineReply{Ok=true,SafeIdle=true,State="idle"};}
 static EngineHost Host(string dir,FakeChild988 child,Action prepare,Func<string,Task<EngineReply>> commands,Action started){
  return new EngineHost(dir,psi=>{if(started!=null)started();return child;},()=>{Directory.CreateDirectory(Path.Combine(dir,"UserData","runtime-work"));if(prepare!=null)prepare();},commands??(cmd=>Task.FromResult(Idle())),1,1);
 }
 static async Task<bool> Failed(Func<Task> action){try{await action();return false;}catch{return true;}}
 static void Log(EngineHost host,string text){File.WriteAllText(Path.Combine(host.WorkDir,"app-engine.log"),text);}
 static string[] Contents(string dir){return Directory.GetFiles(dir,"app-engine.log",SearchOption.AllDirectories).Select(File.ReadAllText).ToArray();}
 static void Main(){root=Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"t988-"+Guid.NewGuid().ToString("N").Substring(0,6));Directory.CreateDirectory(root);
  try{Run().GetAwaiter().GetResult();Console.WriteLine("{\"pass\":true,\"cases\":"+cases+",\"checks\":"+checks+",\"gpu_used\":false,\"real_processes_started\":false,\"system_settings_changed\":false}");}
  finally{Directory.Delete(root,true);}
 }
 static async Task Run(){
  await Case(async delegate{
   var child=new FakeChild988{Id=995,CreationTimeUtcFileTime=995};long seq=1;var host=Host(NewRoot(),child,null,cmd=>{var r=Idle();r.SessionSequence=seq;return Task.FromResult(r);},null);
   await host.EnsureAsync(new Profile());Log(host,"capture-one");await host.StopAsync();seq=2;Log(host,"capture-two-native-stop");await host.CommandAsync("STATUS");
   string zip=await host.ExportDiagnosticsAsync();string unpack=Path.Combine(NewRoot(),"unpacked");System.IO.Compression.ZipFile.ExtractToDirectory(zip,unpack);
   Check(File.ReadAllText(Path.Combine(unpack,"app-engine.log"))=="capture-two-native-stop","retained child latest native-stop session exported");Check(File.ReadAllText(Path.Combine(unpack,"archive-result.json")).Contains("\"SessionSequence\":2"),"archive records capture sequence");child.Dead=true;host.ObserveExit();
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=994,CreationTimeUtcFileTime=994};int stops=0;var host=Host(NewRoot(),child,null,cmd=>{if(cmd=="STOP")stops++;var r=Idle();r.SessionSequence=1;return Task.FromResult(r);},null);
   await host.EnsureAsync(new Profile());host.Last=new EngineReply{Ok=true,Running=true,SafeIdle=false,State="running"};
   var timer=Stopwatch.StartNew();Check(await Failed(()=>host.ExportDiagnosticsAsync()),"active export refused");Check(timer.ElapsedMilliseconds<2000&&stops==0&&host.HasProcess,"export does not stop or occupy lifecycle");
   Log(host,"safe diagnostics export");await host.StopAsync();Check(stops==1,"STOP proceeds after denied export");
   string zip=await host.ExportDiagnosticsAsync();Check(File.Exists(zip),"stopped immutable archive exported");child.Dead=true;host.ObserveExit();
  });
   await Case(async ()=>{var testRoot=NewRoot();var source=Path.Combine(testRoot,"sharing-test.log");var destination=Path.Combine(testRoot,"sharing-copy.log");File.WriteAllText(source,"preserved");var held=new FileStream(source,FileMode.Open,FileAccess.ReadWrite,FileShare.None);var release=Task.Run(()=>{Thread.Sleep(45);held.Dispose();});EngineHost.CopyLogSnapshot993(source,destination);await release;Check(File.ReadAllText(destination)=="preserved","bounded retry preserves transiently locked logs");});

  await Case(async delegate{
   var child=new FakeChild988{Id=112,CreationTimeUtcFileTime=1013};var host=Host(NewRoot(),child,null,null,null);
   await host.EnsureAsync(new Profile());await host.EnsureAsync(new Profile());
   Check(Directory.GetDirectories(host.SessionDir,"a-*").Length==0,"initial sequence zero not auto-archived");child.Dead=true;host.ObserveExit();
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=113,CreationTimeUtcFileTime=1014};long sequence=0;
   var host=Host(NewRoot(),child,null,cmd=>{var r=Idle();r.SessionSequence=sequence;return Task.FromResult(r);},null);
   await host.EnsureAsync(new Profile());Log(host,"emergency-stop-session-one");sequence=1;
   await host.EnsureAsync(new Profile());await host.EnsureAsync(new Profile());
   Check(Directory.GetDirectories(host.SessionDir,"a-*").Length==1,"native idle session archived once before reuse");
   Check(Contents(host.SessionDir).Contains("emergency-stop-session-one"),"emergency stop preserved before subsequent START");
   Log(host,"limit-stop-session-two");sequence=2;await host.EnsureAsync(new Profile());
   Check(Directory.GetDirectories(host.SessionDir,"a-*").Length==2&&Contents(host.SessionDir).Contains("limit-stop-session-two"),"later natural stop gets distinct snapshot");child.Dead=true;host.ObserveExit();
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=114,CreationTimeUtcFileTime=1015};var host=Host(NewRoot(),child,null,cmd=>{var r=Idle();r.SessionSequence=1;return Task.FromResult(r);},null);
   await host.EnsureAsync(new Profile());Log(host,"explicit-stop-session");await host.StopAsync();await host.EnsureAsync(new Profile());
   Check(Directory.GetDirectories(host.SessionDir,"a-*").Length==1,"explicit Stop archive not copied again before reuse");child.Dead=true;host.ObserveExit();
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=115,CreationTimeUtcFileTime=1016};int startCommands=0;
   var host=Host(NewRoot(),child,null,cmd=>{if(cmd.StartsWith("START"))startCommands++;var r=Idle();r.SessionSequence=1;return Task.FromResult(r);},null);
   await host.EnsureAsync(new Profile());Log(host,"temporarily-unreadable");
   using(var locked=new FileStream(Path.Combine(host.WorkDir,"app-engine.log"),FileMode.Open,FileAccess.ReadWrite,FileShare.None)){
    Check(await Failed(()=>host.StartAsync(new WindowTarget(),new Profile())),"incomplete old archive blocks next Start");
    Check(startCommands==0,"archive failure sent no START command");
   }
   await host.EnsureAsync(new Profile());
   Check(Contents(host.SessionDir).Contains("temporarily-unreadable"),"failed archive sequence retries after readable");child.Dead=true;host.ObserveExit();
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=111,CreationTimeUtcFileTime=1012};int starts=0;var entered=new TaskCompletionSource<bool>();var release=new TaskCompletionSource<bool>();
   var host=Host(NewRoot(),child,null,async cmd=>{if(cmd=="STATUS"){entered.TrySetResult(true);await release.Task;}if(cmd.StartsWith("START"))starts++;if(cmd=="QUIT")child.Dead=true;return Idle();},null);
   await host.EnsureAsync(new Profile());var status=host.CommandAsync("STATUS");await entered.Task;
   var method=typeof(EngineHost).GetMethod("CommandCoreAsync",System.Reflection.BindingFlags.Instance|System.Reflection.BindingFlags.NonPublic);
   var pending=(Task<EngineReply>)method.Invoke(host,new object[]{"START 0 0 0",(int?)0});
   var closing=host.ShutdownAsync();release.SetResult(true);await status;
   Check(await Failed(()=>pending),"queued START invalidated before IPC send");await closing;Check(starts==0&&!host.HasProcess,"cancelled queued START never emitted");
  });
  await Case(async delegate{
   string deep=Path.Combine(root,new String('d',Math.Max(1,131-root.Length-1)));Directory.CreateDirectory(deep);
   var child=new FakeChild988{Id=110,CreationTimeUtcFileTime=1011};var host=Host(deep,child,null,null,null);await host.EnsureAsync(new Profile());
   string name="app-hud988.jsonl.previous-999999-999999999999";File.WriteAllText(Path.Combine(host.WorkDir,name),"deep-path-log");child.Dead=true;child.Code=85;var e=host.ObserveExit();
   Check(e!=null&&e.ArchiveSucceeded,"131-character product root archive succeeds");Check(Directory.GetFiles(e.SessionDir,name,SearchOption.AllDirectories).Length==1,"long log filename preserved");
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=100,CreationTimeUtcFileTime=1001};var host=Host(NewRoot(),child,null,null,null);
   await host.EnsureAsync(new Profile());string prior=host.SessionDir;Log(host,"first-fatal-log");
   host.Last=new EngineReply{Ok=true,Running=true,SafeIdle=false,State="running"};child.Dead=true;child.Code=85;
   var exit=host.ObserveExit();Check(exit!=null&&exit.Unexpected&&exit.ExitCode==85&&exit.ExitCodeKnown,"fatal exit facts");
   Check(exit.SessionDir==prior&&exit.ArchiveSucceeded,"old session ownership");
   Check(Contents(prior).Contains("first-fatal-log"),"fatal log archived");
   Check(host.ObserveExit()==null&&child.DisposeCalls==1&&!host.HasProcess,"once-only exit");
   Check(await Failed(()=>host.StopAsync()),"stop after known fatal cannot succeed");
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=101,CreationTimeUtcFileTime=1002};var host=Host(NewRoot(),child,null,null,null);
   await host.EnsureAsync(new Profile());child.Dead=true;child.Code=0;
   var e=host.ObserveExit();Check(e!=null&&!e.Unexpected&&e.ExitCode==0,"idle clean exit is normal");
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=102,CreationTimeUtcFileTime=1003};int starts=0;var host=Host(NewRoot(),child,null,null,()=>starts++);
   await host.EnsureAsync(new Profile());Log(host,"observed-inside-ensure");child.Dead=true;child.Code=86;
   string before=host.SessionDir;Check(await Failed(()=>host.EnsureAsync(new Profile())),"Ensure refuses implicit restart after fatal");
   Check(starts==1&&host.SessionDir==before&&Contents(before).Contains("observed-inside-ensure"),"no new session attributed old logs");
  });
  await Case(async delegate{
   string path=NewRoot();var child=new FakeChild988{Id=103,CreationTimeUtcFileTime=1004};var host=Host(path,child,null,null,null);
   Directory.CreateDirectory(host.WorkDir);Log(host,"prior-desktop-residue");
   await host.EnsureAsync(new Profile());
   var recovered=Directory.GetDirectories(Path.Combine(host.DataDir,"sessions"),"recovered-prior-*");
   Check(recovered.Length==1&&Contents(recovered[0]).Contains("prior-desktop-residue"),"prior unknown logs recovered separately");
   Check(!Contents(host.SessionDir).Contains("prior-desktop-residue")&&!File.Exists(Path.Combine(host.WorkDir,"app-engine.log")),"new run not polluted");
   child.Dead=true;host.ObserveExit();
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=104,CreationTimeUtcFileTime=1005};int starts=0;var host=Host(NewRoot(),child,null,null,()=>starts++);
   await host.EnsureAsync(new Profile());string old=host.SessionDir;Log(host,"old-owned-run");child.Dead=true;child.Code=0;host.ObserveExit();
   child=new FakeChild988(); // Factory deliberately still owns the original variable closure in Host; new host tests recovery separately.
   Check(starts==1&&Directory.GetDirectories(old,"a-*").Length==1,"single old exit archive");
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=105,CreationTimeUtcFileTime=1006,KillThrows=true};var host=Host(NewRoot(),child,null,null,null);
   await host.EnsureAsync(new Profile());Log(host,"kill-failure-retained");
   Check(await Failed(()=>host.ShutdownAsync()),"Kill failure propagated");
   Check(host.HasProcess&&child.DisposeCalls==0&&child.KillCalls==1,"live child retained on Kill failure");
   child.KillThrows=false;await host.ShutdownAsync();
   Check(!host.HasProcess&&child.DisposeCalls==1&&host.LastExit.Forced,"confirmed forced own exit cleanup");
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=106,CreationTimeUtcFileTime=1007,WaitNever=true};var host=Host(NewRoot(),child,null,null,null);
   await host.EnsureAsync(new Profile());
   Check(await Failed(()=>host.ShutdownAsync()),"postkill wait timeout propagated");
   Check(host.HasProcess&&child.DisposeCalls==0,"reference survives unconfirmed termination");
   child.WaitNever=false;child.Dead=true;host.ObserveExit();
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=107,CreationTimeUtcFileTime=1008};int starts=0;var entered=new ManualResetEventSlim(false);var release=new ManualResetEventSlim(false);
   var host=Host(NewRoot(),child,()=>{entered.Set();release.Wait(5000);},null,()=>starts++);
   Task ensuring=host.StartAsync(new WindowTarget(),new Profile());Check(entered.Wait(2000),"asset wait entered");
   Task closing=host.ShutdownAsync();release.Set();
   Check(await Failed(()=>ensuring),"prepare-close cancellation propagated");
   await closing;Check(starts==0&&!host.HasProcess,"close during Start/PrepareAssets prevents late child launch or START");
   entered.Dispose();release.Dispose();
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=108,CreationTimeUtcFileTime=1009};int starts=0;var entered=new TaskCompletionSource<bool>();var release=new TaskCompletionSource<bool>();
   var host=Host(NewRoot(),child,null,async cmd=>{if(cmd=="PING"){entered.TrySetResult(true);await release.Task;}if(cmd=="QUIT")child.Dead=true;return Idle();},()=>starts++);
   Task ensuring=host.EnsureAsync(new Profile());await entered.Task;Task closing=host.ShutdownAsync();release.SetResult(true);
   Check(await Failed(()=>ensuring),"close during PING cancels pending Ensure");
   await closing;Check(starts==1&&!host.HasProcess&&child.DisposeCalls==1,"postlaunch cancellation safely shuts same child");
  });
  await Case(async delegate{
   var child=new FakeChild988{Id=109,CreationTimeUtcFileTime=1010};var host=Host(NewRoot(),child,null,cmd=>{if(cmd=="STOP"){child.Dead=true;child.Code=88;}return Task.FromResult(Idle());},null);
   await host.EnsureAsync(new Profile());host.Last=new EngineReply{Running=true,SafeIdle=false,State="running"};
   Check(await Failed(()=>host.StopAsync()),"fatal while stopping not success");
   Check(!host.HasProcess&&host.LastExit.Unexpected&&host.LastExit.ExitCode==88,"fatal stop archive and identity result");
  });
  await Case(async delegate{
   var a=EngineReply.Parse("{\"ok\":true,\"emergencyHotkeyRegistered\":true}");var b=EngineReply.Parse("{\"ok\":true}");
   Check(a.EmergencyHotkeyKnown&&a.EmergencyHotkeyRegistered&&!b.EmergencyHotkeyKnown,"emergency registration parsed without missing false certainty");
   await Task.FromResult(0);
  });
 }
}
