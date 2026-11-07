
//
// Created by Ayaan on 2026-10-17.
//

#include "ItemView.h"

#include <algorithm>
#include <utility>
#include <QTipUIKit/Widgets/Button/MagickButton.h>
#include "UI/Visibility.h"

using namespace QTip;

namespace {
constexpr float kItemHeight = 100.0f;
constexpr float kMinimumWidth = 100.0f;
constexpr float kHorizontalPadding = 10.0f;
constexpr float kButtonSize = 30.0f;
constexpr float kButtonRightInset = 5.0f;
constexpr float kButtonTopInset = 5.0f;
constexpr float kButtonSpacing = 5.0f;

ButtonStyle makeButtonStyle(Font& font) {
    ButtonStyle style{};
    style.color = Color::clear;
    style.hoverColor = {50, 50, 50};
    style.pressedColor = {60, 60, 60};
    style.font = font;
    style.fontColor = Color::white;
    return style;
}
}

ItemView::ItemView(
    Rect rect,
    Block::Item& item,
    Font& font,
    UITextures& textures,
    ScrollView& parentScrollView,
    Action onEdit,
    Action onDelete
)
    : _item(&item),
      _font(font),
      _textures(textures),
      _parentScrollView(parentScrollView),
      _onEdit(std::move(onEdit)),
      _onDelete(std::move(onDelete)) {
    _rect = rect;
    _preferredRect = rect;

    ButtonStyle style = makeButtonStyle(_font);

    _deleteButton = std::make_unique<Button>(
        Rect{0, 0, kButtonSize, kButtonSize},
        "━",
        style
    );

    _editButton = std::make_unique<MagickButton>(
        Rect{0, 0, kButtonSize, kButtonSize},
        style,
        [this] (Window& window, const Rect& rect) {
            if (_textures.editButton && _textures.editButton->isValid()) {
                const Point origin = rect.origin + Point{2.5f, 2.5f};
                const Point size = rect.size - Point{5.0f, 5.0f};
                window->renderTexture(*_textures.editButton, Rect::zero, {origin, size});
            }
        }
    );

    _deleteButton->parent = this;
    _editButton->parent = this;

    _deleteButton->setOnClick([this] {
        if (_onDelete) {
            _onDelete(*this);
        }
    });

    _editButton->setOnClick([this] {
        if (_onEdit) {
            _onEdit(*this);
        }
    });

    updateLayout();
}

Block::Item& ItemView::item() {
    return *_item;
}

const Block::Item& ItemView::item() const {
    return *_item;
}

bool ItemView::handleEvent(const Event& event) {
    if (!_deleteButton->handleEvent(event)) {
        return false;
    }

    if (!_editButton->handleEvent(event)) {
        return false;
    }

    return UIObject::handleEvent(event);
}

Point ItemView::minimumSize() const {
    return {kMinimumWidth, kItemHeight};
}

Point ItemView::preferredSize() const {
    return {_preferredRect.size.x, kItemHeight};
}

void ItemView::resize(Point size) {
    setRect({_rect.origin, size});
    _preferredRect.size = size;
}

void ItemView::reposition(Point position) {
    const Point delta = position - _rect.origin;
    _rect.origin = position;
    _preferredRect.origin = position;
    _deleteButton->reposition(_deleteButton->rect().origin + delta);
    _editButton->reposition(_editButton->rect().origin + delta);
}

void ItemView::setRect(Rect rect) {
    _rect = rect;

    updateLayout();
}

const Rect& ItemView::rect() {
    return _rect;
}

float ItemView::calculateWidth() const {
    const float nameWidth = _font.getTextWidth(_cachedName);
    const float costWidth = _font.getTextWidth(_cachedCostText);

    const float textWidth = std::max(
        nameWidth + 50.0f,
        costWidth + 2.0f * kHorizontalPadding
    );

    const float buttonWidth =
        2.0f * kButtonSize + kButtonSpacing + kButtonRightInset;

    return std::max({
        kMinimumWidth,
        textWidth,
        buttonWidth + 40.0f
    });
}

void ItemView::updateLayout() {
    _cachedName = _item->name;
    _cachedCostText = std::to_string(_item->cost) + "kr";
    _preferredRect.size.x = calculateWidth();
    _preferredRect.size.y = kItemHeight;

    _deleteButton->reposition({
        _rect.origin.x + _rect.size.x - kButtonSize - kButtonRightInset,
        _rect.origin.y + kButtonTopInset
    });

    _editButton->reposition({
        _rect.origin.x + _rect.size.x - kButtonSize - kButtonRightInset,
        _rect.origin.y + kButtonTopInset + kButtonSize + kButtonSpacing
    });
}

void ItemView::renderImpl(Window& window) {
    if (!isVisibleInScrollView(_rect, _parentScrollView)) {
        return;
    }

    window->setRenderColor({40, 40, 40});
    window->renderRoundedRect(_rect, 10);

    window->renderText(
        _font,
        _cachedName.c_str(),
        _rect.origin.x + kHorizontalPadding,
        _rect.origin.y + 10.0f,
        Color::white
    );

    window->renderText(
        _font,
        _cachedCostText.c_str(),
        _rect.origin.x + kHorizontalPadding,
        _rect.origin.y + 70.0f,
        Color::white
    );

    _deleteButton->render(window);
    _editButton->render(window);

    constexpr float size = 24.0f;
    const float x = _rect.origin.x + _rect.size.x - size - 5.0f;
    const float y = _rect.origin.y + _rect.size.y - size - 5.0f;
    if (_textures.itemMove && _textures.itemMove->isValid()) {
        window->renderTexture(*_textures.itemMove, Rect::zero, {x, y, size, size});
    }
}
