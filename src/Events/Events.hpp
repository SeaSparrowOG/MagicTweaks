#pragma once

namespace Events
{
    [[nodiscard]] inline bool Initialize() {
        REX::INFO("Initializing Event listeners..."sv);
        return true;
    }
}