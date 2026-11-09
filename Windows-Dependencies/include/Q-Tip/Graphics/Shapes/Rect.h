//
// Created by Ayaan on 2026-08-23.
//

#ifndef QTIP_RECT_H
#define QTIP_RECT_H
#include <cmath>

#include "Q-Tip/Config.h"
#include "Q-Tip/Math/Point.h"

struct SDL_FRect;

QTIP_CODE_BEGIN

struct Rect {
    Point origin;
    Point size;

    operator SDL_FRect() const;

    bool operator==(const Rect& other) const {
        return origin.x == other.origin.x && origin.y == other.origin.y && size.x == other.size.x && size.y == other.size.y;
    }

    [[nodiscard]] bool isPointInside(const Point& point) const {
        return origin.x <= point.x && point.x <= origin.x + size.x && origin.y <= point.y && point.y <= origin.y + size.y;
    }

    [[nodiscard]] Point center() const {
        return origin + size / 2.0f;
    }

    [[nodiscard]] Rect intersection(const Rect& other) const {
        const float left = std::fmax(origin.x, other.origin.x);
        const float top = std::fmax(origin.y, other.origin.y);

        const float right = std::fmin(
            origin.x + size.x,
            other.origin.x + other.size.x
        );

        const float bottom = std::fmin(
            origin.y + size.y,
            other.origin.y + other.size.y
        );

        if (right <= left || bottom <= top) {
            return Rect{left, top, 0.0f, 0.0f};
        }

        return Rect{
            left,
            top,
            right - left,
            bottom - top
        };
    }

    static const Rect zero;
};

inline const Rect Rect::zero = {0, 0, 0, 0};

QTIP_CODE_END

#endif //QTIP_RECT_H
