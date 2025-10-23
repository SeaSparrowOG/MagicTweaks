#include "Fixes.h"

#include "RE/Offset.h"
#include "RE/BGSEntryPointFunctionDataTwoValue.h"
#include "Settings/INI/INISettings.h"

#include <xbyak.h>

namespace
{
	bool AllowAbsorb(RE::MagicTarget* a_this,
		RE::Actor* a_actor,
		RE::MagicItem* a_magicItem,
		const RE::Effect* a_effect)
	{
		(void)a_effect;
		if (!a_magicItem ||
			a_magicItem->effects.empty() ||
			!a_actor ||
			!a_this ||
			!a_this->MagicTargetIsActor()) {
			return true;
		}

		bool fixBeneficial = Settings::INI::GetSetting<bool>(Settings::INI::FIX_BENEFICIAL).value_or(false);
		bool fixPoison = Settings::INI::GetSetting<bool>(Settings::INI::FIX_POISON).value_or(false);
		bool fixSelfTarget = Settings::INI::GetSetting<bool>(Settings::INI::FIX_SELF).value_or(false);

		using EffectFlag = RE::EffectSetting::EffectSettingData::Flag;
		using CastFlag = RE::MagicSystem::CastingType;
		using DeliveryFlag = RE::MagicSystem::Delivery;

		const auto* targetActor = static_cast<RE::Actor*>(a_this);
		if (!targetActor) {
			return true;
		}

		if (fixSelfTarget && a_actor == targetActor) {
			return true;
		}

		const auto& effects = a_magicItem->effects;
		bool isHostile = false;
		bool isPoison = false;
		bool isContact = a_magicItem->GetDelivery() == DeliveryFlag::kTouch;
		for (auto it = effects.begin(); it != effects.end(); ++it) {
			const auto* effect = *it;
			const auto* base = effect ? effect->baseEffect : nullptr;
			if (!base) {
				continue;
			}

			const auto& effectData = base->data;

			if (effectData.flags.any(EffectFlag::kHostile)) {
				isHostile = true;
			}
			if (effectData.resistVariable == RE::ActorValue::kPoisonResist) {
				isPoison = true;
			}
		}

		if (!isHostile && fixBeneficial) {
			return false;
		}
		else if (isPoison && isContact && fixPoison) {
			return false;
		}

		return true;
	}
}

namespace Hooks {
	namespace Fixes {
		bool InstallFixes() {
			auto installPoison = Settings::INI::GetSetting<bool>(Settings::INI::FIX_POISON);
			auto installSelf = Settings::INI::GetSetting<bool>(Settings::INI::FIX_SELF);
			auto installBeneficial = Settings::INI::GetSetting<bool>(Settings::INI::FIX_BENEFICIAL);

			bool atLeastOnePatch = installPoison.value_or(false) &&
				installSelf.value_or(false) &&
				installBeneficial.value_or(false);

			if (!atLeastOnePatch) {
				return true;
			}

			bool success = true;
			success &= Character::InstallCharacterFixes();
			success &= Player::InstallPlayerFixes();
			success &= CloakArchetypeFix::InstallCloakFix();

			return success;
		}

		bool Character::InstallCharacterFixes() {
			REL::Relocation<std::uintptr_t> VTABLE{ RE::Character::VTABLE[4] };
			_func = VTABLE.write_vfunc(0xB, CharacterThunk);
			logger::info("    >Installed Character Magic Target VFunc hook."sv);
			return true;
		}

		bool Player::InstallPlayerFixes() {
			REL::Relocation<std::uintptr_t> VTABLE{ RE::PlayerCharacter::VTABLE[4] };
			_func = VTABLE.write_vfunc(0xB, PlayerThunk);
			logger::info("    >Installed Player Character Magic Target VFunc hook."sv);
			return true;
		}

