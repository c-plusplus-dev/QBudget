//
// Created by Ayaan on 2026-10-15.
//

#include "Block.h"
#include "IO/Reader.h"
#include "IO/Writer.h"

Block::Item::Item(Item&& item) noexcept {
    name = std::move(item.name);
    cost = item.cost;
    parent = item.parent;
}

Block::Item::Item(std::string name, std::int32_t cost) {
    this->name = std::move(name);
    this->cost = cost;
}

void Block::Item::writeTo(Writer& writer) const {
    writer.writeString(name);
    writer.writeS32(cost);
}

Block::Item Block::Item::readFrom(Reader& reader) {
    Item item{};
    item.name = reader.readString();
    item.cost = reader.readS32();
    return item;
}

// bookmark: block

Block::Block(Block&& block) noexcept
    : items(std::move(block.items)),
      name(std::move(block.name)) {
    for (auto& item : items) {
        item->parent = this;
    }
}

Block::Block(std::string name, std::vector<std::unique_ptr<Item>> items) {
    this->items = std::move(items);
    this->name = std::move(name);
}

std::int64_t Block::total() const {
    std::int64_t sum = 0;
    for (const auto& item : items) {
        sum += item->cost;
    }
    return sum;
}

void Block::writeTo(Writer& writer) const {
    writer.writeString(name);
    writer.write64(static_cast<std::uint64_t>(items.size()));

    for (auto& item : items)
        item->writeTo(writer);
}

Block Block::readFrom(Reader& reader) {
    Block block{};
    block.name = reader.readString();

    const std::uint64_t length = reader.read64();
    if (length > block.items.max_size()) {
        throw std::runtime_error("Block contains too many items");
    }

    block.items.reserve(static_cast<std::size_t>(length));
    for (std::uint64_t i = 0; i < length; ++i) {
        block.items.push_back(
            std::make_unique<Item>(Item::readFrom(reader))
        );
    }

    for (auto& item : block.items) {
        item->parent = &block;
    }

    return block;
}

void Block::remove(Item& item) {
    auto it = std::ranges::find_if(items, [&item] (const std::unique_ptr<Item>& candidate) {
        return &item == candidate.get();
    });

    if (it == items.end())
        return;

    items.erase(it);
}

void Block::remove(std::size_t index) {
    if (index < items.size()) {
        items.erase(std::next(
            items.begin(),
            static_cast<std::ptrdiff_t>(index)
        ));
    }
}
