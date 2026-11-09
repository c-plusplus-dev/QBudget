//
// Created by Ayaan on 2026-09-10.
//

#ifndef QTIP_MODINIT_H
#define QTIP_MODINIT_H
#include <string>
#include <cstdarg>
#include <Q-Tip/Config.h>
#include <Q-Tip/Logger/Log.h>
#include "Q-Tip/Events/Event.h"

QTIP_CODE_BEGIN

class Window;

inline std::string fmt(const char* fmt_str, ...) {
    char buf[256];  // temporary stack buffer

    va_list args;
    va_start(args, fmt_str);
    vsnprintf(buf, sizeof(buf), fmt_str, args);
    va_end(args);

    return {buf};  // copy into std::string
}

class Mod {
public:
    virtual ~Mod() = default;

    [[nodiscard]] virtual std::string_view id() const = 0;
    [[nodiscard]] virtual std::string_view name() const = 0;

    virtual void init() {}
    virtual void shutdown() {}

    virtual void handleEvent(const Event& event) {}

    virtual void beforePresent(Window& window) {}

    virtual void windowCreated  (Window& window) {}
    virtual void windowDestroyed(Window& window) {}

    void log(const std::string_view message, const LogLevel level = LOG_INFO) const {
        QTipLog(fmt("[%s] %s", name().data(), message.data()), level);
    }
};

QTIP_CODE_END

#endif //QTIP_MODINIT_H
