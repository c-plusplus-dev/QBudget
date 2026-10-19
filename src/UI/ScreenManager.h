//
// Created by Ayaan on 2026-10-19.
//

#ifndef QBUDGET_SCREENMANAGER_H
#define QBUDGET_SCREENMANAGER_H

#include "Screen.h"

#include <concepts>
#include <memory>
#include <utility>
#include <vector>

class ScreenManager : public UIObject { // so we can render it normally
public:


    ScreenManager(QTip::Rect rect);
    ~ScreenManager() override = default;

    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

    template<std::derived_from<Screen> T, typename... Args>
        requires std::constructible_from<T, QTip::Rect, Args...>
    T& push(QTip::Rect rect, Args&&... args) {
        auto screen = std::make_unique<T>(
            rect,
            std::forward<Args>(args)...
        );

        _screens.push_back(std::move(screen));
        T& result = *_screens.back();

        result.parent = this;

        return result;
    }

    void pop();

    bool handleEvent(const SDL_Event& event) override;

    [[nodiscard]] Screen* current();
    [[nodiscard]] const Screen* current() const;

    [[nodiscard]] bool empty() const;
    [[nodiscard]] std::size_t size() const;

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

    void tick(QTip::Clock& clock, double dt) override;

private:
    std::vector<std::unique_ptr<Screen>> _screens;

protected:
    void renderImpl(QTip::Window& window) override;
};

#endif // QBUDGET_SCREENMANAGER_H
