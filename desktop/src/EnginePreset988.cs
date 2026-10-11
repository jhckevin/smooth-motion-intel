using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;

namespace SmoothMotionDesktop
{
    // R987 measured opt-in composition/recovery preset, now bound to R988 hosted names.
    // Caller MUST remove every inherited SMX_* variable from child ProcessStartInfo first.
    // This dictionary does not mutate the current process, registry, power or display settings.
    public static class EnginePreset988
    {
        public static Dictionary<string,string> Create(string workDir, string cacheDir, string shaderDir, string sessionDir, Profile profile)
        {
            if (profile == null) throw new ArgumentNullException("profile");
            if (profile.TimelineMs < 10 || profile.TimelineMs > 200 || (profile.SessionSeconds != 0 && (profile.SessionSeconds < 10 || profile.SessionSeconds > 14400)))
                throw new ArgumentException("Profile timing exceeds supported engine limits");
            if (profile.FrameMultiplier != 2 && profile.FrameMultiplier != 4 && profile.FrameMultiplier != 8) throw new ArgumentException("Unsupported multiplier");
            if (profile.ProcessingSize != "native" && profile.ProcessingSize != "fit1080" && profile.ProcessingSize != "fit1440") throw new ArgumentException("Unsupported processing size");
            if (Array.IndexOf(new[]{"follow","fullscreen","window1080","processing","small"},profile.OutputMode)<0) throw new ArgumentException("Unsupported output mode");
            if (profile.OutputMode != "follow" && profile.CaptureBackend != "wgc") throw new ArgumentException("Independent output preview requires WGC");
            if (profile.CaptureBackend != "wgc" && profile.CaptureBackend != "dxgi") throw new ArgumentException("Unsupported capture backend");
            if (profile.PresentSync < 0 || profile.PresentSync > 1 || profile.PresentDepth < 2 || profile.PresentDepth > 4) throw new ArgumentException("Unsupported presentation settings");
            string work = Absolute(workDir, "workDir"), cache = Absolute(cacheDir, "cacheDir"), shaders = Absolute(shaderDir, "shaderDir");
            Absolute(sessionDir, "sessionDir"); // archive destination belongs to host; live logs stay with the locked model work dir
            var env = new Dictionary<string,string>(StringComparer.OrdinalIgnoreCase);
            env["SMX_HOST_WORK988"] = work;
            env["SMX_MODEL_DIR"] = work;
            env["SMX_BENCH_DIR"] = work;
            env["SMX_MANIFEST"] = "graph922.bin";
            env["SMX_NATIVE_SHADERS969"] = shaders;
            env["SMX_PIPELINE_CACHE_DIR"] = cache;
            env["SMX_PIPELINE905"] = "1";
            env["SMX_PRIVATE_GRAPH"] = "1";
            env["SMX_RETAIN_ENGINE"] = "1";
            env["SMX_ALLOW_NUMERIC_DIFF"] = "1";
            env["SMX_PRIME_RESOURCES"] = "1";
            env["SMX_FUSE842"] = "1";
            env["SMX_READY_INPUT809"] = "0";
            env["SMX_PHASE_LOCK971"] = "1";
            env["SMX_CALIBRATED_TIMING"] = "1";
            // Native chooses eligible queue families on the selected adapter; no B390 queue indices.
            env["SMX_SPLIT_DISPLAY"] = "1";
            env["SMX_DISPLAY_BOOST988"] = "1";
            env["SMX_ORIGINAL_RECOVERY988"] = "1";
            env["SMX_RECOVERY_OUTPUT_HZ988"] = "60";
            env["SMX_RECOVERY_PROGRESS988"] = "1";
            env["SMX_SUSTAINED_RECOVERY988"] = "1";
            env["SMX_CLAIMED_ORIGINAL_RECOVERY988"] = "1";
            env["SMX_DISPLAY_PREPARE_MS988"] = "32";
            if (profile.OpaqueContent) env["SMX_OPAQUE_CONTENT988"] = "1";
            env["SMX_WINDOW_MODE976"] = "layered";
            env["SMX_PRESENT_BACKEND"] = "dxgi";
            env["SMX_PRESENT_OUTPUT874"] = "0";
            env["SMX_PRESENT_SYNC"] = profile.PresentSync.ToString(CultureInfo.InvariantCulture);
            env["SMX_PRESENT_DEPTH"] = profile.PresentDepth.ToString(CultureInfo.InvariantCulture);
            env["SMX_PRESENT_PREFILL"] = "4";
            env["SMX_CAPTURE_INTERVAL_US"] = "0";
            env["SMX_CAPTURE_BACKEND989"] = profile.CaptureBackend;
            env["SMX_PROCESSING_SIZE992"] = profile.ProcessingSize;
            env["SMX_OUTPUT_MODE993"] = profile.OutputMode;
            env["SMX_FRAME_MULTIPLIER989"] = profile.FrameMultiplier.ToString(CultureInfo.InvariantCulture);
            env["SMX_LOG_LIMIT_MB989"] = "16";
            if (profile.CaptureCursor && profile.CaptureBackend == "wgc") env["SMX_CAPTURE_CURSOR"] = "1"; // absent means off; "0" would mean on in native R988
            env["SMX_DIRECT_OUTPUT"] = "1";
            env["SMX_DIRECT_ORIGINALS"] = "1";
            env["SMX_SOURCE_SRV"] = "1";
            env["SMX_NATIVE_DOWNSCALE"] = "1"; // original internal multiscale grid; external capture/output sizes stay native
            env["SMX_WARP_FLAVOR"] = "native-flow";
            env["SMX_DEPENDENCY_BARRIERS"] = "1";
            env["SMX_MODEL_DUTY"] = "0";
            env["SMX_RESOURCE_BUDGET"] = "0";
            env["SMX_DISABLE_ADMISSION"] = "1";
            env["SMX_OUTPUT_GUARD"] = "0";
            env["SMX_TIMELINE_DELAY_MS"] = profile.TimelineMs.ToString(CultureInfo.InvariantCulture);
            env["SMX_UI_SECONDS964"] = profile.SessionSeconds.ToString(CultureInfo.InvariantCulture);
            // HUD state uses explicit host commands. No benchmark, fixture, proof, capture bypass or diagnostics flags.
            return env;
        }
        static string Absolute(string value, string name)
        {
            if (String.IsNullOrWhiteSpace(value) || !Path.IsPathRooted(value)) throw new ArgumentException("Absolute workspace path required",name);
            return Path.GetFullPath(value);
        }
    }
}
