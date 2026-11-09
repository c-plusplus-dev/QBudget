//
// Created by Ayaan on 2026-09-20.
//

#ifndef QTIPUIKIT_HBOX_H
#define QTIPUIKIT_HBOX_H

#include "QTipUIKit/Internal/Box.h"

class HBox : public Detail::Box {
public:
    explicit HBox(QTip::Rect rect)
        : Box(
            rect,
            {
                .direction = Direction::Horizontal
            }
        ) {}
};

#endif // QTIPUIKIT_HBOX_H