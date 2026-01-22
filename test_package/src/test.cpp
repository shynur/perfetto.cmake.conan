#include <perfetto.h>
#include <thread>
using namespace std::literals;
#include <variant>
#include "test.hpp"

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

void DrawPlayer(const int player_number) {
    TRACE_EVENT(
        "rendering", "DrawPlayer",
        "player_number", player_number
    );
}

void DrawGame(const int n) {
    TRACE_EVENT_BEGIN("rendering", "DrawGame");
    DrawPlayer(1);
    DrawPlayer(2);
    TRACE_EVENT_END("rendering");

    TRACE_COUNTER(
        "rendering", "Framerate",
        n
    );
}

void test() {
    for (int i = 0; i < 5; ++i) {
        DrawGame(i);
        std::this_thread::sleep_for(10ms);
    }
}
