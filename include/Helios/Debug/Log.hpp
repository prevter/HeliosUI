#pragma once
#ifndef HELIOS_DEBUG_LOG_HPP
#define HELIOS_DEBUG_LOG_HPP

#include <fmt/format.h>

namespace Helios {
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
    };

    inline std::string_view format_as(LogLevel level) {
        switch (level) {
            case LogLevel::Trace:   return "TRACE";
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARNING";
            case LogLevel::Error:   return "ERROR";
            default:               return "UNKNOWN";
        }
    }

    using LogFunction = void(*)(LogLevel level, std::string_view message);

    namespace Log {
        void setLogFunction(LogFunction func);
        void logImpl(LogLevel level, fmt::string_view format, fmt::format_args args);

        template <typename... Args>
        void log(LogLevel level, fmt::format_string<Args...> format, Args&&... args) {
            logImpl(level, format.get(), fmt::make_format_args(args...));
        }

        template <typename... Args>
        void trace(fmt::format_string<Args...> format, Args&&... args) {
            log(LogLevel::Trace, format, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debug(fmt::format_string<Args...> format, Args&&... args) {
            log(LogLevel::Debug, format, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void info(fmt::format_string<Args...> format, Args&&... args) {
            log(LogLevel::Info, format, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void warn(fmt::format_string<Args...> format, Args&&... args) {
            log(LogLevel::Warning, format, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void error(fmt::format_string<Args...> format, Args&&... args) {
            log(LogLevel::Error, format, std::forward<Args>(args)...);
        }
    }
}

#endif // HELIOS_DEBUG_LOG_HPP