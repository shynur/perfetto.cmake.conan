#include <iostream>
#include <fstream>
#include <perfetto.h>
#include "print_pkg_info.hpp"

PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("rendering")
        .SetDescription("Events from the graphics subsystem"),
    perfetto::Category("network")
        .SetDescription("Network upload and download statistics"));

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

void DrawPlayer(int player_number) {
  TRACE_EVENT("rendering", "DrawPlayer", "player_number", player_number);
  // ...
}

void DrawGame() {
  TRACE_EVENT_BEGIN("rendering", "DrawGame");
  DrawPlayer(1);
  DrawPlayer(2);
  TRACE_EVENT_END("rendering");

  // ...
  TRACE_COUNTER("rendering", "Framerate", 120);
}


int main() {
  perfetto::TracingInitArgs args;
  args.backends |= perfetto::kInProcessBackend;
  perfetto::Tracing::Initialize(args);
  perfetto::TrackEvent::Register();
  //...

    // Create a trace configuration object. This is used to define the buffers,
  // data sources, and other settings for the trace.
  perfetto::TraceConfig cfg;

  // Add a buffer to the config. Traces are written to this buffer in memory.
  cfg.add_buffers()->set_size_kb(1024);  // 1 MB

  // Add a data source to the config. This specifies what kind of data to
  // collect. In this case, we're collecting track events.
  auto* ds_cfg = cfg.add_data_sources()->mutable_config();
  ds_cfg->set_name("track_event");

  // Configure the track event data source. We can specify which categories of
  // events to enable or disable.
  perfetto::protos::gen::TrackEventConfig te_cfg;
  te_cfg.add_disabled_categories("*");         // Disable all categories by default.
  te_cfg.add_enabled_categories("rendering");  // Enable our "rendering" category.
  ds_cfg->set_track_event_config_raw(te_cfg.SerializeAsString());

  // Create a new tracing session.
  std::unique_ptr<perfetto::TracingSession> tracing_session =
      perfetto::Tracing::NewTrace();

  // Set up the tracing session with the configuration.
  tracing_session->Setup(cfg);

  // Start tracing. This will block until the trace is stopped.
  tracing_session->StartBlocking();

  // The tracing_session object must be kept alive for the duration of the
  // trace.

  // ...

    // Stop the tracing session. This will block until all tracing data has been
  // flushed.
  tracing_session->StopBlocking();

  // Read the trace data from the session.
  std::vector<char> trace_data(tracing_session->ReadTraceBlocking());

  // Write the trace data to a file.
  std::ofstream output;
  output.open("example.pftrace", std::ios::out | std::ios::binary);
  output.write(trace_data.data(), std::streamsize(trace_data.size()));
  output.close();
}
