#include <perfetto.h>
#include <perfetto/tracing/internal/track_event_interned_fields.h>

void use_internal(perfetto::EventContext ctx, const char *KEY, const char *TAG) {
    bool(perfetto::internal::IsValidEventNameType<decltype(KEY)>::value);

    auto isctx = ctx.GetIncrementalState();
    auto da = ctx.event()->add_debug_annotations();
    da->set_name_iid(perfetto::internal::InternedDebugAnnotationName::Get(isctx, KEY));
    da->set_proto_type_name_iid(perfetto::internal::InternedDebugAnnotationValueTypeName::Get(isctx, TAG));
}
