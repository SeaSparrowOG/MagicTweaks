#include "ModObjectManager.h"

namespace Data
{
	RE::BSEventNotifyControl ModObjectManager::ProcessEvent(
		const RE::TESQuestInitEvent* a_event,
		[[maybe_unused]] RE::BSTEventSource<RE::TESQuestInitEvent>* a_eventSource)
	{
		using enum RE::BSEventNotifyControl;
		return kContinue;
	}

	bool ModObjectManager::Reload()
	{
		logger::info("==========================================================");
		logger::info("Preloading forms from {} attached on {}...", ScriptName, QuestName);
		const auto quest = RE::TESForm::LookupByEditorID<RE::TESQuest>(QuestName);
		if (!quest) {
			logger::warn("  >Failed to lookup quest."sv);
			return false;
		}
		return Initialize(quest);
	}

	bool ModObjectManager::Initialize(RE::TESQuest* a_quest)
	{
		const auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!vm) {
			logger::error("  >Failed to get VM");
			return false;
		}

		const auto bindPolicy = vm->GetObjectBindPolicy();
		const auto handlePolicy = vm->GetObjectHandlePolicy();

		if (!bindPolicy || !handlePolicy) {
			logger::error("  >Failed to get VM object policies");
			return false;
		}

		const auto handle = handlePolicy->GetHandleForObject(RE::TESQuest::FORMTYPE, a_quest);
		RE::BSTScrapHashMap<RE::BSFixedString, RE::BSScript::Variable> properties;
		std::uint32_t nonConverted;
		bindPolicy->GetInitialPropertyValues(handle, ScriptName, properties, nonConverted);

		objects.clear();

		for (const auto& [name, var] : properties) {
			if (const auto value = var.Unpack<RE::TESForm*>()) {
				objects.emplace(name, value);
			}
		}

		vm->ResetAllBoundObjects(handle);
		logger::info("  >Found {} Mod Objects."sv, properties.size());
		return true;
	}

	RE::TESForm* ModObjectManager::Get(std::string_view a_key) const
	{
		if (const auto it = objects.find(a_key); it != objects.end())
			return it->second;
		return nullptr;
	}
}