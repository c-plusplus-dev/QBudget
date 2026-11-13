#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>
#include <memory>
#include <vector>

#include <Q-Tip/QTip.h>
#include <QTipUIKit/Containers/NavigationView/NavigationView.h>
#include <QTipUIKit/UIKit.h>
#include <QTipUIKit/Widgets/Button/MagickButton.h>

#include "Helpers.h"
#include "Platform.h"
#include "Storage/Database.h"
#include "Storage/DatabaseManager.h"
#include "UI/BarView.h"
#include "UI/BlocksView.h"
#include "UI/MonthsView.h"
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

    Point topLeft = rect.center() -
        Point{size / 2.0f, size / 2.0f - 1.25f};

    buttonWindow->setRenderColor(Color::white);

    for (int i = 0; i < 3; ++i) {
        buttonWindow->renderRoundedRect(
            {topLeft, size, blockHeight},
            5
        );
        topLeft.y += blockHeight + spacing;
    }
}

void pieViewRenderer(Window& buttonWindow, const Rect& rect) {
    const float size = std::min(rect.size.x, rect.size.y);
    const float radius = (size - 8.0f) / 2.0f;
    const Point center = rect.center();

    Detail::drawPieSlice(
        buttonWindow,
        nullptr,
        center.x,
        center.y,
        radius,
        0.75,
        Color::white,
        0
    );
}

void monthsViewRenderer(Window& buttonWindow, const Rect& rect) {
    const float size = std::min(rect.size.x, rect.size.y) - 8.0f;
    const float halfSize = size / 2.0f;
    const Point center = rect.center();

    const Point tl = center + Point{-halfSize, -halfSize};
    const Point tr = center + Point{halfSize, -halfSize};
    const Point bl = center + Point{-halfSize, halfSize};
    const Point br = center + Point{halfSize, halfSize};

    buttonWindow->setRenderColor(Color::white);

    // Outline
    buttonWindow->renderThickLine({tl, bl}, 2, true);
    buttonWindow->renderThickLine({tr, br}, 2, true);
    buttonWindow->renderThickLine({tl, tr}, 2, true);
    buttonWindow->renderThickLine({bl, br}, 2, true);

    // Header separator
    const float headerY = tl.y + size * 0.3f;
    buttonWindow->renderThickLine(
        {{tl.x, headerY}, {tr.x, headerY}}, 2, true
    );

    // Calendar rings
    for (float x : {center.x - size * 0.22f - 1.0f, center.x + size * 0.22f}) {
        buttonWindow->renderThickLine(
            {{x, tl.y - 3}, {x, tl.y + size * 0.12f}}, 3, true
        );
    }
}

void barViewRenderer(Window& buttonWindow, const Rect& rect) {
    const float size = std::min(rect.size.x, rect.size.y) - 8.0f;
    const Point center = rect.center();
    const float baseline = center.y + size * 0.34f;
    const float barWidth = size * 0.16f;

    buttonWindow->setRenderColor(Color::white);

    buttonWindow->renderRoundedRect(
        {
            center.x - size * 0.34f,
            baseline - size * 0.4f,
            barWidth,
            size * 0.38f
        },
        2
    );

    buttonWindow->renderRoundedRect(
        {
            center.x - barWidth * 0.5f,
            baseline - size * 0.62f,
            barWidth,
            size * 0.62f
        },
        2
    );

    buttonWindow->renderRoundedRect(
        {
            center.x + size * 0.18f,
            baseline - size * 0.48f,
            barWidth,
            size * 0.48f
        },
        2
    );
}

} // namespace

