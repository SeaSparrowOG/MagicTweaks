#pragma once

namespace Settings
{
    namespace ModObjects
    {
        class GameObjectCache : public REX::TSingleton<GameObjectCache>
        {
        public:
            [[nodiscard]] bool Load();

            [[nodiscard]] RE::TESForm* Get(const std::string_view str) const noexcept;

        private:
            const std::string QuestName = fmt::format("{}_ModObjectsQuest"sv, Plugin::NAME);
            const std::string ScriptName = fmt::format("{}_ModObjectsScript"sv, Plugin::NAME);

            util::istring_map<RE::TESForm*> objects;
        };

        [[nodiscard]] inline bool Load() {
            REX::INFO("Initializing Game Objects..."sv);
            auto* cache = GameObjectCache::GetSingleton();
            if (!cache) {
                REX::CRITICAL("  - Failed to retrieve internal Game Object cache, aborting..."sv);
                return false;
            }
            return cache->Load();
        }

        inline constexpr const std::string_view SPELL_BINDING_BINDS_HEALTH =  "MagicBindHealth";
        inline constexpr const std::string_view SPELL_BINDING_BINDS_STAMINA = "MagicBindStamina";
        inline constexpr const std::string_view SPELL_BINDING_BINDS_MAGICKA = "MagicBindMagicka";

        inline constexpr const std::string_view PROXY_COUNT_COMMANDED_CONJURATION = "MGT_PRXY_CommandedConjurationCount";
        inline constexpr const std::string_view PROXY_COUNT_SUMMONED = "MGT_PRXY_CommandedConjurationSummon";
        inline constexpr const std::string_view PROXY_COUNT_REANIMATED = "MGT_PRXY_CommandedConjurationReanimated";
        inline constexpr const std::string_view PROXY_COUNT_SUMMONED_FIRE = "MGT_PRXY_CommandedConjurationFire";
        inline constexpr const std::string_view PROXY_COUNT_SUMMONED_FROST = "MGT_PRXY_CommandedConjurationFrost";
        inline constexpr const std::string_view PROXY_COUNT_SUMMONED_SHOCK = "MGT_PRXY_CommandedConjurationShock";

        inline constexpr std::array<std::string_view, 9u> GAME_OBJECTS = {
            SPELL_BINDING_BINDS_HEALTH,
            SPELL_BINDING_BINDS_STAMINA,
            SPELL_BINDING_BINDS_MAGICKA,

            PROXY_COUNT_COMMANDED_CONJURATION,
            PROXY_COUNT_SUMMONED,
            PROXY_COUNT_REANIMATED,
            PROXY_COUNT_SUMMONED_FIRE,
            PROXY_COUNT_SUMMONED_FROST,
            PROXY_COUNT_SUMMONED_SHOCK
        };

        template <typename T>
        [[nodiscard]]
        T* GetObject(const std::string_view edid) noexcept {
            static const auto* cache = GameObjectCache::GetSingleton();
            if (!cache) {
                REX::ERROR("Failed to get internal Game Object Cache during query."sv);
                return nullptr;
            }
            
            auto* form = cache->Get(edid);
            return form ? form->As<T>() : nullptr;
        }
    }
}