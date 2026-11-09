//
// Created by Ayaan on 2026-08-23.
//

#ifndef QTIP_RENDERER_H
#define QTIP_RENDERER_H

#include <Q-Tip/Config.h>
#include <Q-Tip/Graphics/Color.h>

#include "Font.h"
#include "Q-Tip/Mods/Moddable.h"
#include "Shapes/Circle.h"
#include "Shapes/Triangle.h"
#include "Shapes/Rect.h"
#include "Shapes/Polygon.h"
#include "Shapes/Line.h"

struct SDL_Texture;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Rect;

QTIP_CODE_BEGIN

class RenderTexture;

class Texture;

class Window;

enum class RenderBlendMode {
    None          = 0x00000000u,
    Blend         = 0x00000001u,
    BlendPreMulti = 0x00000010u,
    Add           = 0x00000002u,
    AddPreMulti   = 0x00000020u,
    Modulo        = 0x00000004u,
    Multiplied    = 0x00000008u
};

class Renderer {
MODDABLE_ROOT(Renderer)

public:
    explicit Renderer(SDL_Window* window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer(Renderer&& other) noexcept;
    Renderer& operator=(Renderer&& other) noexcept;

    void destroy();

    operator SDL_Renderer*() const;

    void clear();
    void present();

    void setRenderColor(Color color);

    void renderPoint(Point point);
    void renderLine(Line line);
    void renderThickLine(Line line, float thickness = 1.0f, bool rounded_edges = false);
    void renderRect(Rect rect, bool filled = true);
    void renderRoundedRect(Rect rect, int radius);
    void renderCircle(Circle circle, bool filled = true);
    void renderTriangle(Triangle triangle, bool filled = true);
    void renderPolygon(Polygon polygon, bool filled = true);

    void renderText(const Font& font, const char* text, float x, float y, Color color);
    void renderTextCentered(const Font& font, const char* text, float x, float y, Color color);

    void renderTexture(Texture& texture, Rect srcRect, Rect dstRect);

    void setTarget(RenderTexture& renderTarget);
    void resetTarget();

    void setClip(const Rect& clipRect);
    void resetClip();

    void setBlendMode(RenderBlendMode mode);
    void resetBlendMode();

private:
    struct ClipState {
        Rect rect{};
        bool enabled = false;
    };

    SDL_Renderer* _renderer = nullptr;
    std::vector<SDL_Texture*> _renderTargetStack;

    ClipState _currentClip;
    std::vector<ClipState> _clipStack;

    RenderBlendMode _mode = RenderBlendMode::None;
    std::vector<RenderBlendMode> _blendStack;

    Window* _window = nullptr;

    friend class Window;
};

QTIP_CODE_END

#endif //QTIP_RENDERER_H
