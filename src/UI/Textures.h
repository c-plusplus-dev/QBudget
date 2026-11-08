#ifndef QBUDGET_UI_TEXTURES_H
#define QBUDGET_UI_TEXTURES_H

#include <memory>
#include <Q-Tip/Graphics/Texture.h>

struct UITextures {
    std::unique_ptr<QTip::Texture> blockMove;
    std::unique_ptr<QTip::Texture> itemMove;
    std::unique_ptr<QTip::Texture> editButton;
};

#endif // QBUDGET_UI_TEXTURES_H
