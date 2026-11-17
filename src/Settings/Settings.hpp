#pragma once

#include "INI/INIHolder.hpp"
#include "JSON/JSONHolder.hpp"
#include "ModObjects/ModObjects.hpp"

namespace Settings
{
    [[nodiscard]] inline bool Preload() {
        REX::INFO("Preloading settings..."sv);
        bool success = true;
        success &= INI::Preload();
        success &= JSON::Preload();
        return success;
    }

    [[nodiscard]] inline bool PostLoad() {
        return ModObjects::Load();
    }  
}