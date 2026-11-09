//
// Created by Ayaan on 2026-08-23.
//

#ifndef QTIP_FONT_H
#define QTIP_FONT_H

#include <Q-Tip/Config.h>
#include <filesystem>

#include "Q-Tip/Math/Point.h"
#include "Q-Tip/Mods/Moddable.h"

namespace fs = std::filesystem;

struct TTF_Font;

QTIP_CODE_BEGIN

class Font {
MODDABLE_ROOT(Font)

public:
    explicit Font(fs::path path, float size);
    ~Font();

    Font(const Font& other);
    Font& operator=(const Font& other);

    Font(Font&& other) noexcept;
    Font& operator=(Font&& other) noexcept;

    void destroy();

    [[nodiscard]] float getFontHeight() const;

    [[nodiscard]] Point getTextSize(const std::string& text) const;
    [[nodiscard]] float getTextWidth(const std::string& text) const;
    [[nodiscard]] float getTextHeight(const std::string& text) const;

    operator TTF_Font*() const;
private:
    void copy(const Font& other);

    TTF_Font* _font = nullptr;

    typedef struct FontMeta {
        fs::path path;
        float size;
    } FontMeta;

    FontMeta _meta;
};

QTIP_CODE_END

#endif //QTIP_FONT_H
