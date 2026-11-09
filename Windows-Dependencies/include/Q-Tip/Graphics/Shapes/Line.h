//
// Created by Ayaan on 2026-08-23.
//

#ifndef QTIP_LINE_H
#define QTIP_LINE_H
#include "Q-Tip/Config.h"
#include "Q-Tip/Math/Point.h"

QTIP_CODE_BEGIN

struct Line {
    Point start, end;

    bool operator==(const Line& other) const {
        return start == other.start && end == other.end;
    }
};

QTIP_CODE_END

#endif //QTIP_LINE_H
