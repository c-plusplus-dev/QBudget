//
// Created by Ayaan on 2026-09-20.
//

#ifndef QTIPUIKIT_VBOX_H
#define QTIPUIKIT_VBOX_H

#include "QTipUIKit/Internal/Box.h"

class VBox : public Detail::Box {
public:
    explicit VBox(QTip::Rect rect)
        : Box(
            rect,
            {
                .direction = Direction::Vertical
            }
        ) {}
};

#endif // QTIPUIKIT_VBOX_H