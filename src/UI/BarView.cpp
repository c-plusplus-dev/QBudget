#include "UI/BarView.h"

#include "Helpers.h"
#include "UI/MonthPalette.h"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <string>
#include <utility>

using namespace QTip;

namespace {

constexpr float kPadding = 12.0f;

std::string compactValue(const std::int64_t value) {
    if (value >= 1'000'000) {
        return std::to_string(value / 1'000'000) + "m";
    }

    if (value >= 10'000) {
        return std::to_string(value / 1'000) + "k";
    }

    return std::to_string(value);
}

void drawBlockArrow(
    Window& window,
    const Font& font,
    const Rect& rect,
    const char* label,
    const bool enabled,
    const bool hovered
) {
    const Color background = !enabled
        ? Color{25, 25, 25}
        : hovered
            ? Color{55, 55, 55}
            : Color{40, 40, 40};

    window->setRenderColor(background);
    window->renderRoundedRect(rect, 7);

    window->renderText(
        font,
        label,
        rect.origin.x + 9.0f,
        rect.origin.y + 5.0f,
        enabled ? Color::white : Color{90, 90, 90}
    );
}

} // namespace

BarView::BarView(
    const Rect rect,
    DatabaseManager& manager,
    Font& font,
    int& selectedYear
)
    : _manager(manager),
      _font(font),
      _selectedYear(selectedYear) {
    _rect = rect;
    _preferredRect = rect;

    refreshData();
    layout();
}

void BarView::layout() {
    const float width = std::max(0.0f, _rect.size.x);

    _previousBlockRect = {
        _rect.origin.x + kPadding,
        _rect.origin.y + 12.0f,
        32.0f,
        32.0f
    };

    _nextBlockRect = {
        _rect.origin.x + width - kPadding - 32.0f,
        _rect.origin.y + 12.0f,
        32.0f,
        32.0f
    };
}

void BarView::refreshData() {
    const int year = _selectedYear;

    std::array<Database*, 12> databases{};
    std::array<bool, 12> available{};
    std::vector<std::string> names;

    for (const int month : _manager.availableMonths(year)) {
        if (month < 1 || month > 12) {
            continue;
        }

        Database* database = _manager.get(year, month);
        if (!database) {
            continue;
        }

        const std::size_t index = static_cast<std::size_t>(month - 1);
        databases[index] = database;
        available[index] = true;

        for (const auto& block : database->blocks()) {
            if (block->name.empty() ||
                std::ranges::find(names, block->name) != names.end()) {
                continue;
            }

            names.push_back(block->name);
        }
    }

    const std::string previousSelection = _selectedBlock;

    if (std::ranges::find(names, _selectedBlock) == names.end()) {
        _selectedBlock = names.empty() ? std::string{} : names.front();
    }

    std::array<std::int64_t, 12> values{};
    std::array<bool, 12> hasBlock{};

    if (!_selectedBlock.empty()) {
        for (std::size_t month = 0; month < databases.size(); ++month) {
            Database* database = databases[month];
            if (!database) {
                continue;
            }

            const auto block = std::ranges::find_if(
                database->blocks(),
                [this](const std::unique_ptr<Block>& candidate) {
                    return candidate->name == _selectedBlock;
                }
            );

            if (block != database->blocks().end()) {
                hasBlock[month] = true;
                values[month] = (*block)->total();
            }
        }
    }

    const bool changed =
        _cachedYear != year ||
        _blockNames != names ||
        _selectedBlock != previousSelection ||
        _values != values ||
        _availableMonths != available ||
        _hasBlock != hasBlock;

    _blockNames = std::move(names);
    _values = values;
    _availableMonths = available;
    _hasBlock = hasBlock;
    _cachedYear = year;

    if (changed) {
        _barProgress = 0.0f;
    }
}

void BarView::selectBlock(const int direction) {
    if (direction == 0 || _blockNames.empty()) {
        return;
    }

    const auto current = std::ranges::find(_blockNames, _selectedBlock);
    if (current == _blockNames.end()) {
        return;
    }

    const auto index = static_cast<std::ptrdiff_t>(
        current - _blockNames.begin()
    );
    const auto next = index + direction;

    if (next < 0 ||
        next >= static_cast<std::ptrdiff_t>(_blockNames.size())) {
        return;
    }

    _selectedBlock = _blockNames[static_cast<std::size_t>(next)];
    refreshData();
}

bool BarView::handleEvent(const Event& event) {
    if (event.type == EventType::MouseMoved) {
        _mousePosition = event.mouseMove.position;
        return true;
    }

    if (event.type != EventType::MouseButtonReleased ||
        event.mouseButton.button != MouseButton::Left) {
        return true;
    }

    const Point position = event.mouseButton.position;
    _mousePosition = position;

    const auto current = std::ranges::find(_blockNames, _selectedBlock);
    if (current == _blockNames.end()) {
        return true;
    }

    const auto index = current - _blockNames.begin();

    if (_previousBlockRect.isPointInside(position) && index > 0) {
        selectBlock(-1);
        return false;
    }

    if (_nextBlockRect.isPointInside(position) &&
        index + 1 < static_cast<std::ptrdiff_t>(_blockNames.size())) {
        selectBlock(1);
        return false;
    }

    return true;
}

void BarView::tick(Clock&, const double dt) {
    const float safeDt = static_cast<float>(
        std::clamp(dt, 0.0, 0.05)
    );

    _barProgress += (1.0f - _barProgress) *
        (1.0f - std::exp(-10.0f * safeDt));

    _refreshTimer += safeDt;
    if (_refreshTimer >= 0.5f) {
        _refreshTimer = 0.0f;
        refreshData();
    }
}

Point BarView::minimumSize() const {
    return {280.0f, 220.0f};
}

Point BarView::preferredSize() const {
    return _preferredRect.size;
}

void BarView::resize(const Point size) {
    setRect({_rect.origin, size});
}

void BarView::reposition(const Point position) {
    setRect({position, _rect.size});
}

void BarView::setRect(const Rect rect) {
    _rect = rect;
    _preferredRect = rect;
    layout();
}

const Rect& BarView::rect() {
    return _rect;
}

void BarView::renderImpl(Window& window) {
    const auto current = std::ranges::find(_blockNames, _selectedBlock);
    const bool hasSelection = current != _blockNames.end();
    const auto index = hasSelection
        ? current - _blockNames.begin()
        : std::ptrdiff_t{0};

    const bool canGoPrevious = hasSelection && index > 0;
    const bool canGoNext = hasSelection &&
        index + 1 < static_cast<std::ptrdiff_t>(_blockNames.size());

    drawBlockArrow(
        window,
        _font,
        _previousBlockRect,
        "<",
        canGoPrevious,
        canGoPrevious && _previousBlockRect.isPointInside(_mousePosition)
    );

    drawBlockArrow(
        window,
        _font,
        _nextBlockRect,
        ">",
        canGoNext,
        canGoNext && _nextBlockRect.isPointInside(_mousePosition)
    );

    const std::string title = hasSelection
        ? _selectedBlock
        : "No blocks found";

    const float titleWidth = static_cast<float>(title.size()) * 9.6f;
    window->renderText(
        _font,
        title.c_str(),
        _rect.origin.x + (_rect.size.x - titleWidth) * 0.5f,
        _rect.origin.y + 11.0f,
        hasSelection ? Color::white : Color{150, 150, 150}
    );

    const std::string yearText = "Year: " + std::to_string(_selectedYear);
    const float yearWidth = static_cast<float>(yearText.size()) * 9.6f;

    window->renderText(
        _font,
        yearText.c_str(),
        _rect.origin.x + (_rect.size.x - yearWidth) * 0.5f,
        _rect.origin.y + 35.0f,
        Color{160, 160, 160}
    );

    const Rect chartRect{
        _rect.origin.x + kPadding,
        _rect.origin.y + 66.0f,
        std::max(0.0f, _rect.size.x - 2.0f * kPadding),
        std::max(0.0f, _rect.size.y - 78.0f)
    };

    window->setRenderColor({28, 28, 28});
    window->renderRoundedRect(chartRect, 10);

    if (!hasSelection || chartRect.size.x < 100.0f ||
        chartRect.size.y < 100.0f) {
        const std::string message = hasSelection
            ? "Window is too small to display the chart"
            : "Create blocks in this year to compare them here";

        const float messageWidth =
            static_cast<float>(message.size()) * 9.6f;

        window->renderText(
            _font,
            message.c_str(),
            chartRect.origin.x +
                (chartRect.size.x - messageWidth) * 0.5f,
            chartRect.origin.y + chartRect.size.y * 0.5f,
            Color{145, 145, 145}
        );
        return;
    }

    constexpr float leftInset = 60.0f;
    constexpr float rightInset = 12.0f;
    constexpr float topInset = 28.0f;
    constexpr float bottomInset = 42.0f;

    const float plotLeft = chartRect.origin.x + leftInset;
    const float plotRight = chartRect.origin.x +
        chartRect.size.x - rightInset;
    const float plotTop = chartRect.origin.y + topInset;
    const float plotBottom = chartRect.origin.y +
        chartRect.size.y - bottomInset;

    const float plotWidth = std::max(1.0f, plotRight - plotLeft);
    const float plotHeight = std::max(1.0f, plotBottom - plotTop);

    std::int64_t maximum = 0;
    for (const std::int64_t value : _values) {
        maximum = std::max(maximum, value);
    }
    maximum = std::max<std::int64_t>(maximum, 1);

    for (int line = 0; line <= 4; ++line) {
        const float ratio = static_cast<float>(line) / 4.0f;
        const float y = plotBottom - ratio * plotHeight;

        window->setRenderColor({65, 65, 65});
        DrawThickLineHV(
            window,
            plotLeft,
            y,
            plotRight,
            y,
            line == 0 ? 2.0f : 1.0f
        );

        const auto labelValue = static_cast<std::int64_t>(
            static_cast<double>(maximum) * ratio
        );
        const std::string label = compactValue(labelValue);

        window->renderText(
            _font,
            label.c_str(),
            chartRect.origin.x + 8.0f,
            y - 8.0f,
            Color{150, 150, 150}
        );
    }

    const float columnWidth = plotWidth / 12.0f;
    const float barWidth = std::max(
        3.0f,
        std::min(34.0f, columnWidth * 0.62f)
    );

    for (std::size_t month = 0; month < 12; ++month) {
        const float columnX =
            plotLeft + static_cast<float>(month) * columnWidth;
        const float value = static_cast<float>(
            static_cast<double>(std::max<std::int64_t>(0, _values[month])) /
            static_cast<double>(maximum)
        );
        const float barHeight =
            value * plotHeight * std::clamp(_barProgress, 0.0f, 1.0f);

        const float barX = columnX + (columnWidth - barWidth) * 0.5f;
        const float barY = plotBottom - barHeight;

        if (_availableMonths[month] && _hasBlock[month] &&
            barHeight > 0.5f) {
            window->setRenderColor(
                MonthPalette::color(static_cast<int>(month) + 1)
            );

            window->renderRoundedRect(
                {barX, barY, barWidth, barHeight},
                std::min(5.0f, barWidth * 0.2f)
            );

            const std::string valueLabel =
                compactValue(_values[month]);

            const float estimatedWidth =
                static_cast<float>(valueLabel.size()) * 9.6f;

            if (estimatedWidth <= columnWidth + 8.0f) {
                window->renderText(
                    _font,
                    valueLabel.c_str(),
                    columnX + (columnWidth - estimatedWidth) * 0.5f,
                    std::max(plotTop - 19.0f, barY - 20.0f),
                    Color{205, 205, 205}
                );
            }
        }

        const std::string monthLabel{
            MonthPalette::abbreviations[month]
        };
        const float labelWidth =
            static_cast<float>(monthLabel.size()) * 9.6f;

        window->renderText(
            _font,
            monthLabel.c_str(),
            columnX + (columnWidth - labelWidth) * 0.5f,
            plotBottom + 12.0f,
            _availableMonths[month]
                ? Color{205, 205, 205}
                : Color{95, 95, 95}
        );
    }
}