//
// Created by Ayaan on 2026-10-05.
//

#ifndef QTIPUIKIT_PIECHART_TPP
#define QTIPUIKIT_PIECHART_TPP
#include "QTipUIKit/Internal/Helpers.h"
#include <Q-Tip/Graphics/RenderTexture.h>

// in each constructor init the _items list with item, value and label and calculate total.
// then call init to do repetitive code like setting percent.

// the constructors should only populate _items and _total, that's it.

template <typename T>
PieChart<T>::PieChart(QTip::Rect rect, std::vector<T> items, bool tooltip) requires PieChartConvertible<T> {
    setData(items);

    _tooltip = tooltip;

    _rect = rect;
    _preferredRect = rect;
    init(); // call init to do repetitive code
}

template <typename T>
PieChart<T>::PieChart(QTip::Rect rect, std::vector<T> items, Formatters formatters, bool tooltip) {
    setData(items, formatters);

    _tooltip = tooltip;

    _rect = rect;
    _preferredRect = rect;
    init();
}

template <typename T>
PieChart<T>::PieChart(QTip::Rect rect, std::vector<PieChartItem<T>> items, bool tooltip) {
    setData(items);

    _tooltip = tooltip;

    _rect = rect;
    _preferredRect = rect;
    init();
}

template <typename T>
void PieChart<T>::setData(std::vector<T> items) requires PieChartConvertible<T> {
    _items.clear();
    _items.reserve(items.size());
    _total = 0;
    for (auto& item : items) {
        auto value = item.value(); // get the value
        auto label = item.label(); // and label
        QTip::Color color;
        if constexpr (isColorConvertible) {
            color = item.color();
        }
        _items.emplace_back(std::move(item), value, std::move(label), std::move(color)); // move it into the list
        if constexpr (isColorConvertible) {
            _items.back().colorValid = true;
        }
        _total += value; // and add value to total
    }
    init();
}

template <typename T>
void PieChart<T>::setData(std::vector<T> items, Formatters formatters) {
    _items.clear();
    _items.reserve(items.size()); // same, reserve
    _total = 0;
    for (auto& item : items) {
        auto v = formatters.value(item); // get the value
        auto l = formatters.label(item); // and label
        QTip::Color color;
        if (formatters.color)
            color = formatters.color(item);
        _items.emplace_back(std::move(item), v, std::move(l), std::move(color));
        if (formatters.color)
            _items.back().colorValid = true;
        _total += v;
    }
    init();
}

template <typename T>
void PieChart<T>::setData(std::vector<PieChartItem<T>> items) {
    // here we can just init it by moving over because the user has given us label and value themselves
    _items.clear();
    _items.reserve(items.size());
    _total = 0;
    for (auto& item : items) {
        // idk if its okay to do that
        _items.emplace_back(std::move(item.data), item.value, std::move(item.label), std::move(item.color));
        _items.back().colorValid = true;
        _total += _items.back().value;
    }
    init();
}

template <typename T>
void PieChart<T>::setTooltipRenderer(TooltipRenderer renderer) {
    _tooltipRenderer = renderer;
}

template <typename T>
size_t PieChart<T>::hoveredIndex() const {
    return _hoveredOn;
}

template <typename T>
T* PieChart<T>::hoveredItem() const {
    if (_hoveredOn == -1) {
        return nullptr;
    }
    return &_items[_hoveredOn].data;
}

template <typename T>
void PieChart<T>::renderImpl(QTip::Window& window) {
    if (_tooltip && !_tooltipTexture) {
        _tooltipTexture = std::make_unique<QTip::RenderTexture>(
            window,
            200, 200
        );
    }

    double startAngle = -M_PI_2;

    auto [width, height] = _rect.size;
    if (_tooltip) {
        width -= 20;
        height -= 20;
    }
    float radius = std::min(width, height) / 2.0f;
    auto [cx, cy] = _rect.center();
    if (_tooltip) {
        cx -= 10;
    }

    auto mouseX = mousePosition.x + 10;
    auto mouseY = mousePosition.y + 10;
    bool tooltipDrawn = false;
    for (int i = 0; i < _items.size(); i++) {
        auto& item = _items[i];
        startAngle = Detail::drawPieSlice(
            window,
            &item.vb,
            cx, cy,
            radius + item.expand / 2.0f,
            item.animatedPercent,
            item.color,
            startAngle
        );
    }
    if (_hoveredOn != -1) {
        auto& item = _items[_hoveredOn];
        auto alpha = static_cast<uint8_t>(255.0f * (static_cast<float>(item.expand) / 20.0f));
        if (_tooltipRenderer && !tooltipDrawn) {
            tooltipDrawn = true;
            window->setTarget(*_tooltipTexture);
            window->setRenderColor(QTip::Color::clear);
            window->renderRect({0, 0, 200, 200});
            _tooltipRenderer(
                window,
                font,
                item.data,
                item.value,
                item.percent,
                alpha
            );
            window->resetTarget();
            window->renderTexture(*_tooltipTexture, QTip::Rect::zero, QTip::Rect{mouseX, mouseY, 200, 200});
        }
        else if (!tooltipDrawn) {
            tooltipDrawn = true;
            QTip::Color color = {255, 255, 255, alpha};
            window->renderText(font, item.label.c_str(), mouseX, mouseY, color);
            window->renderText(font, Detail::fmt("%.1f%%", item.percent * 100).c_str(), mouseX, mouseY + 16, color);
        }
    }
}

template <typename T>
bool PieChart<T>::handleEvent(const QTip::Event& event) {
    switch (event.type) {
    case QTip::EventType::MouseMoved: {
        mousePosition = {event.mouseMove.position.x, event.mouseMove.position.y};
        if (_tooltip) {
            _hoveredOn = -1;
            for (int i = 0; i < _items.size(); i++) {
                auto& item = _items[i];
                if (Detail::pointInVB(event.mouseMove.position.x, event.mouseMove.position.y, item.vb)) {
                    _hoveredOn = i;
                }
            }
        }
    }
    default:
        break;
    }
    return true;
}

template <typename T>
QTip::Point PieChart<T>::minimumSize() const {
    return {100, 100}; // at least hundred
}

template <typename T>
QTip::Point PieChart<T>::preferredSize() const {
    return _preferredRect.size;
}

template <typename T>
void PieChart<T>::resize(QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;
}

template <typename T>
void PieChart<T>::reposition(QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
}

template <typename T>
void PieChart<T>::setRect(QTip::Rect rect) {
    _rect = rect;
}

template <typename T>
const QTip::Rect& PieChart<T>::rect() {
    return _rect;
}

template <typename T>
void PieChart<T>::tick(QTip::Clock&, double dt) {
    for (int i = 0; i < _items.size(); i++) {
        auto& item = _items[i];
        if (_hoveredOn == i)
            item.expand = std::min(item.expand + 60.0 * dt, 20.0);
        else
            item.expand = std::max(item.expand - 60.0 * dt, 0.0);

        if (item.animatedPercent != item.percent) {
            double speed = (item.percent - item.animatedPercent) / 0.75;
            item.animatedPercent += speed * dt;
            if (item.percent - item.animatedPercent < 0.0001)
                item.animatedPercent = item.percent;
        }
    }
}

template <typename T>
void PieChart<T>::init() {
    double position = 0;
    for (auto& item : _items) {
        item.percent = _total == 0.0 ? 0.0 : item.value / _total;; // calculate percent

        double middle = position + item.percent / 2.0f;
        if (!item.colorValid) {
            item.color = Detail::pieChartColor(middle);
        }
        position += item.percent;
    }
}

#endif //QTIPUIKIT_PIECHART_TPP
