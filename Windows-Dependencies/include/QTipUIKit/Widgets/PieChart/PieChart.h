//
// Created by Ayaan on 2026-10-05.
//

#ifndef QTIPUIKIT_PIECHART_H
#define QTIPUIKIT_PIECHART_H
#include "QTipUIKit/UIObject.h"
#include "QTipUIKit/Internal/Helpers.h"

struct SDL_Vertex;

template <typename T>
struct PieChartItem {
    T data;
    double value;
    std::string label;
    QTip::Color color;
};

template <typename T>
concept PieChartConvertible = requires(const T& value) {
    { value.value() } -> std::convertible_to<double>;
    { value.label() } -> std::convertible_to<std::string>;
};

template <typename T>
concept PieChartColorConvertible = requires(const T& value) {
    { value.color() } -> std::convertible_to<QTip::Color>;
};

template <typename T>
class PieChart : public UIObject {
    struct _pieChartItem {
        T data;
        double value;
        std::string label;
        QTip::Color color;

        double percent = -1;
        bool colorValid = false;
        float expand = 0;
        double animatedPercent = 0;
        std::vector<QTip::Point> vb;
    };

    static constexpr bool isColorConvertible =
        PieChartColorConvertible<T>;
public:
    using ValueFormatter = std::function<double(T&)>;
    using LabelFormatter = std::function<std::string(T&)>;
    using ColorFormatter = std::function<QTip::Color(T&)>;
    struct Formatters {
        ValueFormatter value;
        LabelFormatter label;
        ColorFormatter color;
    };

    using TooltipRenderer = std::function<void(QTip::Window& window, QTip::Font font, T& data, double value,
                                               double percent, uint8_t alpha)>;

    PieChart(
        QTip::Rect rect,
        std::vector<T> items,
        bool tooltip = false
    ) requires PieChartConvertible<T>;

    PieChart(
        QTip::Rect rect,
        std::vector<T> items,
        Formatters formatters,
        bool tooltip = false
    );

    PieChart(
        QTip::Rect rect,
        std::vector<PieChartItem<T>> items,
        bool tooltip = false
    );

    ~PieChart() override = default;

    void setData(
        std::vector<T> items
    ) requires PieChartConvertible<T>;

    void setData(
        std::vector<T> items,
        Formatters formatters
    );

    void setData(
        std::vector<PieChartItem<T>> items
    );

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

    std::vector<_pieChartItem> _items;
    double _total = 0;

    size_t _hoveredOn = -1;

    bool _tooltip = false;

    TooltipRenderer _tooltipRenderer;
    std::unique_ptr<QTip::RenderTexture> _tooltipTexture;

    QTip::Font font{Detail::defaultFontPath(), 16};
    QTip::Point mousePosition;

protected:
    void renderImpl(QTip::Window& window) override;
};

template <typename T>
using PieChartFormatters = PieChart<T>::Formatters;

#include "PieChart.tpp"

#endif //QTIPUIKIT_PIECHART_H
