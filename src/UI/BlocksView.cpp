
#include "UI/BlocksView.h"

#include <algorithm>
#include <cmath>
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
constexpr float kDragHandleSize = 24.0f;
constexpr float kDragHandleInset = 5.0f;
constexpr float kItemHeight = 100.0f;
constexpr float kItemRowSpacing = 10.0f;
constexpr float kDragStartDistance = 5.0f;
constexpr float kAutoScrollEdge = 28.0f;
constexpr float kAutoScrollSpeed = 520.0f;

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


bool BlocksView::beginDrag(const Point position) {
    const Point listPosition = position + _scrollView.scrollPosition();

    // Item handles take precedence over the block handle because an expanded
    // block can have its last item close to the block's lower-right corner.
    for (std::size_t blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex) {
        BlockView* blockView = _blocks[blockIndex];

        if (!blockView->expanded) {
            continue;
        }

        // The nested ScrollView's rectangle is already in the outer
        // block-list coordinate space; item rectangles are in its content
        // coordinate space.
        const Rect itemViewport = blockView->_itemScrollView->rect();

        if (!itemViewport.isPointInside(listPosition)) {
            continue;
        }

        const Point itemPosition =
            listPosition - itemViewport.origin +
            blockView->_itemScrollView->scrollPosition();

        for (std::size_t itemIndex = 0;
             itemIndex < blockView->_itemViews.size();
             ++itemIndex) {
            ItemView* itemView = blockView->_itemViews[itemIndex];
            const Rect itemRect = itemView->rect();

            const Rect handleRect{
                itemRect.origin.x + itemRect.size.x -
                    kDragHandleSize - kDragHandleInset,
                itemRect.origin.y + itemRect.size.y -
                    kDragHandleSize - kDragHandleInset,
                kDragHandleSize,
                kDragHandleSize
            };

            if (!handleRect.isPointInside(itemPosition)) {
                continue;
            }

            _drag = DragState{
                .type = DragType::Item,
                .blockView = blockView,
                .itemView = itemView,
                .sourceIndex = itemIndex,
                .dropIndex = itemIndex,
                .startPosition = position,
                .pointerPosition = position,
                .grabOffset = itemPosition - itemRect.origin
            };
            return true;
        }
    }

    for (std::size_t blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex) {
        BlockView* blockView = _blocks[blockIndex];
        const Rect blockRect = blockView->rect();

        const Rect handleRect{
            blockRect.origin.x + blockRect.size.x -
                kDragHandleSize - kDragHandleInset,
            blockRect.origin.y + blockRect.size.y -
                kDragHandleSize - kDragHandleInset,
            kDragHandleSize,
            kDragHandleSize
        };

        if (!handleRect.isPointInside(listPosition)) {
            continue;
        }

        _drag = DragState{
            .type = DragType::Block,
            .blockView = blockView,
            .sourceIndex = blockIndex,
            .dropIndex = blockIndex,
            .startPosition = position,
            .pointerPosition = position,
            .grabOffset = listPosition - blockRect.origin
        };
        return true;
    }

    return false;
}

