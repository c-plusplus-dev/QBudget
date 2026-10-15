//
// Created by Ayaan on 2026-10-15.
//

#include "Block.h"
#include "Reader.h"
#include "Writer.h"

void Block::Item::writeTo(Writer& writer) const {
    writer.writeString(name);
    writer.writeS32(cost);
}

Block::Item Block::Item::readFrom(Reader& reader) {
    Item item{};
    item.name = reader.readString(0);
    item.cost = reader.readS32();
    return item;
}

std::int64_t Block::total() const {
    std::int64_t sum = 0;
    for (const auto& item : items) {
        sum += item.cost;
    }
    return sum;
}

void Block::writeTo(Writer& writer) const {
    writer.writeString(name);
    writer.write64(static_cast<std::uint64_t>(items.size()));

    for (auto& item : items)
        item.writeTo(writer);
}

Block Block::readFrom(Reader& reader) {
    Block block{};
    block.name = reader.readString(0);
    uint64_t length = reader.read64();
    for (uint64_t i = 0; i < length; i++)
        block.items.push_back(std::move(Item::readFrom(reader)));
    return block;
}
