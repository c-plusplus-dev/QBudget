//
// Created by Ayaan on 2026-10-09.
//

#ifndef QTIPUIKIT_UICONTEXT_H
#define QTIPUIKIT_UICONTEXT_H
#include <vector>
#include <Q-Tip/Clock.h>

#include "Dialogs/Dialog.h"

class UIObject;

class UIContext {
public:
    UIContext();

    ~UIContext();

private:
    std::vector<UIObject*> _objects;
    std::vector<std::unique_ptr<Dialog>> _dialogs;
    std::vector<Dialog*> _removalPendingDialogs;
    std::vector<QTip::Event> _eventPool;

    void add(UIObject* object);
    void remove(UIObject* object);

    void beforePresent(QTip::Window& window, QTip::Clock& clock, double dt);

    void handleEvent(const QTip::Event& event);

    void removeDialog(Dialog* dialog);

    bool _addingChildren = false;

    bool _firstEventPool = true;

    QTip::Window* _window = nullptr;

    friend class UIObject;
    friend class Panel;
    friend class UIKitMod;
};


#endif //QTIPUIKIT_UICONTEXT_H
