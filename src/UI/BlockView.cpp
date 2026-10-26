
//
// Created by Ayaan on 2026-10-15.
//

#include "BlockView.h"

#include <algorithm>
#include <string>
#include <utility>
#include <cinttypes>

using namespace QTip;

namespace {
constexpr float kCollapsedHeight = 100.0f;

constexpr float kItemHeight = 100.0f;
constexpr float kItemSpacing = 10.0f;
constexpr float kItemMargin = 20.0f;
constexpr float kItemTop = 45.0f;

constexpr float kButtonSize = 30.0f;
constexpr float kButtonInset = 5.0f;
constexpr float kButtonSpacing = 5.0f;
}

BlockView::BlockView(
    Rect rect,
    Block& block,
    Font& font,
    OnExpandRequested expandRequested,
    OnDeleteRequested deleteRequested
)
    : _block(block),
      _expandRequested(std::move(expandRequested)),
      _deleteRequested(std::move(deleteRequested)),
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

    _itemScrollView = std::make_unique<ScrollView>(
        Rect{},
        ScrollViewSettings{
            .direction = ScrollDirection::Vertical
        }
    );

    _delete->parent = this;
    _expand->parent = this;
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

    syncItemViews();
    onResize();
}

Block& BlockView::block() noexcept {
    return _block;
}

const Block& BlockView::block() const noexcept {
    return _block;
}

bool BlockView::handleEvent(const SDL_Event& event) {
    if (!_delete->handleEvent(event)) {
        return false;
    }

    if (!_expand->handleEvent(event)) {
        return false;
    }

    if (expanded) {
        processDeletions();

        syncItemViews();

        if (!_itemScrollView->handleEvent(event))
            return false;
    }

    return UIObject::handleEvent(event);
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
        }
    }

    layoutItemViews();
}

void BlockView::layoutItemViews() const {
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

void BlockView::onEdit(ItemView& item) {
    if (!_window)
        return;
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
        fmt("You're about to delete an item called: \"%s\"", item.item().name.c_str()),
        Color::white,
        Point{0, 32},
        _font
    );

    content.add<Label>(
        fmt("The item is worth: %" PRId32 "kr", item.item().cost),
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
    deleteButton.setOnClick([this, &item, &dialog] {
        if (std::ranges::find(_itemsToDelete, &item) == _itemsToDelete.end()) {
            _itemsToDelete.push_back(&item);
        }
        dialog.close();
    });
}

void BlockView::onDelete(ItemView& item) {
    if (!_window)
        return;
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
        fmt("You're about to delete an item called: \"%s\"", item.item().name.c_str()),
        Color::white,
        Point{0, 32},
        _font
    );

    content.add<Label>(
        fmt("The item is worth: %" PRId32 "kr", item.item().cost),
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
    deleteButton.setOnClick([this, &item, &dialog] {
        if (std::ranges::find(_itemsToDelete, &item) == _itemsToDelete.end()) {
            _itemsToDelete.push_back(&item);
        }
        dialog.close();
    });
}

void BlockView::processDeletions() {
    bool changed = false;

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
        changed = true;
    }

    _itemsToDelete.clear();

    if (changed) {
        syncItemViews();
    }
}

void BlockView::onResize() {
    _expand->reposition({
        _rect.origin.x + _rect.size.x - 35.0f,
        _rect.origin.y + kButtonInset
    });

    _delete->reposition({
        _rect.origin.x + _rect.size.x - 70.0f,
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
    window->setRenderColor({30, 30, 30});
    window->renderRoundedRect(_rect, 10);

    window->renderText(
        _font,
        _block.name.c_str(),
        _rect.origin.x + 10.0f,
        _rect.origin.y + 10.0f,
        Color::white
    );

    _delete->render(window);
    _expand->render(window);

    if (expanded) {
        syncItemViews();

        _itemScrollView->render(window);
    }

    const std::string total = std::to_string(_block.total()) + "kr";

    window->renderText(
        _font,
        total.c_str(),
        _rect.origin.x + 10.0f,
        _rect.origin.y + _rect.size.y - 26.0f,
        Color::white
    );
}
