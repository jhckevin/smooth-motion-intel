using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using SmoothMotionDesktop;

class ProductModel988Tests
{
    static int checks, cases;
    static string root;
    static readonly DateTime Utc = DateTime.UtcNow;
    static void Check(bool result, string name) { checks++; if(!result) throw new Exception(name); }
    static void Case(Action test) { test(); cases++; }
    static void Write(string path, string text) { File.WriteAllText(path, text, new UTF8Encoding(false)); File.SetLastWriteTimeUtc(path, Utc); }
    static string Snapshot(string extra)
    {
        return "{\"type\":\"snapshot\",\"qpc\":100,\"display_valid\":true,\"display_hz\":60,\"original_hz\":30,\"generated_hz\":30,\"coverage\":1,\"warming_up\":false,\"stalled\":false,\"low1_valid\":true,\"low1_hz\":58,\"late_completed\":2,\"late_submitted\":1,\"capture_count\":200,\"generation_completed\":180,\"submitted\":370,\"observed_count\":360,\"model_timing_valid\":true,\"model_gpu_ms\":12,\"power_sample_age_s\":0.2,\"gpu_power\":{\"valid\":true,\"watts\":8.5,\"domain_enum\":2,\"fg_exclusive\":false},\"cpu_package_power\":{\"valid\":true,\"watts\":20,\"domain\":\"RAPL_Package0_PKG\",\"age_s\":0.1,\"sample_qpc\":99.9,\"fg_exclusive\":false}"+extra+"}";
    }
    static void Main()
    {
        root = Path.Combine(ProductStore.UserDataDirectory, "model988-tests-"+Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        string cfg = Path.Combine(root,"settings.json"), log=Path.Combine(root,"hud.jsonl"), warning;
        try
        {
            Case(delegate {
                var s = ProductStore.Load(cfg, out warning);
                Check(warning=="","fresh default warning"); Check(s.SchemaVersion==2&&s.Profiles.Count==1,"default schema");
                Check(s.Profiles[0].OpaqueContent&&s.Profiles[0].HudVisible&&!s.Profiles[0].CaptureCursor,"profile defaults");
                Check(s.CountdownSeconds==5&&s.Hotkey=="Ctrl+Alt+S"&&!s.CloseToTray&&!s.MinimizeToTray,"app defaults");
                s.Profiles[0].FrameMultiplier=4;s.Profiles[0].CaptureBackend="dxgi";s.Profiles[0].PresentSync=0;s.Profiles[0].PresentDepth=4;
                ProductStore.Save(cfg,s); var x=ProductStore.Load(cfg,out warning);
                Check(x.Profiles[0].FrameMultiplier==4&&x.Profiles[0].CaptureBackend=="dxgi"&&x.Profiles[0].PresentSync==0&&x.Profiles[0].PresentDepth==4,"new controls persist");
                Check(x.SelectedProfileId==s.SelectedProfileId&&warning=="","roundtrip");
            });
            Case(delegate {
                var s=ProductStore.Load(cfg,out warning); s.Profiles[0].Name="First";
                ProductStore.Save(cfg,s); s.Profiles[0].Name="Second"; ProductStore.Save(cfg,s);
                Check(File.Exists(cfg+".bak"),"backup exists");
                Write(cfg,"{"); var recovered=ProductStore.Load(cfg,out warning);
                Check(recovered.Profiles[0].Name=="First"&&warning.Contains("备份"),"corrupt primary fallback");
                recovered.Profiles[0].Name="Recovered"; ProductStore.Save(cfg,recovered);
                Check(ProductStore.Load(cfg,out warning).Profiles[0].Name=="Recovered","save recovered");
                Write(cfg,"bad again"); Check(ProductStore.Load(cfg,out warning).Profiles[0].Name=="First","good backup preserved");
                Write(cfg+".bak","[]"); Check(ProductStore.Load(cfg,out warning).Profiles[0].Name=="默认"&&warning.Contains("默认"),"both corrupt default");
            });
            Case(delegate {
                var s=new AppSettings(); s.SchemaVersion=99; bool bad=false; try{ProductStore.Save(cfg,s);}catch(SerializationException){bad=true;} Check(bad,"future schema rejected");
                s=new AppSettings(); s.Profiles.Add(s.Profiles[0]); bad=false;try{ProductStore.Validate(s);}catch{bad=true;}Check(bad,"duplicate id rejected");
                s=new AppSettings();s.Profiles[0].TimelineMs=201;bad=false;try{ProductStore.Validate(s);}catch{bad=true;}Check(bad,"timeline range");
                s=new AppSettings();s.Profiles[0].SessionSeconds=-1;bad=false;try{ProductStore.Validate(s);}catch{bad=true;}Check(bad,"session range");
                s=new AppSettings();s.CountdownSeconds=31;bad=false;try{ProductStore.Validate(s);}catch{bad=true;}Check(bad,"countdown range");
                s=new AppSettings();s.SelectedProfileId="missing";bad=false;try{ProductStore.Validate(s);}catch{bad=true;}Check(bad,"selected exists");
                s=new AppSettings();s.Hotkey="Alt+F4";bad=false;try{ProductStore.Validate(s);}catch{bad=true;}Check(bad,"unsupported hotkey");
            });
            Case(delegate {
                string outside=Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"forbidden988.json"); bool bad=false;
                try{ProductStore.Save(outside,new AppSettings());}catch(UnauthorizedAccessException){bad=true;}
                Check(bad&&!File.Exists(outside),"outside UserData rejected");
                var x=ProductStore.Load(outside,out warning);Check(x!=null&&warning.Contains("路径"),"bad path defaults warning");
                Write(cfg,"{\"SchemaVersion\":1,\"Profiles\":[{\"Id\":\"x\",\"Name\":\"Old\"}],\"SelectedProfileId\":\"x\"}");
                var defaults=ProductStore.Load(cfg,out warning);
                Check(defaults.SchemaVersion==2&&defaults.Profiles[0].SessionSeconds==0&&defaults.Profiles[0].FrameMultiplier==2&&defaults.Profiles[0].CaptureBackend=="wgc","legacy schema migration preserves continuous defaults");
                Check(defaults.Profiles[0].OutputMode=="follow","old profile gets aligned output default");
                Check(defaults.Profiles[0].OpaqueContent&&defaults.Profiles[0].TimelineMs==80&&defaults.CountdownSeconds==5,"missing optional fields safe defaults");
            });
            Case(delegate {var o=new AppSettings();o.Profiles[0].OutputMode="fullscreen";o.Profiles[0].ProcessingSize="fit1080";ProductStore.Save(cfg,o);var v=ProductStore.Load(cfg,out warning);Check(v.Profiles[0].OutputMode=="fullscreen"&&v.Profiles[0].ProcessingSize=="fit1080","independent input/output persisted");o.Profiles[0].CaptureBackend="dxgi";bool rejected=false;try{ProductStore.Validate(o);}catch(SerializationException){rejected=true;}Check(rejected,"unsafe independent desktop capture rejected");});
            Case(delegate {
                Write(log,Snapshot("")+"\n{\"type\":\"collector\",\"qpc\":100}\n{\"type\":\"snapshot\"");
                var v=TelemetryReader.ReadAt(log,100.5,Utc.AddSeconds(.5));
                Check(v.Available&&!v.Stale&&v.DisplayValid&&!v.IsLowerBound,"last complete snapshot not collector");
                Check(v.DisplayFps==60&&v.NativeFps==30&&v.GeneratedFps==30&&v.Low1Fps==58,"rates");
                Check(v.GpuPowerValid&&v.GpuPowerWatts==8.5&&v.GpuPowerDomain=="Package","gpu domain");
                Check(v.PackagePowerValid&&v.PackagePowerWatts==20&&v.PackagePowerDomain=="RAPL_Package0_PKG","independent package");
                Check(v.CountersValid&&v.LateCompleted==2&&v.LateSubmitted==1&&v.Captured==200&&v.Submitted==370,"counts");
                Check(Math.Abs(v.GpuPowerAgeSeconds-.7)<1e-9&&v.GpuMs==12,"age and GPU");
                Check(v.RawJson==Snapshot(""),"exact raw snapshot");
            });
            Case(delegate {
                Write(log,Snapshot("").Replace("\"display_valid\":true","\"display_valid\":false").Replace("\"coverage\":1","\"coverage\":0.84")+"\n");
                var v=TelemetryReader.ReadAt(log,100.2,Utc);
                Check(v.IsLowerBound&&!v.DisplayValid&&v.DisplayFps==60&&!v.Low1Valid,"partial observation lower bound");
                Write(log,Snapshot("").Replace("\"stalled\":false","\"stalled\":true")+"\n");
                v=TelemetryReader.ReadAt(log,100.2,Utc);Check(!v.DisplayValid&&!v.IsLowerBound&&Double.IsNaN(v.DisplayFps),"stall no fake rate");
                Write(log,"{\"type\":\"snapshot\",\"qpc\":100}\n");
                v=TelemetryReader.ReadAt(log,100.2,Utc);Check(v.Available&&!v.CountersValid&&!v.DisplayValid&&!v.GpuPowerValid&&Double.IsNaN(v.DisplayFps),"missing fields not zeros");
            });
            Case(delegate {
                Write(log,Snapshot("")+"\n");
                var v=TelemetryReader.ReadAt(log,103.01,Utc);Check(v.Stale&&!v.DisplayValid&&!v.GpuPowerValid&&!v.PackagePowerValid,"QPC stale");
                v=TelemetryReader.ReadAt(log,100.1,Utc.AddSeconds(3.01));Check(v.Stale,"file stale despite qpc");
                v=TelemetryReader.ReadAt(log,99,Utc);Check(v.Stale,"wrong clock domain");
                Write(log,Snapshot("").Replace("\"power_sample_age_s\":0.2","\"power_sample_age_s\":3.1")+"\n");
                v=TelemetryReader.ReadAt(log,100.1,Utc);Check(!v.GpuPowerValid&&v.PackagePowerValid,"power independently stale");
            });
            Case(delegate {
                Write(log,Snapshot("")+"\n{malformed}\n");
                var v=TelemetryReader.ReadAt(log,100.1,Utc);Check(v.Available&&v.Warning.Contains("跳过"),"invalid complete row warning");
                Write(log,Snapshot(""));v=TelemetryReader.ReadAt(log,100.1,Utc);Check(!v.Available,"unterminated only row ignored");
                Write(log,new string('x',300000)+"\n"+Snapshot("")+"\n{\"type\":\"collector\"}\n");
                v=TelemetryReader.ReadAt(log,100.1,Utc);Check(v.Available&&v.DisplayFps==60,"bounded tail skips split first row");
                Write(log,"{\"type\":\"snapshot\",\"qpc\":100,\"display_valid\":true,\"display_hz\":60,\"original_hz\":30,\"generated_hz\":30,\"coverage\":2,\"warming_up\":false,\"stalled\":false}\n");
                v=TelemetryReader.ReadAt(log,100.1,Utc);Check(!v.DisplayValid&&!v.IsLowerBound,"bad coverage rejected");
            });

            Case(delegate {
                Write(log,Snapshot("").Replace("\"watts\":8.5","\"watts\":null")+"\n");
                var v=TelemetryReader.ReadAt(log,100.1,Utc);
                Check(!v.GpuPowerValid&&v.PackagePowerValid,"null watts not zero");
                Write(log,Snapshot("").Replace("\"domain_enum\":2","\"domain_enum\":0")+"\n");
                v=TelemetryReader.ReadAt(log,100.1,Utc);Check(!v.GpuPowerValid&&v.GpuPowerDomain=="?","unknown domain invalid");
                Write(log,Snapshot("").Replace("\"age_s\":0.1","\"age_s\":3.1")+"\n");
                v=TelemetryReader.ReadAt(log,100.1,Utc);Check(v.GpuPowerValid&&!v.PackagePowerValid,"package independently stale");
                Write(log,Snapshot("")+"\n");
                using(var writer=new FileStream(log,FileMode.Open,FileAccess.ReadWrite,FileShare.ReadWrite|FileShare.Delete))
                { v=TelemetryReader.ReadAt(log,100.1,Utc);Check(v.DisplayValid,"shared live-writer read"); }
                Check(!v.PowerIsFgExclusive,"power explicitly nonexclusive");
            });

            Case(delegate {
                string fixture=Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory,@"..\candidate987\hud-monitor-cpu987.jsonl"));
                var actual=TelemetryReader.Read(fixture);
                Check(actual.Available&&actual.RawJson.Contains("LevelZero.Sysman.zesPowerGetEnergyCounter"),"actual R987 emitter fixture parsed");
                Check(!actual.GpuPowerValid&&!actual.PackagePowerValid,"actual unavailable readings remain invalid");
            });
            Case(delegate {
                Check(WindowCatalog.ValidDimensions(1920,1080)&&WindowCatalog.ValidDimensions(2560,1440),"supported sizes");
                Check(WindowCatalog.ValidDimensions(3840,2160)&&WindowCatalog.ValidDimensions(2191,1233)&&WindowCatalog.ValidDimensions(7680,2160)&&!WindowCatalog.ValidDimensions(16385,1)&&!WindowCatalog.ValidDimensions(0,900)&&!WindowCatalog.ValidDimensions(Int32.MaxValue,Int32.MaxValue),"bounded geometry");
                Check(!WindowCatalog.IsCurrent(null)&&!WindowCatalog.IsCurrent(new WindowTarget{Handle=new IntPtr(1234),IdentityKnown=true,ProcessCreationTimeUtcFileTime=1}),"invalid/reused identity fails closed");
                var list=WindowCatalog.List(System.Diagnostics.Process.GetCurrentProcess().Id);
                Check(!list.Any(x=>x.ProcessId==System.Diagnostics.Process.GetCurrentProcess().Id),"own process excluded");
            });
            Console.WriteLine("{\"pass\":true,\"cases\":"+cases+",\"checks\":"+checks+",\"gpu_used\":false,\"windows_created\":false,\"registry_modified\":false}");
        }
        finally { Directory.Delete(root,true); }
    }
}
