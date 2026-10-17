//
// Created by Ayaan on 2026-10-15.
//


#ifndef QBUDGET_WRITER_H
#define QBUDGET_WRITER_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

#include "Reader.h"

class Writer {
public:
    explicit Writer(Endian endian = Endian::Little);

    void write8(std::uint8_t value);
    void writeS8(std::int8_t value);

    void write16(std::uint16_t value);
    void writeS16(std::int16_t value);

    void write32(std::uint32_t value);
    void writeS32(std::int32_t value);

    void write64(std::uint64_t value);
    void writeS64(std::int64_t value);

    void writeBytes(std::span<const std::byte> bytes);
    void writeString(std::string_view value, bool writeLength = true);

    void writeZeros(std::size_t count);

    [[nodiscard]] std::size_t tell() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] Endian endian() const noexcept;

    [[nodiscard]] const std::vector<std::byte>& bytes() const noexcept;

    void save(const std::filesystem::path& path) const;

private:
    void writeUnsigned(std::uint64_t value, std::size_t byteCount);

    std::vector<std::byte> _bytes;
    Endian _endian;
};

#endif
