
//
// Created by Ayaan on 2026-10-17.
//

#include "ItemView.h"

#include <algorithm>
#include <utility>
#include <QTipUIKit/Widgets/Button/MagickButton.h>

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
    Action onEdit,
    Action onDelete
)
    : _item(&item),
      _font(font),
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
            if (_editButtonTexture) {
                auto origin = rect.origin + Point{2.5, 2.5};
                auto size = rect.size - Point{5, 5};
                window->renderTexture(*_editButtonTexture, Rect::zero, {origin, size});
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

bool ItemView::handleEvent(const SDL_Event& event) {
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
    setRect({position, _rect.size});
    _preferredRect.origin = position;
}

void ItemView::setRect(Rect rect) {
    _rect = rect;

    updateLayout();
}

const Rect& ItemView::rect() {
    return _rect;
}

float ItemView::calculateWidth() const {
    const float nameWidth = _font.getTextWidth(_item->name);
    const float costWidth = _font.getTextWidth(
        std::to_string(_item->cost) + "kr"
    );

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
    if (!_editButtonTexture) {
        _editButtonTexture = std::make_unique<Texture>(
            window,
            "assets/edit.png"
        );
    }
    if (!_moveTexture) {
        _moveTexture = std::make_unique<Texture>(
            window,
            "assets/drag2.png"
        );
    }

    window->setRenderColor({40, 40, 40});
    window->renderRoundedRect(_rect, 10);

    window->renderText(
        _font,
        _item->name.c_str(),
        _rect.origin.x + kHorizontalPadding,
        _rect.origin.y + 10.0f,
        Color::white
    );

    const std::string cost = std::to_string(_item->cost) + "kr";

    window->renderText(
        _font,
        cost.c_str(),
        _rect.origin.x + kHorizontalPadding,
        _rect.origin.y + 70.0f,
        Color::white
    );

    _deleteButton->render(window);
    _editButton->render(window);

    auto size = 24.0f;
    auto x = _rect.origin.x + _rect.size.x - size - 5.0f;
    auto y = _rect.origin.y + _rect.size.y - size - 5.0f;
    window->renderTexture(*_moveTexture, Rect::zero, {x, y, size, size});
}
