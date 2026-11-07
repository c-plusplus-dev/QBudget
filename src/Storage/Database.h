#ifndef QBUDGET_DATABASE_H
#define QBUDGET_DATABASE_H

#include "Block.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

class Database {
public:
    Database() = default;
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    [[nodiscard]] std::vector<std::unique_ptr<Block>>& blocks() noexcept;
    [[nodiscard]] const std::vector<std::unique_ptr<Block>>& blocks() const noexcept;

    [[nodiscard]] std::unique_ptr<Block>& addBlock(Block block);

    void clear() noexcept;
    void remove(Block& block);
    void markChanged() noexcept;

    [[nodiscard]] std::uint64_t revision() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    void save(const std::filesystem::path& path) const;
    [[nodiscard]] std::int64_t total() const;

    [[nodiscard]] static Database load(
        const std::filesystem::path& path
    );

private:
    std::vector<std::unique_ptr<Block>> _blocks;
    std::uint64_t _revision = 0;
    mutable std::int64_t _cachedTotal = 0;
    mutable std::uint64_t _cachedRevision = 0;
    mutable bool _totalDirty = true;
};

#endif // QBUDGET_DATABASE_H
