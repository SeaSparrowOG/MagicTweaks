#pragma once

#include "Fixes/Fixes.hpp"
#include "Gameplay/Gameplay.hpp"
#include "Tweaks/Tweaks.hpp"

namespace Hooks
{
    constexpr inline size_t ALLOC_SIZE = Fixes::ALLOC_SIZE + Gameplay::ALLOC_SIZE + Tweaks::ALLOC_SIZE;
    constexpr inline bool plugin_hooks = true;

    [[nodiscard]] inline bool Install() {
        REX::INFO("Installing hooks..."sv);
        bool success = true;
        success &= Fixes::Install();
        success &= Gameplay::Install();
        success &= Tweaks::Install();
        return success;
    }
}