#pragma once

namespace Data
{
	class ModObjectManager : public Utilities::Singleton::EventClass<ModObjectManager, RE::TESQuestInitEvent>
	{
	public:
		const std::string QuestName = fmt::format("{}_ModObjectsQuest"sv, Plugin::NAME);
		const std::string ScriptName = fmt::format("{}_ModObjectsScript"sv, Plugin::NAME);

		RE::BSEventNotifyControl ProcessEvent(
			const RE::TESQuestInitEvent* a_event,
			RE::BSTEventSource<RE::TESQuestInitEvent>* a_eventSource) override;

		bool Reload();

		[[nodiscard]] RE::TESForm* Get(std::string_view a_key) const;

	private:
		bool Initialize(RE::TESQuest* a_quest);

		util::istring_map<RE::TESForm*> objects;
	};

	template <typename T>
	[[nodiscard]] inline T* ModObject(std::string_view a_key)
	{
		if (const auto object = ModObjectManager::Instance().Get(a_key))
			return object->As<T>();
		return nullptr;
	}
}