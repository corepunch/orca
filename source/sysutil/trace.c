#include <include/api.h>

// Engine phases as Instruments signposts. Each phase is an interval on the
// Points of Interest track, aligned with Time Profiler samples and animation
// hitches, so a slow frame shows which phase took the time. Phases do not
// nest with themselves, so one open interval per phase is enough. Other
// platforms compile the calls away.
#if defined(__APPLE__) && __has_include(<os/signpost.h>)
#include <os/signpost.h>

#include <time.h>

static os_log_t trace_log;
static os_signpost_id_t trace_ids[kTracePhaseCount];
// Wall time per phase for the frame profiler, summed until taken.
static bool_t trace_timing;
static uint64_t trace_started[kTracePhaseCount], trace_total[kTracePhaseCount];

ORCA_API void TRACE_EnableTiming(bool_t enable) { trace_timing = enable; }

ORCA_API void
TRACE_TakeTiming(double out_ms[kTracePhaseCount])
{
  for (int i = 0; i < kTracePhaseCount; i++) out_ms[i] = trace_total[i] / 1e6, trace_total[i] = 0;
}

static os_log_t
TRACE_Log(void)
{
  if (!trace_log) trace_log = os_log_create("org.orca.engine", OS_LOG_CATEGORY_POINTS_OF_INTEREST);
  return trace_log;
}

// os_signpost needs literal names, hence one call site per phase.
#define TRACE_PHASES(X) \
  X(kTracePhaseFrame, "Frame") \
  X(kTracePhasePrograms, "Programs") \
  X(kTracePhaseLayout, "Layout") \
  X(kTracePhaseRender, "Render") \
  X(kTracePhasePresent, "Present") \
  X(kTracePhaseBeginFrame, "BeginFrame") \
  X(kTracePhaseScrollSync, "ScrollSync")

void
TRACE_Begin(enum TracePhase phase)
{
  if (trace_timing) trace_started[phase] = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
  os_log_t log = TRACE_Log();
  if (!os_signpost_enabled(log)) return;
  os_signpost_id_t id = trace_ids[phase] = os_signpost_id_generate(log);
  switch (phase) {
#define BEGIN(phase, name) case phase: os_signpost_interval_begin(log, id, name); break;
    TRACE_PHASES(BEGIN)
#undef BEGIN
    default: break;
  }
}

void
TRACE_End(enum TracePhase phase)
{
  if (trace_timing && trace_started[phase]) {
    trace_total[phase] += clock_gettime_nsec_np(CLOCK_UPTIME_RAW) - trace_started[phase];
    trace_started[phase] = 0;
  }
  os_log_t log = TRACE_Log();
  os_signpost_id_t id = trace_ids[phase];
  if (!os_signpost_enabled(log) || !id) return;
  trace_ids[phase] = 0;
  switch (phase) {
#define END(phase, name) case phase: os_signpost_interval_end(log, id, name); break;
    TRACE_PHASES(END)
#undef END
    default: break;
  }
}
#else
void TRACE_Begin(enum TracePhase phase) { (void)phase; }
void TRACE_End(enum TracePhase phase) { (void)phase; }
void TRACE_EnableTiming(bool_t enable) { (void)enable; }
void TRACE_TakeTiming(double out_ms[kTracePhaseCount]) { for (int i = 0; i < kTracePhaseCount; i++) out_ms[i] = 0; }
#endif
