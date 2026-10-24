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

    void processDeletions();

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
    void createBlockViews();
    void updateLayout();
    void resizeBlock(BlockView& block);
    void handleExpansion(BlockView& block);
    void showDeleteConfirmation(BlockView& block) const;

    Window& _window;
    Font& _font;
    Database& _database;
    std::vector<BlockView*>& _blocks;
    std::vector<BlockView*>& _blocksToDelete;
    float _availableHeight = 0.0f;

    ScrollView _scrollView;
    List* _blockList = nullptr;

    MagickButton _addButton;

    Label _totalLabel;
};

#endif // QBUDGET_UI_BLOCKSVIEW_H