void BlocksView::updateDrag(const Point position) {
    if (!_drag) {
        return;
    }

    _drag->pointerPosition = position;

    const float dx = position.x - _drag->startPosition.x;
    const float dy = position.y - _drag->startPosition.y;

    if (!_drag->moved &&
        dx * dx + dy * dy >=
            kDragStartDistance * kDragStartDistance) {
        _drag->moved = true;
    }

    if (!_drag->moved) {
        return;
    }

    if (_drag->type == DragType::Block && _drag->blockView) {
        _drag->blockView->setZIndex(1000);
    } else if (_drag->type == DragType::Item && _drag->itemView) {
        _drag->blockView->_manualItemLayout = true;
        _drag->itemView->setZIndex(1000);
    }

    const Point listPosition = position + _scrollView.scrollPosition();

    if (_drag->type == DragType::Block) {
        _drag->blockView->reposition(listPosition - _drag->grabOffset);
        std::size_t destination = 0;

        for (BlockView* blockView : _blocks) {
            if (blockView == _drag->blockView) {
                continue;
            }

            const Rect rect = blockView->rect();
            const float midpoint = rect.origin.y + rect.size.y * 0.5f;

            if (listPosition.y >= midpoint) {
                ++destination;
            }
        }

        _drag->dropIndex = _blocks.size() <= 1
            ? 0
            : std::min(destination, _blocks.size() - 1);
        return;
    }

    BlockView* blockView = _drag->blockView;
    if (!blockView || !blockView->expanded ||
        blockView->_itemViews.empty()) {
        _drag->dropIndex = 0;
        return;
    }

    const Rect viewport = blockView->_itemScrollView->rect();
    const Point itemPosition =
        listPosition - viewport.origin +
        blockView->_itemScrollView->scrollPosition();
    _drag->itemView->reposition(itemPosition - _drag->grabOffset);

    const float rowHeight = kItemHeight + kItemRowSpacing;
    const int row = std::max(
        0,
        static_cast<int>(std::floor(
            (itemPosition.y + kItemRowSpacing * 0.5f) / rowHeight
        ))
    );

    std::size_t destination = 0;

    for (std::size_t index = 0;
         index < blockView->_itemViews.size();
         ++index) {
        if (index == _drag->sourceIndex) {
            continue;
        }

        const Rect itemRect = blockView->_itemViews[index]->rect();
        const int itemRow = static_cast<int>(std::floor(
            (itemRect.origin.y + kItemRowSpacing * 0.5f) / rowHeight
        ));

        if (itemRow < row ||
            (itemRow == row && itemRect.center().x < itemPosition.x)) {
            ++destination;
        }
    }

    _drag->dropIndex = blockView->_itemViews.size() <= 1
        ? 0
        : std::min(destination, blockView->_itemViews.size() - 1);

    blockView->_itemDropIndex = _drag->dropIndex;
    blockView->_itemDropSourceIndex = _drag->sourceIndex;
}

void BlocksView::clearDragIndicators() {
    _hasDragPlaceholder = false;

    for (BlockView* blockView : _blocks) {
        blockView->_itemDropIndex.reset();
        blockView->_itemDropSourceIndex.reset();
        blockView->_manualItemLayout = false;
        blockView->setZIndex(0);

        for (ItemView* itemView : blockView->_itemViews) {
            itemView->setZIndex(0);
        }

        blockView->layoutItemViews();
    }

    if (_blockList) {
        _blockList->updateLayout();
    }
}

void BlocksView::animateDragLayout(const float dt) {
    if (!_drag || !_drag->moved) {
        return;
    }

    const float blend = 1.0f - std::exp(-18.0f * std::max(0.0f, dt));
    const Point outerScroll = _scrollView.scrollPosition();
    _hasDragPlaceholder = false;

    if (_drag->type == DragType::Block && _drag->blockView) {
        BlockView* dragged = _drag->blockView;
        float y = _blocks.empty() ? 0.0f : _blocks.front()->rect().origin.y;
        std::size_t remainingIndex = 0;

        for (BlockView* blockView : _blocks) {
            if (blockView == dragged) {
                continue;
            }

            if (remainingIndex == _drag->dropIndex) {
                _dragPlaceholder = {
                    {blockView->rect().origin.x, y - outerScroll.y},
                    blockView->rect().size
                };
                _hasDragPlaceholder = true;
                y += dragged->rect().size.y + kBlockSpacing;
            }

            const Rect current = blockView->rect();
            blockView->reposition({
                current.origin.x,
                current.origin.y + (y - current.origin.y) * blend
            });

            y += current.size.y + kBlockSpacing;
            ++remainingIndex;
        }

        if (!_hasDragPlaceholder) {
            const Rect current = dragged->rect();
            _dragPlaceholder = {
                {current.origin.x, y - outerScroll.y},
                current.size
            };
            _hasDragPlaceholder = true;
        }

        const Point pointerInList =
            _drag->pointerPosition + outerScroll;
        dragged->reposition(pointerInList - _drag->grabOffset);
        return;
    }

    if (_drag->type != DragType::Item ||
        !_drag->blockView || !_drag->itemView) {
        return;
    }

    BlockView* blockView = _drag->blockView;
    const auto& itemViews = blockView->_itemViews;
    const Rect viewport = blockView->_itemScrollView->rect();
    const Point innerScroll = blockView->_itemScrollView->scrollPosition();
    const float availableWidth =
        std::max(0.0f, blockView->rect().size.x - 40.0f);

    float x = 0.0f;
    float y = 0.0f;
    std::size_t remainingIndex = 0;
    const float rowStep = kItemHeight + kItemRowSpacing;

    auto placePlaceholder = [&] {
        const Rect itemRect = _drag->itemView->rect();
        _dragPlaceholder = {
            viewport.origin + Point{x - innerScroll.x, y - innerScroll.y} -
                outerScroll,
            itemRect.size
        };
        _hasDragPlaceholder = true;
        x += itemRect.size.x + kItemRowSpacing;
        if (x > 0.0f && x > availableWidth) {
            x = 0.0f;
            y += rowStep;
        }
    };

    for (ItemView* itemView : itemViews) {
        if (itemView == _drag->itemView) {
            continue;
        }

        if (remainingIndex == _drag->dropIndex) {
            placePlaceholder();
        }

        const Rect current = itemView->rect();
        const float width = itemView->preferredSize().x;

        if (x > 0.0f && x + width > availableWidth) {
            x = 0.0f;
            y += rowStep;
        }

        const Point target{x, y};
        itemView->reposition({
            current.origin.x + (target.x - current.origin.x) * blend,
            current.origin.y + (target.y - current.origin.y) * blend
        });

        x += width + kItemRowSpacing;
        ++remainingIndex;
    }

    if (!_hasDragPlaceholder) {
        placePlaceholder();
    }

    const Point listPosition =
        _drag->pointerPosition + outerScroll;
    const Point itemPosition =
        listPosition - viewport.origin + innerScroll;
    _drag->itemView->reposition(itemPosition - _drag->grabOffset);
}

