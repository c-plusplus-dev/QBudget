#ifndef QBUDGET_UI_BLOCKSVIEW_H
#define QBUDGET_UI_BLOCKSVIEW_H

#include <Q-Tip/QTip.h>
#include <QTipUIKit/Containers/List.h>
#include <QTipUIKit/Containers/ScrollView.h>
#include <QTipUIKit/Containers/NavigationView/View.h>

#include "Storage/Database.h"
#include "UI/BlockView.h"

#include <cstddef>
#include <optional>
#include <vector>
#include <QTipUIKit/Widgets/Button/MagickButton.h>

#include "Spacer.h"

using namespace QTip;

class BlocksView final : public View {
public:
    BlocksView(
        QTip::Rect rect,
        QTip::Window& window,
        QTip::Font& font,
        Database& database,
        int currentMonth,
        int currentYear
    );

    ~BlocksView() override = default;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;

    [[nodiscard]] const QTip::Rect& rect() override;
    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    bool handleEvent(const Event& event) override;
    void tick(QTip::Clock& clock, double dt) override;

protected:
    void renderImpl(QTip::Window& window) override;

private:
    void processDeletions();
    void createBlockViews();
    void addBlockView(Block& block, bool moveSpacer = false);
    void addBlock();
    void updateLayout();
    void handleExpansion(BlockView& block);
    void showDeleteConfirmation(BlockView& block);

    struct DragState {
        BlockView* blockView = nullptr;
        std::size_t sourceIndex = 0;
        std::size_t dropIndex = 0;
        QTip::Point startPosition{};
        QTip::Point pointerPosition{};
        QTip::Point grabOffset{};
        bool moved = false;
    };

    [[nodiscard]] bool beginDrag(QTip::Point position);
    void updateDrag(QTip::Point position);
    void finishDrag();
    void clearDragIndicators();
    void animateDragLayout(float dt);
    void rebuildBlockViews();
    void reorderBlock(std::size_t source, std::size_t destination);

    Font& _font;
    Database& _database;
    std::vector<BlockView*> _blocks;
    std::vector<BlockView*> _blocksToDelete;
    int _currentMonth;
    int _currentYear;
    float _availableHeight = 0.0f;

    ScrollView _scrollView;
    List* _blockList = nullptr;
    Spacer* _spacer = nullptr;

    MagickButton _addButton;

    std::optional<DragState> _drag;
    Rect _dragPlaceholder{};
    bool _hasDragPlaceholder = false;
};

#endif // QBUDGET_UI_BLOCKSVIEW_H
