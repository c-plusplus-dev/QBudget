//
// Created by Ayaan on 2026-10-15.
//


#include "Writer.h"

#include <fstream>
#include <limits>
#include <stdexcept>

Writer::Writer(Endian endian)
    : _endian(endian) {}

void Writer::writeUnsigned(
    std::uint64_t value,
    std::size_t byteCount
) {
    if (byteCount > sizeof(value)) {
        throw std::invalid_argument(
            "Integer byte count exceeds 64 bits"
        );
    }

    if (_endian == Endian::Little) {
        for (std::size_t i = 0; i < byteCount; ++i) {
            const auto shift = static_cast<unsigned int>(i * 8);

            write8(static_cast<std::uint8_t>(value >> shift));
        }
    } else {
        for (std::size_t i = byteCount; i > 0; --i) {
            const auto shift = static_cast<unsigned int>((i - 1) * 8);

            write8(static_cast<std::uint8_t>(value >> shift));
        }
    }
}

void Writer::write8(std::uint8_t value) {
    _bytes.push_back(static_cast<std::byte>(value));
}

void Writer::writeS8(std::int8_t value) {
    write8(static_cast<std::uint8_t>(value));
}

void Writer::write16(std::uint16_t value) {
    writeUnsigned(value, 2);
}

void Writer::writeS16(std::int16_t value) {
    write16(static_cast<std::uint16_t>(value));
}

void Writer::write32(std::uint32_t value) {
    writeUnsigned(value, 4);
}

void Writer::writeS32(std::int32_t value) {
    write32(static_cast<std::uint32_t>(value));
}

void Writer::write64(std::uint64_t value) {
    writeUnsigned(value, 8);
}

void Writer::writeS64(std::int64_t value) {
    write64(static_cast<std::uint64_t>(value));
}

void Writer::writeBytes(std::span<const std::byte> bytes) {
    if (bytes.size() > _bytes.max_size() - _bytes.size()) {
        throw std::length_error("Binary data is too large");
    }

    _bytes.insert(_bytes.end(), bytes.begin(), bytes.end());
}


void Writer::writeString(std::string_view value, bool writeLength) {
    constexpr std::size_t lengthSize = sizeof(std::uint64_t);

    if (writeLength &&
        value.size() > std::numeric_limits<std::uint64_t>::max()) {
        throw std::length_error("String is too long to serialize");
        }

    const std::size_t prefixSize = writeLength ? lengthSize : 0;

    if (prefixSize > _bytes.max_size() - _bytes.size() ||
        value.size() > _bytes.max_size() - _bytes.size() - prefixSize) {
        throw std::length_error("Binary data is too large");
        }

    if (writeLength) {
        write64(static_cast<std::uint64_t>(value.size()));
    }

    for (const char character : value) {
        write8(static_cast<std::uint8_t>(
            static_cast<unsigned char>(character)
        ));
    }
}


void Writer::writeZeros(std::size_t count) {
    if (count > _bytes.max_size() - _bytes.size()) {
        throw std::length_error("Binary data is too large");
    }

    _bytes.resize(_bytes.size() + count, std::byte{0});
}

std::size_t Writer::tell() const noexcept {
    return _bytes.size();
}

std::size_t Writer::size() const noexcept {
    return _bytes.size();
}

Endian Writer::endian() const noexcept {
    return _endian;
}

const std::vector<std::byte>& Writer::bytes() const noexcept {
    return _bytes;
}

void Writer::save(const std::filesystem::path& path) const {
    if (_bytes.size() >
        static_cast<std::size_t>(
            std::numeric_limits<std::streamsize>::max()
        )) {
        throw std::runtime_error("Binary data is too large to save");
    }

    if (path.empty()) {
        throw std::invalid_argument("Output path cannot be empty");
    }

    const std::filesystem::path temporaryPath =
        path.parent_path() /
        (path.filename().string() + ".tmp");

    // Write to a temporary file instead of immediately truncating
    // the destination file.
    {
        std::ofstream file(
            temporaryPath,
            std::ios::binary | std::ios::trunc
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to open temporary file for writing: " +
                temporaryPath.string()
            );
        }

        if (!_bytes.empty()) {
            file.write(
                reinterpret_cast<const char*>(_bytes.data()),
                static_cast<std::streamsize>(_bytes.size())
            );
        }

        file.close();

        if (!file) {
            std::error_code error;
            std::filesystem::remove(temporaryPath, error);

            throw std::runtime_error(
                "Failed to write complete temporary file: " +
                temporaryPath.string()
            );
        }
    }

    // Replace the destination only after the temporary file is complete.
    std::error_code error;

    std::filesystem::rename(temporaryPath, path, error);

    if (error) {
        std::error_code cleanupError;
        std::filesystem::remove(temporaryPath, cleanupError);

        throw std::runtime_error(
            "Failed to replace destination file '" +
            path.string() + "': " + error.message()
        );
    }
}
