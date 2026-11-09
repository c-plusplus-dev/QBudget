//
// Created by Ayaan on 2026-09-28.
//

#ifndef QTIPUIKIT_SCROLLBAR_H
#define QTIPUIKIT_SCROLLBAR_H

#include <Q-Tip/Window/Window.h>

#include "QTipUIKit/UIObject.h"

enum class ScrollBarOrientation {
    Horizontal,
    Vertical
};

class ScrollBar : public UIObject {
public:
    ScrollBar(
        QTip::Rect rect,
        ScrollBarOrientation orientation
    );

    void setPosition(float position);
    [[nodiscard]] float position() const;

    void setViewRatio(float ratio);
    [[nodiscard]] float viewRatio() const;

    bool handleEvent(const QTip::Event& event) override;

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

private:
    [[nodiscard]]
    QTip::Rect thumbRect() const;

    [[nodiscard]]
    float trackStart() const;

    [[nodiscard]]
    float trackLength() const;

    [[nodiscard]]
    float thumbLength() const;

    ScrollBarOrientation _orientation;

    float _position = 0.0f;
    float _viewRatio = 1.0f;

    bool _dragging = false;
    float _dragOffset = 0.0f;

protected:
    void renderImpl(QTip::Window& window) override;
};

#endif //QTIPUIKIT_SCROLLBAR_H
