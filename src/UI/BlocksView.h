#ifndef QBUDGET_UI_BLOCKSVIEW_H
#define QBUDGET_UI_BLOCKSVIEW_H

#include <Q-Tip/QTip.h>
#include <QTipUIKit/Containers/List.h>
#include <QTipUIKit/Containers/ScrollView.h>
#include <QTipUIKit/Containers/NavigationView/View.h>

#include "Storage/Database.h"
#include "UI/BlockView.h"

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
        std::vector<BlockView*>& blocks,
        std::vector<BlockView*>& blocksToDelete,
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

    bool handleEvent(const SDL_Event& event) override;
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
    void showDeleteConfirmation(BlockView& block) const;

    Font& _font;
    Database& _database;
    std::vector<BlockView*>& _blocks;
    std::vector<BlockView*>& _blocksToDelete;
    int _currentMonth;
    int _currentYear;
    float _availableHeight = 0.0f;

    ScrollView _scrollView;
    List* _blockList = nullptr;
    Spacer* _spacer = nullptr;

    MagickButton _addButton;
};

#endif // QBUDGET_UI_BLOCKSVIEW_H
