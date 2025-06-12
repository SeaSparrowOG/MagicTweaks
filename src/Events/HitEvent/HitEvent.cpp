#include "HitEvent.h"

namespace Events::HitEvent
{
	RE::BSEventNotifyControl ConjurationHitManager::ProcessEvent(const RE::TESHitEvent* a_event, 
		RE::BSTEventSource<RE::TESHitEvent>*)
	{
		using Control = RE::BSEventNotifyControl;
		if (!a_event) {
			return Control::kContinue;
		}
		
		auto* eventTargetRef = a_event->target.get();
		auto* eventTarget = eventTargetRef ? eventTargetRef->As<RE::Actor>() : nullptr;
		auto* eventCauseRef = a_event->cause.get();
		auto* eventCause = eventCauseRef ? eventCauseRef->As<RE::Actor>() : nullptr;
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!eventTarget || !eventCause || !player || eventTarget->IsDead()) {
			return Control::kContinue;
		}

		if (eventCause == player) {
			auto* eventWeapon = RE::TESForm::LookupByID<RE::TESObjectWEAP>(a_event->source);
			if (!eventWeapon || !eventWeapon->IsBound()) {
				return Control::kContinue;
			}
			player->AddSkillExperience(RE::ActorValue::kConjuration, boundWeaponExp);
		}
		else {
			if (!eventCause->IsCommandedActor()) {
				return Control::kContinue;
			}

			auto commanderHandle = eventCause->GetCommandingActor();
			if (!commanderHandle || commanderHandle.get().get() != player) {
				return Control::kContinue;
			}

			// If hostile or not a conjured/reanimated minion, discard.
			// Kind of hard to read.
			if (eventCause->IsHostileToActor(player) || 
				!(eventCause->IsSummoned() || 
					eventCause->GetLifeState() != RE::ACTOR_LIFE_STATE::kReanimate)) 
			{
				return Control::kContinue;
			}
			player->AddSkillExperience(RE::ActorValue::kConjuration, minionExp);
		}
		return Control::kContinue;
	}
}
