//
// Created by Ayaan on 2026-10-15.
//

#include <cinttypes>
#include <chrono>
#include <Q-Tip/QTip.h>
#include <QTipUIKit/UIKit.h>
#include <QTipUIKit/Containers/List.h>
#include <QTipUIKit/Containers/ScrollView.h>
#include <QTipUIKit/Widgets/Button/MagickButton.h>

#include "Storage/Database.h"
#include "UI/BlockView.h"
#include "Helpers.h"
#include "UI/Spacer.h"
using namespace QTip;
using namespace std;

int main() {
    ModLoader::load<UIKitMod>();

    Window window("QBudget", 800, 600);
    auto [WIDTH, HEIGHT] = window.size();

    Font font("assets/JetBrainsMono-Regular.ttf", 16);

    Database db = Database::load("example.qbdb");

    Panel root({0, 0, 800, 600});

    auto& blockView = root.add<ScrollView>(Rect{10, 10, WIDTH - 50, HEIGHT - 40}, ScrollViewSettings{.showScrollbar = false});
    auto& blockList = blockView.add<List>(Rect{0, 0, WIDTH - 50, 100});
    blockList.setSpacing(10.0f);

    std::vector<BlockView*> blocks{};
    std::vector<BlockView*> blocksToDelete;
    blocks.reserve(db.blocks().size());
    for (auto& block : db.blocks()) {
        blocks.push_back(&blockList.add<BlockView>(Rect{0, 0, WIDTH - 50, 100}, block, font,
            [&](BlockView& view) { // expand
                const float height = view.expanded ? HEIGHT - 100.0f : 100.0f;

                view.resize({WIDTH - 50.0f, height});

                blockList.updateLayout();
                blockView.setContentSize(blockList.rect().size);

                if (view.expanded) {
                    blockView.setScrollSpeed(0);
                    auto it = std::ranges::find(blocks, &view);

                    if (it != blocks.end()) {
                        std::size_t index = it - blocks.begin();
                        blockView.scrollTo({0, static_cast<float>(index) * 110.0f});
                    }
                } else
                    blockView.setScrollSpeed(30);
            },
            [&](BlockView& view) { // delete
                Dialog& dialog = Dialog::create(window);

                auto& content = dialog.content();
                auto& buttons = dialog.buttons();

                content.add<Label>("Are you sure?", Color::white, Point{0, 0}, font);
                content.add<Label>(fmt("You're about to delete a block called: \"%s\"", view.block().name.c_str()), Color::white, Point{0, 32}, font);

                ButtonStyle style;
                style.color = {50, 50, 50};
                style.hoverColor = {70, 70, 70};
                style.pressedColor = {30, 30, 30};
                style.font = font;
                style.fontColor = Color::white;

                auto& cancel = buttons.add<Button>(Rect{0, 0, 100, 100}, "Cancel", style);

                cancel.setOnClick([&] {
                    dialog.close();
                });

                style.color = {255, 0, 0};
                style.hoverColor = {255, 64, 64};
                style.pressedColor = {255, 128, 128};

                auto& delete_ = buttons.add<Button>(Rect{0, 0, 100, 100}, "Delete", style);

                delete_.setOnClick([&] {
                    blocksToDelete.push_back(&view);
                    dialog.close();
                });
            }
        ));
    }

    blockList.add<Spacer>(Rect{0, 0, WIDTH - 50, 60.0f});

    blockView.setContentSize(blockList.rect().size);

    auto now = chrono::system_clock::now();

    auto today = floor<chrono::days>(now);
    chrono::year_month_day ymd = chrono::year_month_day{today};

    auto currMonth = static_cast<int>(static_cast<unsigned>(ymd.month()));
    auto currYear = static_cast<int>(ymd.year());

    auto& bottomTotal = root.add<Label>(fmt("Total: %" PRId64 "kr | Date: %d-%02d", db.total(), currYear, currMonth), Color::white, Point{10, HEIGHT - 26}, font);

    auto& controlList = root.add<List>(Rect{WIDTH - 35, 10, 30, HEIGHT - 20});

    ButtonStyle style;
    style.color = Color::clear;
    style.hoverColor = {40, 40, 40};
    style.pressedColor = {50, 50, 50};
    style.font = font;
    style.fontColor = Color::white;

    auto& addBlock = controlList.add<MagickButton>(Rect{0, 0, 30, 30}, style, [&](Window& w, const Rect& rect) {
        window->setRenderColor(Color::white);
        auto center = rect.center();
        DrawThickLineHV(window, center.x, center.y - 10.0f, center.x, center.y + 10.0f, 3);
        DrawThickLineHV(window, center.x - 10.0f, center.y, center.x + 10.0f, center.y, 3);
    });

    auto& pieChart = controlList.add<MagickButton>(Rect{0, 0, 30, 30}, style, [&](Window& w, const Rect& rect) {
        DrawPieSlice(w, rect.center().x, rect.center().y, (rect.size.x - 10.0f) / 2.0f, 0.75, Color::white, 0);
    });

    auto& view = controlList.add<MagickButton>(Rect{0, 0, 30, 30}, style, [&](Window& w, const Rect& rect) {
        window->setRenderColor(Color::white);

        const float left = rect.origin.x + 5.0f;
        const float top = rect.origin.y + 6.0f;
        const float width = rect.size.x - 10.0f;
        const float height = rect.size.y - 11.0f;

        window->renderRect(
            Rect{left, top, width, height},
            false
        );

        window->renderLine({
            {left, top + 6.0f},
            {left + width - 1.0f, top + 6.0f}
        });

        window->renderLine({
            {left + 5.0f, rect.origin.y + 3.0f},
            {left + 5.0f, top + 3.0f}
        });

        window->renderLine({
            {left + width - 6.0f, rect.origin.y + 3.0f},
            {left + width - 6.0f, top + 3.0f}
        });
    });

    auto& barChart = controlList.add<MagickButton>(Rect{0, 0, 30, 30}, style, [&](Window& w, const Rect& rect) {
        w->setRenderColor(Color::white);
        Rect r{};
        r.size = {5, 15};
        r.origin.x = rect.origin.x + 5;
        r.origin.y = rect.origin.y + rect.size.y - r.size.y - 2;
        w->renderRect(r);
        r.origin.x += 7;
        r.size.y += 5;
        r.origin.y = rect.origin.y + rect.size.y - r.size.y - 2;
        w->renderRect(r);
        r.origin.x += 7;
        r.size.y -= 7;
        r.origin.y = rect.origin.y + rect.size.y - r.size.y - 2;
        w->renderRect(r);
    });

    auto oldWindowSize = Point{-1, -1};
    while (!window.shouldClose()) {
        QTipRuntime::pollEvents();

        for (BlockView* block : blocksToDelete) {
            auto it = std::ranges::find(blocks, block);

            if (it == blocks.end())
                continue;

            blockList.remove(*block);
            blocks.erase(it);
        }

        if (!blocksToDelete.empty()) {
            blockList.updateLayout();
            blockView.setContentSize(blockList.rect().size);
        }

        blocksToDelete.clear();

        if (window.size() != oldWindowSize) {
            oldWindowSize = window.size();
            auto size = oldWindowSize;
            WIDTH = size.x;
            HEIGHT = size.y;

            root.resize(size);

            blockView.resize({WIDTH - 50, HEIGHT - 40});

            for (auto block : blocks) {
                if (block->expanded)
                    block->resize({WIDTH - 50, HEIGHT - 100.0f});
                else
                    block->resize({WIDTH - 50, 100});
            }

            bottomTotal.reposition({10, HEIGHT - 26});

            blockList.resize({WIDTH - 50, blockList.rect().size.y});
            blockList.updateLayout();

            blockView.setContentSize(blockList.rect().size);

            controlList.reposition({WIDTH - 35, 10});
            controlList.resize({30, HEIGHT - 20});
        }

        window->setRenderColor({20, 20, 20});
        window->clear();

        root.render(window);

        DrawScrollGradient(window, WIDTH - 50, 10, HEIGHT - 90, 60, {20, 20, 20});

        window->present();
    }

    return 0;
}
