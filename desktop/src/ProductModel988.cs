// R988 desktop data model. CPU/read-only Win32 only; no capture, GPU commands or system changes.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Text.RegularExpressions;

namespace SmoothMotionDesktop
{
    [DataContract]
    public sealed class Profile
    {
        [DataMember(IsRequired = true)] public string Id { get; set; }
        [DataMember(IsRequired = true)] public string Name { get; set; }
        [DataMember] public string Filter { get; set; }
        [DataMember] public bool CaptureCursor { get; set; }
        [DataMember] public bool HudVisible { get; set; }
        [DataMember] public bool HudDetails { get; set; }
        [DataMember] public bool OpaqueContent { get; set; }
        [DataMember] public int TimelineMs { get; set; }
        [DataMember] public int SessionSeconds { get; set; }
        [DataMember] public bool AutoSelect { get; set; }
        [DataMember] public int FrameMultiplier { get; set; }
        [DataMember] public string CaptureBackend { get; set; }
        [DataMember] public string ProcessingSize { get; set; }
        [DataMember] public string OutputMode { get; set; }
        [DataMember] public int PresentSync { get; set; }
        [DataMember] public int PresentDepth { get; set; }
        public Profile() { Defaults(); }
        void Defaults()
        {
            Id = Guid.NewGuid().ToString("N"); Name = "默认"; Filter = "";
            CaptureCursor = false; HudVisible = true; HudDetails = false;
            OpaqueContent = true; TimelineMs = 80; SessionSeconds = 0; AutoSelect = false;
            FrameMultiplier = 2; CaptureBackend = "wgc"; ProcessingSize = "native"; OutputMode = "follow"; PresentSync = 1; PresentDepth = 3;
        }
        [OnDeserializing] void BeforeRead(StreamingContext unused) { Defaults(); }
    }

    [DataContract]
    public sealed class AppSettings
    {
        [DataMember(IsRequired = true)] public int SchemaVersion { get; set; }
        [DataMember(IsRequired = true)] public List<Profile> Profiles { get; set; }
        [DataMember(IsRequired = true)] public string SelectedProfileId { get; set; }
        [DataMember] public bool MinimizeToTray { get; set; }
        [DataMember] public bool CloseToTray { get; set; }
        [DataMember] public bool StartMinimized { get; set; }
        [DataMember] public int CountdownSeconds { get; set; }
        [DataMember] public string Hotkey { get; set; }
        public AppSettings() { Defaults(); }
        void Defaults()
        {
            SchemaVersion = 2; Profiles = new List<Profile> { new Profile() };
            SelectedProfileId = Profiles[0].Id; CountdownSeconds = 5; Hotkey = "Ctrl+Alt+S";
            MinimizeToTray = CloseToTray = StartMinimized = false;
        }
        [OnDeserializing] void BeforeRead(StreamingContext unused) { Defaults(); }
    }

    public static class ProductStore
    {
        static readonly object Gate = new object();
        public static string UserDataDirectory { get { return Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "UserData"); } }
        public static string SettingsPath { get { return Path.Combine(UserDataDirectory, "settings.json"); } }