int main(int argc, char* argv[]) {
    std::filesystem::path assetDirectory = getAssetDirectory(getExecutableDirectory(argc, argv));

    --argc; // Remove the executable name.
    ++argv; // Skip the executable name.

    bool dev = false;
    if (argc >= 1 && std::string(argv[0]) == "-dev") {
        dev = true;
        assetDirectory = "assets";
    }

    ModLoader::load<UIKitMod>();

    Window window("QBudget", 800, 600);
    auto [width, height] = window.size();

    Font font(assetDirectory / "JetBrainsMono-Regular.ttf", 16);

    const auto now = std::chrono::system_clock::now();
    const auto today = std::chrono::floor<std::chrono::days>(now);
    const std::chrono::year_month_day date{today};

    const int currentMonth =
        static_cast<int>(static_cast<unsigned>(date.month()));
    const int currentYear = static_cast<int>(date.year());

    int selectedMonth = currentMonth;
    int selectedYear = currentYear;

    const auto path = dev
        ? std::filesystem::path{"testDatabase"}
        : getAppDirectory("io.github.c_plusplus_dev/QBudget");

    std::filesystem::create_directories(path);

    DatabaseManager manager{path};
    Database* activeDatabase =
        &manager.getOrCreate(selectedYear, selectedMonth);

    bool rebuildRoot = false;

    const ButtonStyle navigationStyle =
        makeNavigationButtonStyle(font);

    NavigationView* nav = nullptr;
    auto buildRoot = [&]() -> std::unique_ptr<Panel> {
        auto newRoot = std::make_unique<Panel>(
            Rect{0, 0, width, height}
        );

        auto& navigation = newRoot->add<NavigationView>(
            Rect{0, 0, width, height},
            NavigationPosition::Top,
            30.0f
        );

        navigation.addView<BlocksView, MagickButton>(
            [&]() -> std::unique_ptr<BlocksView> {
                return std::make_unique<BlocksView>(
                    Rect{0, 0, width, height},
                    window,
                    font,
                    assetDirectory,
                    *activeDatabase,
                    selectedMonth,
                    selectedYear
                );
            },
            navigationStyle,
            blocksViewRenderer
        );

        navigation.addView<MonthsView, MagickButton>(
            [&]() -> std::unique_ptr<MonthsView> {
                return std::make_unique<MonthsView>(
                    Rect{0, 0, width, height},
                    manager,
                    font,
                    selectedMonth,
                    selectedYear,
                    [&] {
                        if (auto db = manager.get(selectedYear, selectedMonth)) {
                            activeDatabase = db;
                            rebuildRoot = true;
                        }
                    }
                );
            },
            navigationStyle,
            monthsViewRenderer
        );

        navigation.addView<PieView, MagickButton>(
            [&]() -> std::unique_ptr<PieView> {
                return std::make_unique<PieView>(
                    Rect{0, 0, width, height},
                    *activeDatabase,
                    font
                );
            },
            navigationStyle,
            pieViewRenderer
        );

        navigation.addView<BarView, MagickButton>(
            [&]() -> std::unique_ptr<BarView> {
                return std::make_unique<BarView>(
                    Rect{0, 0, width, height},
                    manager,
                    font,
                    selectedYear
                );
            },
            navigationStyle,
            barViewRenderer
        );

        nav = &navigation;

        return newRoot;
    };

    std::unique_ptr<Panel> root = buildRoot();

    Point previousWindowSize{-1, -1};

    while (!window.shouldClose()) {
        QTipRuntime::fetchEvents();

        if (rebuildRoot) {
            root = buildRoot();
            rebuildRoot = false;
            previousWindowSize = {-1, -1};
            if (nav) {
                nav->select(1);
            }
        }

        const Point currentWindowSize = window.size();
        if (currentWindowSize != previousWindowSize) {
            previousWindowSize = currentWindowSize;
            width = currentWindowSize.x;
            height = currentWindowSize.y;

            root->resize(currentWindowSize);

            // Panel::resize() only resizes its own render target; it does not
            // propagate the new size to child objects. Resize NavigationView
            // explicitly so it can relayout its active view and navigation bar.
            if (nav) {
                nav->resize(currentWindowSize);
            }
        }

        window->setRenderColor({20, 20, 20});
        window->clear();

        root->render(window);

        window->present();
    }

    manager.saveAll();

    return 0;
}