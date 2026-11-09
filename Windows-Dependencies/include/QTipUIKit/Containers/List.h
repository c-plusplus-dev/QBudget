//
// Created by Ayaan on 2026-10-03.
//

#ifndef QTIPUIKIT_LIST_H
#define QTIPUIKIT_LIST_H

#include "QTipUIKit/Containers/Panel.h"

class List : public Panel {
public:
    enum class Direction {
        Up,
        Down,
        Left,
        Right
    };

    explicit List(
        QTip::Rect rect,
        Direction direction = Direction::Down,
        float spacing = 5.0f
    );

    ~List() override = default;

    bool handleEvent(const QTip::Event& event) override;

    void remove(UIObject& object) override;

    void clear() override;

    void setDirection(Direction direction);
    Direction direction() const;

    void setSpacing(float spacing);
    float spacing() const;

    void updateLayout();

protected:
    void onAddObject(UIObject* object) override;

    void renderImpl(QTip::Window& window) override;

private:
    Direction _direction;
    float _spacing = 0.0f;

    void layout();
    void resizeToFitChildren();
};

#endif // QTIPUIKIT_LIST_H