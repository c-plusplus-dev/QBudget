//
// Created by Ayaan on 2026-08-24.
//

#ifndef QTIP_TEXTURE_H
#define QTIP_TEXTURE_H

#include <filesystem>
#include <Q-Tip/Config.h>

#include "Q-Tip/Math/Point.h"
#include "Q-Tip/Mods/Moddable.h"

struct SDL_Texture;

QTIP_CODE_BEGIN

namespace fs = std::filesystem;

class Renderer;
class Window;

class Texture {
MODDABLE_ROOT(Texture)

public:
    Texture(Window& window, const fs::path& path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void destroy();

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] SDL_Texture* nativeHandle() const;

    [[nodiscard]] Point size() const;

protected:
    Texture() = default;

    SDL_Texture* _texture = nullptr;
};

QTIP_CODE_END

#endif //QTIP_TEXTURE_H
