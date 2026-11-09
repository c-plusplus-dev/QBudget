//
// Created by Ayaan on 2026-09-30.
//

#ifndef QTIPUIKIT_DIALOG_H
#define QTIPUIKIT_DIALOG_H
#include "QTipUIKit/UIObject.h"
#include "QTipUIKit/Containers/HBox.h"
#include "QTipUIKit/Containers/Panel.h"


class Dialog : public UIObject {
public:
    using ResizeHandler = std::function<void(QTip::Point size)>;

    ~Dialog() override;

    bool handleEvent(const QTip::Event& event) override;

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

    void setContentResizeHandler(ResizeHandler handler);
    void setButtonsResizeHandler(ResizeHandler handler);

    void tick(QTip::Clock& clock, double dt) override;

    Panel& content();
    HBox& buttons();

    void close();

    static Dialog& create(QTip::Window& window);

private:
    Dialog();

    void initialize(QTip::Window& window);

    void end();

    void layout();

    Panel _content;

    HBox _buttons;

    QTip::Window* _window = nullptr;

    bool _requestedClose = false;

    QTip::Point _lastWindowSize;

    ResizeHandler _content_resize_handler;
    ResizeHandler _buttons_resize_handler;

    friend class UIKitMod;

protected:
    void renderImpl(QTip::Window& window) override;
};


#endif //QTIPUIKIT_DIALOG_H
