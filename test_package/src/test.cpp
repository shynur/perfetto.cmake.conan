#include <perfetto.h>
#include <thread>
using namespace std::literals;
#include <variant>
#include "test.hpp"

PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("rendering").SetDescription("Events from the graphics subsystem"),
    perfetto::Category("network").SetDescription("Network upload and download statistics")
);
PERFETTO_TRACK_EVENT_STATIC_STORAGE();

static const auto _init_perfetto = [] {
    perfetto::Tracing::Initialize([] {
        auto args = perfetto::TracingInitArgs{};
        args.backends |= perfetto::kInProcessBackend;
        return args;
    }());
    perfetto::TrackEvent::Register();
    return std::monostate{};
}();

void DrawPlayer(const int player_number) {
    TRACE_EVENT(
        "rendering", "DrawPlayer",
        "player_number", player_number
    );
}

void DrawGame() {
    TRACE_EVENT_BEGIN("rendering", "DrawGame");
    DrawPlayer(1);
    DrawPlayer(2);
    TRACE_EVENT_END("rendering");

    TRACE_COUNTER(
        "rendering", "Framerate",
        120
    );
}

void test() {
    for (int i = 0; i < 100; ++i) {
        DrawGame();
        std::this_thread::sleep_for(10ms);
    }
}
