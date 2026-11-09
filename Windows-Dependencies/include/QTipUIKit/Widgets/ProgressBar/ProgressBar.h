//
// Created by ChatGPT on 2026-11-05.
//

#ifndef QTIPUIKIT_PROGRESSBAR_H
#define QTIPUIKIT_PROGRESSBAR_H

#include "QTipUIKit/UIObject.h"

struct ProgressBarStyle {
    QTip::Color trackColor{48, 48, 48, 255};
    QTip::Color progressColor{90, 170, 255, 255};
    int cornerRadius = 4;
};

class ProgressBar : public UIObject {
MODDABLE_DERIVED(ProgressBar, UIObject)

public:
    explicit ProgressBar(
        QTip::Rect rect,
        const ProgressBarStyle& style = {}
    );

    void setProgress(float progress);
    [[nodiscard]] float progress() const;

    void setStyle(const ProgressBarStyle& style);
    [[nodiscard]] const ProgressBarStyle& style() const;

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

protected:
    void renderImpl(QTip::Window& window) override;

private:
    ProgressBarStyle _style;
    float _progress = 0.0f;
};

#endif //QTIPUIKIT_PROGRESSBAR_H
