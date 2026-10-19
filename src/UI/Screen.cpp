//
// Created by Ayaan on 2026-10-19.
//

#include "Screen.h"

Screen::Screen(QTip::Rect rect) : Panel(rect) {}

void Screen::close() {
    _closeRequested = true;
}

bool Screen::shouldClose() const {
    return _closeRequested;
}