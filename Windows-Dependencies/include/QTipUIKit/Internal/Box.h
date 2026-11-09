//
// Created by Ayaan on 2026-09-20.
//

#ifndef QTIPUIKIT_BOX_H
#define QTIPUIKIT_BOX_H

#include "QTipUIKit/Containers/Panel.h"

namespace Detail {

class Box : public Panel {
public:
    enum class Direction {
        Horizontal,
        Vertical
    };

    enum class Sizing {
        Preferred,
        Stretch
    };

    enum class CrossSizing {
        Center,
        Stretch
    };

    struct Settings {
        Direction direction = Direction::Horizontal;
        Sizing sizing = Sizing::Preferred;
        CrossSizing crossSizing = CrossSizing::Center;

        float spacing = 5.0f;

        float paddingLeft = 0.0f;
        float paddingTop = 0.0f;
        float paddingRight = 0.0f;
        float paddingBottom = 0.0f;
    };

    Box(QTip::Rect rect, Settings settings);

    ~Box() override = default;

    void remove(UIObject& object) override;

    void clear() override;

    bool handleEvent(const QTip::Event& event) override;

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

    void setSettings(Settings settings);
    [[nodiscard]] const Settings& settings() const;

    void setDirection(Direction direction);
    [[nodiscard]] Direction direction() const;

    void setSizing(Sizing sizing);
    [[nodiscard]] Sizing sizing() const;

    void setCrossSizing(CrossSizing sizing);
    [[nodiscard]] CrossSizing crossSizing() const;

    void setSpacing(float spacing);
    [[nodiscard]] float spacing() const;

    void setPadding(
        float left,
        float top,
        float right,
        float bottom
    );

    void setPadding(float horizontal, float vertical);

    [[nodiscard]] float paddingLeft() const;
    [[nodiscard]] float paddingTop() const;
    [[nodiscard]] float paddingRight() const;
    [[nodiscard]] float paddingBottom() const;

    void layout();

protected:

    void onAddObject(UIObject* object) override;

    bool computingLayout = false;

    void renderImpl(QTip::Window& window) override;

private:
    [[nodiscard]] float mainAxis(QTip::Point point) const;
    [[nodiscard]] float crossAxis(QTip::Point point) const;

    [[nodiscard]] QTip::Point makePoint(
        float main,
        float cross
    ) const;

    [[nodiscard]] QTip::Point makeSize(
        float main,
        float cross
    ) const;

    [[nodiscard]] float availableMainSize() const;
    [[nodiscard]] float availableCrossSize() const;

    [[nodiscard]] float totalPreferredMainSize() const;
    [[nodiscard]] float totalMinimumMainSize() const;

    [[nodiscard]] float totalSpacing() const;

    Settings _settings;
};

} // namespace Detail

typedef Detail::Box::Direction   Direction;
typedef Detail::Box::Sizing      Sizing;
typedef Detail::Box::CrossSizing CrossSizing;
typedef Detail::Box::Settings    Settings;

#endif // QTIPUIKIT_BOX_H