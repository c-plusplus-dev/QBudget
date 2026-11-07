#ifndef QBUDGET_UI_PIEVIEW_H
#define QBUDGET_UI_PIEVIEW_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

#include <Q-Tip/QTip.h>
#include <Q-Tip/Graphics/RenderTexture.h>

#include <QTipUIKit/Containers/NavigationView/View.h>
#include <QTipUIKit/Containers/ScrollView.h>
#include <QTipUIKit/Widgets/PieChart/PieChart.h>

#include "Storage/Database.h"

class PieView final : public View {
public:
    PieView(QTip::Rect rect, Database& database, QTip::Font& font);
    ~PieView() override = default;

    void refresh();

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
    struct Entry {
        Block* block = nullptr;
        std::int64_t value = 0;
        double percent = 0.0;
        QTip::Color color = QTip::Color::white;
    };

    static constexpr std::size_t kNoEntry = static_cast<std::size_t>(-1);

    Database& _database;
    QTip::Font& _font;
    ScrollView _legendScrollView;
    std::unique_ptr<PieChart<Block*>> _chart;
    std::unique_ptr<QTip::RenderTexture> _chartCache;
    QTip::Rect _chartRect{};
    bool _chartDirty = true;
    float _chartAnimationRemaining = 0.0f;
    bool _rowAnimationActive = false;
    std::vector<Entry> _entries;
    std::vector<UIObject*> _legendRows;
    std::vector<float> _rowExpandProgress;

    std::uint64_t _cachedDatabaseRevision = std::numeric_limits<std::uint64_t>::max();
    std::size_t _chartHoveredIndex = kNoEntry;
    std::size_t _expandedIndex = kNoEntry;
    float _targetScrollY = 0.0f;
    bool _manualScrolling = false;

    void refreshDataIfNeeded();
    void rebuildData();
    void layout();
    void syncHoveredEntry(bool scrollToEntry);
    void scrollEntryIntoView(std::size_t index);
};

#endif // QBUDGET_UI_PIEVIEW_H
