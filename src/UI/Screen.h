//
// Created by Ayaan on 2026-10-19.
//

#ifndef QBUDGET_SCREEN_H
#define QBUDGET_SCREEN_H
#include <QTipUIKit/Containers/Panel.h>

class Screen : public Panel {
public:
    explicit Screen(QTip::Rect rect);
    ~Screen() override = default;

    void close();

    [[nodiscard]] bool shouldClose() const;

protected:
    bool _closeRequested = false;
};

#endif //QBUDGET_SCREEN_H
