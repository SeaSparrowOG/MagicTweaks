#pragma once

#include "INI/INIHolder.hpp"
#include "JSON/JSONHolder.hpp"

namespace Settings
{
    [[nodiscard]] inline bool Preload() {
        REX::INFO("Preloading settings..."sv);
        bool success = true;
        success &= INI::Preload();
        success &= JSON::Preload();
        return success;
    }

    inline constexpr const std::string_view TWEAK_EXTEND_IN_DIALOGUE = "";

    template <typename T> requires INI::is_ini_setting<T>
    [[nodiscard]]
    inline std::optional<T> GetINISetting(const std::string_view setting) noexcept {
        static const auto* ini = INI::INIHolder::GetSingleton();
        if (!ini) {
            REX::ERROR("Failed to get internal INI holder during setting request."sv);
            return std::nullopt;
        }
        return ini->GetSetting<T>(setting);
    }
}