//
// Created by Ayaan on 2026-10-28.
//

#include "DatabaseManager.h"

#include <ranges>
#include <utility>
#include <QTipUIKit/Internal/Helpers.h>

DatabaseManager::DatabaseManager(std::filesystem::path root) : _root(std::move(root)) {}

DatabaseManager::~DatabaseManager() {
    saveAll();
}

Database* DatabaseManager::get(int year, int month) {
    if (month < 1 || month > 12 || year < 1)
        return nullptr;

    const auto key = std::pair{year, month};

    if (auto it = _cache.find(key); it != _cache.end())
        return it->second->database.get();

    const auto path = pathFor(year, month);

    if (!std::filesystem::is_regular_file(path))
        return nullptr;

    auto database = std::make_unique<Database>(Database::load(path));

    auto entry = std::make_unique<Entry>(
        false,
        std::move(database)
    );

    auto* result = entry->database.get();
    _cache.emplace(key, std::move(entry));

    return result;
}

const Database* DatabaseManager::get(int year, int month) const {
    if (month < 1 || month > 12 || year < 1)
        return nullptr;

    auto it = _cache.find({year, month});
    if (it == _cache.end())
        return nullptr;

    auto& entry = it->second;

    return entry->database.get();
}

void DatabaseManager::save(int year, int month) {
    auto it = getEntry(year, month);
    if (!it)
        return;

    auto& entry = *it;

    auto& database = entry.database;
    database->save(pathFor(year, month));

    entry.dirty = false;
}

void DatabaseManager::saveAll() {
    for (auto [year, month] : _cache | std::views::keys) {
        save(year, month);
    }
}

bool DatabaseManager::unload(int year, int month) {
    auto it = _cache.find({year, month});

    if (it == _cache.end())
        return false;

    if (it->second->dirty)
        return false;

    _cache.erase(it);
    return true;
}

void DatabaseManager::unloadAll() noexcept {
    for (auto [year, month] : _cache | std::views::keys) {
        unload(year, month);
    }
}

DatabaseManager::Entry* DatabaseManager::getEntry(int year, int month) {
    auto it = _cache.find({year, month});
    if (it == _cache.end())
        return nullptr;

    return it->second.get();
}

std::filesystem::path DatabaseManager::pathFor(int year, int month) const {
    return _root / Detail::fmt("%04i", year) / Detail::fmt("%02i.qbdb", month);
}
