#pragma once

template<ModType M>
void ModLoader::load() {
    for (const auto& mod : instance.mods) {
        if (dynamic_cast<M*>(mod.get())) {
            return; // Already loaded
        }
    }

    auto mod = std::make_unique<M>();

    mod->init();

    instance.mods.push_back(std::move(mod));
}

template <ModType M>
M* ModLoader::mod() {
    for (const auto& mod : instance.mods) {
        if (auto* result = dynamic_cast<M*>(mod.get())) {
            return result;
        }
    }

    throw std::logic_error("Requested Mod \"" + std::string(M::NAME) + "\" is not loaded");
}
