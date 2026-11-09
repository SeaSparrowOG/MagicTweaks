#include "ConditionManager.h"

#include "Data/ModObjectManager.h"

namespace ConditionManager
{
	bool Initialize() {
		SECTION_SEPARATOR;
		REX::INFO("Initializing the Condition Manager..."sv);
		auto* manager = ConditionManager::GetSingleton();
		if (!manager) {
			REX::INFO("  >Failed to get internal singleton."sv);
			return false;
		}
		return manager->Initialize();
	}

	bool ConditionManager::Initialize() {
		bool nominal = true;
		REX::INFO("  >Caching mod objects..."sv);

		conjuredCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_SUMMONED);
		if (!conjuredCount) {
			REX::CRITICAL("    >Failed to cache {}", Data::MAGIC_COUNT_SUMMONED);
			nominal = false;
		}
		totalCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_COMMANDED_CONJURATION);
		if (!totalCount) {
			REX::CRITICAL("    >Failed to cache {}", Data::MAGIC_COUNT_COMMANDED_CONJURATION);
			nominal = false;
		}
		fireCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_SUMMONED_FIRE);
		if (!fireCount) {
			REX::CRITICAL("    >Failed to cache {}", Data::MAGIC_COUNT_SUMMONED_FIRE);
			nominal = false;
		}
		frostCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_SUMMONED_FROST);
		if (!frostCount) {
			REX::CRITICAL("    >Failed to cache {}", Data::MAGIC_COUNT_SUMMONED_FROST);
			nominal = false;
		}
		shockCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_SUMMONED_SHOCK);
		if (!shockCount) {
			REX::CRITICAL("    >Failed to cache {}", Data::MAGIC_COUNT_SUMMONED_SHOCK);
			nominal = false;
		}
		reanimatedCount = Data::ModObject<RE::TESBoundObject>(Data::MAGIC_COUNT_REANIMATED);
		if (!reanimatedCount) {
			REX::CRITICAL("    >Failed to cache {}", Data::MAGIC_COUNT_REANIMATED);
			nominal = false;
		}

		if (!nominal) {
			return false;
		}

		REX::INFO("Initialized successfully."sv);
		return true;
	}

	template <class Predicate>
	[[nodiscard]]
	static int32_t get_matching_minions(
		const RE::BSTArray<RE::CommandedActorData>& minions,
		Predicate&& predicate)
	{
		return static_cast<int32_t>(std::ranges::count_if(minions, std::forward<Predicate>(predicate)));
	}

	std::optional<int32_t> ConditionManager::SubstituteItemCount(
		RE::TESBoundObject* a_obj,
		RE::TESObjectREFR* a_ref)
	{
		auto* actor = a_ref ? a_ref->As<RE::Actor>() : nullptr;
		auto* process = actor ? actor->GetMiddleHighProcess() : nullptr;
		if (!process || !a_obj) {
			return std::nullopt;
		}

		const auto& minions = process->commandedActors;

		const auto get_actor = [](const auto& a_data) {
			return a_data.commandedActor ? a_data.commandedActor.get().get() : nullptr;
		};

		const auto is_summoned = [&](const auto& a_data) {
			const auto* actor = get_actor(a_data);
			return actor && actor->IsSummoned();
		};

		const auto is_reanimated = [&](const auto& a_data) {
			const auto* actor = get_actor(a_data);
			return actor && actor->IsReanimated();
		};
		
		const auto has_resistance = [&](const auto& a_data, RE::ActorValue a_value) {
			const auto* actor = get_actor(a_data);
			if (!actor || !actor->IsSummoned() || actor->IsReanimated()) {
				return false;
			}

			const auto* avOwner = actor->As<RE::ActorValueOwner>();
			return avOwner && avOwner->GetBaseActorValue(a_value) >= 100.0f;
		};

		if (conjuredCount == a_obj) {
			return get_matching_minions(minions, is_summoned);
		}
		if (totalCount == a_obj) {
			return get_matching_minions(minions, [](const auto& a_data) {
				return static_cast<bool>(a_data.commandedActor);
			});
		}
		if (fireCount == a_obj) {
			return get_matching_minions(minions, [&](const auto& a_data) {
				return has_resistance(a_data, RE::ActorValue::kResistFire);
			});
		}
		if (frostCount == a_obj) {
			return get_matching_minions(minions, [&](const auto& a_data) {
				return has_resistance(a_data, RE::ActorValue::kResistFrost);
			});
		}
		if (shockCount == a_obj) {
			return get_matching_minions(minions, [&](const auto& a_data) {
				return has_resistance(a_data, RE::ActorValue::kResistShock);
			});
		}
		if (reanimatedCount == a_obj) {
			return get_matching_minions(minions, is_reanimated);
		}

		return std::nullopt;
	}
}