#pragma once

namespace Hooks
{
    namespace Gameplay
    {
        inline constexpr size_t ALLOC_SIZE = 0u;

        [[nodiscard]] inline bool Install() {
            REX::INFO("  - Installing gameplay-related hooks..."sv);
            return true;
        }
    }
}