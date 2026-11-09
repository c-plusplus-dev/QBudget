//
// Created by Ayaan on 2026-10-12.
//

#ifndef QTIPUIKIT_BARCHART_TPP
#define QTIPUIKIT_BARCHART_TPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace Detail {

    inline std::string barChartNumber(double value) {
        if (!std::isfinite(value))
            return "N/A";

        if (value == 0.0)
            return "0";

        const double magnitude = std::abs(value);

        if (magnitude >= 1'000'000.0 || magnitude < 0.001) {
            std::ostringstream stream;
            stream << std::scientific << std::setprecision(2) << value;
            return stream.str();
        }

        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;

        std::string result = stream.str();

        while (!result.empty() && result.back() == '0')
            result.pop_back();

        if (!result.empty() && result.back() == '.')
            result.pop_back();

        return result;
    }

    inline std::string barChartTruncate(
        const QTip::Font& font,
        const std::string& text,
        float maxWidth
    ) {
        if (font.getTextWidth(text) <= maxWidth)
            return text;

        constexpr const char* ellipsis = "...";

        if (maxWidth <= font.getTextWidth(ellipsis))
            return "";

        std::string result = text;

        while (!result.empty()) {
            result.pop_back();

            const std::string candidate = result + ellipsis;

            if (font.getTextWidth(candidate) <= maxWidth)
                return candidate;
        }

        return "";
    }


    struct BarChartAxisScale {
        double minimum = 0.0;
        double maximum = 1.0;
        double step = 0.2;
        int tickCount = 5;
    };

    inline double barChartNiceStep(double rawStep) {
        if (!std::isfinite(rawStep) || rawStep <= 0.0)
            return 1.0;

        const double magnitude = std::pow(
            10.0,
            std::floor(std::log10(rawStep))
        );

        const double fraction = rawStep / magnitude;

        double niceFraction;

        if (fraction <= 1.0)
            niceFraction = 1.0;
        else if (fraction <= 2.0)
            niceFraction = 2.0;
        else if (fraction <= 5.0)
            niceFraction = 5.0;
        else
            niceFraction = 10.0;

        return niceFraction * magnitude;
    }

    inline BarChartAxisScale barChartAxisScale(
        double dataMinimum,
        double dataMaximum,
        int desiredTicks = 5
    ) {
        BarChartAxisScale scale;

        double minimum = std::min(0.0, dataMinimum);
        double maximum = std::max(0.0, dataMaximum);

        if (minimum == maximum) {
            scale.minimum = 0.0;
            scale.maximum = 1.0;
            scale.step = 0.2;
            scale.tickCount = 5;
            return scale;
        }

        desiredTicks = std::max(1, desiredTicks);

        const double rawStep =
            (maximum - minimum) / desiredTicks;

        scale.step = barChartNiceStep(rawStep);

        minimum = std::floor(minimum / scale.step) * scale.step;
        maximum = std::ceil(maximum / scale.step) * scale.step;

        // Ensure that floating-point rounding never clips the data.
        minimum = std::min(minimum, std::min(0.0, dataMinimum));
        maximum = std::max(maximum, std::max(0.0, dataMaximum));

        scale.minimum = minimum;
        scale.maximum = maximum;

        scale.tickCount = static_cast<int>(
            std::llround(
                (scale.maximum - scale.minimum) / scale.step
            )
        );

        return scale;
    }

} // namespace Detail

template <typename T>
BarChart<T>::BarChart(
    QTip::Rect rect,
    std::vector<T> items,
    bool tooltip
) requires BarChartConvertible<T> {
    _rect = rect;
    _preferredRect = rect;
    _tooltip = tooltip;

    setData(std::move(items));
}

template <typename T>
BarChart<T>::BarChart(
    QTip::Rect rect,
    std::vector<T> items,
    Formatters formatters,
    bool tooltip
) {
    _rect = rect;
    _preferredRect = rect;
    _tooltip = tooltip;

    setData(std::move(items), std::move(formatters));
}

template <typename T>
BarChart<T>::BarChart(
    QTip::Rect rect,
    std::vector<BarChartItem<T>> items,
    bool tooltip
) {
    _rect = rect;
    _preferredRect = rect;
    _tooltip = tooltip;

    setData(std::move(items));
}

