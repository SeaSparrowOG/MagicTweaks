#include "Serialization.hpp"

namespace Serialization {
    static void ReportAndFail(const std::string operation, const std::string reason) {
        REX::FAIL(
            fmt::format("Fatal error occured while {} - {}. Check the log at Documents/<y Games/Skyrim Special Edition/SKSE/{}.log",
                operation,
                reason,
                Plugin::NAME
            )
        );
    }

    void SaveCallback(SKSE::SerializationInterface* intfc) {
        assert(intfc);
        auto* serde = SaveLoadManager::GetSingleton();
        if (!serde) {
            REX::CRITICAL("Failed to get internal Save/Load manager during saving!"sv);
            ReportAndFail("Saving", "Failed to get Save/Load Manager");
            return;
        }

        if (!serde->Save(intfc)) {
            ReportAndFail("Saving", "One or more objects reported an error");
        }
    }

    void LoadCallback(SKSE::SerializationInterface* intfc) {
        assert(intfc);
        auto* serde = SaveLoadManager::GetSingleton();
        if (!serde) {
            REX::CRITICAL("Failed to get internal Save/Load manager during loading!"sv);
            ReportAndFail("Loading", "Failed to get Save/Load Manager");
            return;
        }

        if (!serde->Load(intfc)) {
            ReportAndFail("Loading", "One or more objects reported an error");
        }
    }

    void RevertCallback(SKSE::SerializationInterface *intfc) {
        assert(intfc);
        auto* serde = SaveLoadManager::GetSingleton();
        if (!serde) {
            REX::CRITICAL("Failed to get internal Save/Load manager during reverting!"sv);
            ReportAndFail("Reverting", "Failed to get Save/Load Manager");
            return;
        }
        serde->Revert(intfc);
    }

    bool SaveLoadManager::Save(SKSE::SerializationInterface* intfc) {
        bool success = true;
        for (auto [type, obj] : _serializables) {
			success &= obj->Save(intfc);
		}
        return success;
    }

    bool SaveLoadManager::Load(SKSE::SerializationInterface* intfc)
    {
        std::uint32_t type;
		std::uint32_t version;
		std::uint32_t length;
        bool success = true;
		while (intfc->GetNextRecordInfo(type, version, length)) {
			auto it = _serializables.find(type);
            if (it == _serializables.end()) {
                // bad data likely.
                success = false;
                REX::ERROR("Caught unknown record {} while iterating over serialization interface."sv, DecodeType(type));
                continue;
            }
            success &= it->second->Load(intfc, version, length);
		}
        return success;
    }

    void SaveLoadManager::Revert(SKSE::SerializationInterface* intfc) {
        for (auto [type, obj] : _serializables) {
            (void)type;
            obj->Revert(intfc);
        }
    }

    bool SaveLoadManager::RegisterObject(Serializable* obj) {
        assert(obj);
        const auto type = obj->GetType();
        if (_serializables.contains(type)) {
            REX::CRITICAL("Tried to serialize object with ID {} that already exists in the save/load manager."sv, DecodeType(type));
            return false;
        }
        _serializables.emplace(type, obj);
        return true;
    }

    std::string SaveLoadManager::DecodeType(uint32_t type) {
        char chars[4]; 
        chars[0] = (type >> 24u) & 0xFF;
        chars[1] = (type >> 16u) & 0xFF;
        chars[2] = (type >> 8u) & 0xFF;
        chars[3] = type & 0xFF;
        return std::string(chars);
    }

    bool Serializable::Save(SKSE::SerializationInterface* intfc) {
        (void)intfc;
        return true;
    }

    bool Serializable::Load(SKSE::SerializationInterface* intfc, uint32_t version, uint32_t length) {
        (void)intfc;
        (void)version;
        (void)length;
        return true;
    }

    void Serializable::Revert(SKSE::SerializationInterface* intfc) {
        (void)intfc;
    }
}