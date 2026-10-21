//
// Created by Ayaan on 2026-10-15.
//

#include <algorithm>
#include <cinttypes>
#include <chrono>
#include <memory>
#include <ranges>
#include <vector>

#include <Q-Tip/QTip.h>
#include <QTipUIKit/UIKit.h>
#include <QTipUIKit/Containers/List.h>
#include <QTipUIKit/Containers/NavigationView/NavigationView.h>
#include <QTipUIKit/Containers/ScrollView.h>
#include <QTipUIKit/Widgets/Button/MagickButton.h>

#include "Storage/Database.h"
#include "UI/BlockView.h"
#include "Helpers.h"
#include "UI/Spacer.h"

using namespace QTip;
using namespace std;

class BlocksView final : public View {
public:
    BlocksView(
        Rect rect,
        Window& window,
        Font& font,
        Database& database,
        vector<BlockView*>& blocks,
        vector<BlockView*>& blocksToDelete,
        float& width,
        float& height
    )
        : _window(window),
          _font(font),
          _database(database),
          _blocks(blocks),
          _blocksToDelete(blocksToDelete),
          _width(width),
          _height(height),
          _scrollView(
              {10, 10, max(0.0f, rect.size.x - 20.0f),
               max(0.0f, rect.size.y - 40.0f)},
              ScrollViewSettings{.showScrollbar = false}
          ) {
        _rect = rect;
        _preferredRect = rect;

        _blockList = &_scrollView.add<List>(
            Rect{0, 0, max(0.0f, rect.size.x - 20.0f), 100}
        );
        _blockList->setSpacing(10.0f);

        _blocks.reserve(_database.blocks().size());

        for (auto& block : _database.blocks()) {
            _blocks.push_back(&_blockList->add<BlockView>(
                Rect{0, 0, max(0.0f, rect.size.x - 20.0f), 100},
                block,
                _font,
                [this](BlockView& view) {
                    const float blockWidth = _scrollView.rect().size.x;
                    const float blockHeight =
                        view.expanded ? max(0.0f, _height - 100.0f) : 100.0f;

                    view.resize({blockWidth, blockHeight});
                    _blockList->updateLayout();
                    _scrollView.setContentSize(_blockList->rect().size);

                    if (view.expanded) {
                        _scrollView.setScrollSpeed(0);
                        const auto it = ranges::find(_blocks, &view);

                        if (it != _blocks.end()) {
                            const size_t index =
                                static_cast<size_t>(it - _blocks.begin());
                            _scrollView.scrollTo({
                                0,
                                static_cast<float>(index) * 110.0f
                            });
                        }
                    } else {
                        _scrollView.setScrollSpeed(30);
                    }
                },
                [this](BlockView& view) {
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
                        fmt("You're about to delete a block called: \"%s\"",
                            view.block().name.c_str()),
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
                        Rect{0, 0, 100, 100}, "Cancel", style
                    );
                    cancel.setOnClick([&dialog] {
                        dialog.close();
                    });

                    style.color = {255, 0, 0};
                    style.hoverColor = {255, 64, 64};
                    style.pressedColor = {255, 128, 128};

                    auto& deleteButton = buttons.add<Button>(
                        Rect{0, 0, 100, 100}, "Delete", style
                    );
                    deleteButton.setOnClick([this, &view, &dialog] {
                        _blocksToDelete.push_back(&view);
                        dialog.close();
                    });
                }
            ));
        }

        _blockList->add<Spacer>(
            Rect{0, 0, max(0.0f, rect.size.x - 20.0f), 60.0f}
        );

        _blockList->updateLayout();
        _scrollView.setContentSize(_blockList->rect().size);
    }

    void processDeletions() {
        for (BlockView* block : _blocksToDelete) {
            const auto it = ranges::find(_blocks, block);

            if (it == _blocks.end()) {
                continue;
            }

            _blockList->remove(*block);
            _blocks.erase(it);
        }

        if (!_blocksToDelete.empty()) {
            _blockList->updateLayout();
            _scrollView.setContentSize(_blockList->rect().size);
        }

        _blocksToDelete.clear();
    }

    void resize(QTip::Point size) override {
        setRect({_rect.origin.x, _rect.origin.y, size});
    }

    void reposition(QTip::Point position) override {
        setRect({position, _rect.size});
    }

    void setRect(QTip::Rect rect) override {
        _rect = rect;
        _preferredRect = rect;

        const float contentWidth = max(0.0f, rect.size.x - 20.0f);
        const float contentHeight = max(0.0f, rect.size.y - 40.0f);

        _scrollView.setRect({
            10.0f,
            10.0f,
            contentWidth,
            contentHeight
        });

        for (BlockView* block : _blocks) {
            block->resize({
                contentWidth,
                block->expanded ? max(0.0f, _height - 100.0f) : 100.0f
            });
        }

        _blockList->resize({contentWidth, _blockList->rect().size.y});
        _blockList->updateLayout();
        _scrollView.setContentSize(_blockList->rect().size);
    }

    [[nodiscard]] const Rect& rect() override {
        return _rect;
    }

    [[nodiscard]] Point minimumSize() const override {
        return {0, 0};
    }

    [[nodiscard]] Point preferredSize() const override {
        return _preferredRect.size;
    }

    bool handleEvent(const SDL_Event& event) override {
        return _scrollView.handleEvent(event);
    }

    void tick(Clock& clock, double dt) override {
        _scrollView.tick(clock, dt);
    }

