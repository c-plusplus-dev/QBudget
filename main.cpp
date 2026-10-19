#include <Q-Tip/QTip.h>
#include <QTipUIKit/UIKit.h>
#include <QTipUIKit/Containers/Panel.h>

#include <cinttypes>
#include <cstdio>
#include <memory>

#include "Storage/Database.h"
#include "UI/BlockScreen.h"
#include "UI/MagickButton.h"
#include "UI/ScreenManager.h"
#include "Helpers.h"

using namespace QTip;

constexpr int INITIAL_WIDTH = 800;
constexpr int INITIAL_HEIGHT = 600;
constexpr int NAVIGATION_WIDTH = 40;
constexpr int NAVIGATION_BUTTON_SIZE = 30;
constexpr int NAVIGATION_BUTTON_SPACING = 10;

int main() {
    ModLoader::load<UIKitMod>();

    Window window("QBudget", INITIAL_WIDTH, INITIAL_HEIGHT);
    Font font("assets/JetBrainsMono-Regular.ttf", 16);

    Database database = Database::load("example.qbdb");

    auto [width, height] = window.size();

    Panel root({0, 0, width, height});

    // The screen manager occupies the main area, leaving room for navigation.
    auto& screens = root.add<ScreenManager>(
        Rect{0, 0, width - NAVIGATION_WIDTH, height}
    );

    screens.push<BlockScreen>(
        Rect{0, 0, width - NAVIGATION_WIDTH, height},
        window,
        database,
        font
    );

    // Navigation controls stay outside the screen manager.
    Panel& navigation = root.add<Panel>(
        Rect{
            width - NAVIGATION_BUTTON_SIZE - 5,
            10,
            NAVIGATION_BUTTON_SIZE,
            height - 20
        }
    );

    const auto addNavigationButton = [&](int y, auto drawCallback) {
        navigation.add<MagickButton>(
            Rect{0, y, NAVIGATION_BUTTON_SIZE, NAVIGATION_BUTTON_SIZE},
            MagickButton::Style{},
            drawCallback
        );
    };

    // Pie-chart navigation button.
    addNavigationButton(
        0,
        [](Window& target, const Rect& rect) {
            const int centerX = rect.x + rect.size.x / 2;
            const int centerY = rect.y + rect.size.y / 2;
            const int radius = rect.size.x / 3;

            target.setRenderColor({220, 220, 220});
            DrawPieSlice(
                target,
                {centerX, centerY},
                radius,
                0.0,
                270.0
            );
        }
    );

    // Month-view navigation button.
    addNavigationButton(
        NAVIGATION_BUTTON_SIZE + NAVIGATION_BUTTON_SPACING,
        [](Window& target, const Rect& rect) {
            target.setRenderColor({220, 220, 220});

            target->renderRect({
                rect.x + 5,
                rect.y + 6,
                rect.size.x - 10,
                rect.size.y - 11
            });

            target->renderLine(
                {rect.x + 5, rect.y + 12},
                {rect.x + rect.size.x - 5, rect.y + 12}
            );

            target->renderLine(
                {rect.x + 10, rect.y + 3},
                {rect.x + 10, rect.y + 9}
            );

            target->renderLine(
                {rect.x + rect.size.x - 10, rect.y + 3},
                {rect.x + rect.size.x - 10, rect.y + 9}
            );
        }
    );

    // Bar-chart navigation button.
    addNavigationButton(
        2 * (NAVIGATION_BUTTON_SIZE + NAVIGATION_BUTTON_SPACING),
        [](Window& target, const Rect& rect) {
            target.setRenderColor({220, 220, 220});

            const int bottom = rect.y + rect.size.y - 5;

            target->renderRect({
                rect.x + 5,
                bottom - 8,
                5,
                8
            });

            target->renderRect({
                rect.x + 13,
                bottom - 15,
                5,
                15
            });

            target->renderRect({
                rect.x + 21,
                bottom - 21,
                5,
                21
            });
        }
    );

    while (window.isOpen()) {
        QTipRuntime::pollEvents();

        const auto [newWidth, newHeight] = window.size();

        if (newWidth != width || newHeight != height) {
            width = newWidth;
            height = newHeight;

            root.setRect({0, 0, width, height});

            screens.setRect({
                0,
                0,
                width - NAVIGATION_WIDTH,
                height
            });

            navigation.setRect({
                width - NAVIGATION_BUTTON_SIZE - 5,
                10,
                NAVIGATION_BUTTON_SIZE,
                height - 20
            });
        }

        window->setRenderColor({20, 20, 20});
        window->clear();

        root.render(window);

        window->present();
    }

    return 0;
}