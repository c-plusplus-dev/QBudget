//
// Created by Ayaan on 2026-11-02.
//

#ifndef QTIP_EVENTTYPE_H
#define QTIP_EVENTTYPE_H

#include "Q-Tip/Config.h"

QTIP_CODE_BEGIN

enum class EventType {
    // Display
    DisplayConnected,
    DisplayDisconnected,
    DisplayOrientationChanged,

    // Window
    WindowMoved,
    WindowResized,
    WindowMinimized,
    WindowMaximized,
    WindowRestored,
    WindowFocused,
    WindowUnfocused,
    WindowCloseRequested,

    // Keyboard
    KeyPressed,
    KeyReleased,
    TextEntered,

    // Mouse
    MouseMoved,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseScrolled,

    // Application
    ApplicationQuit,

    // Drag and drop
    FileDropped,

    // Clipboard
    ClipboardChanged,

    // Gamepad
    //GamepadConnected,
    //GamepadDisconnected,
    //GamepadButtonPressed,
    //GamepadButtonReleased,
    //GamepadAxisMoved,

    // Touch
    //TouchBegan,
    //TouchMoved,
    //TouchEnded,

    // Pen
    //PenMoved,
    //PenButtonPressed,
    //PenButtonReleased,
    //PenPressureChanged
};

QTIP_CODE_END

#endif // QTIP_EVENTTYPE_H