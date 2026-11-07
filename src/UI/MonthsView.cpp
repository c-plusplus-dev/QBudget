#include "UI/MonthsView.h"

#include "UI/MonthPalette.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

using namespace QTip;

namespace {

constexpr float kSidePadding = 18.0f;
constexpr float kHeaderHeight = 58.0f;
constexpr float kTileGap = 12.0f;

void drawYearButton(
    Window& window,
    const Font& font,
    const Rect& rect,
    const char* label,
    const bool enabled,
    const bool hovered
) {
    const Color background = !enabled
        ? Color{24, 24, 24}
        : hovered
            ? Color{55, 55, 55}
            : Color{38, 38, 38};

    window->setRenderColor(background);
    window->renderRoundedRect(rect, 7);

    window->renderText(
        font,
        label,
        rect.origin.x + rect.size.x * 0.5f - 4.8f,
        rect.origin.y + 5.0f,
        enabled ? Color::white : Color{100, 100, 100}
    );
}

} // namespace

MonthsView::MonthsView(
    const Rect rect,
    DatabaseManager& manager,
    Font& font,
    int& selectedMonth,
    int& selectedYear,
    std::function<void()> selectionChanged
)
    : _manager(manager),
      _font(font),
      _selectedMonth(selectedMonth),
      _selectedYear(selectedYear),
      _selectionChanged(std::move(selectionChanged)) {
    _rect = rect;
    _preferredRect = rect;

    refreshData();
    layout();
}

void MonthsView::refreshData() {
    _years = _manager.availableYears();

    if (std::ranges::find(_years, _selectedYear) == _years.end()) {
        _years.push_back(_selectedYear);
        std::ranges::sort(_years);
    }

    for (auto& tile : _tiles) {
        tile.available = false;
        tile.total = 0;
    }

    for (const int month : _manager.availableMonths(_selectedYear)) {
        if (month < 1 || month > 12) {
            continue;
        }

        Database* database = _manager.get(_selectedYear, month);
        if (!database) {
            continue;
        }

        MonthTile& tile = _tiles[static_cast<std::size_t>(month - 1)];
        tile.available = true;
        tile.total = database->total();
    }
}

void MonthsView::layout() {
    const float width = std::max(0.0f, _rect.size.x);
    const float height = std::max(0.0f, _rect.size.y);

    _previousYearRect = {
        _rect.origin.x + width - 116.0f,
        _rect.origin.y + 12.0f,
        32.0f,
        32.0f
    };

    _nextYearRect = {
        _rect.origin.x + width - 38.0f,
        _rect.origin.y + 12.0f,
        32.0f,
        32.0f
    };

    const float gridX = _rect.origin.x + kSidePadding;
    const float gridY = _rect.origin.y + kHeaderHeight;
    const float gridWidth = std::max(0.0f, width - 2.0f * kSidePadding);
    const float gridHeight = std::max(0.0f, height - kHeaderHeight - 16.0f);

    const float tileWidth =
        std::max(0.0f, (gridWidth - 2.0f * kTileGap) / 3.0f);
    const float tileHeight =
        std::max(48.0f, (gridHeight - 3.0f * kTileGap) / 4.0f);

    for (std::size_t i = 0; i < _tiles.size(); ++i) {
        const auto column = static_cast<float>(i % 3);
        const auto row = static_cast<float>(i / 3);

        _tiles[i].rect = {
            gridX + column * (tileWidth + kTileGap),
            gridY + row * (tileHeight + kTileGap),
            tileWidth,
            tileHeight
        };
    }
}

void MonthsView::selectYear(const int direction) {
    if (direction == 0 || _years.empty()) {
        return;
    }

    const auto current = std::ranges::find(_years, _selectedYear);
    if (current == _years.end()) {
        return;
    }

    const auto index = static_cast<std::ptrdiff_t>(
        current - _years.begin()
    );
    const auto nextIndex = index + direction;

    if (nextIndex < 0 ||
        nextIndex >= static_cast<std::ptrdiff_t>(_years.size())) {
        return;
    }

    const int nextYear = _years[static_cast<std::size_t>(nextIndex)];
    if (nextYear == _selectedYear) {
        return;
    }

    _selectedYear = nextYear;

    const std::vector<int> months =
        _manager.availableMonths(_selectedYear);

    if (std::ranges::find(months, _selectedMonth) == months.end() &&
        !months.empty()) {
        _selectedMonth = months.front();
    }

    refreshData();

    if (_selectionChanged) {
        _selectionChanged();
    }
}

void MonthsView::selectMonth(const int month) const {
    if (month < 1 || month > 12) {
        return;
    }

    const MonthTile& tile = _tiles[static_cast<std::size_t>(month - 1)];
    if (!tile.available || month == _selectedMonth) {
        return;
    }

    _selectedMonth = month;

    if (_selectionChanged) {
        _selectionChanged();
    }
}

