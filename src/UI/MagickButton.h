//
// Created by Ayaan on 2026-10-17.
//

#ifndef QBUDGET_MAGICKBUTTON_H
#define QBUDGET_MAGICKBUTTON_H
#include <QTipUIKit/Widgets/Button/Button.h>

class MagickButton : public Button {
public:
    using RenderCallback = std::function<void(QTip::Window&, const QTip::Rect&)>;

    MagickButton(QTip::Rect rect, const ButtonStyle& style, RenderCallback callback);

protected:
    void renderImpl(QTip::Window& window) override;

    RenderCallback _callback;
};

#endif //QBUDGET_MAGICKBUTTON_H