		bool Player::PlayerThunk(RE::MagicTarget* a_this,
			RE::Actor* a_actor,
			RE::MagicItem* a_magicItem,
			const RE::Effect* a_effect)
		{
			return AllowAbsorb(a_this, a_actor, a_magicItem, a_effect) &&
				_func(a_this, a_actor, a_magicItem, a_effect);
		}

		bool Character::CharacterThunk(RE::MagicTarget* a_this,
			RE::Actor* a_actor,
			RE::MagicItem* a_magicItem,
			const RE::Effect* a_effect)
		{
			return AllowAbsorb(a_this, a_actor, a_magicItem, a_effect) &&
				_func(a_this, a_actor, a_magicItem, a_effect);
		}

		bool CloakArchetypeFix::InstallCloakFix() {
			logger::info("    >Installing the Cloak Archetype Fix..."sv);
			const bool shouldInstall = Settings::INI::GetSetting<bool>(Settings::INI::FIX_CLOAKS).value_or(false);
			if (!shouldInstall) {
				logger::info("      User chose not to install the fix."sv);
				return true;
			}

			REL::Relocation<std::uintptr_t> target{ RE::Offset::AnonymousNamespace::ResetElapsedTimeMagicEffects, 0x72 };
			if (!REL::make_pattern<"E8">().match(target.address())) {
				logger::critical("    >Failed to validate the hook pattern."sv);
				return false;
			}
			auto& trampoline = SKSE::GetTrampoline();
			_func = trampoline.write_call<5>(target.address(), &ResetCloakEffect);
			return true;
		}

		void CloakArchetypeFix::ResetCloakEffect(RE::ActiveEffect* a_effect)
		{
			// Effect might be nullptr, but isn't checked in vanilla
			if (!a_effect ||
				a_effect->castingSource != RE::MagicSystem::CastingSource::kInstant ||
				!a_effect->GetBaseObject())
			{
				_func(a_effect);
				return;
			}

			const auto* base = a_effect->GetBaseObject();
			if (!base ||
				base->data.castingType != RE::MagicSystem::CastingType::kConcentration ||
				base->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kRecover))
			{
				_func(a_effect);
				return;
			}

			a_effect->flags.reset(RE::ActiveEffect::Flag::kDual);
			a_effect->elapsedSeconds = 0.0f;

			// Attempt to re-apply magnitude from perks
			auto* caster = a_effect->caster.get().get();
			auto* target = a_effect->target;
			auto* targetAsActor = target ? target->GetTargetAsActor() : nullptr;
			auto* spell = a_effect->spell ? a_effect->spell->As<RE::SpellItem>() : nullptr;
			if (!caster || !targetAsActor || !spell || spell->effects.empty()) {
				return;
			}

			float magnitude = 0.0f;
			bool foundMagnitude = false;
			auto begin = spell->effects.begin();
			auto end = spell->effects.end();
			for (auto it = begin; !foundMagnitude && it != end; ++it) {
				auto* effectItem = *it;
				if (!effectItem) {
					continue;
				}
				if (base != effectItem->baseEffect) {
					continue;
				}

				foundMagnitude = true;
				magnitude = effectItem->GetMagnitude();
			}
			if (!foundMagnitude) {
				return;
			}

			bool reverse = false;
			RE::BGSEntryPoint::HandleEntryPoint(
				RE::BGSEntryPointPerkEntry::EntryPoint::kModSpellMagnitude,
				caster,
				spell,
				targetAsActor,
				&magnitude);

			if (base) {
				switch (base->GetArchetype()) {
				case RE::EffectSetting::Archetype::kValueModifier:
				case RE::EffectSetting::Archetype::kAbsorb:
				case RE::EffectSetting::Archetype::kDualValueModifier:
				case RE::EffectSetting::Archetype::kAccumulateMagnitude:
				case RE::EffectSetting::Archetype::kPeakValueModifier:
					reverse = true;
					break;
				}
			}

			if (reverse &&
				base->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kDetrimental))
			{
				magnitude = -magnitude;
			}
			a_effect->magnitude = magnitude;
		}
	}
}