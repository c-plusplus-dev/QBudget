//
// Created by Ayaan on 2026-10-17.
//

#include "MagickButton.h"

MagickButton::MagickButton(QTip::Rect rect, const ButtonStyle& style, RenderCallback callback) : Button(rect, "", style) {
    _callback = std::move(callback);
}

void MagickButton::renderImpl(QTip::Window& window) {
    Button::renderImpl(window);
    _callback(window, _rect);
}
