//
// Created by Ayaan on 2026-08-23.
//

#ifndef QTIP_COLOR_H
#define QTIP_COLOR_H
#include <cstdint>

#include "Q-Tip/Config.h"

struct SDL_Color;
struct SDL_FColor;

QTIP_CODE_BEGIN

struct Color {
    uint8_t r, g, b, a = 255;

    operator ::SDL_Color() const;
    operator ::SDL_FColor() const;

    static const Color clear;

    static const Color black;
    static const Color white;
    static const Color red;
    static const Color green;
    static const Color blue;
};

inline const Color Color::clear = {0, 0, 0, 0};

inline const Color Color::black = {0, 0, 0, 255};
inline const Color Color::white = {255, 255, 255, 255};
inline const Color Color::red   = {255, 0, 0, 255};
inline const Color Color::green = {0, 255, 0, 255};
inline const Color Color::blue  = {0, 0, 255, 255};

QTIP_CODE_END

#endif //QTIP_COLOR_H
