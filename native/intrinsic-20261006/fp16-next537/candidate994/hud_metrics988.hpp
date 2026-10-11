#include "bounded_stdio989.h"
#pragma once
// R988: CPU-only, bounded, thread-safe OS display evidence aggregation.
// All doubles named *Time/now/due/presentCpu are QPC seconds, never wall time.
// Display evidence is DXGI-reported presentation, not optical scanout or input latency.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace smx988 {
class HudMetrics {
public:
    struct FramePoint {
        uint64_t presentId = 0;
        double intervalMs = 0;
        bool generated = false, kindKnown = false, gap = false, timed = false;
    };
    struct Snapshot {
        // Hz are directly timed unique observations in (now-2, now] / 2.
        // They are observed rates, never interpolated from missing PresentCount IDs.
        double displayHz = 0, originalHz = 0, generatedHz = 0;
        // Timed observations / ID span within the 2 s observation window.
        // This is observation coverage, NOT the fraction of submitted frames shown.
        double coverage = 0;
        bool displayValid = false, warmingUp = true, stalled = true;
        double low1PercentHz = 0, displayIntervalP95Ms = 0;
        bool low1PercentValid = false, incomplete = true;
        double contentToDisplayMs = 0, presentToDisplayMs = 0;
        bool latencyValid = false;
        // Unique observed counts in this counter epoch, with late mapping supported.
        uint64_t observedDisplayCount = 0, originalDisplayCount = 0, generatedDisplayCount = 0;
        // Diagnostics below cover the last 10 s, except explicit cumulative fields.
        uint64_t missingIds = 0, unmappedCount = 0, untimedCount = 0, validIntervals = 0;
        uint64_t resetCount = 0, disjointCount = 0, statsErrorCount = 0;
        uint64_t duplicateStatsCount = 0, timestampErrorCount = 0, bufferEvictions = 0;
        // Session submission/completion totals. Do NOT subtract epoch observedDisplayCount from these.
        uint64_t submittedCount = 0, generatedSubmittedCount = 0, generationCompleteCount = 0;
        // Same counter epoch as observedDisplayCount; the difference is unconfirmed, NOT queued/lost.
        uint64_t epochSubmittedCount = 0;
        uint64_t lateCompleted = 0, lateSubmitted = 0;
        unsigned inflight = 0;
        double captureHz = 0, modelGpuMs = 0, serviceMs = 0, queueWaitMs = 0;
        bool modelTimingValid = false, queueWaitValid = false;
        double rateWindowSeconds = 2, lastDisplayAgeMs = 0;
        std::vector<FramePoint> graph;
    };
    static constexpr size_t kMaxMetadata = 2048;
    static constexpr size_t kMaxGraph = 120;
    static constexpr uint32_t kStatsDisjoint = 0x887a000bu;

    void recordSubmitted(uint64_t id, bool generated, double sourceTime,
                         double due, double presentCpu) {
        if (!id || !finite(sourceTime) || !finite(due) || !finite(presentCpu)) return;
        std::lock_guard<std::mutex> lock(mutex_);
        if (lastSubmitId_ && id < lastSubmitId_) resetEpochLocked(presentCpu);
        if (!epochStarted_) { epochStart_ = presentCpu; epochStarted_ = true; }
        auto it = submissions_.find(id);
        if (it == submissions_.end()) {
            if (submissions_.size() >= kMaxMetadata) {
                submissions_.erase(submissionOrder_.front()); submissionOrder_.pop_front();
            }
            submissions_.emplace(id, Submission{id, generated, sourceTime, due, presentCpu});
            submissionOrder_.push_back(id);
            ++submittedTotal_; ++epochSubmittedTotal_;
            if (generated) ++generatedSubmittedTotal_;
        } else {
            // Repeated recording never invents a second submission; keep first metadata.
            return;
        }
        lastSubmitId_ = id;
        // The independent stats sampler can see display before the submitting thread logs it.
        // Backfill the already observed ID instead of creating a fictitious display event.
        for (auto& o : observations_) if (o.id == id && !o.mapped) {
            mapLocked(o, submissions_.at(id)); break;
        }
    }

