#include <system_error>
#include <cstdlib>
#include <cctype>
#if __has_include(<version>)
    #include <version>
#endif
#include <cerrno>
#include <limits>

namespace shynur::polyfill <%
    struct from_chars_result {
        const char *ptr;
        std::errc ec;
    };

    template<typename Integer>
    from_chars_result from_chars(const char *const first, const char *last, Integer& value, const int base=10) {
        if (first >= last)
            return {first, std::errc::invalid_argument};

        if (first[0] == '+' or std::isspace(first[0]))
            return {first, std::errc::invalid_argument};

        // 不识别 0x 前缀.
        if ((base == 16 && last-first >= 2) && (first[0] == '0' && std::tolower(first[1]) == 'x')) {
            value = 0;
            return {.ptr = &first[1]};
        }

        const auto result = std::strtoll(first, last, base);
        if (first == last)
            return {first, std::errc::invalid_argument};
        else if (errno == ERANGE) {
            return {.ptr = last, .ec = std::errc::result_out_of_range};
        } else if (result < std::numeric_limits<Integer>::min() || result > std::numeric_limits<Integer>::max()) {
            return {.ptr = last, .ec = std::errc::result_out_of_range};
        } else {
            value = Integer(result);
            return {.ptr = last};
        }
    }
%>

namespace std <%
    #ifndef __cpp_lib_to_chars
        using shynur::polyfill::from_chars;
        using shynur::polyfill::from_chars_result;
    #endif
%>
