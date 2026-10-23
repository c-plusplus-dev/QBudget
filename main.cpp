#include <algorithm>
#include <chrono>
#include <memory>
#include <vector>

#include <Q-Tip/QTip.h>
#include <QTipUIKit/Containers/NavigationView/NavigationView.h>
#include <QTipUIKit/UIKit.h>
#include <QTipUIKit/Widgets/Button/MagickButton.h>

#include "Storage/Database.h"
#include "UI/BlocksView.h"

using namespace QTip;

namespace {
ButtonStyle makeNavigationButtonStyle(const Font& font) {
    ButtonStyle style;
    style.color = Color::clear;
    style.hoverColor = {40, 40, 40};
    style.pressedColor = {50, 50, 50};
    style.disabledColor = {60, 60, 60};
    style.font = font;
    style.fontColor = Color::white;
    return style;
}

void blocksViewRenderer(Window& buttonWindow, const Rect& rect) {
    const float size = std::min(rect.size.x, rect.size.y) - 5.0f;
    const float blockHeight = size / 5.0f;
    const float spacing = (size - 3.0f * blockHeight) / 2.0f;

    Point tl = rect.center() - Point{size / 2.0f, size / 2.0f - 1.25f};

    buttonWindow->setRenderColor(Color::white);

    for (int i = 0; i < 3; ++i) {
        buttonWindow->renderRoundedRect(
            {tl, size, blockHeight},
            5
        );
        tl.y += blockHeight + spacing;
    }
}
}

int main() {
    ModLoader::load<UIKitMod>();

    Window window("QBudget", 800, 600);
    auto [width, height] = window.size();

    Font font("assets/JetBrainsMono-Regular.ttf", 16);
    Database database = Database::load("example.qbdb");

    Panel root({0, 0, width, height});

    std::vector<BlockView*> blocks;
    std::vector<BlockView*> blocksToDelete;
    blocks.reserve(database.blocks().size());

    const auto now = std::chrono::system_clock::now();
    const auto today = std::chrono::floor<std::chrono::days>(now);
    const std::chrono::year_month_day date{today};

    int currentMonth = static_cast<int>(static_cast<unsigned>(date.month()));
    int currentYear = static_cast<int>(date.year());

    auto& navigation = root.add<NavigationView>(
        Rect{0, 0, width, height},
        NavigationPosition::Top,
        30.0f
    );

    const ButtonStyle navigationStyle = makeNavigationButtonStyle(font);

    navigation.addView<BlocksView, MagickButton>(
        [&]() -> std::unique_ptr<BlocksView> {
            return std::make_unique<BlocksView>(
                Rect{0, 0, width, height},
                window,
                font,
                database,
                blocks,
                blocksToDelete,
                currentMonth,
                currentYear
            );
        },
        navigationStyle,
        blocksViewRenderer
    );

    auto& blocksView = dynamic_cast<BlocksView&>(*navigation.selectedView());

    Point previousWindowSize{-1, -1};

    while (!window.shouldClose()) {
        QTipRuntime::pollEvents();

        blocksView.processDeletions();

        const Point currentWindowSize = window.size();
        if (currentWindowSize != previousWindowSize) {
            previousWindowSize = currentWindowSize;
            width = currentWindowSize.x;
            height = currentWindowSize.y;

            root.resize(currentWindowSize);
            navigation.resize({width, height});
        }

        window->setRenderColor({20, 20, 20});
        window->clear();

        root.render(window);

        window->present();
    }

    return 0;
}
