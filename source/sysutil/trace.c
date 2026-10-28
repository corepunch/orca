#include <include/api.h>

// Engine phases as Instruments signposts. Each phase is an interval on the
// Points of Interest track, aligned with Time Profiler samples and animation
// hitches, so a slow frame shows which phase took the time. Phases do not
// nest with themselves, so one open interval per phase is enough. Other
// platforms compile the calls away.
#if defined(__APPLE__) && __has_include(<os/signpost.h>)
#include <os/signpost.h>

static os_log_t trace_log;
static os_signpost_id_t trace_ids[kTracePhaseCount];

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
  X(kTracePhasePresent, "Present")

void
TRACE_Begin(enum TracePhase phase)
{
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
#endif
