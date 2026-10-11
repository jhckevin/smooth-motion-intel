using System;
using System.IO;
using System.IO.Compression;
using System.Text;
using System.Linq;
using System.Collections;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using SmoothMotionDesktop;
class Diagnostics994Tests {
 static int checks;static JavaScriptSerializer j=new JavaScriptSerializer{MaxJsonLength=32*1024*1024};
 static void Check(bool test,string name){checks++;if(!test)throw new Exception(name);}
 static string Hash(string p){using(var f=File.OpenRead(p))using(var h=SHA256.Create())return BitConverter.ToString(h.ComputeHash(f)).Replace("-","").ToLowerInvariant();}
 static Dictionary<string,object> Object(string p){return j.DeserializeObject(File.ReadAllText(p)) as Dictionary<string,object>;}
 static int Main(string[] args){try{
  string baseDir=args.Length>0?Path.GetFullPath(args[0]):Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"diagnostics-cpu-fixtures");Directory.CreateDirectory(baseDir);
  string root=Path.Combine(baseDir,"case-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);string data=Path.Combine(root,"UserData"),work=Path.Combine(data,"runtime-work"),session=Path.Combine(data,"sessions","s1"),archive=Path.Combine(session,"a-case");Directory.CreateDirectory(work);Directory.CreateDirectory(archive);File.WriteAllText(Path.Combine(archive,"archive-result.json"),"{\"Complete\":true}");
  Diagnostics994.Initialize(root);Diagnostics994.Record(root,"test.error","error",new{frame=12},new IOException("test "+root));string events=Path.Combine(data,"desktop-events994.jsonl");var ev=j.DeserializeObject(File.ReadAllLines(events).Last()) as Dictionary<string,object>;
  Check((string)ev["code"]=="test.error","structured event code");Check(ev.ContainsKey("qpc_frequency"),"clock calibration");Check(ev.ContainsKey("sequence"),"sequence");Check(!File.ReadAllText(events).Contains(root),"root redacted");Check(File.ReadAllText(events).Contains("0x"),"raw hresult");
  Parallel.For(0,100,i=>Diagnostics994.Record(root,"test.concurrent","info",new{i=i}));long previous=0;foreach(var line in File.ReadAllLines(events)){var row=j.DeserializeObject(line)as Dictionary<string,object>;long seq=Convert.ToInt64(row["sequence"]);Check(seq>previous,"concurrent sequence monotonic");previous=seq;}
  File.WriteAllText(Path.Combine(session,"launch.json"),j.Serialize(new{Revision="R994",Settings=new{Name="PRIVATE_NAME",Filter="PRIVATE_FILTER",ProcessingSize="native",OutputMode="follow"},Path=root}));
  File.WriteAllText(Path.Combine(archive,"app-commit700.json"),"{\"completed\":4,\"submitted\":3}");File.WriteAllText(Path.Combine(archive,"app-lifecycle988.json"),"{\"completed_cancelled_pause\":1}");File.WriteAllText(Path.Combine(archive,"app-engine.log"),"normal\nGPU device removed test\n");File.WriteAllText(Path.Combine(work,"app-engine.log"),"LIVE DIFFERENT LOG");
  File.WriteAllText(Path.Combine(archive,"app-flight994.decoded.jsonl"),"{\"type\":\"summary\",\"valid\":2}\nBROKEN\n{\"type\":\"open_begin\",\"stage\":\"Present\"}\n");
  File.AppendAllText(Path.Combine(archive,"app-flight994.decoded.jsonl"),"{\"type\":\"event\",\"action\":\"fail\",\"stage_name\":\"vk.submit\",\"domain\":\"vkresult\",\"result\":-4,\"result_hex\":\"0xFFFFFFFC\",\"session_sequence\":2}\n");
  foreach(string name in new[]{"app-device994.json","app-vulkan-queues994.json","app-d3d-device994.json","app-bridge-adapter994.json","app-model994.json"})File.WriteAllText(Path.Combine(archive,name),"{\"fixture\":true}");
  using(var f=File.Create(Path.Combine(archive,"app-phase971.csv")))f.SetLength(21L*1024*1024);
  using(var locked=new FileStream(Path.Combine(archive,"app-copy926.csv"),FileMode.Create,FileAccess.ReadWrite,FileShare.None)){
   File.WriteAllText(Path.Combine(archive,"app-test.png"),"PRIVATE_PIXELS");File.WriteAllText(Path.Combine(archive,"app-input.bin"),"PRIVATE_PIXELS");
   string zip=Diagnostics994.Export(root,session,archive,false);Check(File.Exists(zip),"export produced");string unpack=Path.Combine(root,"unpack");ZipFile.ExtractToDirectory(zip,unpack);
   Check(File.ReadAllText(Path.Combine(unpack,"app-engine.log")).Contains("device removed"),"offline uses archive");string launch=File.ReadAllText(Path.Combine(unpack,"launch.json"));Check(!launch.Contains("PRIVATE_NAME")&&!launch.Contains("PRIVATE_FILTER"),"profile identity redaction");Check(launch.Contains("native"),"effective config preserved");
   Check(!File.Exists(Path.Combine(unpack,"app-test.png"))&&!File.Exists(Path.Combine(unpack,"app-input.bin")),"image and unapproved binary excluded");
   var manifest=Object(Path.Combine(unpack,"bundle-manifest994.json"));var source=(object[])manifest["source_files"];Check(source.Cast<Dictionary<string,object>>().Any(x=>(string)x["status"]=="omitted_limit"),"size omission explicit");Check(source.Cast<Dictionary<string,object>>().Any(x=>(string)x["status"]=="failed"),"sharing failure explicit");
   foreach(var file in (object[])manifest["files"]){var row=(Dictionary<string,object>)file;Check(Hash(Path.Combine(unpack,(string)row["name"]))==(string)row["sha256"],"manifest matches final bytes");}
   var summary=Object(Path.Combine(unpack,"diagnosis-summary994.json"));Check(((object[])summary["missing_files"]).Length>0,"missing evidence listed");Check(((Dictionary<string,object>)summary["native_metadata"]).Count==5,"all device model and queue metadata exported and indexed");Check(((object[])summary["native_evidence"]).Length==3,"native summary open call malformed counted");Check(((object[])summary["native_failure_events"]).Length==1&&Convert.ToInt64(summary["native_failure_count"])==1,"native returned error indexed with session");Check(((object[])summary["error_log_candidates"]).Length==1,"error indexed with line");Check(((object[])summary["findings"]).Length==2,"lifecycle accounting indexed");
  }
  bool liveRejected=false;try{Diagnostics994.Export(root,session,archive,true);}catch(InvalidOperationException){liveRejected=true;}Check(liveRejected,"live export rejected; recorder unaffected");
  bool noArchiveRejected=false;try{Diagnostics994.Export(root,session,work,false);}catch(IOException){noArchiveRejected=true;}Check(noArchiveRejected,"no mutable work fallback");
  File.WriteAllText(Path.Combine(session,"launch.json"),"{PRIVATE_TRUNCATED_LAUNCH");string brokenZip=Diagnostics994.Export(root,session,archive,false);string brokenUnpack=Path.Combine(root,"broken-unpack");ZipFile.ExtractToDirectory(brokenZip,brokenUnpack);Check(!File.Exists(Path.Combine(brokenUnpack,"launch.json")),"failed redaction copy removed");Check(File.ReadAllText(Path.Combine(brokenUnpack,"bundle-manifest994.json")).Contains("failed"),"failed redaction recorded");
  File.WriteAllBytes(Path.Combine(work,"app-flight994.bin"),new byte[8]);Diagnostics994.DecodeNative(root,work);Check(File.Exists(Path.Combine(work,"decoder-failure994.json")),"decoder unavailable explicit");Check(File.Exists(Path.Combine(work,"app-flight994.bin")),"raw preserved");
  string occupied=Path.Combine(baseDir,"occupied-"+Guid.NewGuid().ToString("N"));File.WriteAllText(occupied,"not directory");Diagnostics994.Record(occupied,"test.logging_failure","error",null);Check(true,"logging failure did not throw");
  File.WriteAllText(Path.Combine(baseDir,"diagnostics994-test.json"),j.Serialize(new{pass=true,checks=checks,gpu_used=false,windows_created=false,fixtures=root}));Console.WriteLine("PASS "+checks+" diagnostic checks");return 0;
 }catch(Exception e){Console.Error.WriteLine(e);return 1;}}
}
