//
// Created by Ayaan on 2026-09-11.
//

#ifndef QTIPUIKIT_UIKITMOD_H
#define QTIPUIKIT_UIKITMOD_H
#include <Q-Tip/Mods/Mod.h>
#include <Q-Tip/Clock.h>

#include <vector>

#include "UIContext.h"

class UIObject;
class Dialog;

class UIKitMod : public QTip::Mod {
public:
    static constexpr std::string_view ID = "propythoncoderaya/uikit";
    static constexpr std::string_view NAME = "UIKit";

    UIKitMod();

    ~UIKitMod() override;

    [[nodiscard]] std::string_view id() const override;

    [[nodiscard]] std::string_view name() const override;

    void init() override;

    void shutdown() override;

    void handleEvent(const QTip::Event& event) override;

    void beforePresent(QTip::Window& window) override;

    void windowCreated(QTip::Window& window) override;
    void windowDestroyed(QTip::Window& window) override;

    static UIKitMod* instance();

private:
    std::vector<UIContext*> _contexts;

    UIContext* context(QTip::Window& window);

    Dialog& addDialog(QTip::Window& window, std::unique_ptr<Dialog> dialog);
    void removeDialog(QTip::Window& window, Dialog* dialog);

    QTip::Clock _clock;

    void setAddingChildren(QTip::Window* window, bool adding);

    friend class Dialog;
    friend class UIContext;
    friend class Panel;
};

#endif //QTIPUIKIT_UIKITMOD_H
