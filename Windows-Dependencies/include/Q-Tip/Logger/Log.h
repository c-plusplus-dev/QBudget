//
// Created by Ayaan on 2026-09-27.
//

#ifndef QTIP_LOG_H
#define QTIP_LOG_H
#include <iostream>
#include <string_view>

enum LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_FATAL,
    LOG_ERROR
};

namespace Detail {

#ifdef _WIN32

    bool terminalSupportsANSI();

#else

    inline bool terminalSupportsANSI() {
        // POSIX terminals generally support ANSI escape sequences.
        return true;
    }

#endif

    inline std::string_view levelName(LogLevel level) {
        switch (level) {
        case LOG_DEBUG:
            return "[DEBUG]";

        case LOG_INFO:
            return "[INFO]";

        case LOG_WARNING:
            return "[WARNING]";

        case LOG_FATAL:
            return "[FATAL]";

        case LOG_ERROR:
            return "[ERROR]";

        default:
            return "[UNKNOWN]";
        }
    }

    inline std::string_view levelColor(LogLevel level) {
        switch (level) {
        case LOG_DEBUG:
            return "\033[34m";

        case LOG_INFO:
            return "\033[32m";

        case LOG_WARNING:
            return "\033[33m";

        case LOG_FATAL:
        case LOG_ERROR:
            return "\033[31m";

        default:
            return "";
        }
    }

} // namespace Detail

inline void QTipLog(
    std::string_view message,
    LogLevel level = LOG_INFO
) {
    if (Detail::terminalSupportsANSI()) {
        std::cout
            << "QTip "
            << Detail::levelColor(level)
            << Detail::levelName(level)
            << "\033[0m "
            << message
            << '\n';
    } else {
        std::cout
            << "QTip "
            << Detail::levelName(level)
            << ' '
            << message
            << '\n';
    }
}

#endif //QTIP_LOG_H
