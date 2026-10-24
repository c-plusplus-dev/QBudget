#include "UI/PieView.h"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL_events.h>

#include <QTipUIKit/Internal/Helpers.h>

namespace {

constexpr float kPadding = 10.0f;
constexpr float kGap = 15.0f;
constexpr float kLegendRowHeight = 58.0f;
constexpr float kExpandedLegendRowHeight = 86.0f;
constexpr float kLegendScrollMargin = 8.0f;

class PieLegendRow final : public UIObject {
public:
    using HoverCallback = std::function<void(bool)>;

    PieLegendRow(
        QTip::Rect rect,
        std::string name,
        std::int64_t value,
        double percent,
        QTip::Color color,
        std::size_t itemCount,
        QTip::Font& font,
        HoverCallback hoverCallback
    )
        : _name(std::move(name)),
          _value(value),
          _percent(percent),
          _color(color),
          _itemCount(itemCount),
          _font(font),
          _hoverCallback(std::move(hoverCallback)) {
        _rect = rect;
        _preferredRect = rect;
    }

    bool handleEvent(const SDL_Event& event) override {
        if (event.type != SDL_EVENT_MOUSE_MOTION) {
            return true;
        }

        const bool nowHovered = _rect.isPointInside({
            event.motion.x,
            event.motion.y
        });

        if (nowHovered != _hovered) {
            _hovered = nowHovered;
            if (_hoverCallback) {
                _hoverCallback(_hovered);
            }
        }

        return true;
    }

    void setExpanded(bool expanded) {
        _expanded = expanded;
        const float height = _expanded
            ? kExpandedLegendRowHeight
            : kLegendRowHeight;

        _rect.size.y = height;
        _preferredRect.size.y = height;
    }

    [[nodiscard]] bool hovered() const {
        return _hovered;
    }

    [[nodiscard]] QTip::Point minimumSize() const override {
        return {100.0f, kLegendRowHeight};
    }

    [[nodiscard]] QTip::Point preferredSize() const override {
        return _preferredRect.size;
    }

    void resize(QTip::Point size) override {
        _rect.size = size;
        _preferredRect.size = size;
    }

    void reposition(QTip::Point position) override {
        _rect.origin = position;
        _preferredRect.origin = position;
    }

    void setRect(QTip::Rect rect) override {
        _rect = rect;
        _preferredRect = rect;
    }

    [[nodiscard]] const QTip::Rect& rect() override {
        return _rect;
    }

protected:
    void renderImpl(QTip::Window& window) override {
        constexpr float normalSwatchSize = 24.0f;
        const float swatchSize = _expanded ? 30.0f : normalSwatchSize;

        if (_hovered || _expanded) {
            window->setRenderColor(QTip::Color{45, 45, 45});
            window->renderRoundedRect(_rect, 6);
        }

        const QTip::Rect swatch{
            _rect.origin.x + 8.0f,
            _rect.origin.y + (_rect.size.y - swatchSize) / 2.0f,
            swatchSize,
            swatchSize
        };

        window->setRenderColor(_color);
        window->renderRoundedRect(swatch, 5);

        const float textX = swatch.origin.x + swatch.size.x + 12.0f;
        const float nameY = _rect.origin.y + 7.0f;
        const float detailsY = _rect.origin.y + 30.0f;

        window->renderText(
            _font,
            _name.c_str(),
            textX,
            nameY,
            QTip::Color::white
        );

        std::ostringstream details;
        details << std::fixed << std::setprecision(1)
                << (_percent * 100.0) << "%  |  "
                << _value << " kr";

        const std::string detailsText = details.str();

        window->renderText(
            _font,
            detailsText.c_str(),
            textX,
            detailsY,
            QTip::Color{180, 180, 180}
        );

        if (_expanded) {
            const std::string itemText =
                "Items: " + std::to_string(_itemCount);

            window->renderText(
                _font,
                itemText.c_str(),
                textX,
                _rect.origin.y + 57.0f,
                QTip::Color{205, 205, 205}
            );
        }
    }

private:
    std::string _name;
    std::int64_t _value;
    double _percent;
    QTip::Color _color;
    std::size_t _itemCount = 0;
    QTip::Font& _font;
    HoverCallback _hoverCallback;
    bool _hovered = false;
    bool _expanded = false;
};

std::vector<Block> eligibleBlocks(const Database& database) {
    std::vector<Block> blocks;
    blocks.reserve(database.blocks().size());

    for (const Block& block : database.blocks()) {
        const std::int64_t value = block.total();

        if (block.name.empty() || value <= 0) {
            continue;
        }

        blocks.push_back(block);
    }

    std::stable_sort(
        blocks.begin(),
        blocks.end(),
        [](const Block& lhs, const Block& rhs) {
            const std::int64_t lhsTotal = lhs.total();
            const std::int64_t rhsTotal = rhs.total();

            if (lhsTotal != rhsTotal) {
                return lhsTotal < rhsTotal;
            }

            return lhs.name < rhs.name;
        }
    );

    return blocks;
}

} // namespace

