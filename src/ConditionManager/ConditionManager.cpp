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

		const auto get_effects = [](const auto& a_data) {
			const auto* ame = a_data.activeEffect;
			const auto* spell = ame ? ame->spell : nullptr;
			return spell ? &spell->effects : nullptr;
		};

		const auto is_summoned = [&](const auto& a_data) {
			const auto* actor = get_actor(a_data);
			return actor && actor->IsSummoned();
		};

		const auto is_reanimated = [&](const auto& a_data) {
			const auto* actor = get_actor(a_data);
			return actor && actor->IsReanimated();
		};

		const auto effect_has_keyword = [&](const auto* effect, const auto* kwd) {
			auto* base = effect ? effect->baseEffect : nullptr;
			return base && base->HasKeyword(kwd);
		};
		
		const auto commanding_spell_has_keyword = [&](const auto& a_data, const RE::BGSKeyword* a_kwd) {
			const auto* effects = get_effects(a_data);
			return effects && 
				std::ranges::any_of(*effects, [&](const auto* effect) { 
					return effect_has_keyword(effect, a_kwd); 
				});
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
			static const auto* kwd = RE::TESForm::LookupByID<RE::BGSKeyword>(0x24823);
			return get_matching_minions(minions, [&](const auto& a_data) {
				return commanding_spell_has_keyword(a_data, kwd);
			});
		}
		if (frostCount == a_obj) {
			static const auto* kwd = RE::TESForm::LookupByID<RE::BGSKeyword>(0x24829);
			return get_matching_minions(minions, [&](const auto& a_data) {
				return commanding_spell_has_keyword(a_data, kwd);
			});
		}
		if (shockCount == a_obj) {
			static const auto* kwd = RE::TESForm::LookupByID<RE::BGSKeyword>(0x2482A);
			return get_matching_minions(minions, [&](const auto& a_data) {
				return commanding_spell_has_keyword(a_data, kwd);
			});
		}
		if (reanimatedCount == a_obj) {
			return get_matching_minions(minions, is_reanimated);
		}

		return std::nullopt;
	}
}