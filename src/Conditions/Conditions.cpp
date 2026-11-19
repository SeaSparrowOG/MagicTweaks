#include "Conditions.hpp"

#include "Settings/ModObjects/ModObjects.hpp"

namespace {
    struct EvalContext
    {
        bool                     _evaluateTargetedConditions = false;
        RE::ConditionCheckParams _params{RE::PlayerCharacter::GetSingleton(), nullptr};
    };
    
    [[nodiscard]] bool evaluate_targeted_portions(const RE::EffectSetting* effect) noexcept {
        assert(effect);
        using Delivery = RE::MagicSystem::Delivery;

        switch (effect->data.delivery) {
            case Delivery::kSelf:
            case Delivery::kTargetLocation:
            case Delivery::kNone:
                return true;
            default:
                return false;
        }
    }

    [[nodiscard]] bool ignore_condition(const RE::TESConditionItem* head, const EvalContext& ctx) noexcept {
        assert(head);
        if (ctx._evaluateTargetedConditions) {
            return false;
        }

        const auto& data = head->data;
        
        if (data.runOnRef && data.runOnRef.get()->IsPlayerRef()) {
            return false;
        }
        
        // slightly unintuitive - for magic effects and spells, Target is the CASTER
        // and Self (Subject) is the TARGET.
        // Switch subject and caster flips them. So, we ignore:
        //    -> Target + !Switch
        //    -> Subject + Switch
        const bool switchSubject = data.flags.swapTarget;
        const auto conditionSubject = data.object.get();
        switch (conditionSubject) {
            case RE::CONDITIONITEMOBJECT::kTarget:
                return !switchSubject;
            case RE::CONDITIONITEMOBJECT::kSelf:
                return switchSubject;
            default: break;
        }
        return false;
    }

    RE::TESConditionItem* get_next_and(const RE::TESConditionItem* head) noexcept {
        while (head) {
            if (!head->data.flags.isOR) {
                return head->next;
            }
            head = head->next;
        }
        return nullptr;
    }

    [[nodiscard]] bool evaluate_or_group(const RE::TESConditionItem* head, 
        EvalContext& ctx) noexcept 
    {
        auto& params = ctx._params;
        if (!head) {
            return true;
        }

        bool foundAND = false;
        bool allSkipped = true;
        while (head) {
            const bool skip = ignore_condition(head, ctx);
            allSkipped &= skip;

            if (skip) {
                if (foundAND) {
                    return allSkipped;
                }
                foundAND |= !head->data.flags.isOR;
                head = head->next;
                continue;
            }

            foundAND |= !head->data.flags.isOR;
            if (!head->IsTrue(params)) {
                head = head->next;
                continue;
            }
            return true;
        }
        return allSkipped;
    }

    template <typename T, class U>
    concept is_same_underlying = std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

    template <typename T>
    concept is_spell_or_effect = is_same_underlying<RE::Effect, T> || is_same_underlying<RE::EffectSetting, T>;

    template <typename T> requires is_spell_or_effect<T>
    [[nodiscard]] bool are_conditions_met(const T* form,
        EvalContext& ctx) noexcept
    {
        if (!form) {
            return false;
        }

        auto& params = ctx._params;
        RE::TESConditionItem* head = form->conditions.head;
        while (head) {
            const bool skip = ignore_condition(head, ctx);
            const bool isOR = head->data.flags.isOR;

            if (!skip && !head->IsTrue(params)) {
                return false;
            }

            head = head->next;
            if (isOR && head) {
                if (!evaluate_or_group(head, ctx)) {
                    return false;
                }
                head = get_next_and(head);
            }
        }
        return true;
    }
}

namespace {

    const RE::Actor* get_actor(const RE::CommandedActorData& a_data) {
        return a_data.commandedActor ? a_data.commandedActor.get().get() : nullptr;
    };

    const RE::BSTArray<RE::Effect*>* get_effects(const RE::CommandedActorData& a_data) {
        const auto* ame = a_data.activeEffect;
        const auto* spell = ame ? ame->spell : nullptr;
        return spell ? &spell->effects : nullptr;
    };

    bool is_summoned(const RE::CommandedActorData& a_data) {
        const auto* actor = get_actor(a_data);
        return actor && actor->IsSummoned();
    };

    bool is_reanimated(const RE::CommandedActorData& a_data) {
        const auto* actor = get_actor(a_data);
        return actor && actor->IsReanimated();
    };

    bool effect_has_keyword(const RE::Effect* effect, const RE::BGSKeyword* kwd) {
        auto* base = effect ? effect->baseEffect : nullptr;
        return base && base->HasKeyword(kwd);
    };
    
    bool commanding_spell_has_keyword(const RE::CommandedActorData& a_data, const RE::BGSKeyword* a_kwd) {
        const auto* effects = get_effects(a_data);
        return effects && 
            std::ranges::any_of(*effects, [&](const auto* effect) { 
                return effect_has_keyword(effect, a_kwd); 
            });
    };

    bool is_fire_summon(const RE::CommandedActorData& data) {
        static const RE::BGSKeyword* magicSummonFire = [] {
            auto* result = RE::TESForm::LookupByID<RE::BGSKeyword>(0x24823);
            if (!result) {
                REX::ERROR("Failed to resolve vanilla keyword: MagicSummonFire"sv);
            }
            return result;
        }();
        return commanding_spell_has_keyword(data, magicSummonFire);
    }