PieView::PieView(
    QTip::Rect rect,
    Database& database,
    QTip::Font& font
)
    : _database(database),
      _font(font),
      _legendScrollView(
          QTip::Rect{},
          ScrollViewSettings{
              .direction = ScrollDirection::Vertical,
              .scrollSpeed = 30.0f,
              .showScrollbar = true
          }
      ),
      _chart(
          std::make_unique<PieChart<Block>>(
              QTip::Rect{},
              std::vector<PieChartItem<Block>>{},
              true
          )
      ) {
    _rect = rect;
    _preferredRect = rect;

    _legendScrollView.parent = this;
    _chart->parent = this;

    _chart->setTooltipRenderer(
        [](QTip::Window& window,
           QTip::Font font,
           Block& block,
           double value,
           double percent,
           std::uint8_t alpha) {
            const QTip::Color textColor{255, 255, 255, alpha};

            const std::string valueText =
                std::to_string(static_cast<std::int64_t>(value)) + " kr";

            std::ostringstream percentStream;
            percentStream << std::fixed << std::setprecision(1)
                          << percent * 100.0 << '%';

            const std::string percentText = percentStream.str();

            window->renderText(
                font, block.name.c_str(), 5.0f, 5.0f, textColor
            );
            window->renderText(
                font, valueText.c_str(), 5.0f, 25.0f, textColor
            );
            window->renderText(
                font, percentText.c_str(), 5.0f, 45.0f, textColor
            );
        }
    );

    refresh();
    layout();
}

void PieView::refresh() {
    rebuildData();
    layout();
}

void PieView::refreshDataIfNeeded() {
    const std::vector<Block> currentBlocks = eligibleBlocks(_database);

    if (currentBlocks.size() != _entries.size()) {
        rebuildData();
        return;
    }

    for (std::size_t i = 0; i < currentBlocks.size(); ++i) {
        if (currentBlocks[i].name != _entries[i].block.name ||
            currentBlocks[i].total() != _entries[i].value) {
            rebuildData();
            return;
        }
    }
}

void PieView::rebuildData() {
    std::vector<Block> blocks = eligibleBlocks(_database);

    double total = 0.0;
    for (const Block& block : blocks) {
        total += static_cast<double>(block.total());
    }

    _entries.clear();
    _entries.reserve(blocks.size());

    for (std::size_t i = 0; i < blocks.size(); ++i) {
        const std::int64_t value = blocks[i].total();
        const double percent = total > 0.0
            ? static_cast<double>(value) / total
            : 0.0;

        const double colorPosition =
            (static_cast<double>(i) + 0.5) /
            static_cast<double>(blocks.size());

        _entries.push_back({
            std::move(blocks[i]),
            value,
            percent,
            Detail::pieChartColor(colorPosition)
        });
    }

    std::vector<PieChartItem<Block>> chartItems;
    chartItems.reserve(_entries.size());

    for (const Entry& entry : _entries) {
        chartItems.push_back({
            entry.block,
            static_cast<double>(entry.value),
            entry.block.name,
            entry.color
        });
    }

    _chart->setData(std::move(chartItems));

    _legendScrollView.clear();
    _legendRows.clear();
    _chartHoveredIndex = kNoEntry;
    _legendHoveredIndex = kNoEntry;
    _expandedIndex = kNoEntry;

    for (std::size_t i = 0; i < _entries.size(); ++i) {
        const Entry& entry = _entries[i];

        auto& row = _legendScrollView.add<PieLegendRow>(
            QTip::Rect{0.0f, 0.0f, 0.0f, kLegendRowHeight},
            entry.block.name,
            entry.value,
            entry.percent,
            entry.color,
            entry.block.items.size(),
            _font,
            [this, i](bool hovered) {
                onLegendHover(i, hovered);
            }
        );

        _legendRows.push_back(&row);
    }

    layout();
}

void PieView::layout() {
    const float width = std::max(0.0f, _rect.size.x);
    const float height = std::max(0.0f, _rect.size.y);
    const float availableWidth =
        std::max(0.0f, width - 2.0f * kPadding - kGap);
    const float columnWidth = availableWidth * 0.5f;
    const float contentHeight =
        std::max(0.0f, height - 2.0f * kPadding);

    _legendScrollView.setRect({
        kPadding,
        kPadding,
        columnWidth,
        contentHeight
    });

    float rowY = 0.0f;
    for (std::size_t i = 0; i < _legendRows.size(); ++i) {
        auto* row = static_cast<PieLegendRow*>(_legendRows[i]);
        const float rowHeight = i == _expandedIndex
            ? kExpandedLegendRowHeight
            : kLegendRowHeight;

        row->setRect({0.0f, rowY, columnWidth, rowHeight});
        row->setExpanded(i == _expandedIndex);
        rowY += rowHeight;
    }

    _legendScrollView.setContentSize({
        std::max(1.0f, columnWidth),
        std::max(1.0f, rowY)
    });

    const float chartX = _rect.origin.x + kPadding + columnWidth + kGap;
    _chart->setRect({
        chartX,
        kPadding,
        columnWidth,
        contentHeight
    });
}

