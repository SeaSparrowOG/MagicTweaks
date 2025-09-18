#include "ConditionManager.h"

#include "Data/ModObjectManager.h"

namespace ConditionManager
{
	bool Initialize() {
		SECTION_SEPARATOR;
		logger::info("Initializing the Condition Manager..."sv);
		auto* manager = ConditionManager::GetSingleton();
		if (!manager) {
			logger::info("  >Failed to get internal singleton."sv);
			return false;
		}
		return manager->Initialize();
	}

	bool ConditionManager::Initialize() {
		bool nominal = true;
		logger::info("  >Caching mod objects..."sv);

		conjuredCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_SUMMONED);
		if (!conjuredCount) {
			logger::critical("    >Failed to cache {}", Data::MAGIC_COUNT_SUMMONED);
			nominal = false;
		}
		totalCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_COMMANDED_CONJURATION);
		if (!totalCount) {
			logger::critical("    >Failed to cache {}", Data::MAGIC_COUNT_COMMANDED_CONJURATION);
			nominal = false;
		}
		fireCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_SUMMONED_FIRE);
		if (!fireCount) {
			logger::critical("    >Failed to cache {}", Data::MAGIC_COUNT_SUMMONED_FIRE);
			nominal = false;
		}
		frostCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_SUMMONED_FROST);
		if (!frostCount) {
			logger::critical("    >Failed to cache {}", Data::MAGIC_COUNT_SUMMONED_FROST);
			nominal = false;
		}
		shockCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_SUMMONED_SHOCK);
		if (!shockCount) {
			logger::critical("    >Failed to cache {}", Data::MAGIC_COUNT_SUMMONED_SHOCK);
			nominal = false;
		}
		reanimatedCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_REANIMATED);
		if (!reanimatedCount) {
			logger::critical("    >Failed to cache {}", Data::MAGIC_COUNT_REANIMATED);
			nominal = false;
		}

		if (!nominal) {
			return false;
		}

		logger::info("Initialized successfully."sv);
		return true;
	}

	bool ConditionManager::SubstituteItemCount(RE::TESBoundObject* a_obj,
		RE::TESObjectREFR* a_ref,
		int32_t& a_out)
	{
		auto* asActor = a_ref ? a_ref->As<RE::Actor>() : nullptr;
		auto* asMiddleHigh = asActor ? asActor->GetMiddleHighProcess() : nullptr;
		if (!asMiddleHigh || !a_obj) {
			return false;
		}

		auto& commandedActors = asMiddleHigh->commandedActors;
		if (commandedActors.empty()) {
			return false;
		}

		if (conjuredCount == a_obj) {
			for (const auto actorData : commandedActors) {
				auto* actor = actorData.commandedActor ? actorData.commandedActor.get().get() : nullptr;
				if (!actor) {
					continue;
				}
				a_out += actor->IsSummoned() ? 1 : 0;
			}
		}
		else if (totalCount == a_obj) {
			for (const auto actorData : commandedActors) {
				auto* actor = actorData.commandedActor ? actorData.commandedActor.get().get() : nullptr;
				if (!actor) {
					continue;
				}
				a_out += actor->IsSummoned() || actor->IsReanimated() ? 1 : 0;
			}
		}
		else if (fireCount == a_obj) {
			for (const auto actorData : commandedActors) {
				auto* actor = actorData.commandedActor ? actorData.commandedActor.get().get() : nullptr;
				auto* avOwner = actor ? actor->As<RE::ActorValueOwner>() : nullptr;
				if (!avOwner || !(actor->IsSummoned() || actor->IsReanimated())) {
					continue;
				}
				a_out += avOwner->GetActorValue(RE::ActorValue::kResistFire) >= 100.0f ? 1 : 0;
			}
		}
		else if (frostCount == a_obj) {
			for (const auto actorData : commandedActors) {
				auto* actor = actorData.commandedActor ? actorData.commandedActor.get().get() : nullptr;
				auto* avOwner = actor ? actor->As<RE::ActorValueOwner>() : nullptr;
				if (!avOwner || !(actor->IsSummoned() || actor->IsReanimated())) {
					continue;
				}
				a_out += avOwner->GetActorValue(RE::ActorValue::kResistFrost) >= 100.0f ? 1 : 0;
			}
		}
		else if (shockCount == a_obj) {
			for (const auto actorData : commandedActors) {
				auto* actor = actorData.commandedActor ? actorData.commandedActor.get().get() : nullptr;
				auto* avOwner = actor ? actor->As<RE::ActorValueOwner>() : nullptr;
				if (!avOwner || !(actor->IsSummoned() || actor->IsReanimated())) {
					continue;
				}
				a_out += avOwner->GetActorValue(RE::ActorValue::kResistShock) >= 100.0f ? 1 : 0;
			}
		}
		else if (reanimatedCount == a_obj) {
			for (const auto actorData : commandedActors) {
				auto* actor = actorData.commandedActor ? actorData.commandedActor.get().get() : nullptr;
				if (!actor) {
					continue;
				}
				a_out += actor->IsReanimated() ? 1 : 0;
			}
		}
		else {
			return false;
		}
		return true;
	}
}