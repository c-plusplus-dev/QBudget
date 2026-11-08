#ifndef QBUDGET_DATABASEMANAGER_H
#define QBUDGET_DATABASEMANAGER_H

#include "Database.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <vector>

class DatabaseManager {
public:
    explicit DatabaseManager(std::filesystem::path root);
    ~DatabaseManager();

    [[nodiscard]] Database* get(int year, int month);
    [[nodiscard]] const Database* get(int year, int month) const;
    [[nodiscard]] Database& getOrCreate(int year, int month);

    [[nodiscard]] std::vector<int> availableYears() const;
    [[nodiscard]] std::vector<int> availableMonths(int year) const;
    [[nodiscard]] std::uint64_t revision() const noexcept;

    void save(int year, int month);
    void saveAll();

    bool unload(int year, int month);
    void unloadAll();

private:
    struct Entry {
        std::unique_ptr<Database> database;
    };

    void refreshDirectoryCache() const;

    [[nodiscard]] Entry* getEntry(int year, int month);
    [[nodiscard]] std::filesystem::path pathFor(
        int year,
        int month
    ) const;

    std::filesystem::path _root;
    std::map<std::pair<int, int>, std::unique_ptr<Entry>> _cache;
    std::uint64_t _revision = 0;

    mutable bool _directoryCacheValid = false;
    mutable std::vector<int> _availableYears;
    mutable std::map<int, std::vector<int>> _availableMonths;
};

#endif // QBUDGET_DATABASEMANAGER_H
