//
// Created by Ayaan on 2026-09-11.
//

#ifndef QTIPUIKIT_HELPERS_H
#define QTIPUIKIT_HELPERS_H
#include <filesystem>
#include <Q-Tip/Events/Event.h>
#include <Q-Tip/Graphics/Color.h>
#include <Q-Tip/Math/Point.h>
#include <algorithm>
#include <vector>
#include <SDL3/SDL_events.h>

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923132169163975144
#endif

namespace QTip {
    class Window;
}

namespace fs = std::filesystem;

struct SDL_FColor;
struct SDL_Renderer;
struct SDL_Vertex;

namespace Detail {
    inline fs::path defaultFontPath() {
#ifdef _WIN32
        return "C:/Windows/Fonts/Arial.ttf";
#elif defined(__linux__)
        return "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#elif defined(__APPLE__)
        return "/System/Library/Fonts/SFNS.ttf";
#else
#error "Unsupported platform"
#endif
    }

    QTip::Event transformEvent(const QTip::Event& event, const QTip::Point& position);

    QTip::Point eventPosition(const QTip::Event& event);

    void printMouseEvent(const QTip::Event& event);

    inline QTip::Color pieChartColor(double position) {
        position = std::clamp(position, 0.0, 1.0);

        const double hue = position * 6.0;
        const int section = static_cast<int>(hue);
        const double fraction = hue - section;

        double r = 0.0;
        double g = 0.0;
        double b = 0.0;

        switch (section) {
        case 0:
            r = 1.0;
            g = fraction;
            b = 0.0;
            break;

        case 1:
            r = 1.0 - fraction;
            g = 1.0;
            b = 0.0;
            break;

        case 2:
            r = 0.0;
            g = 1.0;
            b = fraction;
            break;

        case 3:
            r = 0.0;
            g = 1.0 - fraction;
            b = 1.0;
            break;

        case 4:
            r = fraction;
            g = 0.0;
            b = 1.0;
            break;

        case 5:
        default:
            r = 1.0;
            g = 0.0;
            b = 1.0 - fraction;
            break;
        }

        return QTip::Color {
            static_cast<std::uint8_t>(r * 255.0),
            static_cast<std::uint8_t>(g * 255.0),
            static_cast<std::uint8_t>(b * 255.0),
            255
        };
    }

    inline std::string fmt(const char* fmt_str, ...) {
        char buf[256];  // temporary stack buffer

        va_list args;
        va_start(args, fmt_str);
        vsnprintf(buf, sizeof(buf), fmt_str, args);
        va_end(args);

        return {buf};  // copy into std::string
    }

    std::string eventName(const QTip::Event& event);

    bool pointInTriangle(float px, float py, const QTip::Point& v0, const QTip::Point& v1, const QTip::Point& v2);

    bool pointInVB(float px, float py, const std::vector<QTip::Point>& vb);

    double drawPieSlice(QTip::Window& window, std::vector<QTip::Point>* vb, float cx, float cy, float radius, double percent, QTip::Color color, double startAngle = -M_PI_2);
}

static bool operator<(const QTip::Point& lhs, const QTip::Point& rhs) {
    return lhs.x < rhs.x && lhs.y < rhs.y;
}

static bool operator>(const QTip::Point& lhs, const QTip::Point& rhs) {
    return lhs.x > rhs.x && lhs.y > rhs.y;
}

#endif //QTIPUIKIT_HELPERS_H
