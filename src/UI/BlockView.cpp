
//
// Created by Ayaan on 2026-10-15.
//

#include "BlockView.h"

#include <algorithm>
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

bool BlockView::handleEvent(const SDL_Event& event) {
    if (!_expand->handleEvent(event))
        return false;

    if (expanded) {
        processDeletions();

        syncItemViews();

        if (!_add->handleEvent(event))
            return false;

        if (!_itemScrollView->handleEvent(event))
            return false;
    } else
        if (!_delete->handleEvent(event))
            return false;

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
            addItem(*item);
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

        _itemScrollView->render(window);
    } else {
        _delete->render(window);

        auto size = 24.0f;
        auto x = _rect.origin.x + _rect.size.x - size - 5.0f;
        auto y = _rect.origin.y + _rect.size.y - size - 5.0f;
        window->renderTexture(*_moveTexture, Rect::zero, {x, y, size, size});
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
