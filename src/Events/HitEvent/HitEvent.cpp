#include "HitEvent.h"

#include "Settings/INI/INISettings.h"

namespace Events::HitEvent
{
	bool RegisterHitEvent() {
		logger::info("  Registering Conjuration Hit Experience Listener..."sv);
		auto installRaw = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_EARN_EXP);
		if (!installRaw || !installRaw.value()) {
			logger::info("    >User chose not to install the patch."sv);
			return true;
		}

		auto* singleton = ConjurationHitManager::GetSingleton();
		return singleton && singleton->Register();
	}

	bool ConjurationHitManager::Register() {
		auto source = RE::ScriptEventSourceHolder::GetSingleton();
		if (!source) {
			logger::error("    >Failed to get event source holder, aborting load..."sv);
			return false;
		}
		source->AddEventSink(this);
		return true;
	}

	RE::BSEventNotifyControl ConjurationHitManager::ProcessEvent(
		const RE::TESHitEvent* a_event, 
		RE::BSTEventSource<RE::TESHitEvent>*)
	{
		using Control = RE::BSEventNotifyControl;
		if (!a_event) {
			return Control::kContinue;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return Control::kContinue;
		}

		auto* eventTargetRef = a_event->target.get();
		auto* eventTargetActor = eventTargetRef ? eventTargetRef->As<RE::Actor>() : nullptr;
		auto* eventSourceRef = a_event->cause.get();
		auto* eventSourceActor = eventSourceRef ? eventSourceRef->As<RE::Actor>() : nullptr;
		if (!eventTargetActor || !eventSourceActor) {
			return Control::kContinue;
		}

		auto* eventWeapon = RE::TESForm::LookupByID<RE::TESObjectWEAP>(a_event->source);
		bool bound = eventWeapon && eventWeapon->IsBound();

		if (!eventSourceActor->IsCommandedActor() && bound) {
			if (!eventSourceActor->IsPlayerRef()) {
				return Control::kContinue;
			}
			player->AddSkillExperience(RE::ActorValue::kConjuration, minionExp);
		}
		else if (eventSourceActor->IsCommandedActor()) {
			auto commanderHandle = eventSourceActor->GetCommandingActor();
			if (!commanderHandle || !commanderHandle->IsPlayerRef()) {
				return Control::kContinue;
			}

			// If hostile or not a conjured/reanimated minion, discard.
			// Kind of hard to read.
			if (eventSourceActor->IsHostileToActor(player) ||
				!(eventSourceActor->IsSummoned() || eventSourceActor->IsReanimated())
				)
			{
				return Control::kContinue;
			}
			player->AddSkillExperience(RE::ActorValue::kConjuration, minionExp);
		}

		return Control::kContinue;
	}
}