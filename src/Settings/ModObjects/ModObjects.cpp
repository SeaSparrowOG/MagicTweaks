#include "ModObjects.hpp"

namespace ModObjects {
	
    bool GameObjectCache::Load() {
        REX::INFO("  - Looking for script {} on quest {}..."sv, ScriptName, QuestName);
		const auto quest = RE::TESForm::LookupByEditorID<RE::TESQuest>(QuestName);
		if (!quest) {
			REX::CRITICAL("  >Failed to lookup quest."sv);
			return false;
		}

		const auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!vm) {
			REX::CRITICAL("  >Failed to get VM"sv);
			return false;
		}

		const auto bindPolicy = vm->GetObjectBindPolicy();
		const auto handlePolicy = vm->GetObjectHandlePolicy();
		if (!bindPolicy || !handlePolicy) {
			REX::CRITICAL("  >Failed to get VM object policies"sv);
			return false;
		}

		const auto handle = handlePolicy->GetHandleForObject(RE::TESQuest::FORMTYPE, quest);
		RE::BSTScrapHashMap<RE::BSFixedString, RE::BSScript::Variable> properties;
		std::uint32_t nonConverted;
		bindPolicy->GetInitialPropertyValues(handle, ScriptName, properties, nonConverted);
		objects.clear();

		for (const auto& [name, var] : properties) {
			if (var.IsObjectArray()) {
				auto varArray = var.GetArray();

				RE::BSScript::Array::size_type index = 0;
				for (const auto& element : *varArray) {
					if (const auto foundForm = element.Unpack<RE::TESForm*>()) {
						auto keyName = fmt::format<std::string>("{}|{}", name.c_str(), std::to_string(index));
						objects.emplace(keyName, foundForm);
					}
					else {
						REX::CRITICAL("    >Undefined element in array propery {}."sv, name.c_str());
						return false;
					}
					++index;
				}
			}
			else if (var.IsObject()) {
				const auto value = var.Unpack<RE::TESForm*>();
				if (!value) {
					REX::CRITICAL("    >Property {} is null."sv, name.c_str());
					return false;
				}
				objects.emplace(name, value);
			}
			else {
				REX::CRITICAL("    >Undefined property {}", name.c_str());
				return false;
			}
		}

		vm->ResetAllBoundObjects(handle);
		REX::INFO("  - Found {} Mod Objects."sv, properties.size());
        return true;
    }

    RE::TESForm *GameObjectCache::Get(const std::string_view str) const noexcept {
        auto it = objects.find(str);
        return it != objects.end() ? it->second : nullptr;
    }
}