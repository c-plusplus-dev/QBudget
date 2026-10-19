//
// Created by Ayaan on 2026-10-15.
//

#ifndef QBUDGET_HELPERS_H
#define QBUDGET_HELPERS_H
#include <Q-Tip/QTip.h>

inline void DrawScrollGradient(QTip::Window& window, int width, int startX, int startY, int fadeHeight, QTip::Color bg, bool top = false) {
    window->setBlendMode(QTip::RenderBlendMode::Blend);

    // Gradient (fade-in/out)
    if (!top) {
        for (int y = 0; y < fadeHeight; y++) {
            const float ratio = static_cast<float>(y) / static_cast<float>(fadeHeight);
            const auto alpha = static_cast<uint8_t>(255.0f * ratio);
            window->setRenderColor({bg.r, bg.g, bg.b, alpha});
            const auto lineY = static_cast<float>(y + startY);
            window->renderLine({static_cast<float>(startX), lineY, static_cast<float>(startX + width), lineY});
        }
    } else {
        for (int y = 0; y < fadeHeight; y++) {
            const float ratio = static_cast<float>(y) / static_cast<float>(fadeHeight);
            const auto alpha = static_cast<uint8_t>(255.0f - 255.0f * ratio);
            window->setRenderColor({bg.r, bg.g, bg.b, alpha});
            const auto lineY = static_cast<float>(y + startY);
            window->renderLine({static_cast<float>(startX), lineY, static_cast<float>(startX + width), lineY});
        }
    }

    window->resetBlendMode();
}

inline double DrawPieSlice(QTip::Window& window, float cx, float cy, float radius, double percent, QTip::Color color, double startAngle = -M_PI_2) {
    if (percent <= 0.0) return startAngle;
    if (percent > 1.0) percent = 1.0;  // clamp

    constexpr int segments = 30;  // increase for smoother slices
    double endAngle = startAngle + 2.0 * M_PI * percent;

    std::vector<QTip::Point> vertices;
    std::vector<int> indices;

    // Center point
    QTip::Point center{};
    center.x = cx;
    center.y = cy;
    vertices.push_back(center);

    // Arc points
    for (int i = 0; i <= segments; ++i) {
        double theta = startAngle + (endAngle - startAngle) * i / segments;
        QTip::Point v{};
        v.x = static_cast<float>(cx + radius * cos(theta));
        v.y = static_cast<float>(cy + radius * sin(theta));
        vertices.push_back(v);
    }

    // Triangle fan indices
    for (int i = 1; i < static_cast<int>(vertices.size()) - 1; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    window->setRenderColor(color);
    window->renderPolygon(QTip::Polygon{vertices});

    return endAngle;  // return the angle where the next slice should start
}

inline void DrawThickLineHV(QTip::Window& window, float x1, float y1, float x2, float y2, float thickness, bool rounded_edges=false) {
    QTip::Rect rect{};

    if (x1 == x2) {
        // Vertical line
        rect.origin.x = x1 - thickness / 2;
        rect.origin.y = y1 < y2 ? y1 : y2;
        rect.size.x = thickness;
        rect.size.y = abs(y2 - y1);
    } else if (y1 == y2) {
        // Horizontal line
        rect.origin.x = x1 < x2 ? x1 : x2;
        rect.origin.y = y1 - thickness / 2;
        rect.size.x = abs(x2 - x1);
        rect.size.y = thickness;
    } else {
        // Not horizontal or vertical; do nothing
        return;
    }

    window->renderRect(rect);

    if (rounded_edges) {
        window->renderCircle({x1, y1, thickness / 2 - 1});
        window->renderCircle({x2, y2, thickness / 2 - 1});
    }
}

inline std::filesystem::path getUserHome() {
    if (const char* home = std::getenv("HOME"))
        return home;

    if (const char* home = std::getenv("USERPROFILE"))
        return home;

    return {};
}

#endif //QBUDGET_HELPERS_H
