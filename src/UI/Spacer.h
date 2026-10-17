//
// Created by Ayaan on 2026-10-16.
//

#ifndef QBUDGET_SPACER_H
#define QBUDGET_SPACER_H
#include <QTipUIKit/UIObject.h>


class Spacer : public UIObject {
public:
    Spacer(QTip::Rect rect);

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

protected:
    void renderImpl(QTip::Window& window) override;
};


#endif //QBUDGET_SPACER_H
