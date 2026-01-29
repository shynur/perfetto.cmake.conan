#include <system_error>
#include <cstdlib>
#include <cctype>
#if __has_include(<version>)
    #include <version>
#endif
#include <cerrno>
#include <limits>
#include <string>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <iostream>
#include <cassert>

namespace shynur::polyfill <%

struct from_chars_result {
    const char *ptr;
    std::errc ec;
};

template<typename Integer>
from_chars_result from_chars(const char *const first, const char *const last, Integer& value, const int base=10) {
    static_assert(std::is_integral_v<Integer>);

    assert(2 <= base and base <= 36);

    // 构造不出合法的字符串.  直接失败.
    if (last-first <= 0)
        return {
            .ptr = first,
            .ec = std::errc::invalid_argument,
        };
    const auto s = std::string_view{first, std::size_t(last-first)};  // 保证 length>=1

    // 不识别 '+' 符号, 不识别前导空白字符.  直接失败.
    if (s[0] == '+' or std::isspace(s[0]))
        return {
            .ptr = first,
            .ec = std::errc::invalid_argument,
        };

    // 只有单独一个 '-' 符号, 视为没有匹配任何字符.  直接失败.
    if (s == "-")
        return {
            .ptr = first,
            .ec = std::errc::invalid_argument,
        };

    // 16 进制下也不识别 0x 前缀.
    if (base == 16 and (s.length() >= 2 and (s[0] == '0' and std::tolower(s[1]) == 'x'))) {
        value = 0;  // 如果前缀是 0x, 视为 0.
        return { .ptr = &s[1] };  // 0 一定落在合法区间内.
    }

    // End: 处理与 strto* 函数的差异.


    std::size_t eoc;  // 最后一个未参与 conversion 的索引的位置.  相当于参与转换的字符的数量.
        const auto s_cpy = std::string{s};
    try {
        if constexpr (std::is_same_v<Integer, int>)
            value = std::stoi(s_cpy, &eoc, base);
        else if constexpr (std::is_same_v<Integer, long>)
            value = std::stol(s_cpy, &eoc, base);
        else if constexpr (std::is_same_v<Integer, long long>)
            value = std::stoll(s_cpy, &eoc, base);
        else if constexpr (std::is_same_v<Integer, unsigned long>)
            value = std::stoul(s_cpy, &eoc, base);
        else if constexpr (std::is_same_v<Integer, unsigned long long>)
            value = std::stoull(s_cpy, &eoc, base);
        else if constexpr (std::is_same_v<Integer, std::uint32_t> and sizeof(long long) > sizeof(std::uint32_t)) {
            const auto result = std::stoll(s_cpy, &eoc, base);
            if (0 <= result and result <= std::numeric_limits<std::uint32_t>::max())
                value = result;
            else
                throw std::out_of_range{""};
        } else {
            static_assert(
                //false,
                   std::is_same_v<Integer, int>
                or std::is_same_v<Integer, long>
                or std::is_same_v<Integer, long long>
                or std::is_same_v<Integer, unsigned long>
                or std::is_same_v<Integer, unsigned long long>,
                "shynur::polyfill::from_chars unsupported integral type: it'll be implemented in the future"
            );
        }
    } catch(const std::invalid_argument&) {
        return {
            .ptr = first,
            .ec = std::errc::invalid_argument,
        };
    } catch(const std::out_of_range&) {
        return {
            .ptr = first + eoc,
            .ec = std::errc::result_out_of_range,
        };
    }
    return { .ptr = first + eoc };
}

%>

namespace std {
    #ifndef __cpp_lib_to_chars
        using shynur::polyfill::from_chars;
        using shynur::polyfill::from_chars_result;
    #endif
}
