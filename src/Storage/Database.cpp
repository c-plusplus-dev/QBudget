//
// Created by Ayaan on 2026-10-15.
//

#include "Database.h"

#include "IO/Reader.h"
#include "IO/Writer.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

    constexpr std::uint8_t MAGIC[] = {
        'Q', 'B', 'D', 'B'
    };

    constexpr std::uint32_t FORMAT_VERSION = 1;

    constexpr std::size_t MAGIC_SIZE = sizeof(MAGIC);

    // Every serialized block requires at least:
    // - 8 bytes for its name length
    // - 8 bytes for its item count
    constexpr std::size_t MINIMUM_BLOCK_SIZE = 16;

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

}

std::vector<Block>& Database::blocks() noexcept {
    return _blocks;
}

const std::vector<Block>& Database::blocks() const noexcept {
    return _blocks;
}

Block& Database::addBlock(Block block) {
    _blocks.push_back(std::move(block));
    return _blocks.back();
}

void Database::clear() noexcept {
    _blocks.clear();
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
        throw std::length_error(
            "Too many blocks to serialize"
        );
    }

    Writer writer;

    // File header.
    writeMagic(writer);
    writer.write32(FORMAT_VERSION);
    writer.write64(static_cast<std::uint64_t>(_blocks.size()));

    // File contents.
    for (const Block& block : _blocks) {
        block.writeTo(writer);
    }

    // Writer handles writing through a temporary file.
    writer.save(path);
}

std::int64_t Database::total() const {
    std::int64_t total = 0;
    for (auto& block : _blocks) {
        total += block.total();
    }
    return total;
}

Database Database::load(const std::filesystem::path& path) {
    Reader reader(path);

    // Validate the file signature before interpreting its contents.
    if (reader.remaining() < MAGIC_SIZE) {
        throw std::runtime_error(
            "Database file is too small to contain a valid header"
        );
    }

    readAndValidateMagic(reader);

    // Validate the file format version.
    const std::uint32_t version = reader.read32();

    if (version != FORMAT_VERSION) {
        throw std::runtime_error(
            "Unsupported QBudget database format version: " +
            std::to_string(version)
        );
    }

    // Read and validate the number of blocks.
    const std::uint64_t blockCount = reader.read64();

    if (blockCount > std::numeric_limits<std::size_t>::max()) {
        throw std::runtime_error(
            "Database contains too many blocks"
        );
    }

    Database database;

    if (blockCount > database._blocks.max_size()) {
        throw std::runtime_error(
            "Database contains more blocks than can be stored in memory"
        );
    }

    // Reject impossible counts before attempting allocations or reading
    // individual blocks. Every block needs at least 16 bytes.
    if (blockCount > reader.remaining() / MINIMUM_BLOCK_SIZE) {
        throw std::runtime_error(
            "Database contains an invalid block count or truncated data"
        );
    }

    database._blocks.reserve(static_cast<std::size_t>(blockCount));

    for (std::uint64_t i = 0; i < blockCount; ++i) {
        database._blocks.push_back(Block::readFrom(reader));
    }

    // Do not silently accept garbage appended to the database.
    if (!reader.atEnd()) {
        throw std::runtime_error(
            "Database contains unexpected trailing data"
        );
    }

    return database;
}
