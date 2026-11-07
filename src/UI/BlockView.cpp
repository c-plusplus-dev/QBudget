
//
// Created by Ayaan on 2026-10-15.
//

#include "BlockView.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <cinttypes>
#include <charconv>
#include <QTipUIKit/Containers/List.h>

#include "Helpers.h"
#include "Spacer.h"

using namespace QTip;

namespace {
constexpr float kCollapsedHeight = 100.0f;

constexpr float kItemHeight = 100.0f;
constexpr float kItemSpacing = 10.0f;
constexpr float kItemMargin = 20.0f;
constexpr float kDragHandleSize = 24.0f;
constexpr float kDragHandleInset = 5.0f;
constexpr float kDragStartDistance = 5.0f;
constexpr float kAutoScrollEdge = 28.0f;
constexpr float kAutoScrollSpeed = 520.0f;
constexpr float kItemTop = 45.0f;

constexpr float kButtonSize = 30.0f;
constexpr float kButtonInset = 5.0f;
constexpr float kButtonSpacing = 5.0f;

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

bool parse_int32(const std::string& input, int32_t& result) {
    auto [ptr, ec] = std::from_chars(
        input.data(),
        input.data() + input.size(),
        result
    );

    return ec == std::errc{} && ptr == input.data() + input.size();
}
}

BlockView::BlockView(
    Rect rect,
    Block& block,
    Font& font,
    Action expandRequested,
    Action deleteRequested,
    Action addRequested
)
    : _block(block),
      _expandRequested(std::move(expandRequested)),
      _deleteRequested(std::move(deleteRequested)),
      _addRequested(std::move(addRequested)),
      _font(font) {
    _rect = rect;
    _preferredRect = rect;

    ButtonStyle style{};
    style.color = Color::clear;
    style.hoverColor = {40, 40, 40};
    style.pressedColor = {50, 50, 50};
    style.font = font;
    style.fontColor = Color::white;

    _delete = std::make_unique<Button>(
        0, 0, kButtonSize, kButtonSize, "━", style
    );

    _expand = std::make_unique<Button>(
        0, 0, kButtonSize, kButtonSize, "◀", style
    );

    _add = std::make_unique<MagickButton>(
        Rect{0, 0, kButtonSize, kButtonSize},
        style,
        drawAddIcon
    );

    _itemScrollView = std::make_unique<ScrollView>(
        Rect{},
        ScrollViewSettings{
            .direction = ScrollDirection::Vertical
        }
    );

    _delete->parent = this;
    _expand->parent = this;
    _add->parent = this;
    _itemScrollView->parent = this;

    _delete->setOnClick([this] {
        if (_deleteRequested) {
            _deleteRequested(*this);
        }
    });

    _expand->setOnClick([this] {
        expanded = !expanded;
        _expand->setText(expanded ? "▼" : "◀");

        syncItemViews();
        onResize();

        if (_expandRequested) {
            _expandRequested(*this);
        }
    });

    _add->setOnClick([this] {
        onAddItem();
    });

    syncItemViews();
    onResize();
}

Block& BlockView::block() noexcept {
    return _block;
}

const Block& BlockView::block() const noexcept {
    return _block;
}

BlockView::State BlockView::saveState() const {
    return {
        .expanded = expanded,
        .itemScrollPosition = _itemScrollView->scrollPosition()
    };
}

void BlockView::restoreItemScrollPosition(const Point position) {
    _itemScrollView->scrollTo(position);
}

bool BlockView::handleItemDragEvent(
    const Event& event,
    const Point contentPosition,
    const bool allowStart
) {
    if (_itemDrag) {
        if (event.type == EventType::WindowUnfocused) {
            cancelItemDrag();
            return true;
        }

        if (event.type == EventType::MouseMoved) {
            updateItemDrag(contentPosition);
        } else if (event.type == EventType::MouseButtonReleased &&
                   event.mouseButton.button == MouseButton::Left) {
            updateItemDrag(contentPosition);
            finishItemDrag();
        }

        // Keep the pointer gesture captured until its matching release.
        return false;
    }

    if (allowStart &&
        event.type == EventType::MouseButtonPressed &&
        event.mouseButton.button == MouseButton::Left &&
        beginItemDrag(contentPosition)) {
        return false;
    }

    return true;
}

