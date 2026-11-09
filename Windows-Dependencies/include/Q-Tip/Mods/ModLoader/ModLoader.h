//
// Created by Ayaan on 2026-09-10.
//

#ifndef QTIP_MODLOADER_H
#define QTIP_MODLOADER_H
#include <concepts>
#include <Q-Tip/Config.h>
#include <vector>
#include <memory>

#include "Q-Tip/Events/Event.h"
#include "Q-Tip/Mods/Mod.h"

QTIP_CODE_BEGIN

template <typename M>
concept ModType =
    std::derived_from<M, Mod> &&
    requires {
        { M::ID } -> std::convertible_to<std::string_view>;
        { M::NAME } -> std::convertible_to<std::string_view>;
    };

class ModLoader {
public:
    ModLoader(const ModLoader&) = delete;
    ModLoader& operator=(const ModLoader&) = delete;

    ModLoader(ModLoader&&) = delete;
    ModLoader& operator=(ModLoader&&) = delete;

    template<ModType M>
    static void load();

    template<ModType M>
    static M* mod();

private:
    ModLoader() = default;

    ~ModLoader();

    std::vector<std::unique_ptr<Mod>> mods;

    static void handleEvent(const Event& event);

    static void beforePresent(Window& window);

    static void windowCreated  (Window& window);
    static void windowDestroyed(Window& window);

    static ModLoader instance;

    friend class QTipRuntime;
    friend class Window;
    friend class Renderer;
};

#include "ModLoader.tpp"

QTIP_CODE_END

#endif //QTIP_MODLOADER_H