template <typename T>
void BarChart<T>::setData(std::vector<T> items)
    requires BarChartConvertible<T> {
    _items.clear();
    _items.reserve(items.size());

    _hoveredOn = static_cast<size_t>(-1);

    _minimumValue = std::numeric_limits<double>::max();
    _maximumValue = std::numeric_limits<double>::lowest();

    const double colorStep = items.empty()
        ? 0.0
        : 1.0 / static_cast<double>(items.size());

    size_t index = 0;

    for (auto& item : items) {
        const double value = static_cast<double>(item.value());

        if (!std::isfinite(value))
            throw std::invalid_argument(
                "BarChart values must be finite."
            );

        std::string label = item.label();

        QTip::Color color;

        if constexpr (isColorConvertible) {
            color = item.color();
        } else {
            color = Detail::pieChartColor(
                static_cast<double>(index) * colorStep
            );
        }

        _items.emplace_back(
            std::move(item),
            value,
            std::move(label),
            color
        );

        auto& chartItem = _items.back();

        chartItem.colorValid = true;
        chartItem.targetValue = value;
        chartItem.animatedValue = 0.0;
        chartItem.expand = 0.0f;
        chartItem.barRect = QTip::Rect::zero;

        _minimumValue = std::min(_minimumValue, value);
        _maximumValue = std::max(_maximumValue, value);

        ++index;
    }

    if (_items.empty()) {
        _minimumValue = 0.0;
        _maximumValue = 0.0;
    }

    calculateLayout();
}

template <typename T>
void BarChart<T>::setData(
    std::vector<T> items,
    Formatters formatters
) {
    if (!formatters.value || !formatters.label)
        throw std::invalid_argument(
            "BarChart requires value and label formatters."
        );

    _items.clear();
    _items.reserve(items.size());

    _hoveredOn = static_cast<size_t>(-1);

    _minimumValue = std::numeric_limits<double>::max();
    _maximumValue = std::numeric_limits<double>::lowest();

    const double colorStep = items.empty()
        ? 0.0
        : 1.0 / static_cast<double>(items.size());

    size_t index = 0;

    for (auto& item : items) {
        const double value = formatters.value(item);

        if (!std::isfinite(value))
            throw std::invalid_argument(
                "BarChart values must be finite."
            );

        std::string label = formatters.label(item);

        QTip::Color color;

        if (formatters.color) {
            color = formatters.color(item);
        } else {
            color = Detail::pieChartColor(
                static_cast<double>(index) * colorStep
            );
        }

        _items.emplace_back(
            std::move(item),
            value,
            std::move(label),
            color
        );

        auto& chartItem = _items.back();

        chartItem.colorValid = true;
        chartItem.targetValue = value;
        chartItem.animatedValue = 0.0;
        chartItem.expand = 0.0f;
        chartItem.barRect = QTip::Rect::zero;

        _minimumValue = std::min(_minimumValue, value);
        _maximumValue = std::max(_maximumValue, value);

        ++index;
    }

    if (_items.empty()) {
        _minimumValue = 0.0;
        _maximumValue = 0.0;
    }

    calculateLayout();
}

template <typename T>
void BarChart<T>::setData(
    std::vector<BarChartItem<T>> items
) {
    _items.clear();
    _items.reserve(items.size());

    _hoveredOn = static_cast<size_t>(-1);

    _minimumValue = std::numeric_limits<double>::max();
    _maximumValue = std::numeric_limits<double>::lowest();

    for (auto& item : items) {
        if (!std::isfinite(item.value))
            throw std::invalid_argument(
                "BarChart values must be finite."
            );

        const double value = item.value;
        std::string label = std::move(item.label);
        QTip::Color color = item.color;

        _items.emplace_back(
            std::move(item.data),
            value,
            std::move(label),
            color
        );

        auto& chartItem = _items.back();

        chartItem.colorValid = true;
        chartItem.targetValue = value;
        chartItem.animatedValue = 0.0;
        chartItem.expand = 0.0f;
        chartItem.barRect = QTip::Rect::zero;

        _minimumValue = std::min(_minimumValue, value);
        _maximumValue = std::max(_maximumValue, value);
    }

    if (_items.empty()) {
        _minimumValue = 0.0;
        _maximumValue = 0.0;
    }

    calculateLayout();
}

template <typename T>
void BarChart<T>::setOrientation(
    BarChartOrientation orientation
) {
    if (_orientation == orientation)
        return;

    _orientation = orientation;
    _hoveredOn = static_cast<size_t>(-1);

    calculateLayout();
}

template <typename T>
BarChartOrientation BarChart<T>::orientation() const {
    return _orientation;
}

