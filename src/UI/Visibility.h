#ifndef QBUDGET_UI_VISIBILITY_H
#define QBUDGET_UI_VISIBILITY_H

#include <QTipUIKit/Containers/ScrollView.h>

// ScrollView currently asks every child to render, even when it is outside
// the visible viewport. Views can use this inexpensive test to skip their
// own drawing work while remaining in the normal UIKit event/layout tree.
inline bool isVisibleInScrollView(
    const QTip::Rect& contentRect,
    ScrollView& scrollView
) noexcept {
    const QTip::Point scroll = scrollView.scrollPosition();
    const QTip::Point viewportSize = scrollView.rect().size;

    return contentRect.origin.x < scroll.x + viewportSize.x &&
        contentRect.origin.x + contentRect.size.x > scroll.x &&
        contentRect.origin.y < scroll.y + viewportSize.y &&
        contentRect.origin.y + contentRect.size.y > scroll.y;
}

#endif // QBUDGET_UI_VISIBILITY_H
