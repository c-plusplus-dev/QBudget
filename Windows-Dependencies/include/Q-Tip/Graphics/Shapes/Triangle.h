//
// Created by Ayaan on 2026-08-26.
//

#ifndef QTIP_TRIANGLE_H
#define QTIP_TRIANGLE_H
#include "Q-Tip/Config.h"
#include "Q-Tip/Math/Point.h"
#include <cstddef>

QTIP_CODE_BEGIN

struct Triangle {
    Point a, b, c;

    Point* begin() {
        return &a;
    }

    Point* end() {
        return &c + 1;
    }

    const Point* begin() const {
        return &a;
    }

    const Point* end() const {
        return &c + 1;
    }

    Point& operator[](std::size_t index) {
        return (&a)[index];
    }

    const Point& operator[](std::size_t index) const {
        return (&a)[index];
    }

    bool operator==(const Triangle& other) const {
        return a == other.a && b == other.b && c == other.c;
    }
};

QTIP_CODE_END

#endif //QTIP_TRIANGLE_H