template <typename T>
void BarChart<T>::setTooltipRenderer(TooltipRenderer renderer) {
    _tooltipRenderer = std::move(renderer);
}

template <typename T>
size_t BarChart<T>::hoveredIndex() const {
    return _hoveredOn;
}

template <typename T>
T* BarChart<T>::hoveredItem() const {
    if (_hoveredOn >= _items.size())
        return nullptr;

    return &_items[_hoveredOn].data;
}

template <typename T>
bool BarChart<T>::handleEvent(const QTip::Event& event) {
    bool hasMousePosition = false;

    switch (event.type) {
        case QTip::EventType::MouseMoved:
            mousePosition = {
                event.mouseMove.position.x,
                event.mouseMove.position.y
            };
            hasMousePosition = true;
            break;

        case QTip::EventType::MouseButtonPressed:
        case QTip::EventType::MouseButtonReleased:
            mousePosition = {
                event.mouseButton.position.x,
                event.mouseButton.position.y
            };
            hasMousePosition = true;
            break;

        //case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        //    _hoveredOn = static_cast<size_t>(-1);
        //    calculateLayout();
        //    break;

        default:
            break;
    }

    if (hasMousePosition) {
        size_t newHovered = static_cast<size_t>(-1);

        for (size_t i = 0; i < _items.size(); ++i) {
            if (_items[i].barRect.isPointInside(mousePosition)) {
                newHovered = i;
                break;
            }
        }

        if (newHovered != _hoveredOn) {
            _hoveredOn = newHovered;
            calculateLayout();
        }
    }

    return UIObject::handleEvent(event);
}

template <typename T>
QTip::Point BarChart<T>::minimumSize() const {
    constexpr float axisPadding = 12.0f;
    constexpr float minimumPlotWidth = 64.0f;
    constexpr float minimumPlotHeight = 48.0f;
    constexpr float minimumBarThickness = 8.0f;
    constexpr float minimumBarGap = 3.0f;
    constexpr float maximumLabelWidth = 150.0f;

    if (_orientation == BarChartOrientation::Vertical) {
        float requiredWidth = 64.0f;

        for (const auto& item : _items) {
            requiredWidth = std::max(
                requiredWidth,
                font.getTextWidth(item.label) + axisPadding
            );
        }

        const float barSpace =
            static_cast<float>(_items.size()) *
                (minimumBarThickness + minimumBarGap);

        return {
            std::max(
                110.0f,
                requiredWidth + minimumPlotWidth + barSpace
            ),
            minimumPlotHeight +
                font.getTextHeight("0") * 2.0f +
                axisPadding * 2.0f
        };
    }

    float labelWidth = 0.0f;

    for (const auto& item : _items) {
        labelWidth = std::max(
            labelWidth,
            font.getTextWidth(item.label)
        );
    }

    labelWidth = std::min(labelWidth, maximumLabelWidth);

    const float rowHeight =
        minimumBarThickness + minimumBarGap;

    return {
        labelWidth + minimumPlotWidth + axisPadding * 2.0f,
        std::max(
            64.0f,
            static_cast<float>(_items.size()) * rowHeight +
                font.getTextHeight("0") * 2.0f +
                axisPadding * 2.0f
        )
    };
}

template <typename T>
QTip::Point BarChart<T>::preferredSize() const {
    return _preferredRect.size;
}

template <typename T>
void BarChart<T>::resize(QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;

    calculateLayout();
}

template <typename T>
void BarChart<T>::reposition(QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;

    calculateLayout();
}

template <typename T>
void BarChart<T>::setRect(QTip::Rect rect) {
    _rect = rect;

    calculateLayout();
}

template <typename T>
const QTip::Rect& BarChart<T>::rect() {
    return _rect;
}

template <typename T>
void BarChart<T>::tick(QTip::Clock& clock, double dt) {
    UIObject::tick(clock, dt);

    const double deltaTime = std::clamp(dt, 0.0, 0.1);
    const double animationSpeed = 4.0;
    const double animationFactor = std::min(
        1.0,
        animationSpeed * deltaTime
    );

    bool changed = false;

    for (size_t i = 0; i < _items.size(); ++i) {
        auto& item = _items[i];

        const double oldValue = item.animatedValue;

        item.animatedValue +=
            (item.targetValue - item.animatedValue) *
            animationFactor;

        if (std::abs(item.targetValue - item.animatedValue) < 0.001)
            item.animatedValue = item.targetValue;

        const float targetExpand = i == _hoveredOn ? 1.0f : 0.0f;

        item.expand += static_cast<float>(
            (targetExpand - item.expand) * animationFactor
        );

        if (std::abs(item.animatedValue - oldValue) > 0.001)
            changed = true;
    }

    if (changed || !_items.empty())
        calculateLayout();
}

