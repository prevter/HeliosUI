#include <Helios/Debug/Log.hpp>

namespace Helios {
    static LogFunction& getLogFunction() {
        static LogFunction func = nullptr;
        return func;
    }

    void Log::setLogFunction(LogFunction func) {
        getLogFunction() = func;
    }

    void Log::logImpl(LogLevel level, fmt::string_view format, fmt::format_args args) {
        if (auto func = getLogFunction()) {
            fmt::memory_buffer buf;
            fmt::vformat_to(std::back_inserter(buf), format, args);
            func(level, std::string_view(buf.data(), buf.size()));
        }
    }
}
