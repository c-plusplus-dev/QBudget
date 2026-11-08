#include "DatabaseManager.h"

#include <algorithm>
#include <charconv>
#include <iostream>
#include <ranges>
#include <string>
#include <system_error>
#include <utility>

namespace {

bool parsePositiveInteger(const std::string& text, int& value) {
    if (text.empty()) {
        return false;
    }

    const auto [end, error] = std::from_chars(
        text.data(),
        text.data() + text.size(),
        value
    );

    return error == std::errc{} &&
        end == text.data() + text.size() &&
        value > 0;
}

void sortUnique(std::vector<int>& values) {
    std::ranges::sort(values);
    values.erase(
        std::unique(values.begin(), values.end()),
        values.end()
    );
}

} // namespace

DatabaseManager::DatabaseManager(std::filesystem::path root)
    : _root(std::move(root)) {}

DatabaseManager::~DatabaseManager() {
    try {
        saveAll();
    } catch (const std::exception& error) {
        std::cerr << "Failed to save QBudget databases during shutdown: "
                  << error.what() << '\n';
    } catch (...) {
        std::cerr << "Failed to save QBudget databases during shutdown.\n";
    }
}

Database* DatabaseManager::get(const int year, const int month) {
    if (year < 1 || month < 1 || month > 12) {
        return nullptr;
    }

    const auto key = std::pair{year, month};

    if (const auto it = _cache.find(key); it != _cache.end()) {
        return it->second->database.get();
    }

    const auto path = pathFor(year, month);
    std::error_code error;

    if (!std::filesystem::is_regular_file(path, error) || error) {
        return nullptr;
    }

    auto entry = std::make_unique<Entry>();
    entry->database = std::make_unique<Database>(
        Database::load(path)
    );

    Database* result = entry->database.get();
    _cache.emplace(key, std::move(entry));
    ++_revision;

    return result;
}

const Database* DatabaseManager::get(
    const int year,
    const int month
) const {
    if (year < 1 || month < 1 || month > 12) {
        return nullptr;
    }

    const auto it = _cache.find({year, month});
    if (it == _cache.end()) {
        return nullptr;
    }

    return it->second->database.get();
}

Database& DatabaseManager::getOrCreate(
    const int year,
    const int month
) {
    if (year < 1 || month < 1 || month > 12) {
        throw std::invalid_argument(
            "Database year and month must be valid"
        );
    }

    if (Database* database = get(year, month)) {
        return *database;
    }

    auto entry = std::make_unique<Entry>();
    entry->database = std::make_unique<Database>();

    Database* result = entry->database.get();
    _cache.emplace(std::pair{year, month}, std::move(entry));

    ++_revision;
    _directoryCacheValid = false;

    return *result;
}

void DatabaseManager::refreshDirectoryCache() const {
    if (_directoryCacheValid) {
        return;
    }

    _availableYears.clear();
    _availableMonths.clear();

    std::error_code error;
    if (std::filesystem::is_directory(_root, error) && !error) {
        std::filesystem::directory_iterator yearIterator(_root, error);
        const std::filesystem::directory_iterator end;

        while (!error && yearIterator != end) {
            std::error_code entryError;

            if (yearIterator->is_directory(entryError) && !entryError) {
                const std::string yearName =
                    yearIterator->path().filename().string();

                int year = 0;
                if (parsePositiveInteger(yearName, year)) {
                    _availableYears.push_back(year);
                    auto& months = _availableMonths[year];

                    std::error_code monthError;
                    std::filesystem::directory_iterator monthIterator(
                        yearIterator->path(),
                        monthError
                    );

                    while (!monthError && monthIterator != end) {
                        std::error_code fileError;

                        if (monthIterator->is_regular_file(fileError) &&
                            !fileError &&
                            monthIterator->path().extension() == ".qbdb") {
                            const std::string monthName =
                                monthIterator->path().stem().string();

                            int month = 0;
                            if (parsePositiveInteger(monthName, month) &&
                                month <= 12) {
                                months.push_back(month);
                            }
                        }

                        monthIterator.increment(monthError);
                    }
                }
            }

            yearIterator.increment(error);
        }
    }

    for (const auto& [key, entry] : _cache) {
        _availableYears.push_back(key.first);
        _availableMonths[key.first].push_back(key.second);
    }

    sortUnique(_availableYears);

    for (auto& [year, months] : _availableMonths) {
        sortUnique(months);
    }

    _directoryCacheValid = true;
}

std::vector<int> DatabaseManager::availableYears() const {
    refreshDirectoryCache();
    return _availableYears;
}

std::vector<int> DatabaseManager::availableMonths(const int year) const {
    if (year < 1) {
        return {};
    }

    refreshDirectoryCache();

    const auto it = _availableMonths.find(year);
    return it == _availableMonths.end()
        ? std::vector<int>{}
        : it->second;
}

std::uint64_t DatabaseManager::revision() const noexcept {
    std::uint64_t result = _revision;

    for (const auto& [key, entry] : _cache) {
        result += entry->database->revision();
    }

    return result;
}

void DatabaseManager::save(const int year, const int month) {
    Entry* entry = getEntry(year, month);
    if (!entry) {
        return;
    }

    const auto path = pathFor(year, month);
    std::filesystem::create_directories(path.parent_path());

    entry->database->save(path);
}

void DatabaseManager::saveAll() {
    for (const auto& [key, entry] : _cache) {
        const auto path = pathFor(key.first, key.second);
        std::filesystem::create_directories(path.parent_path());
        entry->database->save(path);
    }
}

bool DatabaseManager::unload(const int year, const int month) {
    const auto it = _cache.find({year, month});
    if (it == _cache.end()) {
        return false;
    }

    save(year, month);
    _cache.erase(it);
    ++_revision;
    _directoryCacheValid = false;
    return true;
}

void DatabaseManager::unloadAll() {
    saveAll();
    if (_cache.empty()) {
        return;
    }

    _cache.clear();
    ++_revision;
    _directoryCacheValid = false;
}

DatabaseManager::Entry* DatabaseManager::getEntry(
    const int year,
    const int month
) {
    const auto it = _cache.find({year, month});
    return it == _cache.end() ? nullptr : it->second.get();
}

std::filesystem::path DatabaseManager::pathFor(
    const int year,
    const int month
) const {
    std::string yearName = std::to_string(year);
    if (yearName.size() < 4) {
        yearName.insert(0, 4 - yearName.size(), '0');
    }

    std::string monthName = std::to_string(month);
    if (monthName.size() < 2) {
        monthName.insert(0, 2 - monthName.size(), '0');
    }

    return _root / yearName / (monthName + ".qbdb");
}