void BlocksView::finishDrag() {
    if (!_drag) {
        return;
    }

    updateDrag(_drag->pointerPosition);

    const DragState drag = *_drag;
    clearDragIndicators();
    _drag.reset();

    if (!drag.moved || drag.dropIndex == drag.sourceIndex) {
        return;
    }

    if (drag.type == DragType::Block) {
        reorderBlock(drag.sourceIndex, drag.dropIndex);
    } else if (drag.blockView) {
        reorderItem(
            *drag.blockView,
            drag.sourceIndex,
            drag.dropIndex
        );
    }
}

void BlocksView::reorderBlock(
    const std::size_t source,
    const std::size_t destination
) {
    auto& blocks = _database.blocks();

    if (source >= blocks.size() ||
        destination >= blocks.size() ||
        source == destination) {
        return;
    }

    if (destination < source) {
        std::rotate(
            blocks.begin() + static_cast<std::ptrdiff_t>(destination),
            blocks.begin() + static_cast<std::ptrdiff_t>(source),
            blocks.begin() + static_cast<std::ptrdiff_t>(source + 1)
        );
    } else {
        std::rotate(
            blocks.begin() + static_cast<std::ptrdiff_t>(source),
            blocks.begin() + static_cast<std::ptrdiff_t>(source + 1),
            blocks.begin() + static_cast<std::ptrdiff_t>(destination + 1)
        );
    }

    rebuildBlockViews();
}

void BlocksView::reorderItem(
    BlockView& blockView,
    const std::size_t source,
    const std::size_t destination
) {
    auto& items = blockView.block().items;

    if (source >= items.size() ||
        destination >= items.size() ||
        source == destination) {
        return;
    }

    if (destination < source) {
        std::rotate(
            items.begin() + static_cast<std::ptrdiff_t>(destination),
            items.begin() + static_cast<std::ptrdiff_t>(source),
            items.begin() + static_cast<std::ptrdiff_t>(source + 1)
        );
    } else {
        std::rotate(
            items.begin() + static_cast<std::ptrdiff_t>(source),
            items.begin() + static_cast<std::ptrdiff_t>(source + 1),
            items.begin() + static_cast<std::ptrdiff_t>(destination + 1)
        );
    }

    blockView.rebuildItemViews();
}

void BlocksView::rebuildBlockViews() {
    struct SavedBlockState {
        Block* block;
        bool expanded;
        Point itemScroll;
    };

    std::vector<SavedBlockState> savedStates;
    savedStates.reserve(_blocks.size());

    for (BlockView* blockView : _blocks) {
        savedStates.push_back({
            &blockView->block(),
            blockView->expanded,
            blockView->_itemScrollView->scrollPosition()
        });
    }

    const Point previousScroll = _scrollView.scrollPosition();

    // List owns the views; clear it before rebuilding so no view keeps a
    // reference to a model that is about to be re-ordered or destroyed.
    _blockList->clear();
    _blocks.clear();
    _blocksToDelete.clear();
    _spacer = nullptr;

    createBlockViews();

    for (BlockView* blockView : _blocks) {
        const auto state = std::ranges::find_if(
            savedStates,
            [blockView](const SavedBlockState& entry) {
                return entry.block == &blockView->block();
            }
        );

        if (state != savedStates.end()) {
            blockView->setExpanded(state->expanded);
        }
    }

    updateLayout();

    for (BlockView* blockView : _blocks) {
        const auto state = std::ranges::find_if(
            savedStates,
            [blockView](const SavedBlockState& entry) {
                return entry.block == &blockView->block();
            }
        );

        if (state != savedStates.end()) {
            blockView->_itemScrollView->scrollTo(state->itemScroll);
        }
    }

    _scrollView.scrollTo(previousScroll);
}

