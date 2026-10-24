#ifndef QBUDGET_HELPERS_H
#define QBUDGET_HELPERS_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <numbers>
#include <vector>

#include <Q-Tip/QTip.h>

inline void DrawScrollGradient(
    QTip::Window& window,
    int width,
    int startX,
    int startY,
    int fadeHeight,
    QTip::Color background,
    bool top = false
) {
    if (width <= 0 || fadeHeight <= 0) {
        return;
    }

    window->setBlendMode(QTip::RenderBlendMode::Blend);

    for (int y = 0; y < fadeHeight; ++y) {
        const float ratio =
            static_cast<float>(y) / static_cast<float>(fadeHeight);
        const auto alpha = static_cast<std::uint8_t>(
            255.0f * (top ? 1.0f - ratio : ratio)
        );

        window->setRenderColor({
            background.r,
            background.g,
            background.b,
            alpha
        });

        const float lineY = static_cast<float>(startY + y);
        window->renderLine({
            static_cast<float>(startX),
            lineY,
            static_cast<float>(startX + width),
            lineY
        });
    }

    window->resetBlendMode();
}

inline double DrawPieSlice(
    QTip::Window& window,
    float cx,
    float cy,
    float radius,
    double percent,
    QTip::Color color,
    double startAngle = -std::numbers::pi / 2.0
) {
    if (percent <= 0.0) {
        return startAngle;
    }

    percent = std::min(percent, 1.0);
    constexpr int segments = 30;
    const double endAngle =
        startAngle + 2.0 * std::numbers::pi * percent;

    std::vector<QTip::Point> vertices;
    vertices.reserve(static_cast<std::size_t>(segments) + 2);
    vertices.push_back({cx, cy});

    for (int i = 0; i <= segments; ++i) {
        const double theta = startAngle
            + (endAngle - startAngle) * static_cast<double>(i) / segments;

        vertices.push_back({
            static_cast<float>(cx + radius * std::cos(theta)),
            static_cast<float>(cy + radius * std::sin(theta))
        });
    }

    window->setRenderColor(color);
    window->renderPolygon(QTip::Polygon{vertices});

    return endAngle;
}

inline void DrawThickLineHV(
    QTip::Window& window,
    float x1,
    float y1,
    float x2,
    float y2,
    float thickness,
    bool roundedEdges = false
) {
    if (thickness <= 0.0f) {
        return;
    }

    QTip::Rect rect{};

    if (x1 == x2) {
        rect.origin.x = x1 - thickness / 2.0f;
        rect.origin.y = std::min(y1, y2);
        rect.size.x = thickness;
        rect.size.y = std::abs(y2 - y1);
    } else if (y1 == y2) {
        rect.origin.x = std::min(x1, x2);
        rect.origin.y = y1 - thickness / 2.0f;
        rect.size.x = std::abs(x2 - x1);
        rect.size.y = thickness;
    } else {
        return;
    }

    window->renderRect(rect);

    if (roundedEdges) {
        const float radius = std::max(0.0f, thickness / 2.0f - 1.0f);
        window->renderCircle({x1, y1, radius});
        window->renderCircle({x2, y2, radius});
    }
}

inline std::filesystem::path getUserHome() {
    if (const char* home = std::getenv("HOME")) {
        return home;
    }

    if (const char* home = std::getenv("USERPROFILE")) {
        return home;
    }

    return {};
}

#endif // QBUDGET_HELPERS_H
