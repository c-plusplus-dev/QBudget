
//
// Created by Ayaan on 2026-10-17.
//

#ifndef QBUDGET_ITEMVIEW_H
#define QBUDGET_ITEMVIEW_H

#include <functional>
#include <memory>
#include <string>

#include <QTipUIKit/UIKit.h>
#include <QTipUIKit/Widgets/Button/MagickButton.h>
#include <QTipUIKit/Containers/ScrollView.h>

#include "Storage/Block.h"
#include "UI/Textures.h"

class ItemView : public UIObject {
public:
    using Action = std::function<void(ItemView&)>;

    ItemView(
        QTip::Rect rect,
        Block::Item& item,
        QTip::Font& font,
        UITextures& textures,
        ScrollView& parentScrollView,
        Action onEdit,
        Action onDelete
    );

    ~ItemView() override = default;

    [[nodiscard]] Block::Item& item();
    [[nodiscard]] const Block::Item& item() const;

    bool handleEvent(const QTip::Event& event) override;

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

protected:
    void renderImpl(QTip::Window& window) override;

private:
    friend class BlocksView;
    friend class BlockView;

    void updateLayout();

    [[nodiscard]] float calculateWidth() const;

    Block::Item* _item = nullptr;
    QTip::Font& _font;
    std::string _cachedName;
    std::string _cachedCostText;
    UITextures& _textures;
    ScrollView& _parentScrollView;

    std::unique_ptr<Button> _deleteButton;
    std::unique_ptr<MagickButton> _editButton;

    Action _onEdit;
    Action _onDelete;
};

#endif // QBUDGET_ITEMVIEW_H
