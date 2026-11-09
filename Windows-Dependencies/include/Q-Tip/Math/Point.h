//
// Created by Ayaan on 2026-08-26.
//

#ifndef QTIP_POINT_H
#define QTIP_POINT_H
#include "Q-Tip/Config.h"
#include <cmath>

QTIP_CODE_BEGIN

struct Point {
    float x;
    float y;

    // Addition
    constexpr Point operator+(const Point& other) const {
        return {
            x + other.x,
            y + other.y
        };
    }

    constexpr Point& operator+=(const Point& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    // Subtraction
    constexpr Point operator-(const Point& other) const {
        return {
            x - other.x,
            y - other.y
        };
    }

    constexpr Point& operator-=(const Point& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // Unary plus
    constexpr Point operator+() const {
        return *this;
    }

    // Unary minus
    constexpr Point operator-() const {
        return {
            -x,
            -y
        };
    }

    // Scalar multiplication
    constexpr Point operator*(float scalar) const {
        return {
            x * scalar,
            y * scalar
        };
    }

    constexpr Point& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    // Scalar division
    constexpr Point operator/(float scalar) const {
        return {
            x / scalar,
            y / scalar
        };
    }

    constexpr Point& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Equality
    constexpr bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    // Vector length squared
    constexpr float lengthSquared() const {
        return x * x + y * y;
    }

    // Vector length
    float length() const {
        return std::sqrt(lengthSquared());
    }

    // Distance squared
    constexpr float distanceSquared(const Point& other) const {
        return (*this - other).lengthSquared();
    }

    // Distance
    float distance(const Point& other) const {
        return (*this - other).length();
    }

    // Dot product
    constexpr float dot(const Point& other) const {
        return x * other.x + y * other.y;
    }

    // Normalized vector
    Point normalized() const {
        const float len = length();

        if (len == 0.0f) {
            return {};
        }

        return *this / len;
    }

    // Perpendicular vector
    constexpr Point perpendicular() const {
        return {
            -y,
            x
        };
    }
};

constexpr Point operator*(float scalar, const Point& point) {
    return point * scalar;
}

QTIP_CODE_END

#endif //QTIP_POINT_H