bool BlocksView::handleEvent(const Event& event) {
    if (_drag) {
        if (event.type == EventType::WindowUnfocused) {
            clearDragIndicators();
            _drag.reset();
            return true;
        }

        if (event.type == EventType::MouseMoved) {
            updateDrag(event.mouseMove.position);
            return false;
        }

        if (event.type == EventType::MouseButtonReleased &&
            event.mouseButton.button == MouseButton::Left) {
            updateDrag(event.mouseButton.position);
            finishDrag();
            return false;
        }

        // Keep the pointer gesture captured until its matching release.
        return false;
    }

    processDeletions();

    if (event.type == EventType::MouseButtonPressed &&
        event.mouseButton.button == MouseButton::Left &&
        beginDrag(event.mouseButton.position)) {
        return false;
    }

    if (!_addButton.handleEvent(event)) {
        return false;
    }

    return _scrollView.handleEvent(event);
}

void BlocksView::tick(Clock& clock, const double dt) {
    _scrollView.tick(clock, dt);

    if (!_drag || !_drag->moved) {
        return;
    }

    const float deltaTime = static_cast<float>(std::max(0.0, dt));
    const float viewportHeight = _scrollView.rect().size.y;
    const float pointerY = _drag->pointerPosition.y;

    if (pointerY < kAutoScrollEdge) {
        const float strength = std::clamp(
            (kAutoScrollEdge - pointerY) / kAutoScrollEdge,
            0.0f,
            1.0f
        );
        _scrollView.scrollBy({
            0.0f,
            -kAutoScrollSpeed * strength * deltaTime
        });
    } else if (pointerY > viewportHeight - kAutoScrollEdge) {
        const float strength = std::clamp(
            (pointerY - (viewportHeight - kAutoScrollEdge)) / kAutoScrollEdge,
            0.0f,
            1.0f
        );
        _scrollView.scrollBy({
            0.0f,
            kAutoScrollSpeed * strength * deltaTime
        });
    }

    // The inner item list has its own scroll position. Let an item drag
    // autoscroll it as well when the pointer reaches the top or bottom edge.
    if (_drag->type == DragType::Item && _drag->blockView) {
        BlockView* blockView = _drag->blockView;
        const Point listPosition =
            _drag->pointerPosition + _scrollView.scrollPosition();
        const Rect itemViewport = blockView->_itemScrollView->rect();

        if (itemViewport.isPointInside(listPosition)) {
            const float viewportTop = itemViewport.origin.y;
            const float viewportBottom = viewportTop + itemViewport.size.y;

            if (listPosition.y < viewportTop + kAutoScrollEdge) {
                const float strength = std::clamp(
                    (viewportTop + kAutoScrollEdge - listPosition.y) /
                        kAutoScrollEdge,
                    0.0f,
                    1.0f
                );
                blockView->_itemScrollView->scrollBy({
                    0.0f,
                    -kAutoScrollSpeed * strength * deltaTime
                });
            } else if (listPosition.y > viewportBottom - kAutoScrollEdge) {
                const float strength = std::clamp(
                    (listPosition.y - (viewportBottom - kAutoScrollEdge)) /
                        kAutoScrollEdge,
                    0.0f,
                    1.0f
                );
                blockView->_itemScrollView->scrollBy({
                    0.0f,
                    kAutoScrollSpeed * strength * deltaTime
                });
            }
        }
    }

    updateDrag(_drag->pointerPosition);
    animateDragLayout(deltaTime);
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

    if (_blocks.empty()) {
        window->renderText(
            _font,
            "No blocks yet. Press + to create a new one ->",
            _rect.origin.x + _rect.size.x - 490,
            _rect.origin.y + 5.0f,
            Color::white
        );
    }

    if (_drag && _drag->moved && _hasDragPlaceholder) {
        window->setRenderColor({12, 12, 12});
        window->renderRoundedRect(_dragPlaceholder, 10);
    }}