template <typename T>
void BarChart<T>::init() {
    calculateLayout();
}

template <typename T>
void BarChart<T>::calculateLayout() {
    const float x = _rect.origin.x;
    const float y = _rect.origin.y;
    const float width = std::max(0.0f, _rect.size.x);
    const float height = std::max(0.0f, _rect.size.y);

    for (auto& item : _items)
        item.barRect = QTip::Rect::zero;

    if (_items.empty() || width <= 0.0f || height <= 0.0f)
        return;

    constexpr float outerPadding = 8.0f;
    constexpr float axisGap = 8.0f;
    constexpr float maximumLabelWidth = 150.0f;

    const float fontHeight = std::max(
        1.0f,
        font.getTextHeight("0")
    );

    const Detail::BarChartAxisScale axis = Detail::barChartAxisScale(_minimumValue, _maximumValue);

    const double axisMinimum = axis.minimum;
    const double axisMaximum = axis.maximum;
    const double range = axisMaximum - axisMinimum;

    if (_orientation == BarChartOrientation::Vertical) {
        float numberWidth = 0.0f;

        for (int tick = 0; tick <= axis.tickCount; ++tick) {
            const double value =
                axis.minimum + axis.step * tick;

            numberWidth = std::max(
                numberWidth,
                font.getTextWidth(Detail::barChartNumber(value))
            );
        }

        const float leftMargin =
            outerPadding + numberWidth + axisGap;

        const float rightMargin = outerPadding;
        const float topMargin = outerPadding;
        const float bottomMargin = fontHeight * 2.0f + axisGap;

        const float plotWidth = std::max(
            0.0f,
            width - leftMargin - rightMargin
        );

        const float plotHeight = std::max(
            0.0f,
            height - topMargin - bottomMargin
        );

        const float plotX = x + leftMargin;
        const float plotY = y + topMargin;

        if (plotWidth <= 0.0f || plotHeight <= 0.0f)
            return;

        const float cellWidth =
            plotWidth / static_cast<float>(_items.size());

        const float barGap = std::min(
            8.0f,
            cellWidth * 0.2f
        );

        const double zeroFraction =
            (0.0 - axisMinimum) / range;

        const float zeroY = plotY +
            plotHeight * static_cast<float>(1.0 - zeroFraction);

        for (size_t i = 0; i < _items.size(); ++i) {
            auto& item = _items[i];

            const double value = std::clamp(
                item.animatedValue,
                axisMinimum,
                axisMaximum
            );

            const double fraction =
                (value - axisMinimum) / range;

            const float valueY = plotY +
                plotHeight * static_cast<float>(1.0 - fraction);

            const float barTop = std::min(valueY, zeroY);
            const float barHeight = std::abs(zeroY - valueY);

            const float expandFactor =
                0.85f + 0.15f * item.expand;

            const float barWidth = std::max(
                0.0f,
                (cellWidth - barGap) * expandFactor
            );

            item.barRect = {
                plotX +
                    static_cast<float>(i) * cellWidth +
                    (cellWidth - barWidth) / 2.0f,
                barTop,
                barWidth,
                barHeight
            };
        }

        return;
    }

    float labelWidth = 0.0f;

    for (const auto& item : _items) {
        labelWidth = std::max(
            labelWidth,
            font.getTextWidth(item.label)
        );
    }

    labelWidth = std::min(labelWidth, maximumLabelWidth);

    const float leftMargin =
        outerPadding + labelWidth + axisGap;

    const float rightMargin = outerPadding;
    const float topMargin = outerPadding;
    const float bottomMargin = fontHeight * 2.0f + axisGap;

    const float plotWidth = std::max(
        0.0f,
        width - leftMargin - rightMargin
    );

    const float plotHeight = std::max(
        0.0f,
        height - topMargin - bottomMargin
    );

    const float plotX = x + leftMargin;
    const float plotY = y + topMargin;

    if (plotWidth <= 0.0f || plotHeight <= 0.0f)
        return;

    const float cellHeight =
        plotHeight / static_cast<float>(_items.size());

    const float barGap = std::min(
        8.0f,
        cellHeight * 0.2f
    );

    const double zeroFraction =
        (0.0 - axisMinimum) / range;

    const float zeroX = plotX +
        plotWidth * static_cast<float>(zeroFraction);

    for (size_t i = 0; i < _items.size(); ++i) {
        auto& item = _items[i];

        const double value = std::clamp(
            item.animatedValue,
            axisMinimum,
            axisMaximum
        );

        const double fraction =
            (value - axisMinimum) / range;

        const float valueX = plotX +
            plotWidth * static_cast<float>(fraction);

        const float barLeft = std::min(valueX, zeroX);
        const float barWidth = std::abs(zeroX - valueX);

        const float expandFactor =
            0.85f + 0.15f * item.expand;

        const float barHeight = std::max(
            0.0f,
            (cellHeight - barGap) * expandFactor
        );

        item.barRect = {
            barLeft,
            plotY +
                static_cast<float>(i) * cellHeight +
                (cellHeight - barHeight) / 2.0f,
            barWidth,
            barHeight
        };
    }
}

