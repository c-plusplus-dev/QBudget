#ifndef QBUDGET_DATABASEMANAGER_H
#define QBUDGET_DATABASEMANAGER_H

#include "Database.h"

#include <filesystem>
#include <map>
#include <memory>

class DatabaseManager {
public:
    explicit DatabaseManager(std::filesystem::path root);

    ~DatabaseManager();

    [[nodiscard]] Database* get(int year, int month);
    [[nodiscard]] const Database* get(int year, int month) const;

    void save(int year, int month);
    void saveAll();

    bool unload(int year, int month);
    void unloadAll() noexcept;

private:
    struct Entry {
        bool dirty;
        std::unique_ptr<Database> database;
    };

    Entry* getEntry(int year, int month);

    [[nodiscard]] std::filesystem::path pathFor(
        int year,
        int month
    ) const;

    std::filesystem::path _root;
    std::map<std::pair<int, int>, std::unique_ptr<Entry>> _cache;
};

#endif // QBUDGET_DATABASEMANAGER_H