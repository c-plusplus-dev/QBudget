//
// Created by Ayaan on 2026-08-23.
//

#ifndef QTIP_WINDOW_H
#define QTIP_WINDOW_H

#include <optional>
#include <Q-Tip/Config.h>
#include <Q-Tip/Graphics/Renderer.h>

#include "Q-Tip/Events/Event.h"
#include "Q-Tip/Window/Input.h"

struct SDL_Window;

QTIP_CODE_BEGIN

class Window {
MODDABLE_ROOT(Window)

public:
    Window(const char* title, float width, float height);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    void destroy();

    void open(const char* title, float width, float height);
    void close();

    void requestClose();

    [[nodiscard]] bool isOpen() const;

    Renderer* operator->();

    void show();
    void hide();

    void setTitle(const char* title);
    void setSize(float width, float height);

    [[nodiscard]] float width() const;
    [[nodiscard]] float height() const;
    [[nodiscard]] Point size() const;

    [[nodiscard]] bool shouldClose() const;

    Renderer& getRenderer();

    Input& input();

    void handleEvent(const Event& event);

    [[nodiscard]] std::optional<Event> pollEvent() const;
    [[nodiscard]] const Event* peekEvent() const;

    void beginFrame();

    operator SDL_Window*() const;

private:
    SDL_Window* _window = nullptr;
    std::optional<Renderer> _renderer;
    Input _input{};
    float _width = 0;
    float _height = 0;
    bool _shouldClose = false;
    uint32_t _id = 0;

    friend class Texture;
    friend class QTipRuntime;
};

QTIP_CODE_END

#endif //QTIP_WINDOW_H