void BlockView::setExpanded(const bool value) {
    if (expanded == value) {
        return;
    }

    expanded = value;
    _expand->setText(expanded ? "▼" : "◀");
    syncItemViews();
    onResize();
}

bool BlockView::handleEvent(const Event& event) {
    if (!_expand->handleEvent(event)) {
        return false;
    }

    if (expanded) {
        processDeletions();
        syncItemViews();

        if (!_add->handleEvent(event)) {
            return false;
        }

        if (!_itemScrollView->handleEvent(event)) {
            return false;
        }
    } else if (!_delete->handleEvent(event)) {
        return false;
    }

    return UIObject::handleEvent(event);
}

void BlockView::tick(Clock& clock, const double dt) {
    _itemScrollView->tick(clock, dt);

    if (!_itemDrag || !_itemDrag->moved) {
        return;
    }

    const float deltaTime = static_cast<float>(std::max(0.0, dt));

    // The outer ScrollView owns the block list. While an item is being
    // dragged, keep the pointer anchored to the cursor as that view scrolls.
    ScrollView* outerScrollView = nullptr;
    for (UIObject* ancestor = parent; ancestor; ancestor = ancestor->parent) {
        if (auto* scrollView = dynamic_cast<ScrollView*>(ancestor)) {
            outerScrollView = scrollView;
            break;
        }
    }

    if (outerScrollView) {
        const Point oldScroll = outerScrollView->scrollPosition();
        const Rect outerViewport = outerScrollView->rect();
        const Point pointerInViewport =
            _itemDrag->pointerPosition - oldScroll + outerViewport.origin;

        if (pointerInViewport.y < outerViewport.origin.y + kAutoScrollEdge) {
            const float strength = std::clamp(
                (outerViewport.origin.y + kAutoScrollEdge -
                    pointerInViewport.y) / kAutoScrollEdge,
                0.0f,
                1.0f
            );
            outerScrollView->scrollBy({
                0.0f,
                -kAutoScrollSpeed * strength * deltaTime
            });
        } else if (pointerInViewport.y >
                   outerViewport.origin.y + outerViewport.size.y -
                       kAutoScrollEdge) {
            const float edge = outerViewport.origin.y +
                outerViewport.size.y - kAutoScrollEdge;
            const float strength = std::clamp(
                (pointerInViewport.y - edge) / kAutoScrollEdge,
                0.0f,
                1.0f
            );
            outerScrollView->scrollBy({
                0.0f,
                kAutoScrollSpeed * strength * deltaTime
            });
        }

        _itemDrag->pointerPosition +=
            outerScrollView->scrollPosition() - oldScroll;
    }

    const Rect itemViewport = _itemScrollView->rect();
    if (itemViewport.isPointInside(_itemDrag->pointerPosition)) {
        const float viewportTop = itemViewport.origin.y;
        const float viewportBottom = viewportTop + itemViewport.size.y;

        if (_itemDrag->pointerPosition.y <
            viewportTop + kAutoScrollEdge) {
            const float strength = std::clamp(
                (viewportTop + kAutoScrollEdge -
                    _itemDrag->pointerPosition.y) / kAutoScrollEdge,
                0.0f,
                1.0f
            );
            _itemScrollView->scrollBy({
                0.0f,
                -kAutoScrollSpeed * strength * deltaTime
            });
        } else if (_itemDrag->pointerPosition.y >
                   viewportBottom - kAutoScrollEdge) {
            const float strength = std::clamp(
                (_itemDrag->pointerPosition.y -
                    (viewportBottom - kAutoScrollEdge)) / kAutoScrollEdge,
                0.0f,
                1.0f
            );
            _itemScrollView->scrollBy({
                0.0f,
                kAutoScrollSpeed * strength * deltaTime
            });
        }
    }

    updateItemDrag(_itemDrag->pointerPosition);
    animateItemDragLayout(deltaTime);
}

