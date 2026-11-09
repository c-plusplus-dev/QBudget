//
// Created by Ayaan on 2026-09-12.
//

#ifndef QTIPUIKIT_PANEL_H
#define QTIPUIKIT_PANEL_H
#include <Q-Tip/Types/Lazy.h>

#include "QTipUIKit/UIObject.h"

class Panel : public UIObject {
public:
    Panel(QTip::Rect rect);

    ~Panel() override;

    bool handleEvent(const QTip::Event& event) override;

    template<std::derived_from<UIObject> T, typename... Args>
    T& add(Args&&... args) {
        auto object = std::make_unique<T>(
            std::forward<Args>(args)...
        );

        T* result = object.get();
        _objects.push_back(std::move(object));
        ++_zVersion;

        result->parent = this;

        onAddObject(result);

        return *result;
    }

    template<std::derived_from<UIObject> T>
    T& add(std::unique_ptr<T> object) {
        T* result = object.get();
        _objects.push_back(std::move(object));
        ++_zVersion;

        result->parent = this;

        onAddObject(result);

        return *result;
    }

    virtual void remove(UIObject& object);

    virtual void clear();

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

    void tick(QTip::Clock& clock, double dt) override;

protected:
    std::vector<std::unique_ptr<UIObject>> _objects;

    std::unique_ptr<QTip::RenderTexture> _target;

    Lazy<std::vector<UIObject*>> _sortedObjects{[this] {
        std::vector<UIObject*> sortedObjects;
        for (const auto& object : _objects)
            sortedObjects.push_back(object.get());
        std::ranges::stable_sort(sortedObjects,
                          [](const UIObject* a, const UIObject* b) {
                              return a->zIndex() < b->zIndex();
                          }
        );
        return sortedObjects;
    }, &_zVersion};

    virtual void onAddObject(UIObject* object);

    void renderImpl(QTip::Window& window) override;
    virtual void initTarget();
};

#endif //QTIPUIKIT_PANEL_H
