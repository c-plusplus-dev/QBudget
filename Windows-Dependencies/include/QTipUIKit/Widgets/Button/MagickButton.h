//
// Created by Ayaan on 2026-10-19.
//

#ifndef QTIPUIKIT_MAGICKBUTTON_H
#define QTIPUIKIT_MAGICKBUTTON_H
#include "Button.h"

class MagickButton : public Button {
public:
    using RenderCallback = std::function<void(QTip::Window&, const QTip::Rect&)>;

    MagickButton(QTip::Rect rect, const ButtonStyle& style, RenderCallback callback);

protected:
    void renderImpl(QTip::Window& window) override;

    RenderCallback _callback;
};

#endif //QTIPUIKIT_MAGICKBUTTON_H
