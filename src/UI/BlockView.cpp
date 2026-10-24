
//
// Created by Ayaan on 2026-10-15.
//

#include "BlockView.h"

#include <algorithm>
#include <string>
#include <utility>

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

const Block& BlockView::block() const {
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
                ItemView::Action{},
                ItemView::Action{}
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
        {width, _rect.size.y - kItemTop - kItemMargin}
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
