#pragma once

namespace Data
{
    class GameObjectCache : public REX::TSingleton<GameObjectCache>
    {
    public:
        [[nodiscard]] bool Initialize();
    };

    [[nodiscard]] inline bool Initialize() {
        REX::INFO("Initializing Game Objects..."sv);
        auto* cache = GameObjectCache::GetSingleton();
        if (!cache) {
            REX::CRITICAL("  - Failed to retrieve internal Game Object cache, aborting..."sv);
            return false;
        }
        return cache->Initialize();
    }
}