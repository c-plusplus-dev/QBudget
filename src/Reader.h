//
// Created by Ayaan on 2026-10-15.
//

#ifndef QBUDGET_READER_H
#define QBUDGET_READER_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

enum class Endian {
    Little,
    Big
};

class Reader {
public:
    explicit Reader(
        const std::filesystem::path& path,
        Endian endian = Endian::Little
    );

    explicit Reader(
        std::vector<std::byte> bytes,
        Endian endian = Endian::Little
    );

    [[nodiscard]] std::uint8_t read8();
    [[nodiscard]] std::int8_t readS8();

    [[nodiscard]] std::uint16_t read16();
    [[nodiscard]] std::int16_t readS16();

    [[nodiscard]] std::uint32_t read32();
    [[nodiscard]] std::int32_t readS32();

    [[nodiscard]] std::uint64_t read64();
    [[nodiscard]] std::int64_t readS64();

    [[nodiscard]] std::vector<std::byte> readBytes(std::size_t count);
    // if length is zero, it will read a uint64 for the length instead.
    [[nodiscard]] std::string readString(std::size_t length);

    void skipBytes(std::size_t count);
    void goTo(std::size_t position);

    [[nodiscard]] std::byte byte();

    [[nodiscard]] std::size_t tell() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::size_t remaining() const noexcept;
    [[nodiscard]] bool atEnd() const noexcept;

    [[nodiscard]] Endian endian() const noexcept;

private:
    void require(std::size_t count) const;

    std::vector<std::byte> _bytes;
    std::size_t _position = 0;
    Endian _endian;
};

#endif //QBUDGET_READER_H
