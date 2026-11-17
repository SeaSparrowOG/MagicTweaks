#pragma once

namespace Hooks
{
    namespace Fixes
    {
        inline constexpr size_t ALLOC_SIZE = 0u;

        [[nodiscard]] inline bool Install() {
            REX::INFO("  - Installing fix-related hooks..."sv);
            return true;
        }
    }
}