template <typename T>
void BarChart<T>::renderImpl(QTip::Window& window) {
    const float x = _rect.origin.x;
    const float y = _rect.origin.y;
    const float width = std::max(0.0f, _rect.size.x);
    const float height = std::max(0.0f, _rect.size.y);

    if (width <= 0.0f || height <= 0.0f)
        return;

    auto& renderer = window.getRenderer();

    const QTip::Color textColor = QTip::Color::white;
    const QTip::Color gridColor = {90, 90, 90, 100};
    const QTip::Color axisColor = {170, 170, 170, 255};

    constexpr float outerPadding = 8.0f;
    constexpr float axisGap = 8.0f;
    constexpr float maximumLabelWidth = 150.0f;

    const float fontHeight = std::max(
        1.0f,
        font.getTextHeight("0")
    );

    if (_items.empty()) {
        renderer.renderText(
            font,
            "No data",
            x + outerPadding,
            y + outerPadding,
            textColor
        );
        return;
    }

    const Detail::BarChartAxisScale axis = Detail::barChartAxisScale(_minimumValue, _maximumValue);

    const double axisMinimum = axis.minimum;
    const double axisMaximum = axis.maximum;
    const double range = axisMaximum - axisMinimum;

    float labelWidth = 0.0f;
    float numberWidth = 0.0f;

    for (const auto& item : _items) {
        labelWidth = std::max(
            labelWidth,
            font.getTextWidth(item.label)
        );
    }

    for (int tick = 0; tick <= axis.tickCount; ++tick) {
        const double value =
            axis.minimum + axis.step * tick;

        numberWidth = std::max(
            numberWidth,
            font.getTextWidth(Detail::barChartNumber(value))
        );
    }

    labelWidth = std::min(labelWidth, maximumLabelWidth);

    QTip::Rect plot{};

    if (_orientation == BarChartOrientation::Vertical) {
        const float leftMargin =
            outerPadding + numberWidth + axisGap;

        const float bottomMargin =
            fontHeight * 2.0f + axisGap;

        plot = {
            x + leftMargin,
            y + outerPadding,
            std::max(0.0f, width - leftMargin - outerPadding),
            std::max(0.0f, height - outerPadding - bottomMargin)
        };
    } else {
        const float leftMargin =
            outerPadding + labelWidth + axisGap;

        const float bottomMargin =
            fontHeight * 2.0f + axisGap;

        plot = {
            x + leftMargin,
            y + outerPadding,
            std::max(0.0f, width - leftMargin - outerPadding),
            std::max(0.0f, height - outerPadding - bottomMargin)
        };
    }

    if (plot.size.x <= 0.0f || plot.size.y <= 0.0f)
        return;

    const auto renderAxisLabel = [&](const std::string& label,
                                     float labelX,
                                     float labelY,
                                     QTip::Color color) {
        renderer.renderText(
            font,
            label.c_str(),
            labelX,
            labelY,
            color
        );
    };

    const auto renderGridLine = [&](QTip::Line line,
                                    QTip::Color color) {
        renderer.setRenderColor(color);
        renderer.renderLine(line);
    };


    // Numeric axis and grid lines.
    for (int tick = 0; tick <= axis.tickCount; ++tick) {
        const double value =
            axis.minimum + axis.step * tick;

        const std::string label =
            Detail::barChartNumber(value);

        const float fraction =
            static_cast<float>(tick) /
            static_cast<float>(axis.tickCount);

        if (_orientation == BarChartOrientation::Vertical) {
            const float tickY =
                plot.origin.y +
                plot.size.y * (1.0f - fraction);

            renderGridLine(
                {
                    {plot.origin.x, tickY},
                    {plot.origin.x + plot.size.x, tickY}
                },
                gridColor
            );

            renderAxisLabel(
                label,
                plot.origin.x - axisGap -
                    font.getTextWidth(label),
                tickY - fontHeight / 2.0f,
                textColor
            );
        } else {
            const float tickX =
                plot.origin.x + plot.size.x * fraction;

            renderGridLine(
                {
                    {tickX, plot.origin.y},
                    {tickX, plot.origin.y + plot.size.y}
                },
                gridColor
            );

            renderAxisLabel(
                label,
                tickX - font.getTextWidth(label) / 2.0f,
                plot.origin.y + plot.size.y + axisGap,
                textColor
            );
        }
    }

    // Emphasize the zero baseline.
    if (_orientation == BarChartOrientation::Vertical) {
        const double zeroFraction =
            (0.0 - axisMinimum) / range;

        const float zeroY = plot.origin.y +
            plot.size.y *
                static_cast<float>(1.0 - zeroFraction);

        renderGridLine(
            {
                {plot.origin.x, zeroY},
                {plot.origin.x + plot.size.x, zeroY}
            },
            axisColor
        );
    } else {
        const double zeroFraction =
            (0.0 - axisMinimum) / range;

        const float zeroX = plot.origin.x +
            plot.size.x * static_cast<float>(zeroFraction);

        renderGridLine(
            {
                {zeroX, plot.origin.y},
                {zeroX, plot.origin.y + plot.size.y}
            },
            axisColor
        );
    }

    // Category labels.
    if (_orientation == BarChartOrientation::Vertical) {
        const float cellWidth =
            plot.size.x / static_cast<float>(_items.size());

        for (size_t i = 0; i < _items.size(); ++i) {
            const float centerX =
                plot.origin.x +
                (static_cast<float>(i) + 0.5f) * cellWidth;

            const float availableWidth =
                std::max(0.0f, cellWidth - 2.0f);

            const std::string label = Detail::barChartTruncate(
                font,
                _items[i].label,
                availableWidth
            );

            renderAxisLabel(
                label,
                centerX - font.getTextWidth(label) / 2.0f,
                plot.origin.y + plot.size.y + axisGap,
                textColor
            );
        }
    } else {
        const float cellHeight =
            plot.size.y / static_cast<float>(_items.size());

        for (size_t i = 0; i < _items.size(); ++i) {
            const float centerY =
                plot.origin.y +
                (static_cast<float>(i) + 0.5f) * cellHeight;

            const std::string label = Detail::barChartTruncate(
                font,
                _items[i].label,
                labelWidth
            );

            renderAxisLabel(
                label,
                plot.origin.x - axisGap -
                    font.getTextWidth(label),
                centerY - fontHeight / 2.0f,
                textColor
            );
        }
    }

    // Draw bars, clipped to the plotting region.
    renderer.setClip(plot);

    for (size_t i = 0; i < _items.size(); ++i) {
        const auto& item = _items[i];

        if (item.barRect.size.x <= 0.0f ||
            item.barRect.size.y <= 0.0f) {
            continue;
        }

        QTip::Color color = item.color;

        if (i == _hoveredOn) {
            color.r = static_cast<uint8_t>(
                std::min(255, static_cast<int>(color.r) + 25)
            );
            color.g = static_cast<uint8_t>(
                std::min(255, static_cast<int>(color.g) + 25)
            );
            color.b = static_cast<uint8_t>(
                std::min(255, static_cast<int>(color.b) + 25)
            );
        }

        renderer.setRenderColor(color);
        renderer.renderRect(item.barRect);
    }

    renderer.resetClip();

    QTip::RenderTexture target(window, 100, 100);
    renderer.setTarget(target);
    if (_tooltip && _hoveredOn < _items.size()) {
        auto& item = _items[_hoveredOn];
        if (_tooltipRenderer) {
            _tooltipRenderer(
                window,
                font,
                item.data,
                item.value,
                static_cast<uint8_t>(255)
            );
        } else {
            renderer.renderText(font, item.label.c_str(), 0, 0, QTip::Color::white);
            renderer.renderText(font, std::to_string(item.value).c_str(), 0, 16, QTip::Color::white);
        }
    }
    renderer.resetTarget();
    renderer.renderTexture(target, QTip::Rect::zero, {
        .origin = mousePosition + QTip::Point{10, 10},
        .size = {100, 100}
    });
}

#endif // QTIPUIKIT_BARCHART_TPP
