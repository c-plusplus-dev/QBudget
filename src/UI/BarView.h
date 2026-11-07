#ifndef QBUDGET_UI_BARVIEW_H
#define QBUDGET_UI_BARVIEW_H

#include <Q-Tip/QTip.h>
#include <QTipUIKit/Containers/NavigationView/View.h>

#include <array>
#include <cstddef>
#include <cstdint>
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
    void refreshData();
    void selectBlock(int direction);
    void layout();

    DatabaseManager& _manager;
    QTip::Font& _font;
    int& _selectedYear;

    std::vector<std::string> _blockNames;
    std::string _selectedBlock;

    std::array<std::int64_t, 12> _values{};
    std::array<bool, 12> _availableMonths{};
    std::array<bool, 12> _hasBlock{};

    QTip::Rect _previousBlockRect{};
    QTip::Rect _nextBlockRect{};
    QTip::Point _mousePosition{};

    int _cachedYear = -1;
    float _refreshTimer = 0.0f;
    float _barProgress = 1.0f;
};

#endif // QBUDGET_UI_BARVIEW_H