    void observeDisplay(uint64_t presentCount, uint32_t presentRefreshCount,
                        uint32_t syncRefreshCount, int64_t syncQpc, int32_t hr,
                        double now, double qpcFrequency = 10000000.0) {
        if (!finite(now)) return;
        std::lock_guard<std::mutex> lock(mutex_);
        if (static_cast<uint32_t>(hr) == kStatsDisjoint) {
            if (!inDisjoint_) { ++disjointTotal_; resetEpochLocked(now); }
            inDisjoint_ = true; return;
        }
        inDisjoint_ = false;
        if (hr != 0) { ++statsErrorTotal_; return; }
        if (!presentCount) return;
        if (lastObservedId_ && presentCount < lastObservedId_) resetEpochLocked(now);
        if (!epochStarted_) { epochStart_ = now; epochStarted_ = true; }
        // Usually this is the last element, so the common repeated-poll case is O(1).
        if (lastObservedId_ == presentCount) { ++duplicateTotal_; return; }
        const double t = (finite(qpcFrequency) && qpcFrequency > 0)
                             ? static_cast<double>(syncQpc) / qpcFrequency : -1;
        bool timed = presentRefreshCount == syncRefreshCount && syncQpc > 0
                     && finite(t) && t <= now + 0.002 && t >= epochStart_ - 1.0;
        if (timed && lastDirectTime_ >= 0 && t <= lastDirectTime_) timed = false;
        if (presentRefreshCount == syncRefreshCount && !timed) ++timestampErrorTotal_;
        Observation o;
        o.id = presentCount; o.seenAt = now; o.displayTime = t; o.timed = timed;
        o.gap = lastObservedId_ && presentCount != lastObservedId_ + 1;
        if (lastObservedId_ && presentCount > lastObservedId_ + 1)
            o.missingBefore = presentCount - lastObservedId_ - 1;
        if (!observations_.empty()) {
            const auto& prev = observations_.back();
            o.previousDisplayTime = prev.displayTime;
            o.intervalTimed = prev.timed && timed && t > prev.displayTime;
            if (o.intervalTimed) o.intervalMs = (t - prev.displayTime) * 1000.0;
            o.consecutive = o.intervalTimed && !o.gap;
        }
        auto s = submissions_.find(presentCount);
        if (s != submissions_.end()) mapLocked(o, s->second);
        if (timed) lastDirectTime_ = t;
        lastObservedId_ = presentCount; lastSeenAt_ = now;
        ++observedTotal_;
        if (observations_.size() >= kMaxMetadata) {
            const auto& old = observations_.front();
            evictedAt_ = std::max(evictedAt_, old.timed ? old.displayTime : old.seenAt);
            ++evictionTotal_; observations_.pop_front();
        }
        observations_.push_back(o);
    }

    void recordCapture(double qpcSeconds) {
        if (!finite(qpcSeconds)) return;
        std::lock_guard<std::mutex> lock(mutex_);
        if (captures_.size() == kMaxMetadata) captures_.pop_front();
        captures_.push_back(qpcSeconds);
    }
    void recordGeneration(double gpuSpanMs, double serviceMs, double queueWaitMs = -1) {
        std::lock_guard<std::mutex> lock(mutex_);
        ++generationTotal_;
        if (finite(gpuSpanMs) && gpuSpanMs >= 0 && finite(serviceMs) && serviceMs >= 0) {
            if (timings_.size() == 120) timings_.pop_front();
            timings_.push_back(Timing{gpuSpanMs, serviceMs,
                finite(queueWaitMs) && queueWaitMs >= 0 ? queueWaitMs : -1});
        }
    }
    void setInflight(unsigned count) { std::lock_guard<std::mutex> lock(mutex_); inflight_ = count; }
    void setLateCounts(uint64_t completed, uint64_t presented) {
        std::lock_guard<std::mutex> lock(mutex_); lateCompleted_ = completed; lateSubmitted_ = presented;
    }
    void reset(double now = 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        // An explicit reset starts a NEW session. Internal disjoint/counter resets only
        // discard the evidence epoch, preserving honest session producer totals.
        resetEpochLocked(now);
        captures_.clear(); timings_.clear();
        submittedTotal_ = generatedSubmittedTotal_ = generationTotal_ = 0;
        lateCompleted_ = lateSubmitted_ = 0; inflight_ = 0;
        resetTotal_ = disjointTotal_ = statsErrorTotal_ = duplicateTotal_ = 0;
        timestampErrorTotal_ = evictionTotal_ = 0; inDisjoint_ = false;
    }

