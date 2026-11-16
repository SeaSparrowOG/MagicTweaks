#pragma once

#include "json/json.h"

#include "Common/FormUtils.hpp"

#include <json/json.h>

constexpr std::string_view json_type_to_string(const Json::ValueType type) noexcept {
    switch (type) {
        case Json::ValueType::arrayValue: return "Array";
        case Json::ValueType::intValue: return "Int";
        case Json::ValueType::booleanValue: return "Bool";
        case Json::ValueType::nullValue: return "Null";
        case Json::ValueType::objectValue: return "Object";
        case Json::ValueType::realValue: return "Float";
        case Json::ValueType::stringValue: return "String";
        case Json::ValueType::uintValue: return "Unsigned Int";
        default: std::unreachable();
    }
}
template <>
struct fmt::formatter<Json::ValueType> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const Json::ValueType& val, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "{}", json_type_to_string(val));
    }
};

namespace Settings
{
    namespace JSON
    {
        struct Config
        {
            std::string name = "";
            Json::Value val{};
        };

        class JSONHolder : public REX::TSingleton<JSONHolder>
        {
        public:
            [[nodiscard]] bool Preload();

            [[nodiscard]]
            const std::map<std::string, Json::Value>& GetConfigs() const noexcept { return _configs; }

        private:
            void PreProcess(const std::filesystem::path& path, const std::string& name) noexcept;
            void ProcessObject(Json::Value& obj, std::string& path) noexcept;
            static void NormalizeKeys(Json::Value& json,
                const std::unordered_map<std::string, std::string>& mappings) noexcept;

            struct PreProcessStatus {
                [[nodiscard]]
                bool HasError() const noexcept;
                void LogError() const noexcept;

                bool opened = true;
                std::string exception{};
                std::string jsonParseErrors{};
                std::vector<std::string> multipleKeyDefinitions{};
            };
            PreProcessStatus status{};

            std::map<std::string, Json::Value> _configs{};
        };

        [[nodiscard]] inline bool Preload() {
            REX::INFO("  - Preloading JSON Settings..."sv);

            auto* holder = JSONHolder::GetSingleton();
            if (!holder) {
                REX::CRITICAL("    >Failed to retrieve internal JSON settings holder and reader."sv);
                return false;
            }
            return holder->Preload();
        }
    }
}