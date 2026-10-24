//
// Created by Ayaan on 2026-10-15.
//

#ifndef QBUDGET_DATABASE_H
#define QBUDGET_DATABASE_H

#include "Block.h"

#include <filesystem>
#include <vector>

class Database {
public:
    Database() = default;

    [[nodiscard]] std::vector<std::unique_ptr<Block>>& blocks() noexcept;
    [[nodiscard]] const std::vector<std::unique_ptr<Block>>& blocks() const noexcept;

    [[nodiscard]] std::unique_ptr<Block>& addBlock(Block block);

    void clear() noexcept;
    void remove(std::unique_ptr<Block>& block);

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    void save(const std::filesystem::path& path) const;

    [[nodiscard]] std::int64_t total() const;

    [[nodiscard]] static Database load(
        const std::filesystem::path& path
    );

private:
    std::vector<std::unique_ptr<Block>> _blocks;
};

#endif // QBUDGET_DATABASE_H
