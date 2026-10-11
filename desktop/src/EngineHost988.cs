using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Script.Serialization;

namespace SmoothMotionDesktop {
 public sealed class EngineReply {
  public bool Ok,Running,SafeIdle,ModelRetained,EmergencyHotkeyRegistered;
  public bool EmergencyHotkeyKnown;
  public bool TargetInfoValid990,TargetFullscreen990,TargetForeground990;
  public double Coverage990;
  public bool ModelReady992,ProcessingResampled992;
  public string PreparationStage992="idle";
  public double ForegroundWaitSeconds992,ModelPreparationSeconds992,CaptureSetupSeconds992;
  public int SourceWidth992,SourceHeight992,ProcessingWidth992,ProcessingHeight992;
  public bool OutputInfoValid993,OutputPreview993,OutputVisible993;
  public string OutputMode993="follow",OutputInputPolicy993="";
  public int[] OutputRect993,OutputContentRect993,OutputWindowRect993;
  public string PauseReason990="none";
  public int TargetClientWidth990,TargetClientHeight990,TargetMonitorWidth990,TargetMonitorHeight990;
  public string State="idle",Message="",Error="";
  public long SessionSequence;
  public static EngineReply Parse(string line) {
   var d=new JavaScriptSerializer().Deserialize<Dictionary<string,object>>(line);
   if(d==null)throw new IOException("引擎返回空回复");
   var r=new EngineReply(); object v;
   if(d.TryGetValue("ok",out v))r.Ok=Convert.ToBoolean(v);
   if(d.TryGetValue("running",out v))r.Running=Convert.ToBoolean(v);
   if(d.TryGetValue("safeIdle",out v))r.SafeIdle=Convert.ToBoolean(v);
   if(d.TryGetValue("modelRetained",out v))r.ModelRetained=Convert.ToBoolean(v);
   if(d.TryGetValue("state",out v))r.State=Convert.ToString(v);
   if(d.TryGetValue("message",out v))r.Message=Convert.ToString(v);
   if(d.TryGetValue("error",out v))r.Error=Convert.ToString(v);
   if(d.TryGetValue("sessionSequence",out v))r.SessionSequence=Convert.ToInt64(v);
   if(d.TryGetValue("emergencyHotkeyRegistered",out v)){r.EmergencyHotkeyKnown=true;r.EmergencyHotkeyRegistered=Convert.ToBoolean(v);}
   if(d.TryGetValue("targetInfoValid990",out v))r.TargetInfoValid990=Convert.ToBoolean(v);
   if(d.TryGetValue("targetFullscreen990",out v))r.TargetFullscreen990=Convert.ToBoolean(v);
   if(d.TryGetValue("targetForeground990",out v))r.TargetForeground990=Convert.ToBoolean(v);
   if(d.TryGetValue("coverage990",out v))r.Coverage990=Convert.ToDouble(v,System.Globalization.CultureInfo.InvariantCulture);
   if(d.TryGetValue("pauseReason990",out v))r.PauseReason990=Convert.ToString(v)??"none";
   if(d.TryGetValue("targetClientWidth990",out v))r.TargetClientWidth990=Convert.ToInt32(v);
   if(d.TryGetValue("targetClientHeight990",out v))r.TargetClientHeight990=Convert.ToInt32(v);
   if(d.TryGetValue("targetMonitorWidth990",out v))r.TargetMonitorWidth990=Convert.ToInt32(v);
   if(d.TryGetValue("targetMonitorHeight990",out v))r.TargetMonitorHeight990=Convert.ToInt32(v);
   if(Double.IsNaN(r.Coverage990)||Double.IsInfinity(r.Coverage990)||r.Coverage990<0||r.Coverage990>1||r.TargetClientWidth990<=0||r.TargetClientHeight990<=0||r.TargetMonitorWidth990<=0||r.TargetMonitorHeight990<=0)r.TargetInfoValid990=false;
   if(!r.TargetInfoValid990)r.TargetFullscreen990=false;
   if(d.TryGetValue("modelReady992",out v))r.ModelReady992=Convert.ToBoolean(v);
   if(d.TryGetValue("preparationStage992",out v))r.PreparationStage992=Convert.ToString(v)??"idle";
   if(d.TryGetValue("foregroundWaitSeconds992",out v))r.ForegroundWaitSeconds992=Convert.ToDouble(v,System.Globalization.CultureInfo.InvariantCulture);
   if(d.TryGetValue("modelPreparationSeconds992",out v))r.ModelPreparationSeconds992=Convert.ToDouble(v,System.Globalization.CultureInfo.InvariantCulture);
   if(d.TryGetValue("captureSetupSeconds992",out v))r.CaptureSetupSeconds992=Convert.ToDouble(v,System.Globalization.CultureInfo.InvariantCulture);
   if(d.TryGetValue("sourceWidth992",out v))r.SourceWidth992=Convert.ToInt32(v);
   if(d.TryGetValue("sourceHeight992",out v))r.SourceHeight992=Convert.ToInt32(v);
   if(d.TryGetValue("processingWidth992",out v))r.ProcessingWidth992=Convert.ToInt32(v);
   if(d.TryGetValue("processingHeight992",out v))r.ProcessingHeight992=Convert.ToInt32(v);
   if(d.TryGetValue("processingResampled992",out v))r.ProcessingResampled992=Convert.ToBoolean(v);
   if(d.TryGetValue("outputInfoValid993",out v))r.OutputInfoValid993=Convert.ToBoolean(v);
   if(d.TryGetValue("outputPreview993",out v))r.OutputPreview993=Convert.ToBoolean(v);
   if(d.TryGetValue("outputVisible993",out v))r.OutputVisible993=Convert.ToBoolean(v);
   if(d.TryGetValue("outputMode993",out v))r.OutputMode993=Convert.ToString(v);
   if(d.TryGetValue("outputInputPolicy993",out v))r.OutputInputPolicy993=Convert.ToString(v);
   if(d.TryGetValue("outputRect993",out v))r.OutputRect993=ReadRect993(v);
   if(d.TryGetValue("outputContentRect993",out v))r.OutputContentRect993=ReadRect993(v);
   if(d.TryGetValue("outputWindowRect993",out v))r.OutputWindowRect993=ReadRect993(v);
   if(r.OutputRect993==null||r.OutputContentRect993==null)r.OutputInfoValid993=false;
   return r;
  }
  static int[] ReadRect993(object value){var a=value as System.Collections.IList;if(a==null||a.Count!=4)return null;try{var r=new int[4];for(int i=0;i<4;i++)r[i]=Convert.ToInt32(a[i]);if((long)r[2]-r[0]<=0||(long)r[3]-r[1]<=0||(long)r[2]-r[0]>32768||(long)r[3]-r[1]>32768)return null;return r;}catch{return null;}}
 }
 public sealed class EngineExit {
  public int ExitCode,ProcessId;
  public bool ExitCodeKnown,Unexpected,Forced,ArchiveSucceeded;
  public long CreationTimeUtcFileTime;
  public string SessionDir="",Message="";
  public DateTime ObservedUtc;
 }
 // Testable process boundary; the production implementation owns only Process.Start's child.
 internal interface IEngineProcess988:IDisposable {
  int Id {get;} long CreationTimeUtcFileTime {get;} bool HasExited {get;} int ExitCode {get;}
  bool WaitForExit(int milliseconds); void Kill();
 }
 internal sealed class OwnedEngineProcess988:IEngineProcess988 {
  readonly Process process;
  public OwnedEngineProcess988(Process p){if(p==null)throw new IOException("Process.Start returned no child");process=p;}
  public int Id{get{return process.Id;}}
  public long CreationTimeUtcFileTime{get{return process.StartTime.ToUniversalTime().ToFileTimeUtc();}}
  public bool HasExited{get{process.Refresh();return process.HasExited;}}
  public int ExitCode{get{return process.ExitCode;}}
  public bool WaitForExit(int milliseconds){return process.WaitForExit(milliseconds);}
  public void Kill(){process.Kill();}
  public void Dispose(){process.Dispose();}
 }
 public sealed class EngineHost {
  public readonly string Root,DataDir,WorkDir;
  public string SessionDir {get;private set;}
  public string TelemetryPath {get{return Path.Combine(WorkDir,"app-hud988.jsonl");}}
  public EngineReply Last=new EngineReply{Ok=true,SafeIdle=true};
  public EngineExit LastExit {get;private set;}
  readonly object stateGate=new object(),traceGate=new object();
  readonly SemaphoreSlim commandGate=new SemaphoreSlim(1,1),lifecycleGate=new SemaphoreSlim(1,1);
  IEngineProcess988 child; string pipe,signature,childSessionDir; long childStart;
  bool expectedExit,forcedExit,logsPreserved; int lifecycleGeneration;
  long archivedIdleSessionSequence=-1; // Per child, only advance after a complete safe-idle archive.
  readonly Func<ProcessStartInfo,IEngineProcess988> launchOverride;
  readonly Action prepareOverride;
  readonly Func<string,Task<EngineReply>> commandOverride;
  readonly int shutdownWaitMs,killWaitMs;
  public bool HasProcess {
   get{lock(stateGate){if(child==null)return false;try{return !child.HasExited;}catch{return true;}}}
  }
  public EngineHost(string root):this(root,null,null,null,15000,3000){}
  internal EngineHost(string root,Func<ProcessStartInfo,IEngineProcess988> launch,Action prepare,Func<string,Task<EngineReply>> command,int shutdownGrace,int killGrace){
   Root=Path.GetFullPath(root);DataDir=Path.Combine(Root,"UserData");WorkDir=Path.Combine(DataDir,"runtime-work");
   SafeDirectory(DataDir);Directory.CreateDirectory(DataDir);
   launchOverride=launch;prepareOverride=prepare;commandOverride=command;
   shutdownWaitMs=shutdownGrace;killWaitMs=killGrace;
  }
  public void Trace(string text){Diagnostics994.Record(Root,"host.trace","info",new{session=childSessionDir==null?null:Path.GetFileName(childSessionDir),message=text});lock(traceGate)try{
   string path=Path.Combine(DataDir,"desktop.log");
   if(File.Exists(path)&&new FileInfo(path).Length>=1048576){
    string previous=path+".previous";
    if(File.Exists(previous))File.Delete(previous);
    File.Move(path,previous);
   }
   string bounded=text??"";if(bounded.Length>16384)bounded=bounded.Substring(0,16384)+" [message truncated]";
   File.AppendAllText(path,DateTime.UtcNow.ToString("o")+" "+bounded+Environment.NewLine,Encoding.UTF8);
  }catch{}}
  void CheckGeneration(int generation){if(Volatile.Read(ref lifecycleGeneration)!=generation)throw new OperationCanceledException("启动已取消，未继续发起捕获。");}
  public Task<EngineReply> CommandAsync(string command){return CommandCoreAsync(command,null);}
  async Task<EngineReply> CommandCoreAsync(string command,int? expectedGeneration){
   await commandGate.WaitAsync();
   long commandStart994=Stopwatch.GetTimestamp();string commandName994=(command??"unknown").Split(' ')[0];bool recordCommand994=commandName994!="STATUS"&&commandName994!="PING";
   if(recordCommand994)Diagnostics994.Record(Root,"ipc.begin","info",new{command=commandName994,session=childSessionDir==null?null:Path.GetFileName(childSessionDir)});
   try{
    IEngineProcess988 owned;string ownPipe;
    lock(stateGate){if(expectedGeneration.HasValue)CheckGeneration(expectedGeneration.Value);owned=child;ownPipe=pipe;if(owned==null||owned.HasExited)throw new IOException("插帧引擎已退出，等待退出记录确认。");}
    EngineReply parsed;
    if(commandOverride!=null){Task<EngineReply> sending;lock(stateGate){if(expectedGeneration.HasValue)CheckGeneration(expectedGeneration.Value);sending=commandOverride(command);}parsed=await sending;}
    else using(var client=new NamedPipeClientStream(".",ownPipe,PipeDirection.InOut,PipeOptions.Asynchronous)){
     await Task.Run(()=>client.Connect(1200));
     using(var wr=new StreamWriter(client,new UTF8Encoding(false),1024,true))
     using(var rd=new StreamReader(client,Encoding.UTF8,false,4096,true)){
      wr.AutoFlush=true;Task writing;lock(stateGate){if(expectedGeneration.HasValue)CheckGeneration(expectedGeneration.Value);writing=wr.WriteLineAsync(command);}await writing;
      var reading=rd.ReadLineAsync();
      if(await Task.WhenAny(reading,Task.Delay(2000))!=reading)throw new TimeoutException("引擎响应超时");
      string response=await reading;if(response==null||response.Length>65536)throw new IOException("引擎连接已关闭");
      parsed=EngineReply.Parse(response);
     }
    }
    lock(stateGate){
     if(!Object.ReferenceEquals(owned,child))throw new IOException("回复属于已经结束的引擎进程。");
     if(command=="STATUS"||command.StartsWith("START",StringComparison.Ordinal)||command=="STOP")Last=parsed;
    }
    if(command!="STATUS"&&command!="PING")Trace(command+" -> "+new JavaScriptSerializer().Serialize(parsed));
    if(recordCommand994)Diagnostics994.Record(Root,"ipc.end","info",new{command=commandName994,ok=parsed.Ok,state=parsed.State,elapsed_ms=(Stopwatch.GetTimestamp()-commandStart994)*1000.0/Stopwatch.Frequency});return parsed;
   }catch(Exception e){Diagnostics994.Record(Root,"ipc.failed","error",new{command=commandName994,elapsed_ms=(Stopwatch.GetTimestamp()-commandStart994)*1000.0/Stopwatch.Frequency},e);throw;}finally{commandGate.Release();}
  }
  // Once-only observation: no lifecycle operation can start a replacement while old logs are copied.
  public EngineExit ObserveExit(){
   if(!lifecycleGate.Wait(0))return null;
   try{return CollectExitCore();}finally{lifecycleGate.Release();}
  }
  EngineExit CollectExitCore(){
   IEngineProcess988 owned;EngineExit result;
   lock(stateGate){
    owned=child;if(owned==null)return null;
    try{if(!owned.HasExited)return null;}catch(Exception e){Trace("exit status unavailable; retaining child: "+e.Message);return null;}
    result=new EngineExit{ProcessId=owned.Id,CreationTimeUtcFileTime=childStart,SessionDir=childSessionDir??"",Forced=forcedExit,ObservedUtc=DateTime.UtcNow};
    try{result.ExitCode=owned.ExitCode;result.ExitCodeKnown=true;}catch{result.ExitCode=Int32.MinValue;}
    result.Unexpected=!expectedExit&&(!result.ExitCodeKnown||result.ExitCode!=0||!Last.SafeIdle);
   }
   result.Message=(result.Unexpected?"插帧引擎异常退出":"插帧引擎已退出")+"，退出码 "+(result.ExitCodeKnown?result.ExitCode.ToString():"未知");
   result.ArchiveSucceeded=ArchiveCore(result.SessionDir,"exit-"+(result.ExitCodeKnown?result.ExitCode.ToString():"unknown"));
   logsPreserved=result.ArchiveSucceeded;
   if(result.SessionDir.Length>0)try{File.WriteAllText(Path.Combine(result.SessionDir,"engine-exit.json"),new JavaScriptSerializer().Serialize(result),Encoding.UTF8);}catch(Exception e){Trace("exit metadata "+e.Message);}
   Trace("exit pid="+result.ProcessId+" creation="+result.CreationTimeUtcFileTime+" session="+result.SessionDir+" unexpected="+result.Unexpected+" code="+result.ExitCode+" archive="+result.ArchiveSucceeded);
   lock(stateGate){
    // Every caller holds lifecycleGate. No later child may have replaced this one.
    if(!Object.ReferenceEquals(child,owned))throw new InvalidOperationException("Child lifetime changed while collecting exit");
    child=null;signature=null;pipe=null;childSessionDir=null;childStart=0;expectedExit=forcedExit=false;LastExit=result;
    Last=new EngineReply{Ok=!result.Unexpected,SafeIdle=true,State=result.Unexpected?"error":"idle",Message=result.Message};
   }
   try{owned.Dispose();}catch(Exception e){Trace("exited process handle cleanup "+e.Message);}return result;
  }
  static IOException ExitException(EngineExit exit){return new IOException(exit.Message+"；记录："+exit.SessionDir);}
  static string HashFile(string p){using(var s=File.OpenRead(p))using(var h=SHA256.Create())return BitConverter.ToString(h.ComputeHash(s)).Replace("-","").ToLowerInvariant();}
  static void SafeDirectory(string path){for(var d=new DirectoryInfo(Path.GetFullPath(path));d!=null;d=d.Parent)if(d.Exists&&(d.Attributes&FileAttributes.ReparsePoint)!=0)throw new IOException("运行目录不能是链接或重解析目录。");}
  void PrepareAssets(){SafeDirectory(WorkDir);Directory.CreateDirectory(WorkDir);using(var guard=new FileStream(Path.Combine(WorkDir,"engine988.lock"),FileMode.OpenOrCreate,FileAccess.ReadWrite,FileShare.Read)){PrepareAssetFiles();}}
  void PrepareAssetFiles() {
   string source=Path.Combine(Root,"model");
   var manifest=new JavaScriptSerializer().DeserializeObject(File.ReadAllText(Path.Combine(Root,"assets988.json"),Encoding.UTF8)) as Dictionary<string,object>;
   var files=manifest["files"] as object[];if(files==null||files.Length==0||files.Length>2000)throw new IOException("运行资产清单无效");
   Directory.CreateDirectory(WorkDir);PrepareExternalShaders(manifest);
   foreach(var entry in files){var f=(Dictionary<string,object>)entry;string rel=Convert.ToString(f["path"]).Replace('/',Path.DirectorySeparatorChar);if(Path.IsPathRooted(rel)||rel.Split(Path.DirectorySeparatorChar).Contains(".."))throw new IOException("资产路径无效");
    string src=source==null?null:Path.Combine(source,rel),dst=Path.Combine(WorkDir,rel);long size=Convert.ToInt64(f["size"]);string hash=Convert.ToString(f["sha256"]);
    if(File.Exists(dst)&&new FileInfo(dst).Length==size&&HashFile(dst)==hash)continue;
    if(!File.Exists(src)||new FileInfo(src).Length!=size||HashFile(src)!=hash)throw new IOException("模型文件缺失或校验失败；请按 README 导入有权使用的本地模型："+rel);
    SafeDirectory(Path.GetDirectoryName(dst));if(File.Exists(dst)&&(File.GetAttributes(dst)&FileAttributes.ReparsePoint)!=0)throw new IOException("运行资产不可为链接。");Directory.CreateDirectory(Path.GetDirectoryName(dst));string temp=dst+".copying";File.Copy(src,temp,true);if(File.Exists(dst))File.Delete(dst);File.Move(temp,dst);
   }
  }
  void PrepareExternalShaders(Dictionary<string,object> manifest){
   var external=(Dictionary<string,object>)manifest["externalShaders"];var files=(object[])external["files"];
   string source=Path.Combine(Root,"model","shaders"),dest=Path.Combine(DataDir,"runtime-shaders");SafeDirectory(dest);Directory.CreateDirectory(dest);
   foreach(var entry in files){var f=(Dictionary<string,object>)entry;string name=Convert.ToString(f["path"]);if(name!=Path.GetFileName(name))throw new IOException("着色器路径无效");string dst=Path.Combine(dest,name),src=source==null?null:Path.Combine(source,name),hash=Convert.ToString(f["sha256"]);long size=Convert.ToInt64(f["size"]);
    if(File.Exists(dst)&&new FileInfo(dst).Length==size&&HashFile(dst)==hash)continue;if(!File.Exists(src)||new FileInfo(src).Length!=size||HashFile(src)!=hash)throw new IOException("模型着色器缺失或校验失败；请按 README 导入本地模型："+name);File.Copy(src,dst,true);
   }
  }


