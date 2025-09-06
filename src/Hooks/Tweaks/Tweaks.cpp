#include "Tweaks.h"

#include "Settings/INI/INISettings.h"

#include <xbyak.h>
#undef min
#undef max

namespace Hooks::Tweaks
{
	bool InstallTweaks() {
		bool success = true;
		bool installReductionTweak = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_REDUCTION).value_or(false);
		bool patchEffectsInDialogue = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_DIALOGUE).value_or(false);

		if (installReductionTweak) {
			success &= ModifySpellReduction::PatchSpellReduction();
		}

		if (patchEffectsInDialogue) {
			bool patchCloaks = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_DIALOGUE_CLOAKS).value_or(false);
			bool patchWeapons = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_DIALOGUE_WEAPONS).value_or(false);
			bool patchLights = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_DIALOGUE_LIGHTS).value_or(false);
			bool patchScripts = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_DIALOGUE_SCRIPTS).value_or(false);
			bool patchValues = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_DIALOGUE_VALUES).value_or(false);
			bool patchSummons = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_DIALOGUE_SUMMONS).value_or(false);

			if (patchCloaks) {
				success &= ArchetypePatch<RE::CloakEffect>::Patch();
			}
			if (patchWeapons) {
				success &= ArchetypePatch<RE::BoundItemEffect>::Patch();
			}
			if (patchLights) {
				success &= ArchetypePatch<RE::LightEffect>::Patch();
			}
			if (patchScripts) {
				success &= ArchetypePatch<RE::ScriptEffect>::Patch();
			}
			if (patchValues) {
				success &= ArchetypePatch<RE::ValueModifierEffect>::Patch();
				success &= ArchetypePatch<RE::DualValueModifierEffect>::Patch();
			}
			if (patchSummons) {
				success &= ArchetypePatch<RE::CommandEffect>::Patch();
				success &= ArchetypePatch<RE::CommandSummonedEffect>::Patch();
				success &= ArchetypePatch<RE::SummonCreatureEffect>::Patch();
			}
		}

		return success;
	}

	inline bool ModifySpellReduction::PatchSpellReduction() {
		skillCeilling = Settings::INI::GetSetting<float>(Settings::INI::TWEAK_REDUCTION_MAX).value_or(200.0f);
		skillFloor = Settings::INI::GetSetting<float>(Settings::INI::TWEAK_REDUCTION_MIN).value_or(-45.0f);
		skillWeight = Settings::INI::GetSetting<float>(Settings::INI::TWEAK_REDUCTION_WEIGHT).value_or(0.01f);
		maxReductionPct = Settings::INI::GetSetting<float>(Settings::INI::TWEAK_REDUCTION_REDUCTION_MAX).value_or(0.9f);

		// Thank you FromSoft - er, I mean Nukem
		struct Patch : Xbyak::CodeGenerator
		{
			explicit Patch(uintptr_t OriginalFuncAddr, size_t OriginalByteLength)
			{
				for (size_t i = 0; i < OriginalByteLength; i++)
					db(*reinterpret_cast<uint8_t*>(OriginalFuncAddr + i));

				jmp(qword[rip]);
				dq(OriginalFuncAddr + OriginalByteLength);
			}
		};

		auto& trampoline = SKSE::GetTrampoline();
		const REL::Relocation<std::uintptr_t> target{ RE::Offset::MagicItem::CalculateCost };

		Patch p(target.address(), 5);
		p.ready();

		trampoline.write_branch<5>(target.address(), ProcessSpellReduction);

		auto alloc = trampoline.allocate(p.getSize());
		memcpy(alloc, p.getCode(), p.getSize());

		_func = reinterpret_cast<uintptr_t>(alloc);
		return true;
	}

	inline float ModifySpellReduction::ProcessSpellReduction(RE::MagicItem* a_spell, RE::Actor* a_actor) {
		using AMEFlag = RE::EffectSetting::EffectSettingData::Flag;
		auto* spell = a_spell ? a_spell->As<RE::SpellItem>() : nullptr;
		if (!spell || !a_actor || !a_actor->As<RE::ActorValueOwner>()) {
			return _func(a_spell, a_actor);
		}
		LOG_DEBUG("Call with data: {}/{}/{}"sv, skillFloor, skillCeilling, skillWeight);
		LOG_DEBUG("  >Spell {} (Original cost: {})"sv, a_spell->GetName(), _func(a_spell, a_actor));
		float maxCost = 0.0f;
		float cost = 0.0f;
		auto* avOwner = a_actor->As<RE::ActorValueOwner>();
		if (spell->data.flags.any(RE::SpellItem::SpellFlag::kCostOverride)) {
			cost = static_cast<float>(spell->data.costOverride);
		}
		else {
			auto& effects = a_spell->effects;
			for (const auto* effect : effects) {
				auto* base = effect ? effect->baseEffect : nullptr;
				if (!base || base->data.baseCost <= 0.0f) {
					continue;
				}

				auto baseSkill = base->data.associatedSkill;
				bool adjustCost = false;
				switch (baseSkill) {
				case RE::ActorValue::kIllusion:
				case RE::ActorValue::kConjuration:
				case RE::ActorValue::kDestruction:
				case RE::ActorValue::kRestoration:
				case RE::ActorValue::kAlteration:
					adjustCost = true;
					break;
				default:
					break;
				}
				if (!adjustCost) {
					continue;
				}

				float effectCost = effect->cost;
				float skill = std::clamp(avOwner->GetActorValue(baseSkill), skillFloor, skillCeilling);
				cost += 1.0f / (1.0f + skillWeight * skill) * effectCost;
				maxCost += effectCost;
			}
		}
		cost = std::max(cost, maxCost * (1.0f - maxReductionPct));
		if (a_actor) {
			RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModSpellCost, a_actor, a_spell, &cost);
		}
		LOG_DEBUG("  >Finished with cost: {}", cost);
		return cost;
	}

}