bool BlockView::beginItemDrag(const Point position) {
    if (!expanded || _itemViews.empty()) {
        return false;
    }

    const Rect itemViewport = _itemScrollView->rect();
    if (!itemViewport.isPointInside(position)) {
        return false;
    }

    const Point itemPosition =
        position - itemViewport.origin +
        _itemScrollView->scrollPosition();

    for (std::size_t itemIndex = 0;
         itemIndex < _itemViews.size();
         ++itemIndex) {
        ItemView* itemView = _itemViews[itemIndex];
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

        _itemDrag = ItemDragState{
            .itemView = itemView,
            .sourceIndex = itemIndex,
            .dropIndex = itemIndex,
            .startPosition = position,
            .pointerPosition = position,
            .grabOffset = itemPosition - itemRect.origin
        };
        return true;
    }

    return false;
}

void BlockView::updateItemDrag(const Point position) {
    if (!_itemDrag) {
        return;
    }

    _itemDrag->pointerPosition = position;

    const float dx = position.x - _itemDrag->startPosition.x;
    const float dy = position.y - _itemDrag->startPosition.y;

    if (!_itemDrag->moved &&
        dx * dx + dy * dy >=
            kDragStartDistance * kDragStartDistance) {
        _itemDrag->moved = true;
    }

    if (!_itemDrag->moved) {
        return;
    }

    _manualItemLayout = true;
    _itemDrag->itemView->setZIndex(1000);

    const Rect viewport = _itemScrollView->rect();
    const Point itemPosition =
        position - viewport.origin +
        _itemScrollView->scrollPosition();
    _itemDrag->itemView->reposition(
        itemPosition - _itemDrag->grabOffset
    );

    // Calculate the candidate insertion index from the normal layout,
    // not the currently animated rectangles. This keeps the target stable
    // while the other items are moving towards their preview positions.
    const float availableWidth =
        std::max(0.0f, _rect.size.x - 2.0f * kItemMargin);
    if (availableWidth <= 0.0f) {
        return;
    }

    const float rowHeight = kItemHeight + kItemSpacing;
    const int targetRow = std::max(
        0,
        static_cast<int>(std::floor(
            (itemPosition.y + kItemSpacing * 0.5f) / rowHeight
        ))
    );

    float x = 0.0f;
    float y = 0.0f;
    std::size_t destination = 0;

    for (ItemView* itemView : _itemViews) {
        if (itemView == _itemDrag->itemView) {
            continue;
        }

        const float width = itemView->preferredSize().x;

        if (x > 0.0f && x + width > availableWidth) {
            x = 0.0f;
            y += rowHeight;
        }

        const int itemRow = static_cast<int>(std::floor(
            (y + kItemSpacing * 0.5f) / rowHeight
        ));

        if (itemRow < targetRow ||
            (itemRow == targetRow &&
             x + width * 0.5f < itemPosition.x)) {
            ++destination;
        }

        x += width + kItemSpacing;
    }

    destination = _itemViews.size() <= 1
        ? 0
        : std::min(destination, _itemViews.size() - 1);

    // Do not shift the other items if the dragged item cannot occupy the
    // requested row. Keep the previous valid drop position instead.
    if (canPlaceDraggedItemAt(destination, targetRow)) {
        _itemDrag->dropIndex = destination;
    }
}

bool BlockView::canPlaceDraggedItemAt(
    const std::size_t destination,
    const int targetRow
) const {
    if (!_itemDrag || !_itemDrag->itemView) {
        return false;
    }

    const float availableWidth =
        std::max(0.0f, _rect.size.x - 2.0f * kItemMargin);
    const float draggedWidth =
        _itemDrag->itemView->preferredSize().x;

    // An item wider than the entire row cannot be inserted anywhere.
    if (availableWidth <= 0.0f || draggedWidth > availableWidth) {
        return false;
    }

    const float rowHeight = kItemHeight + kItemSpacing;
    float x = 0.0f;
    float y = 0.0f;
    std::size_t remainingIndex = 0;

    const auto fitsInTargetRow = [&] {
        // The item may wrap to the next row, but it must end up on the
        // row the pointer is targeting.
        if (x > 0.0f && x + draggedWidth > availableWidth) {
            x = 0.0f;
            y += rowHeight;
        }

        const int placementRow = static_cast<int>(std::floor(
            (y + kItemSpacing * 0.5f) / rowHeight
        ));

        return placementRow == targetRow;
    };

    for (ItemView* itemView : _itemViews) {
        if (itemView == _itemDrag->itemView) {
            continue;
        }

        if (remainingIndex == destination) {
            return fitsInTargetRow();
        }

        const float width = itemView->preferredSize().x;

        if (x > 0.0f && x + width > availableWidth) {
            x = 0.0f;
            y += rowHeight;
        }

        x += width + kItemSpacing;
        ++remainingIndex;
    }

    return fitsInTargetRow();
}

