//
// Created by Ayaan on 2026-10-15.
//

#ifndef QBUDGET_BLOCK_H
#define QBUDGET_BLOCK_H
#include <cstdint>
#include <string>
#include <vector>

class Reader;
class Writer;

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

    void writeTo(Writer& writer) const;
    static Block readFrom(Reader& reader);

    void remove(Item& item);
    void remove(std::size_t index);
};

#endif //QBUDGET_BLOCK_H