    bool is_frost_summon(const RE::CommandedActorData& data) {
        static const RE::BGSKeyword* magicSummonFrost = [] {
            auto* result = RE::TESForm::LookupByID<RE::BGSKeyword>(0x24829);
            if (!result) {
                REX::ERROR("Failed to resolve vanilla keyword: MagicSummonFrost"sv);
            }
            return result;
        }();
        return commanding_spell_has_keyword(data, magicSummonFrost);
    }

    bool is_shock_summon(const RE::CommandedActorData& data) {
        static const RE::BGSKeyword* magicSummonShock = [] {
            auto* result = RE::TESForm::LookupByID<RE::BGSKeyword>(0x2482A);
            if (!result) {
                REX::ERROR("Failed to resolve vanilla keyword: MagicSummonShock"sv);
            }
            return result;
        }();
        return commanding_spell_has_keyword(data, magicSummonShock);
    }

    bool is_summoned_reanimated(const RE::CommandedActorData& data) {
        return is_summoned(data) || is_reanimated(data);
    }

    template <class Predicate>
	[[nodiscard]]
	int32_t get_matching_minions(
		const RE::BSTArray<RE::CommandedActorData>& minions,
		Predicate predicate)
	{
		return static_cast<int32_t>(std::ranges::count_if(minions, predicate));
	}
    
    enum class QueryType : uint8_t
    {
        All,
        Conjured,
        Reanimated,
        Fire,
        Frost,
        Shock
    };

    using Predicate = bool (*)(const RE::CommandedActorData&);
    [[nodiscard]] 
    constexpr Predicate get_predicate(const QueryType type) noexcept {
        switch (type) {
            case QueryType::All: return is_summoned_reanimated;
            case QueryType::Conjured: return is_summoned;
            case QueryType::Reanimated: return is_reanimated;
            case QueryType::Fire: return is_fire_summon;
            case QueryType::Frost: return is_frost_summon;
            case QueryType::Shock: return is_shock_summon;
            default: std::unreachable();
        }
    }
}

namespace Conditions {
    std::vector<RE::Effect*> get_valid_effects(const RE::SpellItem *spell) {
        std::vector<RE::Effect*> res{};

        if (!spell) {
            return res;
        }

        const auto& effects = spell->effects;
        if (effects.empty()) {
            return res;
        }

        res.reserve(effects.size());
        EvalContext ctx{};
        for (auto* effect : effects) {
            const auto* base = effect ? effect->baseEffect : nullptr;
            if (!base) {
                continue;
            }
            
            ctx._evaluateTargetedConditions = evaluate_targeted_portions(base);
            if (!are_conditions_met(effect, ctx) || !are_conditions_met(base, ctx)) {
                continue;
            }
            res.push_back(effect);
        }
        return res;
    }

    std::optional<int32_t> get_alt_item_count(const RE::TESBoundObject* form, const RE::TESObjectREFR* ref)
    {
        namespace MO = Settings::ModObjects;
        static auto* totalObj = 
            MO::GetObject<RE::TESBoundObject>(MO::PROXY_COUNT_COMMANDED_CONJURATION);
        static auto* summonedObj = 
            MO::GetObject<RE::TESBoundObject>(MO::PROXY_COUNT_SUMMONED);
        static auto* reanimatedObj = 
            MO::GetObject<RE::TESBoundObject>(MO::PROXY_COUNT_REANIMATED);
        static auto* fireObj = 
            MO::GetObject<RE::TESBoundObject>(MO::PROXY_COUNT_SUMMONED_FIRE);
        static auto* frostObj = 
            MO::GetObject<RE::TESBoundObject>(MO::PROXY_COUNT_SUMMONED_FROST);
        static auto* shockObj = 
            MO::GetObject<RE::TESBoundObject>(MO::PROXY_COUNT_SUMMONED_SHOCK);

        if (!totalObj || !summonedObj || !reanimatedObj ||
            !fireObj || !frostObj || !shockObj) 
        {
            REX::ERROR("At least one form in MagicTweaks.esl is invalid or failed to resolve."sv);
            return std::nullopt;
        }

        static const std::unordered_map<RE::FormID, QueryType> bindings = {
            {totalObj->GetFormID(), QueryType::All},
            {summonedObj->GetFormID(), QueryType::Conjured},
            {reanimatedObj->GetFormID(), QueryType::Reanimated},
            {fireObj->GetFormID(), QueryType::Fire},
            {frostObj->GetFormID(), QueryType::Frost},
            {shockObj->GetFormID(), QueryType::Shock}
        };

        if (!ref || !form) {
            return std::nullopt;
        }

        const auto* actor = ref->As<RE::Actor>();
        const auto* process = actor ? actor->GetMiddleHighProcess() : nullptr;
        if (!process) {
            return std::nullopt;
        }

        auto it = bindings.find(form->GetFormID());
        if (it == bindings.end()) {
            return std::nullopt;
        }

        const auto& minions = process->commandedActors;
        if (minions.empty()) {
            return 0;
        }

        const auto predicate = get_predicate(it->second);
        return get_matching_minions(minions, predicate);
    }
}