    Snapshot snapshot(double now) const {
        std::lock_guard<std::mutex> lock(mutex_);
        Snapshot s;
        s.resetCount = resetTotal_; s.disjointCount = disjointTotal_;
        s.statsErrorCount = statsErrorTotal_; s.duplicateStatsCount = duplicateTotal_;
        s.timestampErrorCount = timestampErrorTotal_; s.bufferEvictions = evictionTotal_;
        s.submittedCount = submittedTotal_; s.generatedSubmittedCount = generatedSubmittedTotal_;
        s.epochSubmittedCount = epochSubmittedTotal_;
        s.generationCompleteCount = generationTotal_; s.lateCompleted = lateCompleted_;
        s.lateSubmitted = lateSubmitted_; s.inflight = inflight_;
        s.observedDisplayCount = observedTotal_; s.originalDisplayCount = originalTotal_;
        s.generatedDisplayCount = generatedTotal_;
        if (!finite(now)) return s;
        s.warmingUp = !epochStarted_ || now - epochStart_ < 2.0;
        s.lastDisplayAgeMs = lastSeenAt_ < 0 ? 0 : std::max(0.0, now - lastSeenAt_) * 1000;
        s.stalled = lastSeenAt_ < 0 || now - lastSeenAt_ > 0.25;
        const double cut2 = now - 2.0, cut10 = now - 10.0;
        uint64_t first2 = 0, last2 = 0, seen2 = 0, timedSeen2 = 0, untimed2 = 0, missing2 = 0;
        std::vector<double> intervals;
        double contentSum = 0, presentSum = 0; size_t latencyCount = 0;
        bool windowIncomplete = evictedAt_ > cut10;
        for (const auto& o : observations_) {
            const double membership = o.timed ? o.displayTime : o.seenAt;
            if (membership > cut2 && membership <= now + 0.002) {
                if (!first2) first2 = o.id;
                last2 = o.id; ++seen2;
                if (o.timed) ++timedSeen2; else ++untimed2;
                missing2 += o.missingBefore;
                if (o.timed) {
                    s.displayHz += 0.5;
                    if (o.mapped) (o.generated ? s.generatedHz : s.originalHz) += 0.5;
                    if (o.mapped && o.sourceTime <= o.displayTime + 0.002
                                 && o.presentCpu <= o.displayTime + 0.002) {
                        contentSum += std::max(0.0, o.displayTime - o.sourceTime) * 1000;
                        presentSum += std::max(0.0, o.displayTime - o.presentCpu) * 1000;
                        ++latencyCount;
                    }
                }
            }
            if (membership <= cut10 || membership > now + 0.002) continue;
            s.missingIds += o.missingBefore;
            if (!o.mapped) ++s.unmappedCount;
            if (!o.timed) ++s.untimedCount;
            if (o.gap || !o.timed || !o.mapped) windowIncomplete = true;
            if (o.consecutive && o.previousDisplayTime > cut10) intervals.push_back(o.intervalMs);
            FramePoint p;
            p.presentId = o.id; p.generated = o.generated; p.kindKnown = o.mapped;
            p.gap = o.gap; p.timed = o.intervalTimed; p.intervalMs = o.intervalMs;
            s.graph.push_back(p);
        }
        if (s.graph.size() > kMaxGraph)
            s.graph.erase(s.graph.begin(), s.graph.end() - kMaxGraph);
        if (first2 && last2 >= first2)
            s.coverage = std::min(1.0, static_cast<double>(timedSeen2) / (last2 - first2 + 1));
        s.displayValid = !s.warmingUp && !s.stalled && seen2 && !untimed2 && !missing2;
        s.incomplete = windowIncomplete;
        s.validIntervals = intervals.size();
        s.displayIntervalP95Ms = percentile(intervals, 0.95);
        // Definition: inverse of the MEAN of the slowest ceil(1%) observed intervals.
        // No missing IDs or untimed observations in the same 10 s window may be hidden.
        s.low1PercentValid = intervals.size() >= 120 && !windowIncomplete && !s.stalled;
        if (s.low1PercentValid) {
            std::sort(intervals.begin(), intervals.end(), std::greater<double>());
            const size_t n = std::max<size_t>(1, static_cast<size_t>(std::ceil(intervals.size() * 0.01)));
            double sum = 0; for (size_t i = 0; i < n; ++i) sum += intervals[i];
            if (sum > 0) s.low1PercentHz = 1000.0 / (sum / n);
            else s.low1PercentValid = false;
        }
        if (latencyCount) {
            s.latencyValid = true; s.contentToDisplayMs = contentSum / latencyCount;
            s.presentToDisplayMs = presentSum / latencyCount;
        }
        for (double t : captures_) if (t > cut2 && t <= now) s.captureHz += 0.5;
        if (!timings_.empty()) {
            double gpu = 0, service = 0, queue = 0; size_t q = 0;
            for (const auto& t : timings_) {
                gpu += t.gpu; service += t.service;
                if (t.queue >= 0) { queue += t.queue; ++q; }
            }
            s.modelTimingValid = true; s.modelGpuMs = gpu / timings_.size();
            s.serviceMs = service / timings_.size();
            if (q) { s.queueWaitValid = true; s.queueWaitMs = queue / q; }
        }
        return s;
    }