        public static void Validate(AppSettings value)
        {
            if (value == null || value.SchemaVersion != 2) throw new SerializationException("不支持的配置版本");
            if (value.Profiles == null || value.Profiles.Count < 1 || value.Profiles.Count > 50)
                throw new SerializationException("配置方案数量须为 1–50");
            var ids = new HashSet<string>(StringComparer.Ordinal);
            foreach (Profile p in value.Profiles)
            {
                if (p == null || String.IsNullOrWhiteSpace(p.Id) || p.Id.Length > 80 || !ids.Add(p.Id))
                    throw new SerializationException("方案 ID 为空、过长或重复");
                if (String.IsNullOrWhiteSpace(p.Name) || p.Name.Length > 80 || p.Name.Any(Char.IsControl))
                    throw new SerializationException("方案名称无效");
                if (p.Filter == null || p.Filter.Length > 512 || p.Filter.Any(Char.IsControl))
                    throw new SerializationException("窗口匹配条件无效");
                if (p.TimelineMs < 10 || p.TimelineMs > 200) throw new SerializationException("时间线须为 10–200 ms");
                if (p.SessionSeconds != 0 && (p.SessionSeconds < 10 || p.SessionSeconds > 14400)) throw new SerializationException("诊断时限须为 10–14400 秒，0 表示持续运行");
                if (p.FrameMultiplier != 2 && p.FrameMultiplier != 4 && p.FrameMultiplier != 8) throw new SerializationException("帧生成倍率无效");
                if (p.ProcessingSize != "native" && p.ProcessingSize != "fit1080" && p.ProcessingSize != "fit1440") throw new SerializationException("处理尺寸选项无效");
                if (!new[]{"follow","fullscreen","window1080","processing","small"}.Contains(p.OutputMode)) throw new SerializationException("输出方式无效");
                if (p.OutputMode != "follow" && p.CaptureBackend != "wgc") throw new SerializationException("独立输出预览须使用 WGC 窗口捕获");
                if (p.CaptureBackend != "wgc" && p.CaptureBackend != "dxgi") throw new SerializationException("捕获 API 无效");
                if (p.PresentSync != 0 && p.PresentSync != 1) throw new SerializationException("同步模式无效");
                if (p.PresentDepth < 2 || p.PresentDepth > 4) throw new SerializationException("显示队列须为 2–4");
            }
            if (value.SelectedProfileId == null || !ids.Contains(value.SelectedProfileId))
                throw new SerializationException("选中的方案不存在");
            if (value.CountdownSeconds < 0 || value.CountdownSeconds > 30)
                throw new SerializationException("倒计时须为 0–30 秒");
            // The desktop host owns registration and must report registration failure.
            if (value.Hotkey == null || !Regex.IsMatch(value.Hotkey, @"^Ctrl\+Alt\+(?:[A-Z]|F(?:[1-9]|1[0-2]))$"))
                throw new SerializationException("快捷键须为 Ctrl+Alt+字母或 F1–F12");
        }

        static string SafePath(string path, bool create)
        {
            if (String.IsNullOrWhiteSpace(path)) throw new ArgumentException("配置路径为空");
            string root = Path.GetFullPath(UserDataDirectory).TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar;
            string full = Path.GetFullPath(path);
            if (!full.StartsWith(root, StringComparison.OrdinalIgnoreCase)) throw new UnauthorizedAccessException("配置仅可位于程序工作区 UserData");
            string directory = Path.GetDirectoryName(full);
            // Refuse existing junctions/symlinks instead of silently writing outside the app workspace.
            var current = new DirectoryInfo(directory);
            while (current != null)
            {
                if (current.Exists && (current.Attributes & FileAttributes.ReparsePoint) != 0)
                    throw new UnauthorizedAccessException("UserData 路径包含重解析目录");
                current = current.Parent;
            }
            if (File.Exists(full) && (File.GetAttributes(full) & FileAttributes.ReparsePoint) != 0)
                throw new UnauthorizedAccessException("配置文件不得为重解析点");
            if (create) Directory.CreateDirectory(directory);
            return full;
        }

