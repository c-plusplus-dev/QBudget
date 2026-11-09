//
// Created by Ayaan on 2026-08-23.
//

#ifndef QTIP_CIRCLE_H
#define QTIP_CIRCLE_H
#include "Q-Tip/Config.h"
#include "Q-Tip/Math/Point.h"

QTIP_CODE_BEGIN
    struct Circle {
    Point center;
    float radius;

    bool operator==(const Circle& other) const {
        return center.x == other.center.x && center.y == other.center.y && radius == other.radius;
    }
};

QTIP_CODE_END

#endif //QTIP_CIRCLE_H