    static bool selfTest(std::string* error = nullptr);
private:
    struct Submission { uint64_t id; bool generated; double sourceTime, due, presentCpu; };
    struct Observation {
        uint64_t id = 0, missingBefore = 0;
        double seenAt = 0, displayTime = 0, previousDisplayTime = 0, intervalMs = 0, sourceTime = 0, presentCpu = 0;
        bool timed = false, intervalTimed = false, consecutive = false, gap = false;
        bool mapped = false, generated = false;
    };
    struct Timing { double gpu, service, queue; };
    static bool finite(double d) { return std::isfinite(d); }
    static double percentile(std::vector<double> v, double q) {
        if (v.empty()) return 0;
        std::sort(v.begin(), v.end());
        const double p = (v.size() - 1) * q;
        const size_t lo = static_cast<size_t>(p), hi = std::min(v.size() - 1, lo + 1);
        return v[lo] + (v[hi] - v[lo]) * (p - lo);
    }
    void mapLocked(Observation& o, const Submission& s) {
        if (o.mapped) return;
        o.mapped = true; o.generated = s.generated;
        o.sourceTime = s.sourceTime; o.presentCpu = s.presentCpu;
        if (s.generated) ++generatedTotal_; else ++originalTotal_;
    }
    void resetEpochLocked(double now) {
        submissions_.clear(); submissionOrder_.clear(); observations_.clear();
        lastSubmitId_ = lastObservedId_ = 0; lastDirectTime_ = lastSeenAt_ = -1;
        epochStart_ = now; epochStarted_ = finite(now) && now > 0;
        observedTotal_ = originalTotal_ = generatedTotal_ = epochSubmittedTotal_ = 0;
        evictedAt_ = -1; ++resetTotal_;
    }
    mutable std::mutex mutex_;
    std::unordered_map<uint64_t, Submission> submissions_;
    std::deque<uint64_t> submissionOrder_;
    std::deque<Observation> observations_;
    std::deque<double> captures_;
    std::deque<Timing> timings_;
    uint64_t lastSubmitId_ = 0, lastObservedId_ = 0;
    uint64_t observedTotal_ = 0, originalTotal_ = 0, generatedTotal_ = 0;
    uint64_t submittedTotal_ = 0, generatedSubmittedTotal_ = 0, generationTotal_ = 0;
    uint64_t epochSubmittedTotal_ = 0;
    uint64_t lateCompleted_ = 0, lateSubmitted_ = 0;
    uint64_t resetTotal_ = 0, disjointTotal_ = 0, statsErrorTotal_ = 0;
    uint64_t duplicateTotal_ = 0, timestampErrorTotal_ = 0, evictionTotal_ = 0;
    double epochStart_ = 0, lastDirectTime_ = -1, lastSeenAt_ = -1, evictedAt_ = -1;
    bool epochStarted_ = false, inDisjoint_ = false;
    unsigned inflight_ = 0;
};

