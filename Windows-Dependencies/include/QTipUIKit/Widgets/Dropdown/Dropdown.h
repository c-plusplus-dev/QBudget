//
// Created by Ayaan on 2026-10-02.
//

#ifndef QTIPUIKIT_DROPDOWN_H
#define QTIPUIKIT_DROPDOWN_H
#include "QTipUIKit/UIObject.h"
#include <concepts>
#include <string>
#include <functional>
#include <vector>

#include "QTipUIKit/Containers/List.h"

template <typename T>
concept StringConvertible = requires(const T& value) {
    { std::string(value) } -> std::same_as<std::string>;
};

template <typename T>
concept Enum = std::is_enum_v<T>;

template <typename T>
struct DropdownItem {
    T value;
    std::string name;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const DropdownItem<T>& item) {
    return os << item.name;
}

template <typename T>
class Dropdown : public UIObject {
public:
    using Formatter = std::function<std::string(const T&)>;

    explicit Dropdown(QTip::Rect rect) requires Enum<T>; // auto convert enum to list
    Dropdown(QTip::Rect rect, std::vector<T> items, Formatter formatter);
    Dropdown(QTip::Rect rect, std::vector<T> items) requires StringConvertible<T>;
    Dropdown(QTip::Rect rect, std::vector<DropdownItem<T>> items);

    void setCallback(std::function<void(const T&, size_t)> callback);

    size_t selectedIndex() const;
    const T* selectedItem() const;

    bool handleEvent(const QTip::Event& event) override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    const QTip::Rect& rect() override;

    QTip::Point minimumSize() const override;
    QTip::Point preferredSize() const override;

    void printItems() const;

private:
    void init();

    void positionView();

    std::vector<DropdownItem<T>> _items;

    ScrollView _itemsView;
    List* _itemsList = nullptr;

    ButtonStyle _buttonStyle{
        .color = {50, 50, 50},
        .hoverColor = {70, 70, 70},
        .pressedColor = {30, 30, 30},
        .disabledColor = {20, 20, 20},
        .fontColor = QTip::Color::white
    };

    size_t _shownItems = 0;

    size_t _selectedIndex = -1;
    T* _selectedItem = nullptr;

    std::function<void(const T&, size_t)> _onChange;

    bool _active = false;

protected:
    void renderImpl(QTip::Window& window) override;
};

#include "Dropdown.tpp"

#endif //QTIPUIKIT_DROPDOWN_H
