//
// Created by Ayaan on 2026-10-28.
//

#include "DatabaseManager.h"

#include <algorithm>
#include <charconv>
#include <ranges>
#include <set>
#include <system_error>
#include <utility>

#include <QTipUIKit/Internal/Helpers.h>

DatabaseManager::DatabaseManager(std::filesystem::path root)
    : _root(std::move(root)) {}

DatabaseManager::~DatabaseManager() {
    saveAll();
}

Database* DatabaseManager::get(int year, int month) {
    if (month < 1 || month > 12 || year < 1) {
        return nullptr;
    }

    const auto key = std::pair{year, month};

    if (auto it = _cache.find(key); it != _cache.end()) {
        return it->second->database.get();
    }

    const auto path = pathFor(year, month);

    if (!std::filesystem::is_regular_file(path)) {
        return nullptr;
    }

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
    if (month < 1 || month > 12 || year < 1) {
        return nullptr;
    }

    const auto it = _cache.find({year, month});
    if (it == _cache.end()) {
        return nullptr;
    }

    return it->second->database.get();
}

Database& DatabaseManager::getOrCreate(int year, int month) {
    if (auto* database = get(year, month)) {
        return *database;
    }

    auto entry = std::make_unique<Entry>(
        true,
        std::make_unique<Database>()
    );

    auto* result = entry->database.get();
    _cache.emplace(std::pair{year, month}, std::move(entry));

    return *result;
}

std::vector<int> DatabaseManager::availableYears() const {
    std::set<int> years;
    std::error_code error;

    if (std::filesystem::is_directory(_root, error) && !error) {
        std::filesystem::directory_iterator iterator(_root, error);
        const std::filesystem::directory_iterator end;

        while (!error && iterator != end) {
            std::error_code entryError;
            if (iterator->is_directory(entryError) && !entryError) {
                const std::string name =
                    iterator->path().filename().string();

                int year = 0;
                const auto [position, parseError] = std::from_chars(
                    name.data(),
                    name.data() + name.size(),
                    year
                );

                if (parseError == std::errc{} &&
                    position == name.data() + name.size() &&
                    year > 0) {
                    years.insert(year);
                }
            }

            iterator.increment(error);
        }
    }

    for (const auto& [key, entry] : _cache) {
        years.insert(key.first);
    }

    return {years.begin(), years.end()};
}

std::vector<int> DatabaseManager::availableMonths(const int year) const {
    if (year < 1) {
        return {};
    }

    std::set<int> months;
    const std::filesystem::path directory =
        _root / Detail::fmt("%04i", year);

    std::error_code error;

    if (std::filesystem::is_directory(directory, error) && !error) {
        std::filesystem::directory_iterator iterator(directory, error);
        const std::filesystem::directory_iterator end;

        while (!error && iterator != end) {
            std::error_code entryError;
            if (iterator->is_regular_file(entryError) && !entryError &&
                iterator->path().extension() == ".qbdb") {
                const std::string name = iterator->path().stem().string();

                int month = 0;
                const auto [position, parseError] = std::from_chars(
                    name.data(),
                    name.data() + name.size(),
                    month
                );

                if (parseError == std::errc{} &&
                    position == name.data() + name.size() &&
                    month >= 1 && month <= 12) {
                    months.insert(month);
                }
            }

            iterator.increment(error);
        }
    }

    for (const auto& [key, entry] : _cache) {
        if (key.first == year) {
            months.insert(key.second);
        }
    }

    return {months.begin(), months.end()};
}

void DatabaseManager::save(int year, int month) {
    auto* entry = getEntry(year, month);
    if (!entry) {
        return;
    }

    const auto path = pathFor(year, month);
    std::filesystem::create_directories(path.parent_path());

    entry->database->save(path);
    entry->dirty = false;
}

void DatabaseManager::saveAll() {
    for (const auto& [year, month] : _cache | std::views::keys) {
        save(year, month);
    }
}

bool DatabaseManager::unload(int year, int month) {
    const auto it = _cache.find({year, month});

    if (it == _cache.end() || it->second->dirty) {
        return false;
    }

    _cache.erase(it);
    return true;
}

void DatabaseManager::unloadAll() noexcept {
    for (const auto& [year, month] : _cache | std::views::keys) {
        unload(year, month);
    }
}

DatabaseManager::Entry* DatabaseManager::getEntry(
    const int year,
    const int month
) {
    const auto it = _cache.find({year, month});
    if (it == _cache.end()) {
        return nullptr;
    }

    return it->second.get();
}

std::filesystem::path DatabaseManager::pathFor(
    const int year,
    const int month
) const {
    return _root /
        Detail::fmt("%04i", year) /
        Detail::fmt("%02i.qbdb", month);
}