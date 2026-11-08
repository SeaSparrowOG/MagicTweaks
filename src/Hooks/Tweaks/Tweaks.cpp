#include "Tweaks.h"

#include "RE/Offset.h"
#include "Settings/INI/INISettings.h"
#include "Settings/JSON/JSONSettings.h"

#include <xbyak.h>
#undef min
#undef max

namespace Hooks::Tweaks
{
	bool InstallTweaks() {
		bool success = true;
		bool installReductionTweak = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_REDUCTION).value_or(false);
		g_extendInDialogue = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_DIALOGUE).value_or(false);
		bool installSheatheTweak = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_DISPEL).value_or(false);
		bool installCastingSpeedTweak = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_CASTING_SPEED).value_or(false);

		if (installReductionTweak) {
			success &= ModifySpellReduction::PatchSpellReduction();
		}
		if (installSheatheTweak) {
			auto* dispeler = SpellDispeler::GetSingleton();
			if (!dispeler) {
				REX::CRITICAL("Failed to get internal spell dispeler manager."sv);
				return false;
			}
			success &= dispeler->Install();
		}
		if (installCastingSpeedTweak) {
			success &= CastSpeedTweaks::PatchSpeed();
		}

		success &= ArchetypePatch<RE::CloakEffect>::Patch();
		success &= ArchetypePatch<RE::BoundItemEffect>::Patch();
		success &= ArchetypePatch<RE::LightEffect>::Patch();
		success &= ArchetypePatch<RE::InvisibilityEffect>::Patch();
		success &= ArchetypePatch<RE::ScriptEffect>::Patch();
		success &= ArchetypePatch<RE::ValueModifierEffect>::Patch();
		success &= ArchetypePatch<RE::PeakValueModifierEffect>::Patch();
		success &= ArchetypePatch<RE::DualValueModifierEffect>::Patch();
		success &= ArchetypePatch<RE::CommandEffect>::Patch();
		success &= ArchetypePatch<RE::CommandSummonedEffect>::Patch();
		success &= ArchetypePatch<RE::SummonCreatureEffect>::Patch();
		success &= ArchetypePatch<RE::CalmEffect>::Patch();
		success &= ArchetypePatch<RE::DemoralizeEffect>::Patch();
		success &= ArchetypePatch<RE::FrenzyEffect>::Patch();

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

		auto& trampoline = REL::GetTrampoline();
		const REL::Relocation<std::uintptr_t> target{ RE::Offset::MagicItem::CalculateCost };

		Patch p(target.address(), 5);
		p.ready();

		trampoline.write_jmp<5>(target.address(), ProcessSpellReduction);

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

	bool SpellDispeler::Install() {
		return PlayerDrawMonitor::Install();
	}

	void SpellDispeler::ClearDispelableSpells(RE::PlayerCharacter* a_player) {
		auto* magicTarget = a_player ? a_player->GetMagicTarget() : nullptr;
		if (!magicTarget || dispelEffects.empty()) {
			return;
		}

		auto effectList = magicTarget->GetActiveEffectList();
		if (effectList->empty()) {
			return;
		}

		for (auto activeEffect : *effectList) {
			const auto* base = activeEffect && activeEffect->effect ?
				activeEffect->effect->baseEffect : nullptr;
			if (!base || !dispelEffects.contains(base)) {
				continue;
			}

			const float effectDuration = activeEffect->duration;
			const float setElapsedTime = std::max(effectDuration - 0.1f, 0.1f);
			activeEffect->elapsedSeconds = setElapsedTime;
		}
	}

	bool SpellDispeler::LoadJSONSettings() {
		auto* reader = Settings::JSON::Reader::GetSingleton();
		if (!reader) {
			REX::CRITICAL("Failed to fetch internal Spell Dispeler singleton."sv);
			return false;
		}

		for (const auto& setting : reader->settings) {
			if (setting.isObject()) {
				const auto& fieldOfInterest = setting[SheatheArray];
				if (!fieldOfInterest) {
					continue;
				}
				if (fieldOfInterest.isString()) {
					auto* form = reader->GetFormFromString<RE::SpellItem>(fieldOfInterest.asString());
					if (!form || form->effects.empty()) {
						continue;
					}

					for (auto* effect : form->effects) {
						auto* base = effect ? effect->baseEffect : nullptr;
						if (!base) {
							continue;
						}
						dispelEffects.insert(base);
					}
				}
				else if (fieldOfInterest.isArray()) {
					for (const auto& formOfInterest : fieldOfInterest) {
						auto* form = reader->GetFormFromString<RE::SpellItem>(formOfInterest.asString());
						if (!form || form->effects.empty()) {
							continue;
						}

						for (auto* effect : form->effects) {
							auto* base = effect ? effect->baseEffect : nullptr;
							if (!base) {
								continue;
							}
							if (dispelEffects.contains(base)) {
								continue;
							}
							LOG_DEBUG("Registered effect to dispel: {}", reader->GetEditorID(base));
							dispelEffects.insert(base);
						}
					}
				}
				else {
					REX::WARN("Ran into non-string and non-array object in {}.", SheatheArray);
				}
			}
			else {
				REX::WARN("Ran into top level non-object config, ignoring for Spell Dispeler."sv);
			}
		}
		return true;
	}

	bool SpellDispeler::PlayerDrawMonitor::Install() {
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			REX::CRITICAL("    >Failed to fetch ini settings holder for the Seathe monitor."sv);
			return false;
		}

		auto installRaw = iniHolder->GetStoredSetting<bool>(setting);
		bool install = installRaw.has_value() ? installRaw.value() : false;
		if (!installRaw.has_value()) {
			REX::WARN("    >Setting {} not found in ini settings, treating as false.", setting);
		}
		if (!install) {
			return true;
		}

		REL::Relocation<std::uintptr_t> VTABLE{ RE::PlayerCharacter::VTABLE[0] };
		_func = VTABLE.write_vfunc(offset, Thunk);
		REX::INFO("    >Installed Player VFunc hook."sv);
		return true;
	}

	void SpellDispeler::PlayerDrawMonitor::Thunk(RE::PlayerCharacter* a_this,
		bool a_draw)
	{
		_func(a_this, a_draw);
		if (!a_draw) {
			if (auto* dispeler = SpellDispeler::GetSingleton(); dispeler) {
				dispeler->ClearDispelableSpells(a_this);
			}
		}
	}

    inline bool CastSpeedTweaks::PatchSpeed() {
		REX::INFO("  >Installing Casting speed tweaks..."sv);
		REL::Relocation<std::uintptr_t> target{ RE::Offset::MagicCaster::ChargeSpell, 0x4B };
		if (!REL::Pattern<"E9">().match(target.address())) {
			REX::CRITICAL("    >Failed to match expected pattern for Casting Speed."sv);
			return false;
		}
		auto& trampoline = REL::GetTrampoline();
		_setCastTimer = trampoline.write_jmp<5>(target.address(), SetCastTimer);
        return true;
    }

	enum class Milestone : uint8_t {
		Talentless = 0u,
		Novice = 15u,
		Apprentice = 25u,
		Adept = 50u,
		Expert = 75u,
		Master = 90u
	};

	static Milestone get_level_milestone(uint8_t level) noexcept {
		if (level < std::to_underlying(Milestone::Novice)) {
			return Milestone::Talentless;
		}
		else if (level < std::to_underlying(Milestone::Apprentice)) {
			return Milestone::Novice;
		}
		else if (level < std::to_underlying(Milestone::Adept)) {
			return Milestone::Apprentice;
		}
		else if (level < std::to_underlying(Milestone::Expert)) {
			return Milestone::Adept;
		}
		else if (level < std::to_underlying(Milestone::Master)) {
			return Milestone::Expert;
		}
		return Milestone::Master;
	}

	static constexpr uint8_t get_milestone_tier(Milestone milestone) noexcept {
		switch (milestone) {
			case Milestone::Talentless: return 0u;
			case Milestone::Novice: return 1u;
			case Milestone::Apprentice: return 2u;
			case Milestone::Adept: return 3u;
			case Milestone::Expert: return 4u;
			case Milestone::Master: return 5u;
			default: std::unreachable();
		}
	}

	static uint8_t get_milestone_gap(Milestone milestone, float skillLevel) {
		const uint8_t floored = skillLevel < 0.0f ? 0u : static_cast<uint8_t>(std::floor(skillLevel));
		const Milestone casterTalent = get_level_milestone(floored);
		if (casterTalent >= milestone) {
			return 0u;
		}

		return get_milestone_tier(milestone) - get_milestone_tier(casterTalent);
	}

	static Milestone get_spell_milestone(const RE::BGSPerk* perk) {
		constexpr std::array<std::pair<RE::FormID, Milestone>, 25> perkMappings = {{
			{0xF2CA6, Milestone::Novice},
			{0xF2CA7, Milestone::Novice},
			{0xF2CA8, Milestone::Novice},
			{0xF2CA9, Milestone::Novice},
			{0xF2CAA, Milestone::Novice},

			{0xC44B7, Milestone::Apprentice},
			{0xC44BB, Milestone::Apprentice},
			{0xC44BF, Milestone::Apprentice},
			{0xC44C3, Milestone::Apprentice},
			{0xC44C7, Milestone::Apprentice},

			{0xC44B8, Milestone::Adept},
			{0xC44BC, Milestone::Adept},
			{0xC44C0, Milestone::Adept},
			{0xC44C4, Milestone::Adept},
			{0xC44C8, Milestone::Adept},

			{0xC44B9, Milestone::Expert},
			{0xC44BD, Milestone::Expert},
			{0xC44C1, Milestone::Expert},
			{0xC44C5, Milestone::Expert},
			{0xC44C9, Milestone::Expert},

			{0xC44BA, Milestone::Master},
			{0xC44BE, Milestone::Master},
			{0xC44C2, Milestone::Master},
			{0xC44C6, Milestone::Master},
			{0xC44CA, Milestone::Master}
		}};
		
		if (!perk) {
			return Milestone::Talentless;
		}

		const auto halfPerkID = perk->GetFormID();
		for (const auto& [id, milestone] : perkMappings) {
			if (halfPerkID == id) {
				return milestone;
			}
		}
		return Milestone::Talentless;
	}

	inline void CastSpeedTweaks::SetCastTimer(RE::MagicCaster *a_this) {
		static const float settingsPenalty = Settings::INI::GetSetting<float>(
			Settings::INI::TWEAK_CASTING_SPEED_PERCENT)
			.value_or(50.0f) / 100.0f;

		static const float clampedPenalty = std::clamp(settingsPenalty, 0.1f, 10.0f) + 1.0f;

		_setCastTimer(a_this);
		const auto* actor = a_this ? a_this->GetCasterAsActor() : nullptr;
		if (!actor) { 
			return;
		}

		const auto* magicItem = a_this->currentSpell;
		const auto* spell = magicItem ? magicItem->As<RE::SpellItem>() : nullptr;
		const auto* halfPerk = spell ? spell->data.castingPerk : nullptr;
		if (!halfPerk) {
			return;
		}
		
		const auto school = spell->GetAssociatedSkill();
		switch (school) {
			case RE::ActorValue::kIllusion:
			case RE::ActorValue::kConjuration:
			case RE::ActorValue::kDestruction:
			case RE::ActorValue::kRestoration:
			case RE::ActorValue::kAlteration:
				break;
			default:
				return;
		}

		const auto spellLevel = get_spell_milestone(halfPerk);
		const auto casterSkill = std::clamp(actor->GetActorValue(school), 0.0f, 100.0f);
		const auto difference = get_milestone_gap(spellLevel, casterSkill);
		if (difference == 0u) {
			return;
		}

		const float penalty = static_cast<float>(difference) * clampedPenalty;
		a_this->castingTimer *= penalty;
	}
}