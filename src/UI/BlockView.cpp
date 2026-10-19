//
// Created by Ayaan on 2026-10-15.
//

#include "BlockView.h"

#include <utility>
using namespace QTip;

BlockView::BlockView(Rect rect, Block& block, Font& font, OnExpandRequested expandRequested, OnDeleteRequested deleteRequested) : _block(&block), _font(font) {
    _rect = rect;
    _preferredRect = rect;

    ButtonStyle style;
    style.color = Color::clear;
    style.hoverColor = {40, 40, 40};
    style.pressedColor = {50, 50, 50};
    style.font = font;
    style.fontColor = Color::white;

    _delete = std::make_unique<Button>(0, 0, 30, 30, "━", style);
    _expand = std::make_unique<Button>(0, 0, 30, 30, "◀", style);

    _delete->parent = this;
    _expand->parent = this;

    onResize();

    _deleteRequested = std::move(deleteRequested);
    _expandRequested = std::move(expandRequested);

    _delete->setOnClick([this] {
        _deleteRequested(*this);
    });

    _expand->setOnClick([this] {
        expanded = !expanded;
        _expand->setText(expanded ? "▼" : "◀"),
        _expandRequested(*this);
    });
}

const Block& BlockView::block() const {
    return *_block;
}

bool BlockView::handleEvent(const SDL_Event& event) {
    _delete->handleEvent(event);
    _expand->handleEvent(event);
    return UIObject::handleEvent(event);
}

Point BlockView::minimumSize() const {
    return {100, 100};
}

Point BlockView::preferredSize() const {
    return _preferredRect.size;
}

void BlockView::resize(Point size) {
    _rect.size = size;
    _preferredRect.size = size;
    onResize();
}

void BlockView::reposition(Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
    onResize();
}

void BlockView::setRect(Rect rect) {
    _rect = rect;
    onResize();
}

const Rect& BlockView::rect() {
    return _rect;
}

void BlockView::renderImpl(Window& window) {
    window->setRenderColor({30, 30, 30});
    window->renderRoundedRect(_rect, 10);

    window->renderText(_font, _block->name.c_str(), _rect.origin.x + 10, _rect.origin.y + 10, Color::white);

    window->renderText(_font, fmt("%skr", std::to_string(_block->total()).c_str()).c_str(), _rect.origin.x + 10, _rect.origin.y + _rect.size.y - 26, Color::white);

    _delete->render(window);
    _expand->render(window);
}

void BlockView::onResize() const {
    _expand->reposition({_rect.origin.x + _rect.size.x - 35, _rect.origin.y + 5});
    _delete->reposition({_rect.origin.x + _rect.size.x - 70, _rect.origin.y + 5});
}
