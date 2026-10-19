
//
// Created by Ayaan on 2026-10-19.
//

#include "ScreenManager.h"

ScreenManager::ScreenManager(QTip::Rect rect) {
    _rect = rect;
    _preferredRect = rect;
}

void ScreenManager::pop() {
    if (!_screens.empty()) {
        _screens.pop_back();
    }
}

bool ScreenManager::handleEvent(const SDL_Event& event) {
    Screen* screen = current();

    if (!screen) {
        return true;
    }

    const bool propagate = screen->handleEvent(event);
    const bool shouldClose = screen->shouldClose();

    if (shouldClose) {
        pop();
    }

    return propagate;
}

Screen* ScreenManager::current() {
    return _screens.empty() ? nullptr : _screens.back().get();
}

const Screen* ScreenManager::current() const {
    return _screens.empty() ? nullptr : _screens.back().get();
}

bool ScreenManager::empty() const {
    return _screens.empty();
}

std::size_t ScreenManager::size() const {
    return _screens.size();
}

QTip::Point ScreenManager::minimumSize() const {
    return _rect.size;
}

QTip::Point ScreenManager::preferredSize() const {
    return _preferredRect.size;
}

void ScreenManager::resize(QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;
}

void ScreenManager::reposition(QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
}

void ScreenManager::setRect(QTip::Rect rect) {
    _rect = rect;
}

const QTip::Rect& ScreenManager::rect() {
    return _rect;
}

void ScreenManager::tick(QTip::Clock& clock, double dt) {
    if (Screen* screen = current()) {
        screen->tick(clock, dt);
    }
}

void ScreenManager::renderImpl(QTip::Window& window) {
    Screen* screen = current();

    if (!screen) {
        return;
    }

    screen->render(window);
}
