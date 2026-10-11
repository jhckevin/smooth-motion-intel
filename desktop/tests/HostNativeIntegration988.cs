using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using SmoothMotionDesktop;

public static class HostNativeIntegration988 {
 static readonly object Sync = new object();
 static readonly List<object> Beats = new List<object>();
 static string Hash(string path) {using(var input=File.OpenRead(path))using(var hash=SHA256.Create())return BitConverter.ToString(hash.ComputeHash(input)).Replace("-","").ToLowerInvariant();}
 static void Check(bool condition,string message){if(!condition)throw new Exception(message);}
 static async Task Run(string root) {
  root=Path.GetFullPath(root);Directory.CreateDirectory(Path.Combine(root,"UserData"));
  var settings=new AppSettings();settings.Profiles[0].Name="Native integration CPU only";
  string settingsPath=Path.Combine(root,"UserData","integration-profile988.json");
  ProductStore.Save(settingsPath,settings);string warning;var loaded=ProductStore.Load(settingsPath,out warning);
  Check(string.IsNullOrEmpty(warning),"ProductStore emitted a warning");Check(loaded.Profiles[0].Id==settings.Profiles[0].Id,"Profile atomic store roundtrip mismatch");
  var profile=loaded.Profiles[0];var host=new EngineHost(root);var cts=new CancellationTokenSource();bool shuttingDown=false,ready=false;
  var heartbeat=Task.Run(async delegate {
   var watch=Stopwatch.StartNew();long next=0;
   while(!cts.IsCancellationRequested){
    if(host.HasProcess){var started=Stopwatch.GetTimestamp();try{var reply=await host.CommandAsync("STATUS");lock(Sync)Beats.Add(new {qpc=started,ok=reply.Ok,state=reply.State,safeIdle=reply.SafeIdle});if(ready&&!shuttingDown)Check(reply.Ok&&reply.SafeIdle&&!reply.Running,"Heartbeat found active/unsafe engine in CPU test");}catch(Exception ex){if(ready&&!shuttingDown)throw new Exception("Heartbeat failed",ex);}}
    next+=1000;int wait=(int)Math.Max(1,next-watch.ElapsedMilliseconds);try{await Task.Delay(wait,cts.Token);}catch(TaskCanceledException){break;}
   }
  });
  var report=new Dictionary<string,object>();report["gpu_used"]=false;report["valid_start_sent"]=false;report["profile_store_roundtrip"]=true;
  try {
   await host.EnsureAsync(profile);ready=true;Check(host.HasProcess,"Ensure did not retain an engine process");
   var ping=await host.CommandAsync("PING");Check(ping.Ok&&ping.SafeIdle&&!ping.Running&&ping.SessionSequence==0&&!ping.ModelRetained,"Initial native status invalid");
   string firstSession=host.SessionDir;await host.EnsureAsync(profile);Check(firstSession==host.SessionDir,"Same profile unexpectedly restarted engine");
   foreach(string command in new[]{"HUD 0","DETAIL 1","HUD 1","DETAIL 0"}){var reply=await host.CommandAsync(command);Check(reply.Ok&&reply.SafeIdle&&!reply.Running,"Live control failed: "+command);}
   var serializer=new JavaScriptSerializer();var manifest=serializer.DeserializeObject(File.ReadAllText(Path.Combine(root,"assets988.json"),Encoding.UTF8)) as Dictionary<string,object>;
   var files=(object[])manifest["files"];long bytes=0;foreach(var raw in files){var file=(Dictionary<string,object>)raw;string p=Path.Combine(host.WorkDir,Convert.ToString(file["path"]).Replace('/',Path.DirectorySeparatorChar));Check(File.Exists(p),"Prepared asset missing");Check(new FileInfo(p).Length==Convert.ToInt64(file["size"])&&Hash(p)==Convert.ToString(file["sha256"]),"Prepared asset hash mismatch");bytes+=new FileInfo(p).Length;}
   Check(Directory.Exists(Path.Combine(host.DataDir,"pipeline-cache")),"First cache directory was not created");
   var launch=serializer.DeserializeObject(File.ReadAllText(Path.Combine(host.SessionDir,"launch.json"),Encoding.UTF8)) as Dictionary<string,object>;
   var env=(Dictionary<string,object>)launch["Environment"];string shaderDir=Convert.ToString(env["SMX_NATIVE_SHADERS969"]);
   Check(Path.GetFullPath(shaderDir).StartsWith(Path.GetFullPath(host.DataDir)+Path.DirectorySeparatorChar,StringComparison.OrdinalIgnoreCase),"Native shaders still depend on external original directory");
   var ext=(Dictionary<string,object>)manifest["externalShaders"];var shaderFiles=(object[])ext["files"];
   foreach(var raw in shaderFiles){var file=(Dictionary<string,object>)raw;string p=Path.Combine(shaderDir,Convert.ToString(file["path"]));Check(File.Exists(p)&&Hash(p)==Convert.ToString(file["sha256"]),"Native shader copy hash mismatch");}
   report["runtime_assets"]=files.Length;report["runtime_asset_bytes"]=bytes;report["private_shaders"]=shaderFiles.Length;report["cache_created"]=true;report["same_profile_process_reused"]=true;
   string marker="CPU integration fixture; no capture/START/GPU submission\n";string markerName="app-hostintegration988.txt";File.WriteAllText(Path.Combine(host.WorkDir,markerName),marker,Encoding.UTF8);
   await Task.Delay(3200);await host.StopAsync();Check(host.Last.SafeIdle&&!host.Last.Running,"STOP did not reach safe idle");
   var archive=Directory.GetDirectories(host.SessionDir,"a-*").OrderBy(x=>x).LastOrDefault();Check(archive!=null&&File.ReadAllText(Path.Combine(archive,markerName),Encoding.UTF8)==marker,"Safe-idle archive copy missing");
   report["safe_idle_archive"]=true;report["session_dir"]=host.SessionDir;
   shuttingDown=true;await host.ShutdownAsync();Check(!host.HasProcess,"QUIT failed to retire owned child");report["child_exit"]=true;
   cts.Cancel();await heartbeat;lock(Sync){Check(Beats.Count>=3,"Too few 1 Hz heartbeat observations");report["heartbeat"]=Beats.ToArray();}
   report["pass"]=true;report["scope"]="Actual EngineHost + ProductStore + preset + native pipe, no valid START, no GPU inference/capture";
   File.WriteAllText(Path.Combine(root,"tests","host-native-integration988.json"),serializer.Serialize(report),new UTF8Encoding(false));Console.WriteLine("PASS actual C# host/native integration: assets, private shaders, 1Hz heartbeat, safe idle archive, QUIT; GPU work=0");
  } finally {shuttingDown=true;try{host.ShutdownAsync().GetAwaiter().GetResult();}finally{cts.Cancel();try{heartbeat.GetAwaiter().GetResult();}catch{}}}
 }
 public static int Main(string[] args){try{if(args.Length!=1)throw new ArgumentException("Expected consumer root");Run(args[0]).GetAwaiter().GetResult();return 0;}catch(Exception ex){Console.Error.WriteLine(ex);return 1;}}
}
