#pragma once

#if __has_include(<print>) && (__cpp_lib_print >= 202207L)
    #include <print>
    namespace mally::statlib {
        template <typename... Args>
        void print(std::format_string<Args...> fmt, Args&&... args) {
            std::print(fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void println(std::format_string<Args...> fmt, Args&&... args) {
            std::println(fmt, std::forward<Args>(args)...);
        }
    }
#else
    #include <fmt/core.h>
    #include <fmt/ranges.h>
    namespace mally::statlib {
        template <typename... Args>
        void print(fmt::format_string<Args...> fmt, Args&&... args) {
            fmt::print(fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void println(fmt::format_string<Args...> fmt, Args&&... args) {
            fmt::println(fmt, std::forward<Args>(args)...);
        }
    }
#endif