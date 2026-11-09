#ifndef QTIPUIKIT_NAVIGATIONVIEW_H
#define QTIPUIKIT_NAVIGATIONVIEW_H

#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "QTipUIKit/Containers/Panel.h"
#include "QTipUIKit/Containers/NavigationView/View.h"
#include "QTipUIKit/Widgets/Button/Button.h"

enum class NavigationPosition {
    Top,
    Bottom,
    Left,
    Right
};

class NavigationView : public Panel {
public:
    explicit NavigationView(
        QTip::Rect rect,
        NavigationPosition position = NavigationPosition::Top,
        float barSize = 40.0f
    );

    ~NavigationView() override;

    template<
        std::derived_from<View> V,
        std::derived_from<Button> B = Button,
        typename Factory,
        typename... ButtonArgs
    >
    requires std::invocable<Factory&> &&
        std::convertible_to<
            std::invoke_result_t<Factory&>,
            std::unique_ptr<V>
        > &&
        std::constructible_from<B, QTip::Rect, ButtonArgs...>
    V& addView(
        Factory&& factory,
        ButtonArgs&&... buttonArgs
    ) {
        auto v = std::invoke(
            std::forward<Factory>(factory)
        );

        auto button = std::make_unique<B>(
            QTip::Rect{},
            std::forward<ButtonArgs>(buttonArgs)...
        );

        V* viewResult = v.get();

        button->setOnClick(
            [this, viewResult] {
                select(*viewResult);
            }
        );

        auto panel = std::make_unique<Panel>(QTip::Rect{});

        auto& view = panel->add<V>(std::move(v));

        Entry entry{
            std::move(button),
            std::move(panel),
            &view,
            {}
        };

        _entries.push_back(std::move(entry));

        _entries.back().button->parent = this;
        _entries.back().panel->parent = this;

        if (_entries.size() == 1) {
            _selectedIndex = 0;
            _entries.front().button->setDisabled(true);
        }

        layout();

        return *viewResult;
    }

    void select(View& view);

    void select(std::size_t index);

    [[nodiscard]] View* selectedView() const;

    [[nodiscard]] std::size_t selectedIndex() const;

    [[nodiscard]] std::size_t viewCount() const;

    void setPosition(NavigationPosition position);

    [[nodiscard]] NavigationPosition position() const;

    void setBarSize(float size);

    [[nodiscard]] float barSize() const;

    bool handleEvent(const QTip::Event& event) override;

    [[nodiscard]] QTip::Point minimumSize() const override;

    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;

    void reposition(QTip::Point position) override;

    void setRect(QTip::Rect rect) override;

    [[nodiscard]] const QTip::Rect& rect() override;

    void tick(QTip::Clock& clock, double dt) override;

protected:
    void renderImpl(QTip::Window& window) override;

private:
    struct Entry {
        std::unique_ptr<Button> button;
        std::unique_ptr<Panel> panel;
        View* view;
        QTip::Rect viewRect;
    };

    std::vector<Entry> _entries;

    NavigationPosition _position;
    float _barSize;

    std::size_t _selectedIndex = 0;

    Entry* selectedEntry();

    void layout();
};

#endif // QTIPUIKIT_NAVIGATIONVIEW_H