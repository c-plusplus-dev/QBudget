//
// Created by Ayaan on 2026-10-15.
//

#ifndef QBUDGET_HELPERS_H
#define QBUDGET_HELPERS_H
#include <Q-Tip/QTip.h>

inline void DrawScrollGradient(QTip::Window& window, int width, int startX, int startY, int fadeHeight, QTip::Color bg, bool top = false) {
    window->setBlendMode(QTip::RenderBlendMode::Blend);

    // Gradient (fade-in/out)
    if (!top) {
        for (int y = 0; y < fadeHeight; y++) {
            const float ratio = static_cast<float>(y) / static_cast<float>(fadeHeight);
            const auto alpha = static_cast<uint8_t>(255.0f * ratio);
            window->setRenderColor({bg.r, bg.g, bg.b, alpha});
            const auto lineY = static_cast<float>(y + startY);
            window->renderLine({static_cast<float>(startX), lineY, static_cast<float>(startX + width), lineY});
        }
    } else {
        for (int y = 0; y < fadeHeight; y++) {
            const float ratio = static_cast<float>(y) / static_cast<float>(fadeHeight);
            const auto alpha = static_cast<uint8_t>(255.0f - 255.0f * ratio);
            window->setRenderColor({bg.r, bg.g, bg.b, alpha});
            const auto lineY = static_cast<float>(y + startY);
            window->renderLine({static_cast<float>(startX), lineY, static_cast<float>(startX + width), lineY});
        }
    }

    window->resetBlendMode();
}

#endif //QBUDGET_HELPERS_H