protected:
    void renderImpl(Window& window) override {
        _scrollView.render(window);
    }

private:
    Window& _window;
    Font& _font;
    Database& _database;
    vector<BlockView*>& _blocks;
    vector<BlockView*>& _blocksToDelete;
    float& _width;
    float& _height;

    ScrollView _scrollView;
    List* _blockList = nullptr;
};

int main() {
    ModLoader::load<UIKitMod>();

    Window window("QBudget", 800, 600);
    auto [WIDTH, HEIGHT] = window.size();

    Font font("assets/JetBrainsMono-Regular.ttf", 16);
    Database db = Database::load("example.qbdb");

    Panel root({0, 0, WIDTH, HEIGHT});

    vector<BlockView*> blocks;
    vector<BlockView*> blocksToDelete;
    blocks.reserve(db.blocks().size());

    auto& navigation = root.add<NavigationView>(
        Rect{0, 0, WIDTH, HEIGHT},
        NavigationPosition::Right,
        40.0f
    );

    ButtonStyle navigationStyle;
    navigationStyle.color = Color::clear;
    navigationStyle.hoverColor = {40, 40, 40};
    navigationStyle.pressedColor = {50, 50, 50};
    navigationStyle.disabledColor = {35, 35, 35};
    navigationStyle.font = font;
    navigationStyle.fontColor = Color::white;

    navigation.addView<BlocksView, MagickButton>(
        [&]() -> unique_ptr<BlocksView> {
            return make_unique<BlocksView>(
                Rect{0, 0, WIDTH - 40.0f, HEIGHT},
                window,
                font,
                db,
                blocks,
                blocksToDelete,
                WIDTH,
                HEIGHT
            );
        },
        navigationStyle,
        [](Window& w, const Rect& rect) {
            w->setRenderColor(Color::white);
            const float left = rect.origin.x + (rect.size.x - 18.0f) / 2.0f;
            const float top = rect.origin.y + (rect.size.y - 18.0f) / 2.0f;
            w->renderRect({left, top, 18.0f, 18.0f}, false);
            w->renderLine({{left, top + 6.0f}, {left + 17.0f, top + 6.0f}});
            w->renderLine({{left + 5.0f, top + 1.0f}, {left + 5.0f, top + 4.0f}});
            w->renderLine({{left + 13.0f, top + 1.0f}, {left + 13.0f, top + 4.0f}});
        }
    );

    auto& blocksView = static_cast<BlocksView&>(*navigation.selectedView());

    auto now = chrono::system_clock::now();
    auto today = floor<chrono::days>(now);
    chrono::year_month_day ymd = chrono::year_month_day{today};

    const int currMonth = static_cast<int>(static_cast<unsigned>(ymd.month()));
    const int currYear = static_cast<int>(ymd.year());

    auto& bottomTotal = root.add<Label>(
        fmt("Total: %" PRId64 "kr | Date: %d-%02d",
            db.total(), currYear, currMonth),
        Color::white,
        Point{10, HEIGHT - 26},
        font
    );

    // The add action remains separate from navigation destinations.
    auto& addBlockList = root.add<List>(
        Rect{WIDTH - 35, 50, 30, max(0.0f, HEIGHT - 60.0f)}
    );

    ButtonStyle addStyle;
    addStyle.color = Color::clear;
    addStyle.hoverColor = {40, 40, 40};
    addStyle.pressedColor = {50, 50, 50};
    addStyle.font = font;
    addStyle.fontColor = Color::white;

    addBlockList.add<MagickButton>(
        Rect{0, 0, 30, 30},
        addStyle,
        [](Window& w, const Rect& rect) {
            w->setRenderColor(Color::white);
            const auto center = rect.center();
            DrawThickLineHV(w, center.x, center.y - 10.0f, center.x, center.y + 10.0f, 3);
            DrawThickLineHV(w, center.x - 10.0f, center.y, center.x + 10.0f, center.y, 3);
        }
    );

    auto oldWindowSize = Point{-1, -1};

    while (!window.shouldClose()) {
        QTipRuntime::pollEvents();

        blocksView.processDeletions();

        if (window.size() != oldWindowSize) {
            oldWindowSize = window.size();
            WIDTH = oldWindowSize.x;
            HEIGHT = oldWindowSize.y;

            root.resize(oldWindowSize);
            navigation.resize(oldWindowSize);

            bottomTotal.reposition({10, HEIGHT - 26});
            addBlockList.reposition({WIDTH - 35, 50});
            addBlockList.resize({30, max(0.0f, HEIGHT - 60.0f)});
        }

        window->setRenderColor({20, 20, 20});
        window->clear();

        root.render(window);

        DrawScrollGradient(
            window,
            WIDTH - 50,
            10,
            max(0.0f, HEIGHT - 90.0f),
            60,
            {20, 20, 20}
        );

        window->present();
    }

    return 0;
}
