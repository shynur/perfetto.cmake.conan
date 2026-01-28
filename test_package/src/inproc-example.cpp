#include <iostream>
#include <fstream>
#include <perfetto.h>
#include "print_pkg_info.hpp"
#include "test.hpp"

static const auto _init_perfetto = [] {
    perfetto::Tracing::Initialize([] {
        auto args = perfetto::TracingInitArgs{};
        args.backends |= perfetto::kInProcessBackend;
        return args;
    }());
    perfetto::TrackEvent::Register();
    return std::monostate{};
}();

int main() {
    const auto tracing_session = [] {
        auto session = perfetto::Tracing::NewTrace();
        session->Setup([] {
            auto cfg = perfetto::TraceConfig{};
            cfg.add_buffers()->set_size_kb(1024);
            {
                const auto ds_cfg = cfg.add_data_sources()->mutable_config();
                ds_cfg->set_name("track_event");
                ds_cfg->set_track_event_config_raw([] {
                    auto te_cfg = perfetto::protos::gen::TrackEventConfig{};

                    te_cfg.add_disabled_categories("*");
                    te_cfg.add_enabled_categories("rendering");

                    return te_cfg;
                }().SerializeAsString());
            }
            return cfg;
        }());
        session->StartBlocking();
        return session;
    }();

    test();

    tracing_session->StopBlocking();
    const auto trace_data = tracing_session->ReadTraceBlocking();
    std::ofstream{
        "/tmp/a.pftrace", std::ios::out | std::ios::binary
    }.write(trace_data.data(), std::streamsize(trace_data.size()));
}
