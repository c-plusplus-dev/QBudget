//
// Created by Ayaan on 2026-08-26.
//

#ifndef QTIP_POLYGON_H
#define QTIP_POLYGON_H
#include <vector>

#include "Q-Tip/Config.h"
#include "Q-Tip/Math/Point.h"

QTIP_CODE_BEGIN

struct Polygon {
    std::vector<Point> points;

    bool operator==(const Polygon& other) const {
        return points == other.points;
    }

    void addPoint(const Point& point) {
        points.push_back(point);
    }
};

QTIP_CODE_END

#endif //QTIP_POLYGON_H
