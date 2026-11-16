#pragma once

namespace Hooks
{
    constexpr inline size_t alloc_size = 0u;
    constexpr inline bool plugin_hooks = true;

    [[nodiscard]] inline bool Install() {
        REX::INFO("Installing hooks..."sv);
        return true;
    }
}