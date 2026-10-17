//
// Created by Ayaan on 2026-10-16.
//

#include "Spacer.h"

Spacer::Spacer(QTip::Rect rect) {
    _rect = rect;
    _preferredRect = rect;
}

QTip::Point Spacer::minimumSize() const {
    return {0, 0};
}

QTip::Point Spacer::preferredSize() const {
    return _preferredRect.size;
}

void Spacer::resize(QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;
}

void Spacer::reposition(QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
}

void Spacer::setRect(QTip::Rect rect) {
    _rect = rect;
}

const QTip::Rect& Spacer::rect() {
    return _rect;
}

void Spacer::renderImpl(QTip::Window& window) {
}
