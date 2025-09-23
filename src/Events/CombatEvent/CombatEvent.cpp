#include "CombatEvent.h"

#include "BoundEffectManager/BoundEffectManager.h"
#include "Settings/INI/INISettings.h"

namespace Events::CombatEvent
{
	bool RegisterCombatEvent() {
		logger::info("  Registering Combat State Listener..."sv);
		auto install = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_EARN_EXP).value_or(false);
		if (!install) {
			logger::info("    >User chose not to install the patch."sv);
			return true;
		}

		auto* singleton = CombatChangeManager::GetSingleton();
		return singleton && singleton->RegisterCombatListener();
	}

	bool CombatChangeManager::RegisterCombatListener() {
		auto source = RE::ScriptEventSourceHolder::GetSingleton();
		if (!source) {
			logger::error("    >Failed to get event source holder, aborting load..."sv);
			return false;
		}
		source->AddEventSink(this);
		return true;
	}

	RE::BSEventNotifyControl CombatChangeManager::ProcessEvent(const RE::TESCombatEvent* a_event,
		RE::BSTEventSource<RE::TESCombatEvent>*)
	{
		using Control = RE::BSEventNotifyControl;
		if (!a_event || 
			!a_event->targetActor.get() || 
			!a_event->targetActor->IsPlayerRef()) 
		{
			return Control::kContinue;
		}

		if (a_event->newState == RE::ACTOR_COMBAT_STATE::kNone) {
			auto* player = RE::PlayerCharacter::GetSingleton();
			outOfCombat = !player || player->IsInCombat();
			return Control::kContinue;
		}
		if (!outOfCombat) {
			return Control::kContinue;
		}

		auto* manager = BoundEffectManager::BoundEffectManager::GetSingleton();
		if (manager) {
			manager->GainExperience();
		}
		return Control::kContinue;
	}
}