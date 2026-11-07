#ifndef QBUDGET_UI_MONTHSVIEW_H
#define QBUDGET_UI_MONTHSVIEW_H

#include <Q-Tip/QTip.h>
#include <QTipUIKit/Containers/NavigationView/View.h>
#include <QTipUIKit/Widgets/Button/Button.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "Storage/DatabaseManager.h"

class MonthsView final : public View {
public:
    MonthsView(
        QTip::Rect rect,
        DatabaseManager& manager,
        QTip::Font& font,
        int& selectedMonth,
        int& selectedYear,
        std::function<void()> selectionChanged
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
    struct MonthTile {
        QTip::Rect rect{};
        std::int64_t total = 0;
        bool available = false;
    };

    void refreshData();
    void layout();
    void selectYear(int direction);
    void selectMonth(int month) const;

    DatabaseManager& _manager;
    QTip::Font& _font;
    int& _selectedMonth;
    int& _selectedYear;
    std::function<void()> _selectionChanged;

    std::vector<int> _years;
    std::array<MonthTile, 12> _tiles{};

    std::unique_ptr<Button> _previousYearButton;
    std::unique_ptr<Button> _nextYearButton;

    float _refreshTimer = 0.0f;
};

#endif // QBUDGET_UI_MONTHSVIEW_H