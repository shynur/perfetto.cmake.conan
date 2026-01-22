#include <perfetto.h>
#include "test.hpp"

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
    for (int i = 0; i < 5; ++i) {
        DrawGame();
    }
}
