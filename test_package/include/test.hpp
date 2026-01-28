#include <perfetto.h>

void test();

PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("rendering").SetDescription("Events from the graphics subsystem"),
    perfetto::Category("network").SetDescription("Network upload and download statistics")
);