bool MonthsView::handleEvent(const Event& event) {
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

    const auto yearIt = std::ranges::find(_years, _selectedYear);
    if (yearIt != _years.end()) {
        const auto index = yearIt - _years.begin();

        if (_previousYearRect.isPointInside(position) && index > 0) {
            selectYear(-1);
            return false;
        }

        if (_nextYearRect.isPointInside(position) &&
            index + 1 < static_cast<std::ptrdiff_t>(_years.size())) {
            selectYear(1);
            return false;
        }
    }

    for (std::size_t i = 0; i < _tiles.size(); ++i) {
        if (_tiles[i].available &&
            _tiles[i].rect.isPointInside(position)) {
            selectMonth(static_cast<int>(i) + 1);
            return false;
        }
    }

    return true;
}

void MonthsView::tick(Clock&, const double dt) {
    _refreshTimer += static_cast<float>(std::max(0.0, dt));

    if (_refreshTimer >= 0.5f) {
        _refreshTimer = 0.0f;
        refreshData();
    }
}

Point MonthsView::minimumSize() const {
    return {260.0f, 250.0f};
}

Point MonthsView::preferredSize() const {
    return _preferredRect.size;
}

void MonthsView::resize(const Point size) {
    setRect({_rect.origin, size});
}

void MonthsView::reposition(const Point position) {
    setRect({position, _rect.size});
}

void MonthsView::setRect(const Rect rect) {
    _rect = rect;
    _preferredRect = rect;
    layout();
}

const Rect& MonthsView::rect() {
    return _rect;
}

void MonthsView::renderImpl(Window& window) {
    window->renderText(
        _font,
        "Months",
        _rect.origin.x + kSidePadding,
        _rect.origin.y + 17.0f,
        Color::white
    );

    const auto yearIt = std::ranges::find(_years, _selectedYear);
    const bool canGoPrevious = yearIt != _years.end() &&
        yearIt != _years.begin();
    const bool canGoNext = yearIt != _years.end() &&
        yearIt + 1 != _years.end();

    drawYearButton(
        window,
        _font,
        _previousYearRect,
        "<",
        canGoPrevious,
        canGoPrevious && _previousYearRect.isPointInside(_mousePosition)
    );

    drawYearButton(
        window,
        _font,
        _nextYearRect,
        ">",
        canGoNext,
        canGoNext && _nextYearRect.isPointInside(_mousePosition)
    );

    const std::string yearText = std::to_string(_selectedYear);
    const float yearTextWidth = static_cast<float>(yearText.size()) * 9.6f;

    window->renderText(
        _font,
        yearText.c_str(),
        _previousYearRect.origin.x + 36.0f +
            (42.0f - yearTextWidth) * 0.5f,
        _rect.origin.y + 17.0f,
        Color::white
    );

    for (std::size_t i = 0; i < _tiles.size(); ++i) {
        const MonthTile& tile = _tiles[i];
        const bool selected =
            tile.available && static_cast<int>(i) + 1 == _selectedMonth;

        const Color background = selected
            ? Color{48, 48, 48}
            : Color{29, 29, 29};

        window->setRenderColor(background);
        window->renderRoundedRect(tile.rect, 9);

        const Rect accent{
            tile.rect.origin.x,
            tile.rect.origin.y + 8.0f,
            5.0f,
            std::max(0.0f, tile.rect.size.y - 16.0f)
        };

        window->setRenderColor(
            tile.available
                ? MonthPalette::color(static_cast<int>(i) + 1)
                : Color{65, 65, 65}
        );
        window->renderRoundedRect(accent, 3);

        const Color primary = tile.available
            ? Color::white
            : Color{115, 115, 115};

        const std::string monthName{MonthPalette::names[i]};

        window->renderText(
            _font,
            monthName.c_str(),
            tile.rect.origin.x + 16.0f,
            tile.rect.origin.y + 12.0f,
            primary
        );

        const std::string detail = tile.available
            ? std::to_string(tile.total) + " kr"
            : "No saved data";

        window->renderText(
            _font,
            detail.c_str(),
            tile.rect.origin.x + 16.0f,
            tile.rect.origin.y + 43.0f,
            tile.available ? Color{185, 185, 185} : Color{95, 95, 95}
        );

        if (selected) {
            window->setRenderColor(MonthPalette::color(static_cast<int>(i) + 1));
            window->renderRoundedRect(
                {
                    tile.rect.origin.x + tile.rect.size.x - 12.0f,
                    tile.rect.origin.y + 12.0f,
                    6.0f,
                    6.0f
                },
                3
            );
        }
    }
}