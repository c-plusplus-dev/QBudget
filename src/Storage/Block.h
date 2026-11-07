#ifndef QBUDGET_BLOCK_H
#define QBUDGET_BLOCK_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Reader;
class Writer;
class Database;

class Block {
public:
    class Item {
    public:
        Item() = default;
        Item(Item&& item) noexcept;
        Item(std::string name, std::int32_t cost);

        std::string name;
        std::int32_t cost = 0;

        void writeTo(Writer& writer) const;
        static Item readFrom(Reader& reader);

        Block* parent = nullptr;
    };

    Block() = default;
    Block(Block&& block) noexcept;
    Block(std::string name, std::vector<std::unique_ptr<Item>> items);

    std::string name;
    std::vector<std::unique_ptr<Item>> items;

    [[nodiscard]] std::int64_t total() const;
    [[nodiscard]] std::uint64_t revision() const noexcept;
    void markChanged() noexcept;

    void writeTo(Writer& writer) const;
    static Block readFrom(Reader& reader);

    void remove(Item& item);
    void remove(std::size_t index);

private:
    friend class Database;

    Database* _parent = nullptr;
    mutable std::int64_t _cachedTotal = 0;
    mutable bool _totalDirty = true;
    std::uint64_t _revision = 0;
};

#endif // QBUDGET_BLOCK_H