void PieView::onLegendHover(const std::size_t index, const bool hovered) {
    if (hovered) {
        _legendHoveredIndex = index;
    } else if (_legendHoveredIndex == index) {
        _legendHoveredIndex = kNoEntry;
    }

    syncHoveredEntry(true);
}

void PieView::syncHoveredEntry(const bool scrollToEntry) {
    const std::size_t activeIndex =
        _legendHoveredIndex != kNoEntry
            ? _legendHoveredIndex
            : _chartHoveredIndex;

    if (activeIndex == _expandedIndex) {
        return;
    }

    _expandedIndex = activeIndex;
    layout();

    if (scrollToEntry && _expandedIndex != kNoEntry) {
        scrollEntryIntoView(_expandedIndex);
    }
}

void PieView::scrollEntryIntoView(const std::size_t index) {
    if (index >= _legendRows.size()) {
        return;
    }

    auto* row = static_cast<PieLegendRow*>(_legendRows[index]);
    const float rowTop = row->rect().origin.y;
    const float rowBottom = rowTop + row->rect().size.y;
    const float viewportTop = _legendScrollView.scrollPosition().y;
    const float viewportBottom =
        viewportTop + _legendScrollView.rect().size.y;

    float targetY = viewportTop;

    if (rowTop < viewportTop + kLegendScrollMargin) {
        targetY = rowTop - kLegendScrollMargin;
    } else if (rowBottom > viewportBottom - kLegendScrollMargin) {
        targetY = rowBottom - _legendScrollView.rect().size.y
            + kLegendScrollMargin;
    } else {
        return;
    }

    _legendScrollView.scrollTo({0.0f, std::max(0.0f, targetY)});
}

bool PieView::handleEvent(const SDL_Event& event) {
    // ScrollView does not forward mouse motion that occurs outside its viewport,
    // so update the legend hover state here as well to clear it on mouse exit.
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        const QTip::Point mouse{
            event.motion.x,
            event.motion.y
        };

        std::size_t hoveredIndex = kNoEntry;
        if (_legendScrollView.rect().isPointInside(mouse)) {
            const QTip::Point contentMouse{
                mouse.x - _legendScrollView.rect().origin.x
                    + _legendScrollView.scrollPosition().x,
                mouse.y - _legendScrollView.rect().origin.y
                    + _legendScrollView.scrollPosition().y
            };

            for (std::size_t i = 0; i < _legendRows.size(); ++i) {
                if (_legendRows[i]->rect().isPointInside(contentMouse)) {
                    hoveredIndex = i;
                    break;
                }
            }
        }

        if (hoveredIndex != _legendHoveredIndex) {
            _legendHoveredIndex = hoveredIndex;
            syncHoveredEntry(true);
        }
    }

    if (!_legendScrollView.handleEvent(event)) {
        return false;
    }

    return _chart->handleEvent(event);
}

void PieView::tick(QTip::Clock& clock, const double dt) {
    refreshDataIfNeeded();

    _legendScrollView.tick(clock, dt);
    _chart->tick(clock, dt);

    const std::size_t hovered = _chart->hoveredIndex();
    if (hovered != _chartHoveredIndex) {
        _chartHoveredIndex = hovered;
        syncHoveredEntry(true);
    }
}

QTip::Point PieView::minimumSize() const {
    return {240.0f, 160.0f};
}

QTip::Point PieView::preferredSize() const {
    return _preferredRect.size;
}

void PieView::resize(QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;
    layout();
}

void PieView::reposition(QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
    layout();
}

void PieView::setRect(QTip::Rect rect) {
    _rect = rect;
    _preferredRect = rect;
    layout();
}

const QTip::Rect& PieView::rect() {
    return _rect;
}

void PieView::renderImpl(QTip::Window& window) {
    _legendScrollView.render(window);

    if (_chart->rect().size.x > 20.0f &&
        _chart->rect().size.y > 20.0f) {
        _chart->render(window);
    }

    if (_entries.empty()) {
        window->renderText(
            _font,
            "No positive block totals",
            _rect.size.x * 0.5f - 100.0f,
            _rect.size.y * 0.5f,
            QTip::Color{160, 160, 160}
        );
    }
}
