//
// Created by Ayaan on 2026-09-12.
//

#ifndef QTIPUIKIT_LABEL_H
#define QTIPUIKIT_LABEL_H
#include "QTipUIKit/UIObject.h"

class Label : public UIObject {
MODDABLE_DERIVED(Label, UIObject)

public:
    Label(const std::string& text, QTip::Color color, QTip::Point position, const std::optional<QTip::Font>& font = std::nullopt, bool centered = false);

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

    void setText(const std::string& text);
    [[nodiscard]] const std::string& text() const;

private:
    std::string _text;
    QTip::Color _color;
    QTip::Font _font;
    bool _centered;
    QTip::Point _position{};

protected:
    void renderImpl(QTip::Window& window) override;
};

#endif //QTIPUIKIT_LABEL_H