void BlockView::clearItemDragIndicators() {
    _hasItemDragPlaceholder = false;
    _manualItemLayout = false;

    for (ItemView* itemView : _itemViews) {
        itemView->setZIndex(0);
    }

    layoutItemViews();
}

void BlockView::cancelItemDrag() {
    if (!_itemDrag) {
        return;
    }

    clearItemDragIndicators();
    _itemDrag.reset();
}

void BlockView::finishItemDrag() {
    if (!_itemDrag) {
        return;
    }

    updateItemDrag(_itemDrag->pointerPosition);
    const ItemDragState drag = *_itemDrag;

    clearItemDragIndicators();
    _itemDrag.reset();

    auto& items = _block.items;
    if (!drag.moved ||
        drag.sourceIndex >= items.size() ||
        drag.dropIndex >= items.size() ||
        drag.sourceIndex == drag.dropIndex) {
        return;
    }

    if (drag.dropIndex < drag.sourceIndex) {
        std::rotate(
            items.begin() + static_cast<std::ptrdiff_t>(drag.dropIndex),
            items.begin() + static_cast<std::ptrdiff_t>(drag.sourceIndex),
            items.begin() + static_cast<std::ptrdiff_t>(drag.sourceIndex + 1)
        );
    } else {
        std::rotate(
            items.begin() + static_cast<std::ptrdiff_t>(drag.sourceIndex),
            items.begin() + static_cast<std::ptrdiff_t>(drag.sourceIndex + 1),
            items.begin() + static_cast<std::ptrdiff_t>(drag.dropIndex + 1)
        );
    }

    rebuildItemViews();
}

