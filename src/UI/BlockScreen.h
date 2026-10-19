
//
// Created by Ayaan on 2026-10-19.
//

#ifndef QBUDGET_BLOCKSCREEN_H
#define QBUDGET_BLOCKSCREEN_H

#include "Screen.h"

#include <Q-Tip/QTip.h>
#include <QTipUIKit/Containers/List.h>
#include <QTipUIKit/Containers/ScrollView.h>
#include <QTipUIKit/Widgets/Label/Label.h>

#include "Storage/Database.h"
#include "UI/BlockView.h"
#include "UI/MagickButton.h"

#include <vector>

class BlockScreen : public Screen {
public:
    BlockScreen(
        QTip::Rect rect,
        QTip::Window& window,
        Database& database,
        QTip::Font& font
    );

    ~BlockScreen() override = default;

    void tick(QTip::Clock& clock, double dt) override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;

protected:
    void renderImpl(QTip::Window& window) override;

private:
    void createBlocks();
    void requestDelete(BlockView& view);
    void processDeletions();
    void updateLayout();
    void updateBlockSize(BlockView& view);
    void onBlockExpanded(BlockView& view);
    void onResize();

    QTip::Window& _window;
    Database& _database;
    QTip::Font& _font;

    ScrollView* _blockView = nullptr;
    List* _blockList = nullptr;
    Label* _bottomTotal = nullptr;
    MagickButton* _addBlock = nullptr;

    std::vector<BlockView*> _blocks;
    std::vector<BlockView*> _blocksToDelete;
};

#endif // QBUDGET_BLOCKSCREEN_H
