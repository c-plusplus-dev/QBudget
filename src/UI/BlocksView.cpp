#include "UI/BlocksView.h"

#include <algorithm>
#include <cinttypes>
#include <ranges>
#include <QTipUIKit/Widgets/Button/MagickButton.h>


#include "Helpers.h"
#include "UI/Spacer.h"

using namespace QTip;

namespace {
constexpr float kVerticalInset = 25.0f;
constexpr float kBlockHeight = 100.0f;
constexpr float kBlockSpacing = 10.0f;
constexpr float kBottomSpacerHeight = 60.0f;
constexpr float kExpandedHeightReduction = 100.0f;
constexpr float kAddBlockButtonSize = 30.0f;

float nonNegative(const float value) {
    return std::max(0.0f, value);
}

ButtonStyle makeAddButtonStyle(const Font& font) {
    ButtonStyle style{};
    style.color = Color::clear;
    style.hoverColor = {40, 40, 40};
    style.pressedColor = {50, 50, 50};
    style.font = font;
    style.fontColor = Color::white;
    return style;
}

void drawAddIcon(Window& window, const Rect& rect) {
    window->setRenderColor(Color::white);

    const Point center = rect.center();
    DrawThickLineHV(
        window,
        center.x, center.y - 10.0f,
        center.x, center.y + 10.0f,
        3
    );
    DrawThickLineHV(
        window,
        center.x - 10.0f, center.y,
        center.x + 10.0f, center.y,
        3
    );
}
}

BlocksView::BlocksView(
    const Rect rect,
    Window& window,
    Font& font,
    Database& database,
    std::vector<BlockView*>& blocks,
    std::vector<BlockView*>& blocksToDelete,
    int currentMonth,
    int currentYear
)
    : _window(window),
      _font(font),
      _database(database),
      _blocks(blocks),
      _blocksToDelete(blocksToDelete),
      _availableHeight(_rect.size.y),
      _scrollView(
          {
              0,
              0,
              nonNegative(rect.size.x),
              nonNegative(rect.size.y - kVerticalInset)
          },
          ScrollViewSettings{.showScrollbar = false}
      ),
      _addButton(
          Rect{0, 0, kAddBlockButtonSize, kAddBlockButtonSize},
          makeAddButtonStyle(font),
          drawAddIcon
      ),
      _totalLabel(
          fmt(
              "Total: %" PRId64 "kr | Date: %d-%02d",
              database.total(),
              currentYear,
              currentMonth
          ),
          Color::white,
          Point{10, _rect.size.y - 26.0f},
          _font
      ) {
    _rect = rect;
    _preferredRect = rect;

    _blockList = &_scrollView.add<List>(
        Rect{0, 0, nonNegative(rect.size.x), 100}
    );
    _blockList->setSpacing(kBlockSpacing);

    _scrollView.parent = this;
    _addButton.parent = this;

    createBlockViews();
    updateLayout();
}

void BlocksView::createBlockViews() {
    _blocks.reserve(_blocks.size() + _database.blocks().size());

    for (auto& block : _database.blocks()) {
        auto& blockView = _blockList->add<BlockView>(
            Rect{0, 0, nonNegative(_rect.size.x), kBlockHeight},
            block,
            _font,
            [this](BlockView& view) {
                handleExpansion(view);
            },
            [this](BlockView& view) {
                showDeleteConfirmation(view);
            }
        );

        _blocks.push_back(&blockView);
    }

    _blockList->add<Spacer>(
        Rect{0, 0, nonNegative(_rect.size.x), kBottomSpacerHeight}
    );
}

void BlocksView::handleExpansion(BlockView& block) {
    resizeBlock(block);
    updateLayout();

    if (block.expanded) {
        _scrollView.setScrollSpeed(0);

        const auto it = std::ranges::find(_blocks, &block);
        if (it != _blocks.end()) {
            const auto index = static_cast<std::size_t>(it - _blocks.begin());
            _scrollView.scrollTo({
                0,
                static_cast<float>(index) * (kBlockHeight + kBlockSpacing)
            });
        }
    } else {
        _scrollView.setScrollSpeed(30);
    }
}

