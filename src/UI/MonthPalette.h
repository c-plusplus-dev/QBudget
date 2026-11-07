#ifndef QBUDGET_UI_MONTHPALETTE_H
#define QBUDGET_UI_MONTHPALETTE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include <Q-Tip/QTip.h>

namespace MonthPalette {

inline constexpr std::array<std::string_view, 12> names{
    "January", "February", "March", "April",
    "May", "June", "July", "August",
    "September", "October", "November", "December"
};

inline constexpr std::array<std::string_view, 12> abbreviations{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

inline constexpr std::array<std::array<std::uint8_t, 3>, 12> rgb{{
    {163, 206, 241}, // January
    {245, 183, 177}, // February
    {169, 223, 191}, // March
    {247, 220, 111}, // April
    {125, 206, 160}, // May
    {248, 196, 113}, // June
    {236, 112, 99},  // July
    {243, 156, 18},  // August
    {133, 193, 233}, // September
    {214, 137, 16},  // October
    {165, 105, 189}, // November
    {84, 153, 199}   // December
}};

[[nodiscard]] inline QTip::Color color(const int month) noexcept {
    if (month < 1 || month > 12) {
        return {120, 120, 120};
    }

    const auto& value = rgb[static_cast<std::size_t>(month - 1)];
    return {value[0], value[1], value[2]};
}

} // namespace MonthPalette

#endif // QBUDGET_UI_MONTHPALETTE_H