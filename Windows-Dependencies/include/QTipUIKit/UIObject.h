//
// Created by Ayaan on 2026-08-31.
//

#ifndef QTIP_UIOBJECT_H
#define QTIP_UIOBJECT_H

#include <Q-Tip/Clock.h>
#include <Q-Tip/Graphics/RenderTexture.h>
#include <Q-Tip/Window/Window.h>
#include <Q-Tip/Mods/ModLoader/ModLoader.h>

class UIContext;
struct SDL_Window;

class UIObject {
MODDABLE_ROOT(UIObject)

public:
    UIObject() = default;

    virtual ~UIObject();

    int zIndex() const {return _z;}
    void setZIndex(const int z) {
        _z = z;
        if (parent)
            parent->_zVersion++;
    }

    void render(QTip::Window& window) {
        regWindow(window);
        renderImpl(window);
    }

    // returns true to continue propagating, false if consumed
    virtual bool handleEvent(const QTip::Event& event) {return true;}

    [[nodiscard]] virtual QTip::Point minimumSize() const = 0;
    [[nodiscard]] virtual QTip::Point preferredSize() const = 0;

    virtual void resize(QTip::Point size) = 0;
    virtual void reposition(QTip::Point position) = 0;
    virtual void setRect(QTip::Rect rect) = 0;
    [[nodiscard]] virtual const QTip::Rect& rect() = 0;

    UIObject* parent = nullptr;

    virtual void tick(QTip::Clock& clock, double dt) {}

protected:
    QTip::Rect _rect{};
    QTip::Rect _preferredRect{};
    static SDL_Window* window(QTip::Window& window) {
        return window;
    }

    virtual void renderImpl(QTip::Window& window) = 0;

    QTip::Window* _window = nullptr;
    UIContext* _context = nullptr;

    int _z = 0;

    // Mostly only for panels, just helps with sorting
    int _zVersion = 0;

    struct NoRegTag {};

    explicit UIObject(NoRegTag) : _z(0) {}

private:
    void regWindow(QTip::Window& window);
};

#endif //QTIP_UIOBJECT_H
