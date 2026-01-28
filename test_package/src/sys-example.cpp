#include <iostream>
#include <fstream>
#include <perfetto.h>
#include "print_pkg_info.hpp"
#include "test.hpp"

static const auto _init_perfetto = [] {
    perfetto::Tracing::Initialize([] {
        auto args = perfetto::TracingInitArgs{};
        args.backends |= perfetto::kSystemBackend;
        return args;
    }());
    perfetto::TrackEvent::Register();
    return std::monostate{};
}();

int main() {
    test();
}
