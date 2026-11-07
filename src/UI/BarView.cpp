#include "UI/BarView.h"

#include "UI/MonthPalette.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>

using namespace QTip;

namespace {

constexpr float kPadding = 12.0f;
constexpr float kHeaderHeight = 66.0f;
constexpr float kBottomPadding = 12.0f;

ButtonStyle makeBlockButtonStyle(const Font& font) {
    return {
        Color{40, 40, 40},
        Color{55, 55, 55},
        Color{70, 70, 70},
        Color{25, 25, 25},
        font,
        Color::white
    };
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
    const ButtonStyle buttonStyle = makeBlockButtonStyle(_font);

    _previousBlockButton = std::make_unique<Button>(
        Rect{}, "◀", buttonStyle
    );
    _nextBlockButton = std::make_unique<Button>(
        Rect{}, "▶", buttonStyle
    );

    _previousBlockButton->setOnClick([this] {
        selectBlock(-1);
    });
    _nextBlockButton->setOnClick([this] {
        selectBlock(1);
    });

    _chart = std::make_unique<BarChart<MonthBar>>(
        Rect{},
        std::vector<BarChartItem<MonthBar>>{},
        true
    );
    _chart->parent = this;
    _chart->setTooltipRenderer(
        [](Window& window,
           Font font,
           MonthBar& month,
           double,
           const std::uint8_t alpha) {
            const Color color{255, 255, 255, alpha};
            const std::string monthName{
                MonthPalette::names[static_cast<std::size_t>(month.month - 1)]
            };
            const std::string amount = month.hasBlock
                ? std::to_string(month.amount) + " kr"
                : "No entry";

            window->renderText(font, monthName.c_str(), 5.0f, 5.0f, color);
            window->renderText(font, amount.c_str(), 5.0f, 25.0f, color);
        }
    );

    _rect = rect;
    _preferredRect = rect;

    refreshData();
    layout();
}

void BarView::layout() {
    const float width = std::max(0.0f, _rect.size.x);
    const float height = std::max(0.0f, _rect.size.y);

    _previousBlockButton->setRect({
        _rect.origin.x + kPadding,
        _rect.origin.y + 12.0f,
        32.0f,
        32.0f
    });

    _nextBlockButton->setRect({
        _rect.origin.x + width - kPadding - 32.0f,
        _rect.origin.y + 12.0f,
        32.0f,
        32.0f
    });

    _chartRect = {
        _rect.origin.x + kPadding,
        _rect.origin.y + kHeaderHeight,
        std::max(0.0f, width - 2.0f * kPadding),
        std::max(0.0f, height - kHeaderHeight - kBottomPadding)
    };

    _chart->setRect({
        0.0f,
        0.0f,
        _chartRect.size.x,
        _chartRect.size.y
    });

    _chartCache.reset();
    _chartDirty = true;
}

void BarView::updateNavigationButtons() {
    const auto current = std::ranges::find(_blockNames, _selectedBlock);
    const bool hasSelection = current != _blockNames.end();
    const auto index = hasSelection
        ? current - _blockNames.begin()
        : std::ptrdiff_t{0};

    _previousBlockButton->setDisabled(!hasSelection || index == 0);
    _nextBlockButton->setDisabled(
        !hasSelection ||
        index + 1 >= static_cast<std::ptrdiff_t>(_blockNames.size())
    );
}

void BarView::refreshData() {
    const int year = _selectedYear;

    std::array<Database*, 12> databases{};
    std::array<bool, 12> available{};
    std::vector<std::string> names;
    std::unordered_set<std::string> seenNames;

    const std::vector<int> months = _manager.availableMonths(year);
    seenNames.reserve(months.size() * 4);

    for (const int month : months) {
        if (month < 1 || month > 12) {
            continue;
        }

        Database* database = _manager.get(year, month);
        if (!database) {
            continue;
        }

        const auto index = static_cast<std::size_t>(month - 1);
        databases[index] = database;
        available[index] = true;

        for (const auto& block : database->blocks()) {
            if (block && !block->name.empty() &&
                seenNames.insert(block->name).second) {
                names.push_back(block->name);
            }
        }
    }

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
                    return candidate && candidate->name == _selectedBlock;
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
        _cachedSelection != _selectedBlock ||
        _values != values ||
        _availableMonths != available ||
        _hasBlock != hasBlock;

    _blockNames = std::move(names);
    _titleText = _selectedBlock.empty() ? "No blocks found" : _selectedBlock;
    _yearText = "Year: " + std::to_string(year);
    _values = values;
    _availableMonths = available;
    _hasBlock = hasBlock;
    _cachedYear = year;
    _cachedSelection = _selectedBlock;
    _cachedManagerRevision = _manager.revision();

    if (!changed) {
        updateNavigationButtons();
        return;
    }

    std::vector<BarChartItem<MonthBar>> chartItems;
    chartItems.reserve(12);

    for (std::size_t index = 0; index < 12; ++index) {
        const int month = static_cast<int>(index) + 1;
        const bool monthAvailable = _availableMonths[index];
        const bool hasSelectedBlock = _hasBlock[index];

        chartItems.push_back({
            MonthBar{
                month,
                _values[index],
                monthAvailable,
                hasSelectedBlock
            },
            static_cast<double>(_values[index]),
            std::string{MonthPalette::abbreviations[index]},
            monthAvailable
                ? MonthPalette::color(month)
                : Color{75, 75, 75}
        });
    }

    _chart->setData(std::move(chartItems));
    _chartAnimationRemaining = 1.8f;
    _chartDirty = true;
    updateNavigationButtons();
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
    Event chartEvent = event;
    bool hasPointerPosition = false;

    switch (event.type) {
    case EventType::MouseMoved:
        chartEvent.mouseMove.position =
            event.mouseMove.position - _chartRect.origin;
        hasPointerPosition = true;
        break;

    case EventType::MouseButtonPressed:
    case EventType::MouseButtonReleased:
        chartEvent.mouseButton.position =
            event.mouseButton.position - _chartRect.origin;
        hasPointerPosition = true;
        break;

    case EventType::MouseScrolled:
        chartEvent.mouseScroll.position =
            event.mouseScroll.position - _chartRect.origin;
        hasPointerPosition = true;
        break;

    default:
        break;
    }

    const auto handleChartEvent = [&] {
        const std::size_t previousHovered = _chart->hoveredIndex();
        const bool propagate = _chart->handleEvent(chartEvent);
        const std::size_t currentHovered = _chart->hoveredIndex();

        if (currentHovered != previousHovered) {
            _chartAnimationRemaining = 0.8f;
            _chartDirty = true;
        }

        if (hasPointerPosition &&
            (previousHovered != static_cast<std::size_t>(-1) ||
             currentHovered != static_cast<std::size_t>(-1))) {
            _chartDirty = true;
        }

        return propagate;
    };

    if ((event.type == EventType::MouseButtonPressed ||
         event.type == EventType::MouseButtonReleased) &&
        event.mouseButton.button != MouseButton::Left) {
        return handleChartEvent();
    }

    updateNavigationButtons();

    if (!_previousBlockButton->handleEvent(event)) {
        return false;
    }

    if (!_nextBlockButton->handleEvent(event)) {
        return false;
    }

    return handleChartEvent();
}

void BarView::tick(Clock& clock, const double dt) {
    if (_chartAnimationRemaining > 0.0f) {
        const float safeDt = static_cast<float>(std::clamp(dt, 0.0, 0.1));
        _chart->tick(clock, safeDt);
        _chartAnimationRemaining = std::max(
            0.0f,
            _chartAnimationRemaining - safeDt
        );
        _chartDirty = true;
    }

    if (_cachedYear != _selectedYear ||
        _cachedManagerRevision != _manager.revision()) {
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
    updateNavigationButtons();

    _previousBlockButton->render(window);
    _nextBlockButton->render(window);

    const bool hasSelection = !_selectedBlock.empty();
    const float titleWidth = _font.getTextWidth(_titleText);
    window->renderText(
        _font,
        _titleText.c_str(),
        _rect.origin.x + (_rect.size.x - titleWidth) * 0.5f,
        _rect.origin.y + 11.0f,
        hasSelection ? Color::white : Color{150, 150, 150}
    );

    const float yearWidth = _font.getTextWidth(_yearText);

    window->renderText(
        _font,
        _yearText.c_str(),
        _rect.origin.x + (_rect.size.x - yearWidth) * 0.5f,
        _rect.origin.y + 35.0f,
        Color{160, 160, 160}
    );

    const Rect chartRect = _chartRect;
    window->setRenderColor({28, 28, 28});
    window->renderRoundedRect(chartRect, 10);

    if (chartRect.size.x < 100.0f || chartRect.size.y < 100.0f) {
        window->renderText(
            _font,
            "Window is too small to display the chart",
            chartRect.origin.x + 12.0f,
            chartRect.origin.y + chartRect.size.y * 0.5f,
            Color{145, 145, 145}
        );
        return;
    }

    if (!hasSelection) {
        window->renderText(
            _font,
            "Create blocks in this year to compare them here",
            chartRect.origin.x + 12.0f,
            chartRect.origin.y + chartRect.size.y * 0.5f,
            Color{145, 145, 145}
        );
        return;
    }

    const int cacheWidth = std::max(
        1,
        static_cast<int>(std::ceil(chartRect.size.x))
    );
    const int cacheHeight = std::max(
        1,
        static_cast<int>(std::ceil(chartRect.size.y))
    );

    if (!_chartCache ||
        _chartCache->size().x != static_cast<float>(cacheWidth) ||
        _chartCache->size().y != static_cast<float>(cacheHeight)) {
        _chartCache = std::make_unique<RenderTexture>(
            window,
            cacheWidth,
            cacheHeight
        );
        _chartDirty = true;
    }

    if (!_chartCache->isValid()) {
        return;
    }

    // BarChart currently allocates an internal tooltip RenderTexture on
    // every render. Cache its finished output and only invoke it while
    // data/hover animation or pointer interaction changes the chart.
    if (_chartDirty) {
        window->setTarget(*_chartCache);
        window->setRenderColor(Color::clear);
        window->clear();
        _chart->render(window);
        window->resetTarget();
        _chartDirty = false;
    }

    window->renderTexture(*_chartCache, Rect::zero, chartRect);
}
