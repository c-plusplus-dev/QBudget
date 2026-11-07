#include "Database.h"

#include "IO/Reader.h"
#include "IO/Writer.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

constexpr std::uint8_t MAGIC[] = {'Q', 'B', 'D', 'B'};
constexpr std::uint32_t FORMAT_VERSION = 1;
constexpr std::size_t MAGIC_SIZE = sizeof(MAGIC);
constexpr std::size_t MINIMUM_BLOCK_SIZE = 16;

void checkedAdd(std::int64_t& total, const std::int64_t value) {
    if (value > 0 &&
        total > std::numeric_limits<std::int64_t>::max() - value) {
        throw std::overflow_error("Database total exceeds int64_t range");
    }

    if (value < 0 &&
        total < std::numeric_limits<std::int64_t>::min() - value) {
        throw std::overflow_error("Database total exceeds int64_t range");
    }

    total += value;
}

void writeMagic(Writer& writer) {
    for (const std::uint8_t byte : MAGIC) {
        writer.write8(byte);
    }
}

void readAndValidateMagic(Reader& reader) {
    for (const std::uint8_t expected : MAGIC) {
        if (reader.read8() != expected) {
            throw std::runtime_error(
                "Invalid QBudget database signature"
            );
        }
    }
}

} // namespace

Database::Database(Database&& other) noexcept
    : _blocks(std::move(other._blocks)),
      _revision(other._revision),
      _cachedTotal(other._cachedTotal),
      _cachedRevision(other._cachedRevision),
      _totalDirty(other._totalDirty) {
    for (auto& block : _blocks) {
        if (block) {
            block->_parent = this;
        }
    }

    other._revision = 0;
    other._cachedTotal = 0;
    other._cachedRevision = 0;
    other._totalDirty = true;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    _blocks = std::move(other._blocks);
    _revision = other._revision;
    _cachedTotal = other._cachedTotal;
    _cachedRevision = other._cachedRevision;
    _totalDirty = other._totalDirty;

    for (auto& block : _blocks) {
        if (block) {
            block->_parent = this;
        }
    }

    other._revision = 0;
    other._cachedTotal = 0;
    other._cachedRevision = 0;
    other._totalDirty = true;

    return *this;
}

std::vector<std::unique_ptr<Block>>& Database::blocks() noexcept {
    return _blocks;
}

const std::vector<std::unique_ptr<Block>>& Database::blocks() const noexcept {
    return _blocks;
}

std::unique_ptr<Block>& Database::addBlock(Block block) {
    _blocks.push_back(std::make_unique<Block>(std::move(block)));
    _blocks.back()->_parent = this;
    markChanged();
    return _blocks.back();
}

void Database::clear() noexcept {
    if (_blocks.empty()) {
        return;
    }

    for (auto& block : _blocks) {
        if (block) {
            block->_parent = nullptr;
        }
    }

    _blocks.clear();
    markChanged();
}

void Database::remove(Block& block) {
    const auto it = std::ranges::find_if(
        _blocks,
        [&block](const std::unique_ptr<Block>& candidate) {
            return candidate.get() == &block;
        }
    );

    if (it == _blocks.end()) {
        return;
    }

    (*it)->_parent = nullptr;
    _blocks.erase(it);
    markChanged();
}

void Database::markChanged() noexcept {
    ++_revision;
    _totalDirty = true;
}

std::uint64_t Database::revision() const noexcept {
    return _revision;
}

bool Database::empty() const noexcept {
    return _blocks.empty();
}

std::size_t Database::size() const noexcept {
    return _blocks.size();
}

void Database::save(const std::filesystem::path& path) const {
    if (_blocks.size() >
        std::numeric_limits<std::uint64_t>::max()) {
        throw std::length_error("Too many blocks to serialize");
    }

    Writer writer;
    writeMagic(writer);
    writer.write32(FORMAT_VERSION);
    writer.write64(static_cast<std::uint64_t>(_blocks.size()));

    for (const auto& block : _blocks) {
        if (!block) {
            throw std::logic_error("Cannot serialize a null block");
        }
        block->writeTo(writer);
    }

    writer.save(path);
}

std::int64_t Database::total() const {
    const std::uint64_t currentRevision = revision();
    if (!_totalDirty && _cachedRevision == currentRevision) {
        return _cachedTotal;
    }

    std::int64_t result = 0;
    for (const auto& block : _blocks) {
        if (block) {
            checkedAdd(result, block->total());
        }
    }

    _cachedTotal = result;
    _cachedRevision = currentRevision;
    _totalDirty = false;
    return result;
}

Database Database::load(const std::filesystem::path& path) {
    Reader reader(path);

    if (reader.remaining() < MAGIC_SIZE) {
        throw std::runtime_error(
            "Database file is too small to contain a valid header"
        );
    }

    readAndValidateMagic(reader);

    const std::uint32_t version = reader.read32();
    if (version != FORMAT_VERSION) {
        throw std::runtime_error(
            "Unsupported QBudget database format version: " +
            std::to_string(version)
        );
    }

    const std::uint64_t blockCount = reader.read64();
    if (blockCount > std::numeric_limits<std::size_t>::max()) {
        throw std::runtime_error("Database contains too many blocks");
    }

    Database database;
    if (blockCount > database._blocks.max_size()) {
        throw std::runtime_error(
            "Database contains more blocks than can be stored in memory"
        );
    }

    if (blockCount > reader.remaining() / MINIMUM_BLOCK_SIZE) {
        throw std::runtime_error(
            "Database contains an invalid block count or truncated data"
        );
    }

    database._blocks.reserve(static_cast<std::size_t>(blockCount));

    for (std::uint64_t i = 0; i < blockCount; ++i) {
        auto block = std::make_unique<Block>(Block::readFrom(reader));
        block->_parent = &database;
        database._blocks.push_back(std::move(block));
    }

    if (!reader.atEnd()) {
        throw std::runtime_error(
            "Database contains unexpected trailing data"
        );
    }

    return database;
}
