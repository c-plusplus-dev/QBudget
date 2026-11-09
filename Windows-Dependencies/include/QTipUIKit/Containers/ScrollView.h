//
// Created by Ayaan on 2026-09-27.
//

#ifndef QTIPUIKIT_SCROLLVIEW_H
#define QTIPUIKIT_SCROLLVIEW_H

#include <Q-Tip/Graphics/RenderTexture.h>

#include "QTipUIKit/Containers/Panel.h"
#include "QTipUIKit/Widgets/ScrollBar/ScrollBar.h"

enum class ScrollDirection {
    Horizontal,
    Vertical,
    Both
};

struct ScrollViewSettings {
    ScrollDirection direction = ScrollDirection::Vertical;
    float scrollSpeed = 30.0f;
    bool showScrollbar = true;
};

class ScrollView : public Panel {
public:
    explicit ScrollView(
        QTip::Rect rect,
        ScrollViewSettings settings = {}
    );

    ~ScrollView() override;

    void scrollTo(QTip::Point position);
    void scrollBy(QTip::Point delta);

    [[nodiscard]]
    QTip::Point scrollPosition() const;

    void setContentSize(QTip::Point size);

    [[nodiscard]]
    QTip::Point contentSize() const;

    [[nodiscard]]
    const ScrollViewSettings& settings() const;

    void setScrollSpeed(float scrollSpeed);
    void showScrollbars(bool show);

    bool handleEvent(const QTip::Event& event) override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    const QTip::Rect& rect() override;

protected:
    void clampScrollPosition();

    void updateScrollBars();

    void updateScrollBarsDimensions();

    bool x() const;
    bool y() const;

    ScrollViewSettings _settings;
    QTip::Point _scrollPosition{};
    QTip::Point _contentSize{};

    ScrollBar* _horizontalScrollBar = nullptr;
    ScrollBar* _verticalScrollBar = nullptr;

    void renderImpl(QTip::Window& window) override;

    void initTarget() override;
};


#endif
