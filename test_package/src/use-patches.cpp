#include <functional>
#include <string>
#include <iostream>

void tr_expand_tag(std::function<void(const std::string&)> set, const char *value) {
    static auto hello_patch [[maybe_unused]] = [] {
        std::cout << "SEER Applied patch" << std::endl;
        return 0;
    }();
	set(value);
}

namespace shynur::test_symbol {
    void real_tr_expand_tag();
}

static auto test_patched_func = []{
    ::shynur::test_symbol::real_tr_expand_tag();
    return 0;
}();
