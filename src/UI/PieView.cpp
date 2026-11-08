#include "UI/PieView.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <QTipUIKit/Internal/Helpers.h>

#include "Spacer.h"
#include "UI/Visibility.h"

namespace {

constexpr float kPadding = 10.0f;
constexpr float kGap = 15.0f;
constexpr float kLegendRowHeight = 58.0f;
constexpr float kLegendRowSpacing = 8.0f;
constexpr float kExpandedLegendRowHeight = 86.0f;
constexpr float kLegendScrollMargin = 20.0f;
constexpr float kHoverAnimationSpeed = 8.0f;
constexpr float kScrollAnimationSpeed = 10.0f;

class PieLegendRow final : public UIObject {
public:
    PieLegendRow(
        QTip::Rect rect,
        std::string name,
        std::int64_t value,
        double percent,
        QTip::Color color,
        std::size_t itemCount,
        ScrollView& parentScrollView,
        QTip::Font& font
    )
        : _name(std::move(name)),
          _details([&] {
              std::ostringstream stream;
              stream << std::fixed << std::setprecision(1)
                     << (percent * 100.0) << "%  |  " << value << " kr";
              return stream.str();
          }()),
          _itemCountText("Items: " + std::to_string(itemCount)),
          _color(color),
          _parentScrollView(parentScrollView),
          _font(font) {
        _rect = rect;
        _preferredRect = rect;
    }

    bool handleEvent(const QTip::Event&) override {
        // Legend rows are display-only. Hover interactions are driven exclusively
        // by PieChart::hoveredIndex().
        return true;
    }

