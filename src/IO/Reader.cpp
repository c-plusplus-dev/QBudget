//
// Created by Ayaan on 2026-10-15.
//

#include "Reader.h"

#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

std::vector<std::byte> readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file) {
        throw std::runtime_error(
            "Failed to open file: " + path.string()
        );
    }

    const std::ifstream::pos_type end = file.tellg();

    if (end < 0) {
        throw std::runtime_error(
            "Failed to determine file size: " + path.string()
        );
    }

    const auto fileSize = static_cast<std::uintmax_t>(end);

    if (fileSize > std::numeric_limits<std::size_t>::max()
        || fileSize > static_cast<std::uintmax_t>(
            std::numeric_limits<std::streamsize>::max()
        )) {
        throw std::runtime_error("File is too large to read");
    }

    std::vector<std::byte> bytes(
        static_cast<std::size_t>(fileSize)
    );

    file.seekg(0, std::ios::beg);

    if (!file) {
        throw std::runtime_error(
            "Failed to seek to beginning of file: " + path.string()
        );
    }

    if (!bytes.empty()) {
        file.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size())
        );

        if (!file || file.gcount() != static_cast<std::streamsize>(bytes.size())) {
            throw std::runtime_error(
                "Failed to read complete file: " + path.string()
            );
        }
    }

    return bytes;
}

} // namespace

Reader::Reader(
    const std::filesystem::path& path,
    Endian endian
)
    : _bytes(readFile(path)),
      _endian(endian) {}

Reader::Reader(
    std::vector<std::byte> bytes,
    Endian endian
)
    : _bytes(std::move(bytes)),
      _endian(endian) {}

void Reader::require(std::size_t count) const {
    if (_position > _bytes.size()
        || count > _bytes.size() - _position) {
        throw std::runtime_error(
            "Attempted to read past end of binary data"
        );
    }
}

std::byte Reader::byte() {
    require(1);
    return _bytes[_position++];
}

std::uint8_t Reader::read8() {
    return std::to_integer<std::uint8_t>(byte());
}

std::int8_t Reader::readS8() {
    const std::uint8_t value = read8();

    if (value <= 0x7F) {
        return static_cast<std::int8_t>(value);
    }

    return static_cast<std::int8_t>(
        static_cast<std::int16_t>(value) - 256
    );
}

std::uint16_t Reader::read16() {
    const std::uint16_t b0 = read8();
    const std::uint16_t b1 = read8();

    if (_endian == Endian::Little) {
        return static_cast<std::uint16_t>(b0 | (b1 << 8));
    }

    return static_cast<std::uint16_t>((b0 << 8) | b1);
}

std::int16_t Reader::readS16() {
    const std::uint16_t value = read16();

    if (value <= 0x7FFF) {
        return static_cast<std::int16_t>(value);
    }

    return static_cast<std::int16_t>(
        static_cast<std::int32_t>(value) - 65536
    );
}

std::uint32_t Reader::read32() {
    std::uint32_t result = 0;

    if (_endian == Endian::Little) {
        for (unsigned int shift = 0; shift < 32; shift += 8) {
            result |= static_cast<std::uint32_t>(read8()) << shift;
        }
    } else {
        for (unsigned int shift = 0; shift < 32; shift += 8) {
            result = (result << 8) | read8();
        }
    }

    return result;
}

std::int32_t Reader::readS32() {
    const std::uint32_t value = read32();

    if (value <= 0x7FFFFFFF) {
        return static_cast<std::int32_t>(value);
    }

    const std::int64_t signedValue =
        static_cast<std::int64_t>(value) - 4294967296LL;

    return static_cast<std::int32_t>(signedValue);
}

std::uint64_t Reader::read64() {
    std::uint64_t result = 0;

    if (_endian == Endian::Little) {
        for (unsigned int shift = 0; shift < 64; shift += 8) {
            result |= static_cast<std::uint64_t>(read8()) << shift;
        }
    } else {
        for (unsigned int shift = 0; shift < 64; shift += 8) {
            result = (result << 8) | read8();
        }
    }

    return result;
}

std::int64_t Reader::readS64() {
    const std::uint64_t value = read64();

    if (value <= 0x7FFFFFFFFFFFFFFFULL) {
        return static_cast<std::int64_t>(value);
    }

    // Avoid converting an out-of-range unsigned integer directly
    // to a signed integer.
    return -1 - static_cast<std::int64_t>(~value);
}

std::vector<std::byte> Reader::readBytes(std::size_t count) {
    require(count);

    const auto first = _bytes.begin()
        + static_cast<std::vector<std::byte>::difference_type>(_position);

    const auto last = first
        + static_cast<std::vector<std::byte>::difference_type>(count);

    std::vector<std::byte> result(first, last);
    _position += count;

    return result;
}

std::string Reader::readString() {
    const std::uint64_t storedLength = read64();

    if (storedLength > std::numeric_limits<std::size_t>::max()) {
        throw std::length_error("String is too large");
    }

    auto length = static_cast<std::size_t>(storedLength);

    return readString(length);
}

std::string Reader::readString(std::size_t length) {
    require(length);

    std::string result(length, '\0');

    if (length != 0) {
        for (std::size_t i = 0; i < length; ++i) {
            result[i] = static_cast<char>(read8());
        }
    }

    return result;
}

void Reader::skipBytes(std::size_t count) {
    require(count);
    _position += count;
}

void Reader::goTo(std::size_t position) {
    if (position > _bytes.size()) {
        throw std::runtime_error(
            "Attempted to seek past end of binary data"
        );
    }

    _position = position;
}

std::size_t Reader::tell() const noexcept {
    return _position;
}

std::size_t Reader::size() const noexcept {
    return _bytes.size();
}

std::size_t Reader::remaining() const noexcept {
    return _bytes.size() - _position;
}

bool Reader::atEnd() const noexcept {
    return _position == _bytes.size();
}

Endian Reader::endian() const noexcept {
    return _endian;
}
