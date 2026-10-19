
//
// Created by Ayaan on 2026-10-19.
//

#include "BlockScreen.h"

#include <cinttypes>
#include <chrono>
#include <ranges>
#include <string>

#include <QTipUIKit/Dialogs/Dialog.h>

#include "Helpers.h"
#include "UI/Spacer.h"

using namespace QTip;
using namespace std;

BlockScreen::BlockScreen(
    Rect rect,
    Window& window,
    Database& database,
    Font& font
)
    : Screen(rect),
      _window(window),
      _database(database),
      _font(font) {

    _blockView = &add<ScrollView>(
        Rect{10, 10, rect.size.x - 50, rect.size.y - 40},
        ScrollViewSettings{
            .showScrollbar = false
        }
    );

    _blockList = &_blockView->add<List>(
        Rect{0, 0, rect.size.x - 50, 100}
    );

    _blockList->setSpacing(10.0f);

    createBlocks();

    _bottomTotal = &add<Label>(
        fmt(
            "Total: %" PRId64 "kr | Date: 0000-00",
            _database.total()
        ),
        Color::white,
        Point{10, rect.size.y - 26},
        _font
    );

    // The add button belongs to this screen, not the navigation controls.
    ButtonStyle style;
    style.color = Color::clear;
    style.hoverColor = {40, 40, 40};
    style.pressedColor = {50, 50, 50};
    style.font = _font;
    style.fontColor = Color::white;

    _addBlock = &add<MagickButton>(
        Rect{rect.size.x - 30, 10, 30, 30},
        style,
        [](Window& window, const Rect& buttonRect) {
            window->setRenderColor(Color::white);

            const auto center = buttonRect.center();

            DrawThickLineHV(
                window,
                center.x,
                center.y - 10.0f,
                center.x,
                center.y + 10.0f,
                3
            );

            DrawThickLineHV(
                window,
                center.x - 10.0f,
                center.y,
                center.x + 10.0f,
                center.y,
                3
            );
        }
    );

    const auto now = chrono::system_clock::now();
    const auto today = floor<chrono::days>(now);
    const chrono::year_month_day ymd{today};

    const auto currMonth =
        static_cast<int>(static_cast<unsigned>(ymd.month()));

    const auto currYear =
        static_cast<int>(ymd.year());

    _bottomTotal->setText(
        fmt(
            "Total: %" PRId64 "kr | Date: %d-%02d",
            _database.total(),
            currYear,
            currMonth
        )
    );

    onResize();
}

void BlockScreen::createBlocks() {
    _blocks.reserve(_database.blocks().size());

    for (auto& block : _database.blocks()) {
        auto& view = _blockList->add<BlockView>(
            Rect{0, 0, _rect.size.x - 50, 100},
            block,
            _font,

            [this](BlockView& expandedView) {
                onBlockExpanded(expandedView);
            },

            [this](BlockView& deletedView) {
                requestDelete(deletedView);
            }
        );

        _blocks.push_back(&view);
    }

    _blockList->add<Spacer>(
        Rect{0, 0, _rect.size.x - 50, 60}
    );

    _blockView->setContentSize(_blockList->rect().size);
}

void BlockScreen::requestDelete(BlockView& view) {
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
        fmt(
            "You're about to delete a block called: \"%s\"",
            view.block().name.c_str()
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

    auto& cancel = buttons.add<Button>(
        Rect{0, 0, 100, 100},
        "Cancel",
        style
    );

    cancel.setOnClick([&dialog] {
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

    deleteButton.setOnClick([this, &view, &dialog] {
        _blocksToDelete.push_back(&view);
        dialog.close();
    });
}

void BlockScreen::processDeletions() {
    if (_blocksToDelete.empty()) {
        return;
    }

    for (BlockView* block : _blocksToDelete) {
        const auto it = ranges::find(_blocks, block);

        if (it == _blocks.end()) {
            continue;
        }

        _blockList->remove(*block);
        _blocks.erase(it);
    }

    _blocksToDelete.clear();

    updateLayout();
}

void BlockScreen::updateLayout() {
    _blockList->updateLayout();
    _blockView->setContentSize(_blockList->rect().size);
}

void BlockScreen::updateBlockSize(BlockView& view) {
    const float height = view.expanded
        ? _rect.size.y - 100.0f
        : 100.0f;

    view.resize({
        _rect.size.x - 50.0f,
        height
    });
}

void BlockScreen::onBlockExpanded(BlockView& view) {
    updateBlockSize(view);
    updateLayout();

    if (view.expanded) {
        _blockView->setScrollSpeed(0);

        const auto it = ranges::find(_blocks, &view);

        if (it != _blocks.end()) {
            const auto index =
                static_cast<size_t>(it - _blocks.begin());

            _blockView->scrollTo({
                0,
                static_cast<float>(index) * 110.0f
            });
        }
    } else {
        _blockView->setScrollSpeed(30);
    }
}

void BlockScreen::tick(Clock& clock, double dt) {
    processDeletions();

    Panel::tick(clock, dt);
}

void BlockScreen::resize(Point size) {
    Screen::resize(size);
    onResize();
}

void BlockScreen::reposition(Point position) {
    Screen::reposition(position);
    onResize();
}

void BlockScreen::setRect(Rect rect) {
    Screen::setRect(rect);
    onResize();
}

void BlockScreen::onResize() {
    const float width = _rect.size.x;
    const float height = _rect.size.y;

    if (_blockView) {
        _blockView->setRect({
            10,
            10,
            width - 50,
            height - 40
        });
    }

    if (_blockList) {
        _blockList->resize({
            width - 50,
            _blockList->rect().size.y
        });

        for (BlockView* block : _blocks) {
            updateBlockSize(*block);
        }

        _blockList->updateLayout();
        _blockView->setContentSize(_blockList->rect().size);
    }

    if (_bottomTotal) {
        _bottomTotal->reposition({
            10,
            height - 26
        });
    }

    if (_addBlock) {
        _addBlock->reposition({
            width - 30,
            10
        });
    }
}

void BlockScreen::renderImpl(Window& window) {
    Panel::renderImpl(window);

    // Fade the bottom edge of the scrolling block list.
    DrawScrollGradient(
        window,
        static_cast<int>(_blockView->rect().size.x),
        10,
        static_cast<int>(_rect.size.y - 90),
        60,
        {20, 20, 20}
    );
}
