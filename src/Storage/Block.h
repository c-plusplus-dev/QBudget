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
        std::string name;
        std::int32_t cost = 0;

        void writeTo(Writer& writer) const;
        static Item readFrom(Reader& reader);
    };

    std::string name;
    std::vector<Item> items;

    [[nodiscard]] std::int64_t total() const;

    void writeTo(Writer& writer) const;
    static Block readFrom(Reader& reader);
};

#endif //QBUDGET_BLOCK_H
