#include <algorithm>
#include <chrono>
#include <memory>
#include <vector>

#include <Q-Tip/QTip.h>
#include <QTipUIKit/Containers/NavigationView/NavigationView.h>
#include <QTipUIKit/UIKit.h>
#include <QTipUIKit/Widgets/Button/MagickButton.h>

#include "Storage/Database.h"
#include "Storage/DatabaseManager.h"
#include "UI/BlocksView.h"
#include "UI/PieView.h"

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

void pieViewRenderer(Window& buttonWindow, const Rect& rect) {
    const float size = std::min(rect.size.x, rect.size.y);
    const float radius = (size - 8.0f) / 2.0f;
    const Point center = rect.center();

    Detail::drawPieSlice(buttonWindow, nullptr, center.x, center.y, radius, 0.75, Color::white, 0);
}
}

int main() {
    ModLoader::load<UIKitMod>();

    Window window("QBudget", 800, 600);
    auto [width, height] = window.size();

    Font font("assets/JetBrainsMono-Regular.ttf", 16);

    const auto now = std::chrono::system_clock::now();
    const auto today = std::chrono::floor<std::chrono::days>(now);
    const std::chrono::year_month_day date{today};

    int currentMonth = static_cast<int>(static_cast<unsigned>(date.month()));
    int currentYear = static_cast<int>(date.year());

    DatabaseManager manager{"testDatabase"};
    auto* it = manager.get(currentYear, currentMonth);
    if (!it)
        return 1;
    Database& database = *it;

    Panel root({0, 0, width, height});

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
                currentMonth,
                currentYear
            );
        },
        navigationStyle,
        blocksViewRenderer
    );

    navigation.addView<PieView, MagickButton>(
        [&]() -> std::unique_ptr<PieView> {
            return std::make_unique<PieView>(
                Rect{0, 0, width, height},
                database,
                font
            );
        },
        navigationStyle,
        pieViewRenderer
    );

    Point previousWindowSize{-1, -1};

    while (!window.shouldClose()) {
        QTipRuntime::pollEvents();

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
