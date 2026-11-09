//
// Created by Ayaan on 2026-10-02.
//

#ifndef QTIPUIKIT_DROPDOWN_TPP
#define QTIPUIKIT_DROPDOWN_TPP

#include <magic_enum/magic_enum.hpp>

#include "QTipUIKit/Containers/List.h"

template <typename T>
Dropdown<T>::Dropdown(QTip::Rect rect) requires Enum<T> : _itemsView({}) {
    _rect = rect;
    _preferredRect = rect;

    constexpr auto values = magic_enum::enum_values<T>();
    _items.reserve(values.size());
    for (const T value : values) {
        _items.emplace_back(
            value,
            std::string(magic_enum::enum_name(value))
        );
    }

    init();
}

template <typename T>
Dropdown<T>::Dropdown(QTip::Rect rect, std::vector<T> items, Formatter formatter) : _itemsView({}) {
    _rect = rect;
    _preferredRect = rect;

    _items.reserve(items.size());
    for (auto& item : items) {
        auto name = formatter(item);
        _items.emplace_back(std::move(item), std::move(name));
    }

    init();
}

template <typename T>
Dropdown<T>::Dropdown(QTip::Rect rect, std::vector<T> items) requires StringConvertible<T> : _itemsView({}) {
    _rect = rect;
    _preferredRect = rect;

    _items.reserve(items.size());
    for (auto& item : items) {
        auto name = std::string(item);
        _items.emplace_back(std::move(item), std::move(name));
    }

    init();
}

template <typename T>
Dropdown<T>::Dropdown(QTip::Rect rect, std::vector<DropdownItem<T>> items) : _items(std::move(items)), _itemsView({}) {
    _rect = rect;
    _preferredRect = rect;

    init();
}

template <typename T>
void Dropdown<T>::setCallback(std::function<void(const T&, size_t)> callback) {
    _onChange = callback;
}

template <typename T>
size_t Dropdown<T>::selectedIndex() const {
    return _selectedIndex;
}

template <typename T>
const T* Dropdown<T>::selectedItem() const {
    return _selectedItem;
}

template <typename T>
void Dropdown<T>::renderImpl(QTip::Window& window) {
    window->setRenderColor({50, 50, 50});

    window->renderRoundedRect(_rect, 10);

    std::string text = "Select...";
    if (_selectedIndex != -1) {
        auto& item = _items[_selectedIndex];
        text = item.name;
    }
    window->renderTextCentered(_buttonStyle.font, text.c_str(), _rect.center().x, _rect.center().y, _buttonStyle.fontColor);

    if (_active) {
        QTip::Rect rect = _rect;
        rect.origin.y += rect.size.y;
        rect.size.y += 5.0f;
        rect.size.y *= _shownItems;

        window->renderRoundedRect(rect, 10);

        _itemsView.render(window);
    }
}

template <typename T>
bool Dropdown<T>::handleEvent(const QTip::Event& event) {
    if (_active) {
        if (!_itemsView.handleEvent(event))
            return false;
    }

    switch (event.type) {
        case QTip::EventType::MouseButtonPressed: {
            if (event.mouseButton.button != QTip::Left)
                break;

            if (_rect.isPointInside({
                event.mouseButton.position.x,
                event.mouseButton.position.y
            })) {
                _active = !_active;
                return false;
            }
            _active = false;
            break;
        }
        default:
            break;
    }
    QTip::Rect rect = _rect;
    if (_active)
        rect.size.y = (_rect.size.y + 5) * _shownItems - 10;
    if (_active) {
        auto position = Detail::eventPosition(event);
        if (position != QTip::Point{-1, -1} && rect.isPointInside(position))
            return false;
    }
    return true;
}

template <typename T>
void Dropdown<T>::resize(QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;

    positionView();
}

template <typename T>
void Dropdown<T>::reposition(QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;

    positionView();
}

template <typename T>
void Dropdown<T>::setRect(QTip::Rect rect) {
    _rect = rect;

    positionView();
}

template <typename T>
const QTip::Rect& Dropdown<T>::rect() {
    return _rect;
}

template <typename T>
QTip::Point Dropdown<T>::minimumSize() const {
    return {}; // not now
}

template <typename T>
QTip::Point Dropdown<T>::preferredSize() const {
    return _preferredRect.size;
}

template <typename T>
void Dropdown<T>::printItems() const {
    for (const auto& item : _items) {
        std::cout << item << std::endl;
    }
}

template <typename T>
void Dropdown<T>::init() {
    _shownItems = std::min(std::size_t{4}, _items.size());
    positionView();

    const float itemHeight = _rect.size.y;
    constexpr float spacing = 5.0f;

    _itemsList = &_itemsView.add<List>(
        QTip::Rect{
            0,
            0,
            _rect.size.x - 10,
            itemHeight * _items.size() + spacing * (_items.size() - 1)
        },
        List::Direction::Down,
        spacing
    );

    for (const auto& item : _items) {
        auto& button = _itemsList->add<Button>(
            QTip::Rect{0, 0, _rect.size.x - 10, itemHeight},
            item.name,
            _buttonStyle
        );

        button.setOnClick([&, this] {
            auto it = std::ranges::find_if(
                _items,
                [&] (const auto& item_) {
                    return item_.name == button.text();
                }
            );
            if (it != _items.end()) {
                _selectedItem = &it->value;
                _selectedIndex = std::distance(_items.begin(), it);
                _active = false;
                if (_onChange)
                    _onChange(*_selectedItem, _selectedIndex);
            }
        });
    }

    _itemsView.setContentSize({_rect.size.x - 10, itemHeight * _items.size() + spacing * (_items.size() - 1)});
}

template <typename T>
void Dropdown<T>::positionView() {
    _itemsView.resize({_rect.size.x - 10, (_rect.size.y + 5) * _shownItems - 10});
    _itemsView.reposition({_rect.origin.x + 5, _rect.origin.y + 5 + _rect.size.y});
}

#endif //QTIPUIKIT_DROPDOWN_TPP
