//
// Created by Ayaan on 2026-10-12.
//

#ifndef QTIPUIKIT_BARCHART_H
#define QTIPUIKIT_BARCHART_H

#include "QTipUIKit/UIObject.h"
#include "QTipUIKit/Internal/Helpers.h"

enum class BarChartOrientation {
    Vertical,
    Horizontal
};

template <typename T>
struct BarChartItem {
    T data;
    double value;
    std::string label;
    QTip::Color color;
};

template <typename T>
concept BarChartConvertible = requires(const T& value) {
    { value.value() } -> std::convertible_to<double>;
    { value.label() } -> std::convertible_to<std::string>;
};

template <typename T>
concept BarChartColorConvertible = requires(const T& value) {
    { value.color() } -> std::convertible_to<QTip::Color>;
};

template <typename T>
class BarChart : public UIObject {
    struct _barChartItem {
        T data;
        double value;
        std::string label;
        QTip::Color color;

        bool colorValid = false;

        double targetValue = 0;
        double animatedValue = 0;
        float expand = 0;

        QTip::Rect barRect;
    };

    static constexpr bool isColorConvertible =
        BarChartColorConvertible<T>;

public:
    using ValueFormatter = std::function<double(T&)>;
    using LabelFormatter = std::function<std::string(T&)>;
    using ColorFormatter = std::function<QTip::Color(T&)>;

    struct Formatters {
        ValueFormatter value;
        LabelFormatter label;
        ColorFormatter color;
    };

    using TooltipRenderer = std::function<void(
        QTip::Window& window,
        QTip::Font font,
        T& data,
        double value,
        uint8_t alpha
    )>;

    BarChart(
        QTip::Rect rect,
        std::vector<T> items,
        bool tooltip = false
    ) requires BarChartConvertible<T>;

    BarChart(
        QTip::Rect rect,
        std::vector<T> items,
        Formatters formatters,
        bool tooltip = false
    );

    BarChart(
        QTip::Rect rect,
        std::vector<BarChartItem<T>> items,
        bool tooltip = false
    );

    ~BarChart() override = default;

    void setData(
        std::vector<T> items
    ) requires BarChartConvertible<T>;

    void setData(
        std::vector<T> items,
        Formatters formatters
    );

    void setData(
        std::vector<BarChartItem<T>> items
    );

    void setOrientation(BarChartOrientation orientation);
    BarChartOrientation orientation() const;

    void setTooltipRenderer(TooltipRenderer renderer);

    size_t hoveredIndex() const;
    T* hoveredItem() const;

    bool handleEvent(const QTip::Event& event) override;

    QTip::Point minimumSize() const override;
    QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    const QTip::Rect& rect() override;

    void tick(QTip::Clock& clock, double dt) override;

private:
    void init();
    void calculateLayout();

    std::vector<_barChartItem> _items;

    double _minimumValue = 0;
    double _maximumValue = 0;

    size_t _hoveredOn = -1;

    bool _tooltip = false;

    BarChartOrientation _orientation =
        BarChartOrientation::Vertical;

    TooltipRenderer _tooltipRenderer;

    QTip::Font font{Detail::defaultFontPath(), 16};
    QTip::Point mousePosition{};

protected:
    void renderImpl(QTip::Window& window) override;
};

template <typename T>
using BarChartFormatters = BarChart<T>::Formatters;

#include "BarChart.tpp"

#endif //QTIPUIKIT_BARCHART_H
