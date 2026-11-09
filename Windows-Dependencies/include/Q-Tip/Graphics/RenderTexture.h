//
// Created by Ayaan on 2026-09-07.
//

#ifndef QTIP_RENDERTEXTURE_H
#define QTIP_RENDERTEXTURE_H
#include <Q-Tip/Config.h>

#include "Texture.h"
#include "Q-Tip/Math/Point.h"
#include "Q-Tip/Mods/Moddable.h"

QTIP_CODE_BEGIN
class Window;

class RenderTexture : public Texture {
MODDABLE_DERIVED(RenderTexture, Texture)

public:
    RenderTexture(Window& window, int width, int height);

    RenderTexture(Window& window, Point size);
};

QTIP_CODE_END

#endif //QTIP_RENDERTEXTURE_H