  string Signature(Profile p){return p.CaptureCursor+"|"+p.OpaqueContent+"|"+p.TimelineMs+"|"+p.SessionSeconds+"|"+p.FrameMultiplier+"|"+p.CaptureBackend+"|"+p.PresentSync+"|"+p.PresentDepth+"|"+p.ProcessingSize+"|"+p.OutputMode;}
  public async Task EnsureAsync(Profile p){
   int generation=Volatile.Read(ref lifecycleGeneration);
   await lifecycleGate.WaitAsync();
   try{CheckGeneration(generation);await EnsureCoreAsync(p,generation);}finally{lifecycleGate.Release();}
  }
  async Task EnsureCoreAsync(Profile p,int generation){
   CheckGeneration(generation);
   var exited=CollectExitCore();if(exited!=null&&exited.Unexpected)throw ExitException(exited);
   if(HasProcess&&signature==Signature(p)){
    // Native emergency stop and the session limit bypass host StopAsync. Refresh the
    // child identity's state before START can overwrite the previous run's logs.
    var retained=await CommandCoreAsync("STATUS",generation);CheckGeneration(generation);
    if(!retained.Ok)throw new IOException("无法确认保留引擎的会话状态："+retained.Error);
    ArchiveIdleSessionOnce(retained,"before-start",false);
    return;
   }
   if(HasProcess){await ShutdownCoreAsync();CheckGeneration(generation);}
   if(launchOverride==null){
    foreach(var other in Process.GetProcesses()){
     string name=null;int pid=0;
     try{pid=other.Id;name=other.ProcessName;}catch(InvalidOperationException){}catch(System.ComponentModel.Win32Exception){}finally{other.Dispose();}
     if(pid!=Process.GetCurrentProcess().Id&&name!=null&&(name.StartsWith("SmoothMotionB390",StringComparison.OrdinalIgnoreCase)||name.StartsWith("SmoothMotion-Engine",StringComparison.OrdinalIgnoreCase)))
      throw new InvalidOperationException("请先停止并关闭另一个插帧版本，避免同时占用 GPU。");
    }
   }
   await Task.Run(()=>{if(prepareOverride!=null)prepareOverride();else PrepareAssets();});
   CheckGeneration(generation);
   // The previous process's logs are archived before any new SessionDir is assigned.
   PreservePriorLogs();
   CheckGeneration(generation);
   string id=DateTime.Now.ToString("yyyyMMdd-HHmmss")+"-"+Guid.NewGuid().ToString("N").Substring(0,6);
   string newSession=Path.Combine(DataDir,"sessions",id);SafeDirectory(newSession);Directory.CreateDirectory(newSession);
   string newPipe="SmoothMotion988-"+Guid.NewGuid().ToString("N");
   var own=Process.GetCurrentProcess();var psi=new ProcessStartInfo(Path.Combine(Root,"SmoothMotion-Engine.exe"));
   psi.Arguments="--hosted --pipe "+newPipe+" --host-pid "+own.Id+" --host-start "+own.StartTime.ToUniversalTime().ToFileTimeUtc();
   psi.UseShellExecute=false;psi.CreateNoWindow=true;psi.WorkingDirectory=WorkDir;
   foreach(string key in psi.EnvironmentVariables.Keys.Cast<string>().Where(k=>k.StartsWith("SMX_",StringComparison.OrdinalIgnoreCase)).ToArray())psi.EnvironmentVariables.Remove(key);
   string cache=Path.Combine(DataDir,"pipeline-cache");SafeDirectory(cache);Directory.CreateDirectory(cache);
   var preset=EnginePreset988.Create(WorkDir,cache,Path.Combine(DataDir,"runtime-shaders"),WorkDir,p);
   preset["SMX_DIAGNOSTIC_SESSION994"]=id;preset["SMX_DIAGNOSTIC_DIR994"]=WorkDir;
   foreach(var kv in preset)psi.EnvironmentVariables[kv.Key]=kv.Value;
   File.WriteAllText(Path.Combine(newSession,"launch.json"),new JavaScriptSerializer().Serialize(new{Revision="R995",EngineBaseline="R987",Software=Diagnostics994.SoftwareIdentity(Root),Settings=p,Environment=preset,StartedUtc=DateTime.UtcNow.ToString("o")}),Encoding.UTF8);
   CheckGeneration(generation);
   lock(stateGate){
    CheckGeneration(generation);
    SessionDir=newSession;childSessionDir=newSession;pipe=newPipe;expectedExit=forcedExit=logsPreserved=false;
    child=launchOverride!=null?launchOverride(psi):new OwnedEngineProcess988(Process.Start(psi));
    if(child==null)throw new IOException("引擎启动没有返回进程");
    childStart=child.CreationTimeUtcFileTime;signature=Signature(p);archivedIdleSessionSequence=-1;
    Last=new EngineReply{Ok=true,SafeIdle=true,State="idle"};
   }
   Trace("engine pid="+child.Id+" session="+id);
   Exception last=null;
   for(int i=0;i<10;i++){
    CheckGeneration(generation);
    exited=CollectExitCore();if(exited!=null)throw ExitException(exited);
    try{var ping=await CommandAsync("PING");CheckGeneration(generation);if(ping.Ok)return;}catch(OperationCanceledException){throw;}catch(Exception e){last=e;}
    await Task.Delay(250);CheckGeneration(generation);
   }
   await ShutdownCoreAsync();throw new IOException("无法连接插帧引擎。",last);
  }
  public async Task<EngineReply> StartAsync(WindowTarget target,Profile p){
   int generation=Volatile.Read(ref lifecycleGeneration);
   await lifecycleGate.WaitAsync();
   try{
    CheckGeneration(generation);await EnsureCoreAsync(p,generation);CheckGeneration(generation);
    if(!WindowCatalog.IsCurrent(target))throw new InvalidOperationException("目标窗口已关闭或身份已变化，请重新选择。");
    CheckGeneration(generation);
    var r=await CommandCoreAsync("START "+target.Handle.ToInt64()+" "+target.ProcessId+" "+target.ProcessCreationTimeUtcFileTime,generation);
    CheckGeneration(generation);
    if(!r.Ok)throw new InvalidOperationException(string.IsNullOrEmpty(r.Error)?r.Message:r.Error);
    await CommandCoreAsync("HUD "+(p.HudVisible?"1":"0"),generation);CheckGeneration(generation);
    await CommandCoreAsync("DETAIL "+(p.HudDetails?"1":"0"),generation);CheckGeneration(generation);
    return r;
   }finally{lifecycleGate.Release();}
  }
  public async Task StopAsync(){
   lock(stateGate){Interlocked.Increment(ref lifecycleGeneration);}
   await lifecycleGate.WaitAsync();
   try{
    var exited=CollectExitCore();if(exited!=null&&exited.Unexpected)throw ExitException(exited);
    if(!HasProcess){if(LastExit!=null&&LastExit.Unexpected)throw ExitException(LastExit);return;}
    var response=await CommandAsync("STOP");if(!response.Ok)throw new IOException("引擎拒绝停止："+response.Error);
    long until=Stopwatch.GetTimestamp()+Stopwatch.Frequency*15;
    while(Stopwatch.GetTimestamp()<until){
     await Task.Delay(250);
     exited=CollectExitCore();if(exited!=null){if(exited.Unexpected)throw ExitException(exited);return;}
     var r=await CommandAsync("STATUS");
     if(r.Ok&&r.SafeIdle){ArchiveIdleSessionOnce(r,"stopped",true);return;}
    }
    throw new TimeoutException("安全停止超过 15 秒；进程身份仍保留，需执行退出清理。");
   }finally{lifecycleGate.Release();}
  }
  void ArchiveIdleSessionOnce(EngineReply state,string tag,bool includeInitial){ // lifecycleGate held
   if(!state.Ok||!state.SafeIdle||state.Running)return;
   long sequence=state.SessionSequence;
   if(sequence<0)throw new IOException("引擎会话序号无效，未开始新的捕获。");
   if((sequence==0&&!includeInitial)||sequence<=archivedIdleSessionSequence)return;
   if(!ArchiveCore(childSessionDir??SessionDir,tag+"-"+sequence))
    throw new IOException("引擎已安全停止，但前一会话日志未完整归档；已取消新的捕获。");
   archivedIdleSessionSequence=sequence;
   Trace("safe-idle session archived once sequence="+sequence+" reason="+tag);
  }
  static bool ArchiveMatches994(string path,long? required){try{var json=new JavaScriptSerializer().Deserialize<Dictionary<string,object>>(File.ReadAllText(Path.Combine(path,"archive-result.json")));if(!required.HasValue)return true;object seq;return json.TryGetValue("SessionSequence",out seq)&&Convert.ToInt64(seq)==required.Value;}catch{return false;}}
  public async Task<string> ExportDiagnosticsAsync(){
   string selected=null,archivePath=null;long? requiredSequence994=null;
   if(!await lifecycleGate.WaitAsync(0))throw new IOException("正在启动或停止；诊断记录继续保存，安全停止后可导出。");
   try{
    if(HasProcess&&!Last.SafeIdle)throw new IOException("插帧运行期间持续记录诊断；请安全停止后再导出，避免影响游戏与停止操作。");
    if(HasProcess){var fresh=await CommandAsync("STATUS");if(!fresh.Ok||!fresh.SafeIdle)throw new IOException("引擎尚未安全停止；记录继续保存，请稍后导出。");requiredSequence994=fresh.SessionSequence;ArchiveIdleSessionOnce(fresh,"diagnostic-export",true);}
    selected=SessionDir;
    if(String.IsNullOrEmpty(selected)){string sessions=Path.Combine(DataDir,"sessions");if(Directory.Exists(sessions)){var last=new DirectoryInfo(sessions).GetDirectories().OrderByDescending(x=>x.LastWriteTimeUtc).FirstOrDefault();if(last!=null)selected=last.FullName;}}
    if(String.IsNullOrEmpty(selected)||!Directory.Exists(selected))throw new IOException("尚无会话归档。日志仍保留在本地目录。");
    var snapshot=new DirectoryInfo(selected).GetDirectories("a-*").Where(d=>ArchiveMatches994(d.FullName,requiredSequence994)).OrderByDescending(x=>x.LastWriteTimeUtc).FirstOrDefault();
    if(snapshot==null)throw new IOException("本次会话没有已完成归档，未混入其他运行日志；原始日志仍保留在本地目录。");
    archivePath=snapshot.FullName;
   }finally{lifecycleGate.Release();}
   return await Task.Run(()=>Diagnostics994.Export(Root,selected,archivePath,false));
  }
  public void Archive(string tag){
   if(!lifecycleGate.Wait(0)){Trace("archive deferred while lifecycle operation active");return;}
   try{ArchiveCore(SessionDir,tag);}finally{lifecycleGate.Release();}
  }
  List<string> CurrentLogs(){
   if(!Directory.Exists(WorkDir))return new List<string>();
   var paths=Directory.GetFiles(WorkDir,"app-*",SearchOption.TopDirectoryOnly).ToList();
   string startup=Path.Combine(WorkDir,"startup-verification.json");if(File.Exists(startup))paths.Add(startup);return paths;
  }
  internal static void CopyLogSnapshot993(string source,string destination){
   FileStream input=null;
   for(int attempt=0;;attempt++)try{input=new FileStream(source,FileMode.Open,FileAccess.Read,FileShare.ReadWrite|FileShare.Delete);break;}
   catch(IOException e){int code=e.HResult&65535;if(attempt>=3||(code!=32&&code!=33))throw;Thread.Sleep(20<<attempt);}
   using(input)using(var output=new FileStream(destination,FileMode.CreateNew,FileAccess.Write,FileShare.Read)){input.CopyTo(output);output.Flush(true);}
  }
  bool ArchiveCore(string destination,string tag){
   if(String.IsNullOrEmpty(destination))return false;
   try{
    var files=CurrentLogs();string safeTag=new string(tag.Where(c=>Char.IsLetterOrDigit(c)||c=='-'||c=='_').ToArray());
    string dir=Path.Combine(destination,"a-"+Guid.NewGuid().ToString("N").Substring(0,10));
    SafeDirectory(dir);Directory.CreateDirectory(dir);
    bool complete=true;
    foreach(string file in files)try{
     if((File.GetAttributes(file)&FileAttributes.ReparsePoint)!=0)throw new IOException("日志不得为链接");
     CopyLogSnapshot993(file,Path.Combine(dir,Path.GetFileName(file)));
    }catch(Exception ex){complete=false;Trace("archive "+Path.GetFileName(file)+" "+ex.Message);}
    Diagnostics994.DecodeNative(Root,dir);Diagnostics994.Analyze(dir);
    File.WriteAllText(Path.Combine(dir,"archive-result.json"),new JavaScriptSerializer().Serialize(new{Complete=complete,Tag=safeTag,SessionDir=destination,SessionSequence=Last.SessionSequence,ChildPid=child==null?0:child.Id,ChildCreationTime=childStart,FileCount=files.Count,CreatedUtc=DateTime.UtcNow.ToString("o")}),Encoding.UTF8);
    return complete;
   }catch(Exception ex){Trace("archive destination "+ex.Message);return false;}
  }
  void PreservePriorLogs(){
   SafeDirectory(WorkDir);Directory.CreateDirectory(WorkDir);
   using(var guard=new FileStream(Path.Combine(WorkDir,"engine988.lock"),FileMode.OpenOrCreate,FileAccess.ReadWrite,FileShare.Read)){
    var files=CurrentLogs();if(files.Count==0)return;
    if(!logsPreserved){
     string prior=SessionDir;
     if(String.IsNullOrEmpty(prior)){
      prior=Path.Combine(DataDir,"sessions","recovered-prior-"+Guid.NewGuid().ToString("N").Substring(0,8));
      SafeDirectory(prior);Directory.CreateDirectory(prior);
      File.WriteAllText(Path.Combine(prior,"recovery.json"),new JavaScriptSerializer().Serialize(new{RecoveredPrior=true,OriginalSessionUnknown=true,RecoveredUtc=DateTime.UtcNow.ToString("o"),Scope="Residual logs from a prior desktop process; not attributed to the next run."}),Encoding.UTF8);
     }
     if(!ArchiveCore(prior,"before-restart"))throw new IOException("旧会话日志未完整保存，已取消新启动。");
    }
    foreach(string file in files)File.Delete(file);
    logsPreserved=false;
   }
  }
  public async Task ShutdownAsync(){
   // Invalidate even when no child exists yet: Ensure may be awaiting asset preparation.
   lock(stateGate){Interlocked.Increment(ref lifecycleGeneration);}
   await lifecycleGate.WaitAsync();
   try{await ShutdownCoreAsync();}finally{lifecycleGate.Release();}
  }
  async Task ShutdownCoreAsync(){
   var exited=CollectExitCore();if(exited!=null)return;
   IEngineProcess988 owned;long identity;
   lock(stateGate){owned=child;identity=childStart;if(owned==null)return;expectedExit=true;}
   try{await CommandAsync("STOP");}catch(Exception ex){Trace("shutdown STOP "+ex.Message);}
   try{await CommandAsync("QUIT");}catch(Exception ex){Trace("shutdown QUIT "+ex.Message);}
   bool done=await Task.Run(()=>owned.WaitForExit(shutdownWaitMs));
   if(!done){
    bool same=false;
    lock(stateGate){same=Object.ReferenceEquals(child,owned)&&identity>0&&owned.CreationTimeUtcFileTime==identity;}
    if(!same)throw new IOException("退出超时且无法核实原子进程身份；已保留进程引用，未终止其他进程。");
    Diagnostics994.Record(Root,"host.shutdown_timeout","error",new{pid=owned.Id,creation=identity,grace_ms=shutdownWaitMs,stage="before_owned_child_termination"});
    Trace("owned child exceeded shutdown grace; terminate pid="+owned.Id);
    // Kill/Wait exceptions deliberately leave the live owned process attached.
    lock(stateGate){forcedExit=true;}
    owned.Kill();
    done=await Task.Run(()=>owned.WaitForExit(killWaitMs));
    if(!done||!owned.HasExited)throw new TimeoutException("自有引擎终止尚未确认；保留进程身份，禁止启动替代进程。");
   }
   exited=CollectExitCore();
   if(exited==null)throw new IOException("引擎退出未确认；保留原进程身份。");
  }
 }
}