void BlockView::animateItemDragLayout(const float dt) {
    if (!_itemDrag || !_itemDrag->moved) {
        return;
    }

    const float blend = 1.0f - std::exp(-18.0f * std::max(0.0f, dt));
    const Rect viewport = _itemScrollView->rect();
    const Point innerScroll = _itemScrollView->scrollPosition();
    const float availableWidth =
        std::max(0.0f, _rect.size.x - 2.0f * kItemMargin);

    float x = 0.0f;
    float y = 0.0f;
    std::size_t remainingIndex = 0;
    const float rowStep = kItemHeight + kItemSpacing;

    auto placePlaceholder = [&] {
        const Rect itemRect = _itemDrag->itemView->rect();

        if (x > 0.0f && x + itemRect.size.x > availableWidth) {
            x = 0.0f;
            y += rowStep;
        }

        _itemDragPlaceholder = {
            viewport.origin + Point{x - innerScroll.x, y - innerScroll.y},
            itemRect.size
        };
        _hasItemDragPlaceholder = true;
        x += itemRect.size.x + kItemSpacing;
    };

    for (ItemView* itemView : _itemViews) {
        if (itemView == _itemDrag->itemView) {
            continue;
        }

        if (remainingIndex == _itemDrag->dropIndex) {
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

        x += width + kItemSpacing;
        ++remainingIndex;
    }

    if (!_hasItemDragPlaceholder) {
        placePlaceholder();
    }

    const Point itemPosition =
        _itemDrag->pointerPosition - viewport.origin + innerScroll;
    _itemDrag->itemView->reposition(
        itemPosition - _itemDrag->grabOffset
    );
}

Point BlockView::minimumSize() const {
    return {100.0f, kCollapsedHeight};
}

Point BlockView::preferredSize() const {
    return _preferredRect.size;
}

void BlockView::resize(Point size) {
    setRect({_rect.origin, size});
}

void BlockView::reposition(Point position) {
    setRect({position, _rect.size});
}

void BlockView::setRect(Rect rect) {
    _rect = rect;
    _preferredRect = rect;

    syncItemViews();
    onResize();
}

const Rect& BlockView::rect() {
    return _rect;
}

void BlockView::syncItemViews() {
    // Rebuild when the number of items changes so the views reference
    // the current elements in Block::items.
    if (_itemViews.size() != _block.items.size()) {
        _itemViews.clear();
        _itemScrollView->clear();
        _itemViews.reserve(_block.items.size());

        for (auto& item : _block.items) {
            addItem(*item);
        }
    }

    layoutItemViews();
}

void BlockView::rebuildItemViews() {
    const Point previousScroll = _itemScrollView->scrollPosition();

    _itemViews.clear();
    _itemScrollView->clear();
    _itemViews.reserve(_block.items.size());

    for (const auto& item : _block.items) {
        addItem(*item);
    }

    layoutItemViews();
    _itemScrollView->scrollTo(previousScroll);
}

void BlockView::layoutItemViews() const {
    if (_manualItemLayout) {
        return;
    }

    if (!expanded || _itemViews.empty()) {
        _itemScrollView->setContentSize({0.0f, 0.0f});
        return;
    }

    const float left = 0.0f;
    const float right = _rect.size.x - 2.0f * kItemMargin;

    float x = left;
    float y = 0.0f;
    int rows = 1;

    for (const auto& itemView : _itemViews) {
        const float itemWidth = itemView->preferredSize().x;

        if (x > left && x + itemWidth > right) {
            x = left;
            y += kItemHeight + kItemSpacing;
            ++rows;
        }

        itemView->setRect({
            {x, y},
            {itemWidth, kItemHeight}
        });

        x += itemWidth + kItemSpacing;
    }

    const float itemsHeight =
        static_cast<float>(rows) * kItemHeight +
        static_cast<float>(rows - 1) * kItemSpacing;

    _itemScrollView->setContentSize({right - left, itemsHeight});
}

void BlockView::onEdit(ItemView& itemView) {
    if (!_window)
        return;
    Dialog& dialog = Dialog::create(*_window);

    auto& c = dialog.content();
    auto& content = c.add<List>(Rect{0, 0, c.rect().size});
    auto& buttons = dialog.buttons();

    auto& item = itemView.item();

    content.add<Label>(
        "Edit Item",
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

    nameTextbox.type(item.name);

    content.add<Label>(
        "Cost:",
        Color::white,
        Point{0, 96},
        _font
    );

    auto& costTextbox = content.add<Textbox>(
        Rect{0, 128, 200, 0},
        _font
    );

    costTextbox.type(std::to_string(item.cost));

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

    auto& saveButton = buttons.add<Button>(
        Rect{0, 0, 100, 100},
        "Save",
        style
    );
    saveButton.setOnClick([&] {
        int32_t cost;
        if (!parse_int32(costTextbox.getText(), cost)) {
            std::cout << "Cost must be a number" << std::endl;
            return;
        }

        item.name = nameTextbox.getText();
        item.cost = cost;
        dialog.close();
    });
}

void BlockView::onDelete(ItemView& itemView) {
    if (!_window)
        return;
    Dialog& dialog = Dialog::create(*_window);

    auto& c = dialog.content();
    auto& content = c.add<List>(Rect{0, 0, c.rect().size});
    auto& buttons = dialog.buttons();

    content.add<Label>(
        "Are you sure?",
        Color::white,
        Point{0, 0},
        _font
    );

    content.add<Spacer>(Rect{0, 0, 10, 20});

    content.add<Label>(
        fmt("You're about to delete an item called: \"%s\"", itemView.item().name.c_str()),
        Color::white,
        Point{0, 32},
        _font
    );

    content.add<Label>(
        fmt("The item is worth: %" PRId32 "kr", itemView.item().cost),
        Color::white,
        Point{0, 52},
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
    deleteButton.setOnClick([this, &itemView, &dialog] {
        if (std::ranges::find(_itemsToDelete, &itemView) == _itemsToDelete.end()) {
            _itemsToDelete.push_back(&itemView);
        }
        dialog.close();
    });
}

void BlockView::onAddItem() {
    if (!_window)
        return;
    Dialog& dialog = Dialog::create(*_window);

    auto& c = dialog.content();
    auto& content = c.add<List>(Rect{0, 0, c.rect().size});
    auto& buttons = dialog.buttons();

    content.add<Label>(
        "Add Item",
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

    content.add<Label>(
        "Cost:",
        Color::white,
        Point{0, 96},
        _font
    );

    auto& costTextbox = content.add<Textbox>(
        Rect{0, 128, 200, 0},
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
    createButton.setOnClick([&] {
        int32_t cost;
        if (!parse_int32(costTextbox.getText(), cost)) {
            std::cout << "Cost must be a number" << std::endl;
            return;
        }
        _block.items.push_back(std::make_unique<Block::Item>(nameTextbox.getText(), cost));
        addItem(*_block.items.back());
        dialog.close();
    });
}

ItemView& BlockView::addItem(Block::Item& item) {
    auto& itemView = _itemScrollView->add<ItemView>(
        Rect{0, 0, 100, kItemHeight},
        item,
        _font,
        [this] (ItemView& item) {
            onEdit(item);
        },
        [this] (ItemView& item) {
            onDelete(item);
        }
    );

    _itemViews.push_back(&itemView);

    return itemView;
}

void BlockView::processDeletions() {
    for (auto item : _itemsToDelete) {
        const auto it = std::ranges::find(_itemViews, item);
        if (it == _itemViews.end()) {
            continue;
        }

        Block::Item& model = item->item();

        // Destroy the view before removing the model it references.
        _itemScrollView->remove(*item);
        _block.remove(model);
        _itemViews.erase(it);
    }

    _itemsToDelete.clear();
}

void BlockView::onResize() {
    _expand->reposition({
        _rect.origin.x + _rect.size.x - 35.0f,
        _rect.origin.y + kButtonInset
    });

    _delete->reposition({
        _expand->rect().origin.x - kButtonSize - kButtonSpacing,
        _rect.origin.y + kButtonInset
    });

    _add->reposition({
        _expand->rect().origin.x - kButtonSize - kButtonSpacing,
        _rect.origin.y + kButtonInset
    });

    const float left = _rect.origin.x + kItemMargin;
    const float top = _rect.origin.y + kItemTop;
    const float width = _rect.size.x - 2.0f * kItemMargin;

    _itemScrollView->setRect({
        {left, top},
        {width, _rect.size.y - kItemTop - kItemMargin - 10.0f}
    });

    layoutItemViews();
}

void BlockView::renderImpl(Window& window) {
    if (!_moveTexture) {
        _moveTexture = std::make_unique<Texture>(
            window,
            "assets/drag1.png"
        );
    }

    window->setRenderColor({30, 30, 30});
    window->renderRoundedRect(_rect, 10);

    window->renderText(
        _font,
        _block.name.c_str(),
        _rect.origin.x + 10.0f,
        _rect.origin.y + 10.0f,
        Color::white
    );

    _expand->render(window);

    if (expanded) {
        _add->render(window);

        syncItemViews();

        if (_itemDrag && _itemDrag->moved && _hasItemDragPlaceholder) {
            window->setRenderColor({12, 12, 12});
            window->renderRoundedRect(_itemDragPlaceholder, 10);
        }

        _itemScrollView->render(window);
    } else {
        _delete->render(window);
    }

    // Keep the block handle available in both expanded and collapsed states.
    const float handleSize = 24.0f;
    const float handleX = _rect.origin.x + _rect.size.x - handleSize - 5.0f;
    const float handleY = _rect.origin.y + _rect.size.y - handleSize - 5.0f;
    window->renderTexture(
        *_moveTexture,
        Rect::zero,
        {handleX, handleY, handleSize, handleSize}
    );

    const std::string total = std::to_string(_block.total()) + "kr";

    window->renderText(
        _font,
        total.c_str(),
        _rect.origin.x + 10.0f,
        _rect.origin.y + _rect.size.y - 26.0f,
        Color::white
    );
}
