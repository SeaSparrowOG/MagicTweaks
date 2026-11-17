#pragma once

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