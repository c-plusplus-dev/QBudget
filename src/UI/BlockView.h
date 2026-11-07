
//
// Created by Ayaan on 2026-10-15.
//

#ifndef QBUDGET_BLOCKVIEW_H
#define QBUDGET_BLOCKVIEW_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <QTipUIKit/UIKit.h>
#include <QTipUIKit/Containers/ScrollView.h>

#include "Storage/Block.h"
#include "UI/ItemView.h"

class BlockView : public UIObject {
public:
    using Action = std::function<void(BlockView&)>;

    // Requires the block to stay alive for the lifetime of BlockView.
    explicit BlockView(
        QTip::Rect rect,
        Block& block,
        QTip::Font& font,
        Action expandRequested,
        Action deleteRequested,
        Action addRequested
    );

    ~BlockView() override = default;

    [[nodiscard]] Block& block() noexcept;
    [[nodiscard]] const Block& block() const noexcept;

    struct State {
        bool expanded = false;
        QTip::Point itemScrollPosition{};
    };

    [[nodiscard]] State saveState() const;
    void restoreItemScrollPosition(QTip::Point position);

    void setExpanded(bool expanded);

    bool handleEvent(const QTip::Event& event) override;
    void tick(QTip::Clock& clock, double dt) override;

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

    bool expanded = false;

protected:
    void renderImpl(QTip::Window& window) override;

private:
    friend class BlocksView;

    struct ItemDragState {
        ItemView* itemView = nullptr;
        std::size_t sourceIndex = 0;
        std::size_t dropIndex = 0;
        QTip::Point startPosition{};
        QTip::Point pointerPosition{};
        QTip::Point grabOffset{};
        bool moved = false;
    };

    bool handleItemDragEvent(
        const QTip::Event& event,
        QTip::Point contentPosition,
        bool allowStart
    );
    bool beginItemDrag(QTip::Point position);
    void updateItemDrag(QTip::Point position);
    void finishItemDrag();
    void cancelItemDrag();
    void clearItemDragIndicators();
    void animateItemDragLayout(float dt);

    void onResize();
    void syncItemViews();
    void rebuildItemViews();
    void layoutItemViews() const;

    void onEdit(ItemView& itemView);
    void onDelete(ItemView& itemView);
    void onAddItem();

    ItemView& addItem(Block::Item& item);

    void processDeletions();

    Block& _block;

    std::unique_ptr<Button> _delete;
    std::unique_ptr<Button> _expand;
    std::unique_ptr<Button> _add;

    std::unique_ptr<QTip::Texture> _moveTexture;

    std::vector<ItemView*> _itemViews;
    std::unique_ptr<ScrollView> _itemScrollView;
    std::optional<ItemDragState> _itemDrag;
    QTip::Rect _itemDragPlaceholder{};
    bool _hasItemDragPlaceholder = false;
    bool _manualItemLayout = false;

    std::vector<ItemView*> _itemsToDelete;

    Action _expandRequested;
    Action _deleteRequested;
    Action _addRequested;

    QTip::Font& _font;
};

#endif // QBUDGET_BLOCKVIEW_H
