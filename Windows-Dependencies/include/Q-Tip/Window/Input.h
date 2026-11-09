#ifndef QTIP_INPUT_H
#define QTIP_INPUT_H

#include <Q-Tip/Config.h>

#include <array>
#include <string>
#include "Scancode.h"
#include "Q-Tip/Events/Event.h"
#include "Q-Tip/Mods/Moddable.h"

QTIP_CODE_BEGIN

class Window;

class Input {
MODDABLE_ROOT(Input)

public:
    Input();

    [[nodiscard]]
    bool keyDown(Scancode key) const;

    [[nodiscard]]
    bool keyWasPressed(Scancode key) const;

    [[nodiscard]]
    bool keyWasReleased(Scancode key) const;

    [[nodiscard]]
    bool mouseButtonDown(MouseButton button) const;

    [[nodiscard]]
    bool mouseButtonWasPressed(MouseButton button) const;

    [[nodiscard]]
    bool mouseButtonWasReleased(MouseButton button) const;

    [[nodiscard]]
    float mouseX() const;

    [[nodiscard]]
    float mouseY() const;

    [[nodiscard]]
    float mouseDeltaX() const;

    [[nodiscard]]
    float mouseDeltaY() const;

    [[nodiscard]]
    std::string textInput();

private:
    void beginFrame();
    void processEvent(const Event& event);

    static constexpr std::size_t KeyCount =
        static_cast<std::size_t>(Scancode::COUNT);

    std::array<bool, KeyCount> _keys{};
    std::array<bool, KeyCount> _pressed{};
    std::array<bool, KeyCount> _released{};

    std::array<bool, 5> _mouseButtons{};
    std::array<bool, 5> _mousePressed{};
    std::array<bool, 5> _mouseReleased{};

    float _mouseX = 0.0f;
    float _mouseY = 0.0f;

    float _mouseDeltaX = 0.0f;
    float _mouseDeltaY = 0.0f;

    std::string _text;

    friend class Window;
};

QTIP_CODE_END

#endif
