//
// Created by Ayaan on 2026-09-11.
//

#ifndef QTIPUIKIT_CHECKBOX_H
#define QTIPUIKIT_CHECKBOX_H
#include "QTipUIKit/UIObject.h"

#include "QTipUIKit/Internal/Helpers.h"

struct CheckboxStyle {
    QTip::Color color;
    QTip::Color hoverColor;
    QTip::Color pressedColor;
    QTip::Color disabledColor;
};

enum class CheckboxState {
    Normal,
    Hovered,
    Pressed,
    Disabled
};

class Checkbox : public UIObject {
MODDABLE_DERIVED(Checkbox, UIObject)

public:
    Checkbox(float x, float y, float width, float height, const CheckboxStyle& style);
    Checkbox(QTip::Rect rect, const CheckboxStyle& style);

    bool handleEvent(const QTip::Event& event) override;

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point rect) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

    void disable();
    void enable();
    [[nodiscard]] bool isDisabled() const;

    bool checked;

private:
    CheckboxStyle _style;
    CheckboxState _state = CheckboxState::Normal;

    bool _disabled = false;

    static void renderCheckmark(QTip::Window& window, QTip::Rect rect);

protected:
    void renderImpl(QTip::Window& window) override;
};

#endif //QTIPUIKIT_CHECKBOX_H