        static AppSettings ReadValid(string path)
        {
            using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                if (stream.Length < 2 || stream.Length > 1048576) throw new SerializationException("配置文件大小无效");
                var serializer = new DataContractJsonSerializer(typeof(AppSettings));
                AppSettings value = serializer.ReadObject(stream) as AppSettings;
                if (value != null && value.SchemaVersion == 1)
                {
                    value.SchemaVersion = 2;
                    if (value.Profiles != null) foreach (Profile profile in value.Profiles) if (profile != null) profile.SessionSeconds = 0;
                }
                Validate(value);
                return value;
            }
        }

        public static AppSettings Load(string path, out string warning)
        {
            lock (Gate)
            {
                warning = "";
                string full;
                try { full = SafePath(path, false); }
                catch (Exception e) { warning = "配置路径不可用，已使用安全默认值：" + e.Message; return new AppSettings(); }
                bool mainExists = File.Exists(full), backupExists = File.Exists(full + ".bak");
                if (mainExists)
                {
                    try { return ReadValid(full); }
                    catch (Exception e) { warning = "主配置无效：" + e.Message; }
                }
                if (backupExists)
                {
                    try
                    {
                        SafePath(full + ".bak", false);
                        var recovered = ReadValid(full + ".bak");
                        warning = (warning.Length == 0 ? "主配置缺失。" : warning + "。") + "已读取备份；原文件未覆盖。";
                        return recovered;
                    }
                    catch (Exception e) { warning += " 备份无效：" + e.Message; }
                }
                if (mainExists || backupExists) warning += " 已使用安全默认值，未改写损坏文件。";
                return new AppSettings();
            }
        }

        public static void Save(string path, AppSettings settings)
        {
            lock (Gate)
            {
                Validate(settings);
                string full = SafePath(path, true);
                string backup = SafePath(full + ".bak", false);
                string temporary = full + "." + Guid.NewGuid().ToString("N") + ".tmp";
                try
                {
                    using (var stream = new FileStream(temporary, FileMode.CreateNew, FileAccess.Write, FileShare.None, 4096, FileOptions.WriteThrough))
                    {
                        new DataContractJsonSerializer(typeof(AppSettings)).WriteObject(stream, settings);
                        stream.Flush(true);
                    }
                    // Check the exact serialized object before committing.
                    ReadValid(temporary);
                    if (File.Exists(full))
                    {
                        bool validOld = false;
                        try { ReadValid(full); validOld = true; } catch { }
                        // Never overwrite a good backup with an already corrupt primary.
                        File.Replace(temporary, full, validOld ? backup : null, true);
                    }
                    else File.Move(temporary, full);
                }
                finally { if (File.Exists(temporary)) File.Delete(temporary); }
            }
        }
    }

    public sealed class WindowTarget
    {
        public IntPtr Handle { get; set; }
        public int ProcessId { get; set; }
        public long ProcessCreationTimeUtcFileTime { get; set; }
        public string Title { get; set; }
        public string ExecutablePath { get; set; }
        public int ClientWidth { get; set; }
        public int ClientHeight { get; set; }
        public bool IdentityKnown { get; set; }
        public bool IsVisible { get; set; }
        public bool IsMinimized { get; set; }
        public bool IsSupported { get; set; }
        public string UnsupportedReason { get; set; }
        public string DisplayName
        {
            get { return (Title ?? "") + "  ·  " + ClientWidth + "×" + ClientHeight + (IsSupported ? "" : "  ·  " + UnsupportedReason); }
        }
        public override string ToString() { return DisplayName; }
    }

    public static class WindowCatalog
    {
        const uint QueryLimitedInformation = 0x1000;
        delegate bool EnumProc(IntPtr window, IntPtr argument);
        [StructLayout(LayoutKind.Sequential)] struct Rect { public int Left, Top, Right, Bottom; }
        [StructLayout(LayoutKind.Sequential)] struct NativeFileTime { public uint Low, High; public long Value { get { return ((long)High << 32) | Low; } } }
        [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc callback, IntPtr argument);
        [DllImport("user32.dll")] static extern bool IsWindow(IntPtr window);
        [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr window);
        [DllImport("user32.dll")] static extern bool IsIconic(IntPtr window);
        [DllImport("user32.dll")] static extern IntPtr GetWindow(IntPtr window, uint command);
        [DllImport("user32.dll", CharSet = CharSet.Unicode)] static extern int GetWindowText(IntPtr window, StringBuilder text, int count);
        [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr window, out uint processId);
        [DllImport("user32.dll")] static extern bool GetClientRect(IntPtr window, out Rect rectangle);
        [DllImport("kernel32.dll", SetLastError = true)] static extern IntPtr OpenProcess(uint access, bool inherit, uint processId);
        [DllImport("kernel32.dll")] static extern bool GetProcessTimes(IntPtr process, out NativeFileTime creation, out NativeFileTime exit, out NativeFileTime kernel, out NativeFileTime user);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] static extern bool QueryFullProcessImageName(IntPtr process, uint flags, StringBuilder name, ref int size);
        [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);

        static WindowTarget Inspect(IntPtr handle)
        {
            if (handle == IntPtr.Zero || !IsWindow(handle)) return null;
            uint pid; if (GetWindowThreadProcessId(handle, out pid) == 0 || pid == 0 || pid > Int32.MaxValue) return null;
            var title = new StringBuilder(1024); GetWindowText(handle, title, title.Capacity);
            var target = new WindowTarget { Handle = handle, ProcessId = (int)pid, Title = title.ToString(), ExecutablePath = "", IsVisible = IsWindowVisible(handle), IsMinimized = IsIconic(handle), UnsupportedReason = "" };
            Rect rectangle;
            if (GetClientRect(handle, out rectangle)) { target.ClientWidth = rectangle.Right - rectangle.Left; target.ClientHeight = rectangle.Bottom - rectangle.Top; }
            IntPtr process = OpenProcess(QueryLimitedInformation, false, pid);
            if (process != IntPtr.Zero)
            {
                try
                {
                    NativeFileTime creation, exit, kernel, user;
                    if (GetProcessTimes(process, out creation, out exit, out kernel, out user) && creation.Value > 0)
                    { target.IdentityKnown = true; target.ProcessCreationTimeUtcFileTime = creation.Value; }
                    var path = new StringBuilder(32768); int size = path.Capacity;
                    if (QueryFullProcessImageName(process, 0, path, ref size)) target.ExecutablePath = path.ToString();
                }
                finally { CloseHandle(process); }
            }
            uint afterPid; GetWindowThreadProcessId(handle, out afterPid);
            if (!IsWindow(handle) || afterPid != pid) { target.IdentityKnown = false; target.UnsupportedReason = "窗口已变化，请刷新"; }
            else if (!target.IdentityKnown) target.UnsupportedReason = "无法核实进程身份";
            else if (!target.IsVisible) target.UnsupportedReason = "窗口不可见";
            else if (target.IsMinimized) target.UnsupportedReason = "窗口已最小化";
            else if (!ValidDimensions(target.ClientWidth, target.ClientHeight)) target.UnsupportedReason = "客户区尺寸无效或超过 D3D11 纹理维度；启动时另检查设备与显存预算";
            target.IsSupported = target.UnsupportedReason.Length == 0;
            return target;
        }

        public static bool ValidDimensions(int width, int height)
        { return width > 0 && height > 0 && width <= 16384 && height <= 16384; }

        public static List<WindowTarget> List(int excludedPid)
        {
            var result = new List<WindowTarget>();
            EnumProc callback = delegate(IntPtr h, IntPtr unused)
            {
                try
                {
                    if (!IsWindowVisible(h) || GetWindow(h, 4) != IntPtr.Zero) return true;
                    WindowTarget item = Inspect(h);
                    if (item != null && item.ProcessId != excludedPid && !String.IsNullOrWhiteSpace(item.Title)) result.Add(item);
                }
                catch { /* A dying or access-denied process must not abort enumeration. */ }
                return true;
            };
            EnumWindows(callback, IntPtr.Zero);
            return result.OrderBy(x => x.Title, StringComparer.CurrentCultureIgnoreCase).ThenBy(x => x.ProcessId).ToList();
        }

        public static bool IsCurrent(WindowTarget target)
        {
            if (target == null || !target.IdentityKnown || target.Handle == IntPtr.Zero || target.ProcessCreationTimeUtcFileTime <= 0) return false;
            try
            {
                WindowTarget current = Inspect(target.Handle);
                return current != null && current.IsSupported && current.ProcessId == target.ProcessId
                    && current.ProcessCreationTimeUtcFileTime == target.ProcessCreationTimeUtcFileTime;
            }
            catch { return false; }
        }
    }

    // Picker policy is independent of capture eligibility and never starts capture.
    public static class TargetPicker991
    {
        public static bool MatchesFilter(string filter, WindowTarget target)
        {
            if (target == null || String.IsNullOrWhiteSpace(filter)) return false;
            foreach (string piece in filter.Split(';'))
            {
                if (String.IsNullOrWhiteSpace(piece)) continue;
                string pattern = "^" + Regex.Escape(piece.Trim()).Replace("\\*", ".*").Replace("\\?", ".") + "$";
                foreach (string value in new[] { target.Title, target.ExecutablePath, Path.GetFileName(target.ExecutablePath ?? "") })
                    if (Regex.IsMatch(value ?? "", pattern, RegexOptions.IgnoreCase, TimeSpan.FromMilliseconds(100))) return true;
            }
            return false;
        }
        public static bool SameIdentity(WindowTarget a, WindowTarget b)
        {
            return a != null && b != null && a.IdentityKnown && b.IdentityKnown && a.Handle == b.Handle
                && a.ProcessId == b.ProcessId && a.ProcessCreationTimeUtcFileTime == b.ProcessCreationTimeUtcFileTime;
        }
        public static List<WindowTarget> Order(IEnumerable<WindowTarget> items, string filter)
        {
            return items.OrderByDescending(x => MatchesFilter(filter, x)).ThenByDescending(x => x.IsSupported)
                .ThenBy(x => x.Title, StringComparer.CurrentCultureIgnoreCase).ThenBy(x => x.ProcessId).ToList();
        }
        public static WindowTarget Choose(List<WindowTarget> items, WindowTarget previous, bool explicitChoice, string filter)
        {
            if (previous != null) return items.FirstOrDefault(x => SameIdentity(x, previous));
            if (explicitChoice) return null; // Keep a user's foreground-window choice across refreshes.
            var matches = items.Where(x => x.IsSupported && MatchesFilter(filter, x)).Take(2).ToList();
            return matches.Count == 1 ? matches[0] : null;
        }
    }

    public sealed class TelemetrySnapshot
    {
        public bool Available { get; set; }
        public bool Stale { get; set; }
        public bool DisplayValid { get; set; }
        public bool IsLowerBound { get; set; }
        public bool WarmingUp { get; set; }
        public bool Stalled { get; set; }
        public bool Low1Valid { get; set; }
        public bool GpuPowerValid { get; set; }
        public bool PackagePowerValid { get; set; }
        public bool ModelTimingValid { get; set; }
        public bool CountersValid { get; set; }
        public double AgeSeconds { get; set; }
        public double DisplayFps { get; set; }
        public double NativeFps { get; set; }
        public double GeneratedFps { get; set; }
        public double Coverage { get; set; }
        public double Low1Fps { get; set; }
        public double GpuPowerWatts { get; set; }
        public double PackagePowerWatts { get; set; }
        public double GpuPowerAgeSeconds { get; set; }
        public double PackagePowerAgeSeconds { get; set; }
        public double GpuMs { get; set; }
        public ulong LateCompleted { get; set; }
        public ulong LateSubmitted { get; set; }
        public ulong Captured { get; set; }
        public ulong GeneratedCompleted { get; set; }
        public ulong Submitted { get; set; }
        public ulong Observed { get; set; }
        public string GpuPowerDomain { get; set; }
        public bool PowerIsFgExclusive { get { return false; } }
        public string PackagePowerDomain { get; set; }
        public string RawJson { get; set; }
        public string Warning { get; set; }
        public TelemetrySnapshot()
        {
            Stale = true; AgeSeconds = Double.PositiveInfinity;
            DisplayFps = NativeFps = GeneratedFps = Coverage = Low1Fps = GpuPowerWatts = PackagePowerWatts = GpuPowerAgeSeconds = PackagePowerAgeSeconds = GpuMs = Double.NaN;
            GpuPowerDomain = "?"; PackagePowerDomain = "RAPL_Package0_PKG"; RawJson = Warning = "";
        }
    }

    // Nullable fields prevent absent values from becoming apparently valid zero readings.
    [DataContract] sealed class PowerJson988
    {
        [DataMember(Name = "valid")] public bool? Valid { get; set; }
        [DataMember(Name = "watts")] public double? Watts { get; set; }
        [DataMember(Name = "domain_enum")] public uint? DomainEnum { get; set; }
        [DataMember(Name = "domain")] public string Domain { get; set; }
        [DataMember(Name = "age_s")] public double? Age { get; set; }
        [DataMember(Name = "sample_qpc")] public double? SampleQpc { get; set; }
        [DataMember(Name = "fg_exclusive")] public bool? FgExclusive { get; set; }
    }
    [DataContract] sealed class HudJson988
    {
        [DataMember(Name = "type")] public string Type { get; set; }
        [DataMember(Name = "qpc")] public double? Qpc { get; set; }
        [DataMember(Name = "display_valid")] public bool? DisplayValid { get; set; }
        [DataMember(Name = "display_hz")] public double? Display { get; set; }
        [DataMember(Name = "original_hz")] public double? Original { get; set; }
        [DataMember(Name = "generated_hz")] public double? Generated { get; set; }
        [DataMember(Name = "coverage")] public double? Coverage { get; set; }
        [DataMember(Name = "warming_up")] public bool? Warming { get; set; }
        [DataMember(Name = "stalled")] public bool? Stalled { get; set; }
        [DataMember(Name = "low1_valid")] public bool? LowValid { get; set; }
        [DataMember(Name = "low1_hz")] public double? Low { get; set; }
        [DataMember(Name = "late_completed")] public ulong? LateCompleted { get; set; }
        [DataMember(Name = "late_submitted")] public ulong? LateSubmitted { get; set; }
        [DataMember(Name = "model_timing_valid")] public bool? ModelValid { get; set; }
        [DataMember(Name = "model_gpu_ms")] public double? ModelMs { get; set; }
        [DataMember(Name = "capture_count")] public ulong? Captured { get; set; }
        [DataMember(Name = "generation_completed")] public ulong? Completed { get; set; }
        [DataMember(Name = "submitted")] public ulong? Submitted { get; set; }
        [DataMember(Name = "observed_count")] public ulong? Observed { get; set; }
        [DataMember(Name = "power_sample_age_s")] public double? GpuAge { get; set; }
        [DataMember(Name = "gpu_power")] public PowerJson988 GpuPower { get; set; }
        [DataMember(Name = "cpu_package_power")] public PowerJson988 PackagePower { get; set; }
    }

    public static class TelemetryReader
    {
        const int TailLimit = 262144; // bounded allocation and I/O, no whole-session reread
        static bool Finite(double v) { return !Double.IsNaN(v) && !Double.IsInfinity(v); }
        static bool Nonnegative(double? v) { return v.HasValue && Finite(v.Value) && v.Value >= 0; }
        public static TelemetrySnapshot Read(string path)
        { return ReadAt(path, Stopwatch.GetTimestamp() / (double)Stopwatch.Frequency, DateTime.UtcNow); }

        // Deterministic CPU tests can supply the same clock domain as the engine's QPC.
        internal static TelemetrySnapshot ReadAt(string path, double nowQpc, DateTime nowUtc)
        {
            var unavailable = new TelemetrySnapshot();
            try
            {
                byte[] bytes;
                bool startsMidFile;
                DateTime modified;
                using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete))
                {
                    long length = stream.Length; int count = (int)Math.Min(length, TailLimit); startsMidFile = length > count;
                    stream.Seek(length - count, SeekOrigin.Begin); bytes = new byte[count];
                    int read = 0; while (read < count) { int n = stream.Read(bytes, read, count - read); if (n == 0) break; read += n; }
                    if (read != count) Array.Resize(ref bytes, read);
                    modified = File.GetLastWriteTimeUtc(path);
                }
                // Decode only whole lines. The last partial line may be concurrently written.
                string text = Encoding.UTF8.GetString(bytes); int end = text.LastIndexOf('\n');
                if (end < 0) { unavailable.Warning = "尚无完整监测记录"; return unavailable; }
                text = text.Substring(0, end);
                if (startsMidFile) { int first = text.IndexOf('\n'); if (first < 0) { unavailable.Warning = "监测记录超过读取上限"; return unavailable; } text = text.Substring(first + 1); }
                string[] lines = text.Split('\n'); bool damaged = false;
                for (int i = lines.Length - 1; i >= 0; --i)
                {
                    string line = lines[i].Trim().TrimStart('\uFEFF'); if (line.Length == 0) continue;
                    HudJson988 record;
                    try
                    {
                        using (var buffer = new MemoryStream(Encoding.UTF8.GetBytes(line)))
                            record = (HudJson988)new DataContractJsonSerializer(typeof(HudJson988)).ReadObject(buffer);
                    }
                    catch { damaged = true; continue; }
                    if (record == null || record.Type != "snapshot") continue;
                    if (!Nonnegative(record.Qpc)) { damaged = true; continue; }
                    var value = Convert(record, line, nowQpc, nowUtc, modified);
                    if (damaged) value.Warning = "跳过无效日志行；" + value.Warning;
                    return value;
                }
                unavailable.Warning = damaged ? "没有可解析的完整快照" : "等待监测快照";
            }
            catch (Exception e) { unavailable.Warning = "监测暂不可用：" + e.Message; }
            return unavailable;
        }

        static TelemetrySnapshot Convert(HudJson988 r, string raw, double nowQpc, DateTime nowUtc, DateTime modified)
        {
            double elapsed = nowQpc - r.Qpc.Value, fileAge = (nowUtc - modified).TotalSeconds;
            var v = new TelemetrySnapshot { Available = true, RawJson = raw };
            bool clockInvalid = !Finite(elapsed) || elapsed < -0.05 || fileAge < -1;
            v.AgeSeconds = clockInvalid ? Double.PositiveInfinity : Math.Max(0, Math.Max(elapsed, fileAge));
            v.Stale = clockInvalid || v.AgeSeconds > 3;
            v.WarmingUp = r.Warming != false; v.Stalled = r.Stalled != false;
            v.Coverage = Nonnegative(r.Coverage) && r.Coverage.Value <= 1 ? r.Coverage.Value : Double.NaN;
            bool rates = Nonnegative(r.Display) && Nonnegative(r.Original) && Nonnegative(r.Generated);
            bool active = !v.Stale && !v.WarmingUp && !v.Stalled && rates && v.Coverage > 0;
            v.DisplayValid = active && r.DisplayValid == true && v.Coverage >= 1;
            v.IsLowerBound = active && !v.DisplayValid;
            if (active) { v.DisplayFps = r.Display.Value; v.NativeFps = r.Original.Value; v.GeneratedFps = r.Generated.Value; }
            v.Low1Valid = v.DisplayValid && r.LowValid == true && Nonnegative(r.Low);
            if (v.Low1Valid) v.Low1Fps = r.Low.Value;
            v.ModelTimingValid = !v.Stale && r.ModelValid == true && Nonnegative(r.ModelMs);
            if (v.ModelTimingValid) v.GpuMs = r.ModelMs.Value;
            v.CountersValid = !v.Stale && r.LateCompleted.HasValue && r.LateSubmitted.HasValue && r.Captured.HasValue && r.Completed.HasValue && r.Submitted.HasValue && r.Observed.HasValue;
            v.LateCompleted = r.LateCompleted ?? 0; v.LateSubmitted = r.LateSubmitted ?? 0;
            v.Captured = r.Captured ?? 0; v.GeneratedCompleted = r.Completed ?? 0; v.Submitted = r.Submitted ?? 0; v.Observed = r.Observed ?? 0;
            if (r.GpuPower != null)
            {
                v.GpuPowerDomain = DomainName(r.GpuPower.DomainEnum);
                v.GpuPowerAgeSeconds = Nonnegative(r.GpuAge) ? r.GpuAge.Value + Math.Max(0, elapsed) : Double.NaN;
                v.GpuPowerValid = !v.Stale && r.GpuPower.Valid == true && Nonnegative(r.GpuPower.Watts) && v.GpuPowerAgeSeconds >= 0 && v.GpuPowerAgeSeconds <= 3 && r.GpuPower.DomainEnum.HasValue && r.GpuPower.DomainEnum.Value >= 1 && r.GpuPower.DomainEnum.Value <= 5;
                if (v.GpuPowerValid) v.GpuPowerWatts = r.GpuPower.Watts.Value;
            }
            if (r.PackagePower != null)
            {
                v.PackagePowerDomain = r.PackagePower.Domain ?? "?";
                double age = Nonnegative(r.PackagePower.Age) ? r.PackagePower.Age.Value + Math.Max(0, elapsed) : Double.NaN;
                if (Nonnegative(r.PackagePower.SampleQpc)) age = Math.Max(age, nowQpc - r.PackagePower.SampleQpc.Value);
                v.PackagePowerAgeSeconds = age;
                v.PackagePowerValid = !v.Stale && r.PackagePower.Valid == true && Nonnegative(r.PackagePower.Watts) && age >= 0 && age <= 3 && v.PackagePowerDomain == "RAPL_Package0_PKG";
                if (v.PackagePowerValid) v.PackagePowerWatts = r.PackagePower.Watts.Value;
            }
            if (v.Stale) v.Warning = "监测数据已过期";
            else if (v.WarmingUp) v.Warning = "监测预热中";
            else if (v.Stalled) v.Warning = "等待显示事件";
            else if (v.IsLowerBound) v.Warning = "显示采样不完整，FPS 为已确认下限";
            return v;
        }
        static string DomainName(uint? domain)
        {
            if (!domain.HasValue) return "?";
            switch (domain.Value) { case 1: return "Card"; case 2: return "Package"; case 3: return "Stack"; case 4: return "Memory"; case 5: return "GPU"; default: return "?"; }
        }
    }
}
