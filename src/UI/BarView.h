#ifndef QBUDGET_UI_BARVIEW_H
#define QBUDGET_UI_BARVIEW_H

#include <Q-Tip/QTip.h>
#include <Q-Tip/Graphics/RenderTexture.h>
#include <QTipUIKit/Containers/NavigationView/View.h>
#include <QTipUIKit/Widgets/BarChart/BarChart.h>
#include <QTipUIKit/Widgets/Button/Button.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Storage/DatabaseManager.h"

class BarView final : public View {
public:
    BarView(
        QTip::Rect rect,
        DatabaseManager& manager,
        QTip::Font& font,
        int& selectedYear
    );

    bool handleEvent(const QTip::Event& event) override;
    void tick(QTip::Clock& clock, double dt) override;

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

protected:
    void renderImpl(QTip::Window& window) override;

private:
    struct MonthBar {
        int month = 1;
        std::int64_t amount = 0;
        bool available = false;
        bool hasBlock = false;
    };

    void refreshData();
    void selectBlock(int direction);
    void layout();
    void updateNavigationButtons();

    DatabaseManager& _manager;
    QTip::Font& _font;
    int& _selectedYear;

    std::vector<std::string> _blockNames;
    std::string _selectedBlock;
    std::string _titleText = "No blocks found";
    std::string _yearText;

    std::array<std::int64_t, 12> _values{};
    std::array<bool, 12> _availableMonths{};
    std::array<bool, 12> _hasBlock{};

    std::unique_ptr<Button> _previousBlockButton;
    std::unique_ptr<Button> _nextBlockButton;
    std::unique_ptr<BarChart<MonthBar>> _chart;
    std::unique_ptr<QTip::RenderTexture> _chartCache;
    QTip::Rect _chartRect{};
    bool _chartDirty = true;
    float _chartAnimationRemaining = 0.0f;

    std::uint64_t _cachedManagerRevision = UINT64_MAX;
    int _cachedYear = -1;
    std::string _cachedSelection;
};

#endif // QBUDGET_UI_BARVIEW_H