void BlocksView::showDeleteConfirmation(BlockView& block) const {
    Dialog& dialog = Dialog::create(_window);

    auto& content = dialog.content();
    auto& buttons = dialog.buttons();

    content.add<Label>(
        "Are you sure?",
        Color::white,
        Point{0, 0},
        _font
    );

    content.add<Label>(
        fmt("You're about to delete a block called: \"%s\"", block.block().name.c_str()),
        Color::white,
        Point{0, 32},
        _font
    );

    ButtonStyle style;
    style.color = {50, 50, 50};
    style.hoverColor = {70, 70, 70};
    style.pressedColor = {30, 30, 30};
    style.font = _font;
    style.fontColor = Color::white;

    auto& cancelButton = buttons.add<Button>(
        Rect{0, 0, 100, 100},
        "Cancel",
        style
    );
    cancelButton.setOnClick([&dialog] {
        dialog.close();
    });

    style.color = {255, 0, 0};
    style.hoverColor = {255, 64, 64};
    style.pressedColor = {255, 128, 128};

    auto& deleteButton = buttons.add<Button>(
        Rect{0, 0, 100, 100},
        "Delete",
        style
    );
    deleteButton.setOnClick([this, &block, &dialog] {
        if (std::ranges::find(_blocksToDelete, &block) == _blocksToDelete.end()) {
            _blocksToDelete.push_back(&block);
        }
        dialog.close();
    });
}

void BlocksView::processDeletions() {
    bool changed = false;

    for (BlockView* block : _blocksToDelete) {
        const auto it = std::ranges::find(_blocks, block);
        if (it == _blocks.end()) {
            continue;
        }

        auto& b = const_cast<Block&>(block->block());
        _blockList->remove(*block);
        _database.remove(b);
        _blocks.erase(it);
        changed = true;
    }

    _blocksToDelete.clear();

    if (changed) {
        updateLayout();
    }
}

void BlocksView::resizeBlock(BlockView& block) {
    const float blockHeight = block.expanded
        ? nonNegative(_availableHeight - kExpandedHeightReduction)
        : kBlockHeight;

    block.resize({_scrollView.rect().size.x, blockHeight});
}

void BlocksView::updateLayout() {
    _blockList->updateLayout();
    _scrollView.setContentSize(_blockList->rect().size);

    _addButton.reposition({_rect.origin.x + _rect.size.x - kAddBlockButtonSize, 0});

    _totalLabel.reposition({_rect.origin.x + 5.0f, _rect.origin.y + _rect.size.y - 22});
}

void BlocksView::resize(const Point size) {
    setRect({_rect.origin, size});
}

void BlocksView::reposition(const Point position) {
    setRect({position, _rect.size});
}

void BlocksView::setRect(const Rect rect) {
    _rect = rect;
    _preferredRect = rect;

    const float contentWidth = nonNegative(rect.size.x - kAddBlockButtonSize - 5.0f);
    const float contentHeight = nonNegative(rect.size.y - kVerticalInset);

    _scrollView.setRect({
        0,
        0,
        contentWidth,
        contentHeight
    });

    for (BlockView* block : _blocks) {
        resizeBlock(*block);
    }

    _blockList->resize({contentWidth, _blockList->rect().size.y});
    updateLayout();
}

const Rect& BlocksView::rect() {
    return _rect;
}

Point BlocksView::minimumSize() const {
    return {0, 0};
}

Point BlocksView::preferredSize() const {
    return _preferredRect.size;
}

bool BlocksView::handleEvent(const SDL_Event& event) {
    bool status = true;
    if (!_addButton.handleEvent(event)) {
        status = false;
        goto end;
    }
    if (!_scrollView.handleEvent(event)) {
        status = false;
    }

end:
    return status;
}

void BlocksView::tick(Clock& clock, const double dt) {
    _scrollView.tick(clock, dt);
}

void BlocksView::renderImpl(Window& window) {
    _scrollView.render(window);

    _addButton.render(window);

    auto startY = _rect.origin.y + _rect.size.y - 60.0f - kVerticalInset;
    DrawScrollGradient(
        window,
        static_cast<int>(_scrollView.rect().size.x),
        0,
        static_cast<int>(
            startY
        ),
        60,
        {20, 20, 20}
    );

    window->setRenderColor({20, 20, 20});
    window->renderRect({0, startY + 60, _scrollView.rect().size.x, 10});

    _totalLabel.render(window);
}
