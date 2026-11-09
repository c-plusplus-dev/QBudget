#ifndef QTIP_CLOCK_H
#define QTIP_CLOCK_H
#include <Q-Tip/Config.h>
#include <cstdint>

QTIP_CODE_BEGIN

typedef uint64_t ms;

class Clock {
public:
    Clock();

    [[nodiscard]] ms now() const;

    [[nodiscard]] ms elapsedFromLastCall();

private:
    uint64_t start;
    uint64_t lastCall;
};

QTIP_CODE_END

#endif // QTIP_CLOCK_H