    void setExpandProgress(float progress) {
        _expandProgress = std::clamp(progress, 0.0f, 1.0f);
        const float height = kLegendRowHeight
            + (kExpandedLegendRowHeight - kLegendRowHeight) * _expandProgress;
        _rect.size.y = height;
        _preferredRect.size.y = height;
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
        if (!isVisibleInScrollView(_rect, _parentScrollView)) {
            return;
        }

        constexpr float normalSwatchSize = 24.0f;
        const float swatchSize = normalSwatchSize + 6.0f * _expandProgress;

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

        window->renderText(
            _font,
            _details.c_str(),
            textX,
            detailsY,
            QTip::Color{180, 180, 180}
        );

        if (_expandProgress > 0.35f) {
            const float fade = std::clamp((_expandProgress - 0.35f) / 0.65f, 0.0f, 1.0f);
            const auto alpha = static_cast<std::uint8_t>(205.0f * fade);
            window->renderText(
                _font,
                _itemCountText.c_str(),
                textX,
                _rect.origin.y + 57.0f,
                QTip::Color{205, 205, 205, alpha}
            );
        }
    }

private:
    std::string _name;
    std::string _details;
    std::string _itemCountText;
    QTip::Color _color;
    ScrollView& _parentScrollView;
    QTip::Font& _font;
    float _expandProgress = 0.0f;
};

std::vector<Block*> eligibleBlocks(const Database& database) {
    std::vector<Block*> blocks;
    blocks.reserve(database.blocks().size());

    for (auto& block : database.blocks()) {
        const std::int64_t value = block->total();

        if (block->name.empty() || value <= 0) {
            continue;
        }

        blocks.push_back(&*block);
    }

    std::ranges::stable_sort(
        blocks,
        [](const Block* lhs, const Block* rhs) {
            const std::int64_t lhsTotal = lhs->total();
            const std::int64_t rhsTotal = rhs->total();

            if (lhsTotal != rhsTotal) {
               return lhsTotal > rhsTotal;
            }

            return lhs->name > rhs->name;
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
              .showScrollbar = false
          }
      ),
      _chart(
          std::make_unique<PieChart<Block*>>(
              QTip::Rect{},
              std::vector<PieChartItem<Block*>>{},
              true
          )
      ) {
    _rect = rect;
    _preferredRect = rect;

    _legendScrollView.parent = this;
    _chart->parent = this;

    _chart->setTooltipRenderer(
        [](QTip::Window& window,
           const QTip::Font& font,
           Block*& block,
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
                font, block->name.c_str(), 5.0f, 5.0f, textColor
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
}

void PieView::refresh() {
    rebuildData();
    layout();
}

void PieView::refreshDataIfNeeded() {
    // Database revisions include block and item edits, so this avoids
    // repeatedly sorting and comparing every item on every frame.
    if (_cachedDatabaseRevision == _database.revision()) {
        return;
    }

    rebuildData();
}

void PieView::rebuildData() {
    std::vector<Block*> blocks = eligibleBlocks(_database);

    double total = 0.0;
    for (auto block : blocks) {
        total += static_cast<double>(block->total());
    }

    _entries.clear();
    _entries.reserve(blocks.size());

    for (std::size_t i = 0; i < blocks.size(); ++i) {
        const std::int64_t value = blocks[i]->total();
        const double percent = total > 0.0
            ? static_cast<double>(value) / total
            : 0.0;

        const double colorPosition =
            (static_cast<double>(i) + 0.5) /
            static_cast<double>(blocks.size());

        _entries.push_back({
            blocks[i],
            value,
            percent,
            Detail::pieChartColor(colorPosition)
        });
    }

    std::vector<PieChartItem<Block*>> chartItems;
    chartItems.reserve(_entries.size());

    for (const Entry& entry : _entries) {
        chartItems.push_back({
            entry.block,
            static_cast<double>(entry.value),
            entry.block->name,
            entry.color
        });
    }

    _chart->setData(std::move(chartItems));

    // PieChart currently retains its previous hovered index across setData().
    // Clear it before the old Block pointers can become invalid.
    QTip::Event resetHover{};
    resetHover.type = QTip::EventType::MouseMoved;
    resetHover.mouseMove.position = {-10000.0f, -10000.0f};
    _chart->handleEvent(resetHover);

    _cachedDatabaseRevision = _database.revision();

    _legendScrollView.clear();
    _legendRows.clear();
    _chartHoveredIndex = kNoEntry;
    _expandedIndex = kNoEntry;
    _rowExpandProgress.assign(_entries.size(), 0.0f);
    _rowAnimationActive = false;

    for (const auto & entry : _entries) {
        auto& row = _legendScrollView.add<PieLegendRow>(
            QTip::Rect{0.0f, 0.0f, 0.0f, kLegendRowHeight},
            entry.block->name,
            entry.value,
            entry.percent,
            entry.color,
            entry.block->items.size(),
            _legendScrollView,
            _font
        );

        _legendRows.push_back(&row);
    }

    _legendScrollView.add<Spacer>(QTip::Rect{0.0f, 0.0f, 0.0f, kLegendRowHeight});

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

    const QTip::Rect legendRect{
        _rect.origin.x + kPadding,
        _rect.origin.y + kPadding,
        columnWidth,
        contentHeight
    };
    const QTip::Rect& currentLegendRect = _legendScrollView.rect();

    if (currentLegendRect.origin.x != legendRect.origin.x ||
        currentLegendRect.origin.y != legendRect.origin.y ||
        currentLegendRect.size.x != legendRect.size.x ||
        currentLegendRect.size.y != legendRect.size.y) {
        _legendScrollView.setRect(legendRect);
    }

    float rowY = 0.0f;
    for (std::size_t i = 0; i < _legendRows.size(); ++i) {
        auto* row = dynamic_cast<PieLegendRow*>(_legendRows[i]);
        const float progress = i < _rowExpandProgress.size()
            ? _rowExpandProgress[i]
            : 0.0f;
        const float rowHeight = kLegendRowHeight
            + (kExpandedLegendRowHeight - kLegendRowHeight) * progress;

        const QTip::Rect rowRect{0.0f, rowY, columnWidth, rowHeight};
        const QTip::Rect& currentRowRect = row->rect();

        if (currentRowRect.origin.x != rowRect.origin.x ||
            currentRowRect.origin.y != rowRect.origin.y ||
            currentRowRect.size.x != rowRect.size.x ||
            currentRowRect.size.y != rowRect.size.y) {
            row->setRect(rowRect);
        }

        row->setExpandProgress(progress);
        rowY += rowHeight + kLegendRowSpacing;
    }

    // Reserve the largest possible row expansion up front. This keeps the
    // ScrollView's backing RenderTexture stable while a legend row animates.
    const float maximumContentHeight =
        static_cast<float>(_legendRows.size()) *
            (kLegendRowHeight + kLegendRowSpacing) +
        kLegendRowHeight +
        (_legendRows.empty()
            ? 0.0f
            : kExpandedLegendRowHeight - kLegendRowHeight);

    const QTip::Point legendContentSize{
        std::max(1.0f, columnWidth),
        std::max(1.0f, maximumContentHeight)
    };
    const QTip::Point currentContentSize =
        _legendScrollView.contentSize();

    if (currentContentSize.x != legendContentSize.x ||
        currentContentSize.y != legendContentSize.y) {
        _legendScrollView.setContentSize(legendContentSize);
    }

    const QTip::Rect nextChartRect{
        _rect.origin.x + kPadding + columnWidth + kGap,
        _rect.origin.y + kPadding,
        columnWidth,
        contentHeight
    };

    const bool chartSizeChanged =
        nextChartRect.size.x != _chartRect.size.x ||
        nextChartRect.size.y != _chartRect.size.y;
    const bool chartOriginChanged =
        nextChartRect.origin.x != _chartRect.origin.x ||
        nextChartRect.origin.y != _chartRect.origin.y;

    _chartRect = nextChartRect;

    if (chartSizeChanged) {
        _chart->setRect(_chartRect);
    }

    if (chartOriginChanged &&
        _chart->hoveredIndex() != kNoEntry) {
        QTip::Event resetHover{};
        resetHover.type = QTip::EventType::MouseMoved;
        resetHover.mouseMove.position = {-10000.0f, -10000.0f};
        _chart->handleEvent(resetHover);

        _chartHoveredIndex = kNoEntry;
        _expandedIndex = kNoEntry;
        _rowAnimationActive = true;
    }
}

void PieView::syncHoveredEntry(const bool scrollToEntry) {
    // Only the pie chart's hovered slice controls the selected legend row.
    if (_chartHoveredIndex == _expandedIndex) {
        return;
    }

    _expandedIndex = _chartHoveredIndex;
    _rowAnimationActive = true;
    if (_expandedIndex != kNoEntry && scrollToEntry) {
        scrollEntryIntoView(_expandedIndex);
    }
}

void PieView::scrollEntryIntoView(const std::size_t index) {
    if (index >= _legendRows.size()) {
        return;
    }

    auto* row = dynamic_cast<PieLegendRow*>(_legendRows[index]);
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
        _targetScrollY = viewportTop;
        return;
    }

    _targetScrollY = std::max(0.0f, targetY);
}

bool PieView::handleEvent(const QTip::Event& event) {
    if (event.type == QTip::EventType::MouseScrolled) {
        _manualScrolling = true;
    } else if (event.type == QTip::EventType::MouseButtonPressed) {
        const QTip::Point mouse{
            event.mouseButton.position.x,
            event.mouseButton.position.y
        };

        if (_legendScrollView.rect().isPointInside(mouse)) {
            _manualScrolling = true;
        }
    }

    const bool propagate = _legendScrollView.handleEvent(event);

    // Remember the position after manual scrolling so the animation
    // never pulls the legend back to an old target.
    if (_manualScrolling) {
        _targetScrollY = _legendScrollView.scrollPosition().y;
    }

    if (!propagate) {
        return false;
    }

    return _chart->handleEvent(event);
}

void PieView::tick(QTip::Clock& clock, const double dt) {
    refreshDataIfNeeded();

    _legendScrollView.tick(clock, dt);

    const float safeDt = static_cast<float>(std::clamp(dt, 0.0, 0.05));
    _chart->tick(clock, safeDt);

    const std::size_t hovered = _chart->hoveredIndex();

    if (hovered != _chartHoveredIndex) {
        _chartHoveredIndex = hovered;

        // A new chart hover can trigger automatic scrolling again.
        _manualScrolling = false;
        syncHoveredEntry(true);
    }

    bool rowAnimationChanged = false;
    if (_rowAnimationActive) {
        const float hoverStep = std::min(1.0f, kHoverAnimationSpeed * safeDt);

        for (std::size_t i = 0; i < _rowExpandProgress.size(); ++i) {
            const float target = i == _expandedIndex ? 1.0f : 0.0f;
            float& progress = _rowExpandProgress[i];
            const float next = progress + (target - progress) * hoverStep;

            if (std::abs(target - next) < 0.002f) {
                progress = target;
            } else {
                progress = next;
            }

            if (std::abs(progress - target) > 0.002f) {
                rowAnimationChanged = true;
            }
        }

        _rowAnimationActive = rowAnimationChanged;
    }

    const QTip::Point currentScroll = _legendScrollView.scrollPosition();
    const float scrollDifference = _targetScrollY - currentScroll.y;

    if (!_manualScrolling && std::abs(scrollDifference) > 0.5f) {
        const float scrollStep =
            std::min(1.0f, kScrollAnimationSpeed * safeDt);

        _legendScrollView.scrollTo({
            0.0f,
            currentScroll.y + scrollDifference * scrollStep
        });
    } else if (!_manualScrolling && std::abs(scrollDifference) > 0.0f) {
        _legendScrollView.scrollTo({0.0f, _targetScrollY});
    }

    if (rowAnimationChanged) {
        layout();
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

    if (_chartRect.size.x > 20.0f && _chartRect.size.y > 20.0f) {
        _chart->render(window);
    }

    if (_entries.empty()) {
        window->renderText(
            _font,
            "No positive block totals",
            _rect.origin.x + _rect.size.x * 0.5f - 100.0f,
            _rect.origin.y + _rect.size.y * 0.5f,
            QTip::Color{160, 160, 160}
        );
    }
}
