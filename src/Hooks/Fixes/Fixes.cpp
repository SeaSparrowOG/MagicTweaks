#include "Fixes.h"

#include "Settings/INI/INISettings.h"

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
	}
}