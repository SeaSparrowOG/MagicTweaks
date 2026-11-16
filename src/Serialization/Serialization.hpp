#pragma once

namespace Serialization
{
    [[nodiscard]] inline bool Initialize() {
        REX::INFO("Initializing Serialization..."sv);
        return true;
    }
}