
#include "UI/BlocksView.h"

#include <algorithm>
#include <cinttypes>
#include <ranges>
#include <string>
#include <utility>

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
    int currentMonth,
    int currentYear
)
    : _font(font),
      _database(database),
      _currentMonth(currentMonth),
      _currentYear(currentYear),
      _availableHeight(rect.size.y),
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
      ) {
    _rect = rect;
    _preferredRect = rect;
    _window = &window;

    _blockList = &_scrollView.add<List>(
        Rect{0, 0, nonNegative(rect.size.x), 100}
    );

    _blockList->setSpacing(kBlockSpacing);

    _scrollView.parent = this;
    _addButton.parent = this;

    _addButton.setOnClick([this] {
        addBlock();
    });

    createBlockViews();
    updateLayout();
}

void BlocksView::createBlockViews() {
    _blocks.clear();

    _blocks.reserve(_database.blocks().size());

    for (auto& block : _database.blocks()) {
        addBlockView(*block);
    }

    _spacer = &_blockList->add<Spacer>(
        Rect{
            0,
            0,
            nonNegative(_rect.size.x),
            kBottomSpacerHeight
        }
    );

    updateLayout();
}

void BlocksView::addBlockView(Block& block, bool moveSpacer) {
    if (moveSpacer && _spacer) {
        _blockList->remove(*_spacer);
        _spacer = nullptr;
    }

    auto& blockView = _blockList->add<BlockView>(
        Rect{
            0,
            0,
            nonNegative(_scrollView.rect().size.x),
            kBlockHeight
        },
        block,
        _font,
        [this](BlockView& view) {
            handleExpansion(view);
        },
        [this](BlockView& view) {
            showDeleteConfirmation(view);
        },
        [this](BlockView&) {
            addBlock();
        }
    );

    _blocks.push_back(&blockView);

    if (moveSpacer) {
        _spacer = &_blockList->add<Spacer>(
            Rect{
                0,
                0,
                nonNegative(_rect.size.x),
                kBottomSpacerHeight
            }
        );
    }
}

void BlocksView::addBlock() {
    if (!_window) {
        return;
    }

    Dialog& dialog = Dialog::create(*_window);

    auto& c = dialog.content();
    auto& content = c.add<List>(Rect{0, 0, c.rect().size});
    auto& buttons = dialog.buttons();

    content.add<Label>(
        "Create new block",
        Color::white,
        Point{0, 0},
        _font
    );

    content.add<Spacer>(Rect{0, 0, 10, 20});

    content.add<Label>(
        "Name:",
        Color::white,
        Point{0, 32},
        _font
    );

    auto& nameTextbox = content.add<Textbox>(
        Rect{0, 64, 200, 0},
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

    auto& createButton = buttons.add<Button>(
        Rect{0, 0, 100, 100},
        "Create",
        style
    );

    createButton.setOnClick([this, &nameTextbox, &dialog] {
        Block block{};
        block.name = nameTextbox.getText();

        Block& newBlock = *_database.addBlock(std::move(block));

        addBlockView(newBlock, true);
        updateLayout();

        dialog.close();
    });
}

void BlocksView::handleExpansion(BlockView& block) {
    updateLayout();

    if (block.expanded) {
        _scrollView.setScrollSpeed(0);

        const auto it = std::ranges::find(_blocks, &block);

        if (it != _blocks.end()) {
            const auto index = static_cast<std::size_t>(
                it - _blocks.begin()
            );

            _scrollView.scrollTo({
                0,
                static_cast<float>(index) *
                    (kBlockHeight + kBlockSpacing)
            });
        }
    } else {
        _scrollView.setScrollSpeed(30);
    }
}

void BlocksView::showDeleteConfirmation(BlockView& block) {
    if (!_window) {
        return;
    }

    Dialog& dialog = Dialog::create(*_window);

    auto& content = dialog.content();
    auto& buttons = dialog.buttons();

    content.add<Label>(
        "Are you sure?",
        Color::white,
        Point{0, 0},
        _font
    );

    content.add<Label>(
        fmt(
            "You're about to delete a block called: \"%s\"",
            block.block().name.c_str()
        ),
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
        if (std::ranges::find(_blocksToDelete, &block) ==
            _blocksToDelete.end()) {
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

        Block& model = block->block();

        // Destroy the view before removing the model it references.
        _blockList->remove(*block);
        _database.remove(model);
        _blocks.erase(it);

        changed = true;
    }

    _blocksToDelete.clear();

    if (changed) {
        updateLayout();
    }
}

void BlocksView::updateLayout() {
    const float width = nonNegative(_rect.size.x);
    const float height = nonNegative(_rect.size.y);

    _availableHeight = height;

    const float contentWidth = nonNegative(
        width - kAddBlockButtonSize - 5.0f
    );

    const float contentHeight = nonNegative(
        height - kVerticalInset
    );

    // Resize the scroll viewport.
    _scrollView.setRect({
        0,
        0,
        contentWidth,
        contentHeight
    });

    // Resize each block, preserving its expanded state.
    for (BlockView* block : _blocks) {
        const float blockHeight = block->expanded
            ? nonNegative(contentHeight - kExpandedHeightReduction)
            : kBlockHeight;

        block->resize({
            contentWidth,
            blockHeight
        });
    }

    // Keep the bottom spacer aligned with the content width.
    if (_spacer) {
        _spacer->resize({
            contentWidth,
            kBottomSpacerHeight
        });
    }

    // Resize the list and recalculate its children.
    _blockList->resize({
        contentWidth,
        _blockList->rect().size.y
    });

    _blockList->updateLayout();

    // Match the scrollable content to the completed list layout.
    _scrollView.setContentSize(_blockList->rect().size);

    // Position the add button against the right edge.
    _addButton.reposition({
        _rect.origin.x + width - kAddBlockButtonSize,
        0
    });
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
    processDeletions();

    if (!_addButton.handleEvent(event)) {
        return false;
    }

    return _scrollView.handleEvent(event);
}

void BlocksView::tick(Clock& clock, const double dt) {
    _scrollView.tick(clock, dt);
}

void BlocksView::renderImpl(Window& window) {
    _scrollView.render(window);
    _addButton.render(window);

    const float startY =
        _rect.origin.y +
        _rect.size.y -
        kBottomSpacerHeight -
        kVerticalInset;

    DrawScrollGradient(
        window,
        static_cast<int>(_scrollView.rect().size.x),
        0,
        static_cast<int>(startY),
        static_cast<int>(kBottomSpacerHeight),
        {20, 20, 20}
    );

    window->setRenderColor({20, 20, 20});

    window->renderRect({
        0,
        startY + kBottomSpacerHeight,
        _scrollView.rect().size.x,
        10
    });

    const std::string totalText = fmt(
        "Total: %" PRId64 "kr | Date: %d-%02d",
        _database.total(),
        _currentYear,
        _currentMonth
    );

    window->renderText(
        _font,
        totalText.c_str(),
        _rect.origin.x + 5.0f,
        _rect.origin.y + _rect.size.y - 22.0f,
        Color::white
    );
}
