#pragma once

namespace Serialization
{
    inline constexpr std::uint32_t SERDE_ID = 'MGTW';
    inline constexpr std::uint32_t SERDE_VERSION = 1;

    void SaveCallback(SKSE::SerializationInterface* intfc);
    void LoadCallback(SKSE::SerializationInterface* intfc);
    void RevertCallback(SKSE::SerializationInterface* intfc);

    [[nodiscard]] inline bool Initialize() {
        REX::INFO("Initializing Serialization..."sv);
        auto* serde = SKSE::GetSerializationInterface();
        if (!serde) {
            REX::CRITICAL("  - Failed to get SKSE's serialization interface!"sv);
            return false;
        }

        serde->SetUniqueID('MGTW');
        serde->SetSaveCallback(SaveCallback);
        serde->SetLoadCallback(LoadCallback);
        serde->SetRevertCallback(RevertCallback);
        return true;
    }

    class Serializable
    {
    public:
        virtual bool Save(SKSE::SerializationInterface* intfc);
        virtual bool Load(SKSE::SerializationInterface* intfc, uint32_t version, uint32_t length);
        virtual void Revert(SKSE::SerializationInterface* intfc);
        virtual ~Serializable() = default;

        uint32_t GetType() const noexcept { return type; };
        void SetType(uint32_t a_type) noexcept { type = a_type; };

    protected:
        uint32_t type = 0u;
    };

    class SaveLoadManager : 
        public REX::TSingleton<SaveLoadManager>
    {
    public:
        bool Save(SKSE::SerializationInterface* intfc);
        bool Load(SKSE::SerializationInterface* intfc);
        void Revert(SKSE::SerializationInterface* intfc);

        bool RegisterObject(Serializable* obj);

    private:
        std::unordered_map<uint32_t, Serializable*> _serializables{};

        static std::string DecodeType(uint32_t type);
    };

    template <typename T> requires std::is_convertible_v<T, Serializable>
    [[nodiscard]]
    inline bool register_serializable(T* self) noexcept {
        auto* serde = SaveLoadManager::GetSingleton();
        if (!serde) {
            REX::CRITICAL("Failed to get internal save/load manager!"sv);
            return false;
        }
        auto* serializable = static_cast<Serializable*>(self);
        return serde->RegisterObject(serializable);
    }
}