inline bool HudMetrics::selfTest(std::string* error) {
    auto fail = [&](const char* message) { if (error) *error = message; return false; };
    auto nearValue = [](double a, double b, double tol = 0.01) { return std::fabs(a - b) <= tol; };
    auto frame = [](HudMetrics& m, uint64_t id, double t, bool generated = false, bool observe = true) {
        m.recordSubmitted(id, generated, t - 0.12, t - 0.001, t - 0.004);
        if (observe) m.observeDisplay(id, static_cast<uint32_t>(id), static_cast<uint32_t>(id),
            static_cast<int64_t>(std::llround(t * 10000000)), 0, t + 0.001);
    };
    {
        HudMetrics m;
        for (uint64_t i = 1; i <= 600; ++i) {
            frame(m, i, 100.0 + i / 60.0, i % 2 == 0);
            if (i % 2) m.recordCapture(100.0 + i / 60.0);
        }
        auto s = m.snapshot(110.001);
        if (!s.displayValid || !nearValue(s.displayHz, 60) || !nearValue(s.originalHz, 30)
            || !nearValue(s.generatedHz, 30) || !s.low1PercentValid || !nearValue(s.low1PercentHz, 60)
            || s.graph.size() != 120 || s.observedDisplayCount != 600 || !nearValue(s.captureHz, 30))
            return fail("regular 30 original + 30 generated display evidence");
        if (!s.latencyValid || !nearValue(s.contentToDisplayMs, 120) || !nearValue(s.presentToDisplayMs, 4))
            return fail("content and submission latency definitions");
        m.observeDisplay(600, 600, 600, 1100000000, 0, 110.002);
        if (m.snapshot(110.003).observedDisplayCount != 600) return fail("duplicate statistics dedup");
        auto stopped = m.snapshot(113);
        if (stopped.displayHz != 0 || !stopped.stalled || stopped.low1PercentValid)
            return fail("stopped stream rate decay and stale low rejection");
    }
    {
        HudMetrics m;
        for (uint64_t i = 1; i <= 180; ++i) frame(m, i, 100 + i / 60.0);
        auto s = m.snapshot(103.001);
        if (!s.displayValid || !nearValue(s.displayHz, 60) || !nearValue(s.originalHz, 60)
            || s.generatedHz != 0 || !s.low1PercentValid || !nearValue(s.low1PercentHz, 60))
            return fail("regular native 60 Hz with zero generated frames");
    }
    {
        HudMetrics m;
        for (uint64_t i = 1; i <= 240; ++i) frame(m, i, 100 + i / 60.0, false, i != 200);
        auto s = m.snapshot(104.001);
        if (s.observedDisplayCount != 239 || !nearValue(s.displayHz, 59.5)
            || s.missingIds != 1 || s.coverage >= 1 || s.low1PercentValid || !s.incomplete)
            return fail("missing statistics ID never filled as a displayed frame");
    }
    {
        HudMetrics m;
        m.observeDisplay(1, 1, 1, 1000000000, 0, 100.001);
        if (m.snapshot(100.002).unmappedCount != 1) return fail("unknown ID remains unknown");
        m.recordSubmitted(1, true, 99.88, 100, 99.996);
        auto s = m.snapshot(100.003);
        if (s.unmappedCount || s.generatedDisplayCount != 1 || s.generatedHz != 0.5
            || s.observedDisplayCount != 1) return fail("async late submission metadata mapping");
    }
    {
        HudMetrics m;
        for (uint64_t i = 1; i <= 150; ++i) frame(m, i, 100 + i / 60.0);
        m.observeDisplay(151, 151, 152, 1026000000, 0, 102.601);
        auto s = m.snapshot(102.602);
        if (s.untimedCount != 1 || s.low1PercentValid || s.displayValid)
            return fail("unequal refresh counters withhold precise timing");
        m.observeDisplay(0, 0, 0, 0, static_cast<int32_t>(kStatsDisjoint), 102.7);
        s = m.snapshot(102.701);
        if (s.observedDisplayCount || s.disjointCount != 1 || s.low1PercentValid)
            return fail("DXGI disjoint resets evidence window");
        frame(m, 1, 103);
        frame(m, 2, 103.02);
        frame(m, 1, 104);
        if (m.snapshot(104.001).observedDisplayCount != 1)
            return fail("submitted counter restart resets evidence");
    }
    {
        // Capacity turnover must not invalidate a complete 10 s window when the
        // discarded observations are already older than that window.
        for (unsigned hz : {60u, 120u}) {
            HudMetrics m;
            for (uint64_t i = 1; i <= hz * 70; ++i) frame(m, i, 100 + double(i) / hz, i % 2 == 0);
            auto s = m.snapshot(170.001);
            if (!s.bufferEvictions || s.incomplete || !s.low1PercentValid
                || !nearValue(s.displayHz, hz) || !nearValue(s.low1PercentHz, hz))
                return fail("70 s at 60/120 Hz retains valid complete window after capacity turnover");
        }
    }
    {
        HudMetrics m;
        for (uint64_t i = 1; i <= 180; ++i) frame(m, i, 100 + i / 60.0);
        m.recordGeneration(4, 6); m.recordCapture(103); m.setLateCounts(2, 3); m.setInflight(1);
        m.observeDisplay(0, 0, 0, 0, static_cast<int32_t>(kStatsDisjoint), 103.01);
        frame(m, 181, 103.02);
        auto s = m.snapshot(103.021);
        if (s.submittedCount != 181 || s.epochSubmittedCount != 1 || s.observedDisplayCount != 1
            || s.generationCompleteCount != 1 || s.disjointCount != 1)
            return fail("disjoint keeps session producer totals separate from evidence epoch totals");
        m.reset(104);
        s = m.snapshot(104.001);
        if (s.submittedCount || s.epochSubmittedCount || s.observedDisplayCount || s.generationCompleteCount
            || s.lateCompleted || s.lateSubmitted || s.inflight || s.disjointCount || s.modelTimingValid
            || s.captureHz != 0 || !s.graph.empty()) return fail("Stop/Start explicit reset fully clears session");
        frame(m, 1, 104.1);
        s = m.snapshot(104.101);
        if (s.submittedCount != 1 || s.epochSubmittedCount != 1 || s.observedDisplayCount != 1)
            return fail("Stop/Start new session starts from fresh counters");
    }
    {
        HudMetrics m; double t = 100;
        for (uint64_t i = 1; i <= 201; ++i) {
            if (i > 1) t += i == 200 ? 0.04 : i == 201 ? 0.02 : 0.01;
            frame(m, i, t);
        }
        auto s = m.snapshot(t + 0.001);
        if (!s.low1PercentValid || !nearValue(s.low1PercentHz, 1000.0 / 30.0)
            || s.validIntervals != 200) return fail("1 percent low means inverse slowest 1 percent average");
    }
    {
        HudMetrics m;
        for (uint64_t i = 1; i <= 2500; ++i) frame(m, i, 100 + i / 300.0);
        auto s = m.snapshot(100 + 2500.0 / 300 + 0.001);
        if (!s.bufferEvictions || s.low1PercentValid || !s.incomplete || s.graph.size() != 120)
            return fail("bounded 2048 metadata protects against silently truncated low");
        m.recordGeneration(4, 6); m.setInflight(2); m.setLateCounts(3, 5);
        s = m.snapshot(110);
        if (!s.modelTimingValid || s.modelGpuMs != 4 || s.serviceMs != 6
            || s.queueWaitValid || s.inflight != 2 || s.lateCompleted != 3 || s.lateSubmitted != 5)
            return fail("external counters and unavailable queue wait");
    }
    if (error) error->clear();
    return true;
}
} // namespace smx988
