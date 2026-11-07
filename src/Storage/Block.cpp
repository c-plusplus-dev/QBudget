#include "Block.h"

#include "IO/Reader.h"
#include "IO/Writer.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

constexpr std::uint64_t kMinimumSerializedItemSize = 12;

void checkedAdd(std::int64_t& total, const std::int32_t value) {
    if (value > 0 &&
        total > std::numeric_limits<std::int64_t>::max() - value) {
        throw std::overflow_error("Block total exceeds int64_t range");
    }

    if (value < 0 &&
        total < std::numeric_limits<std::int64_t>::min() - value) {
        throw std::overflow_error("Block total exceeds int64_t range");
    }

    total += value;
}

} // namespace

Block::Item::Item(Item&& item) noexcept
    : name(std::move(item.name)),
      cost(item.cost),
      parent(item.parent) {}

Block::Item::Item(std::string name, const std::int32_t cost)
    : name(std::move(name)),
      cost(cost) {}

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

Block::Block(Block&& block) noexcept
    : name(std::move(block.name)),
      items(std::move(block.items)) {
    for (auto& item : items) {
        item->parent = this;
    }
}

Block::Block(
    std::string name,
    std::vector<std::unique_ptr<Item>> items
)
    : name(std::move(name)),
      items(std::move(items)) {
    for (auto& item : this->items) {
        if (item) {
            item->parent = this;
        }
    }
}

std::int64_t Block::total() const {
    if (!_totalDirty) {
        return _cachedTotal;
    }

    std::int64_t sum = 0;
    for (const auto& item : items) {
        if (item) {
            checkedAdd(sum, item->cost);
        }
    }

    _cachedTotal = sum;
    _totalDirty = false;
    return sum;
}

void Block::markChanged() noexcept {
    _totalDirty = true;
}

void Block::writeTo(Writer& writer) const {
    writer.writeString(name);
    writer.write64(static_cast<std::uint64_t>(items.size()));

    for (const auto& item : items) {
        item->writeTo(writer);
    }
}

Block Block::readFrom(Reader& reader) {
    Block block{};
    block.name = reader.readString();

    const std::uint64_t length = reader.read64();
    if (length > block.items.max_size()) {
        throw std::runtime_error("Block contains too many items");
    }

    if (length > reader.remaining() / kMinimumSerializedItemSize) {
        throw std::runtime_error(
            "Block contains an invalid item count or truncated data"
        );
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
    const auto it = std::ranges::find_if(
        items,
        [&item](const std::unique_ptr<Item>& candidate) {
            return candidate.get() == &item;
        }
    );

    if (it == items.end()) {
        return;
    }

    items.erase(it);
    markChanged();
}

void Block::remove(const std::size_t index) {
    if (index >= items.size()) {
        return;
    }

    items.erase(items.begin() + static_cast<std::ptrdiff_t>(index));
    markChanged();
}
