#include "Fixes.h"

#include "RE/Offset.h"
#include "Settings/INISettings.h"

namespace Hooks::Fixes
{
	bool Install() {
		logger::info("  >Installing fixes"sv);
		auto* listener = Listener::GetSingleton();
		if (!listener) {
			logger::critical("    >Failed to get Fixes Listener."sv);
			return false;
		}
		return listener->Install();
	}

	bool Listener::Install() {
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			logger::info("    >Failed to fetch INI settings holder for Fixes."sv);
			return false;
		}

		auto selfAbsorbFixRaw = iniHolder->GetStoredSetting<bool>("Fixes|bNeverAbsorbSelfTargettingEffects");
		fixSelfTarget = selfAbsorbFixRaw.has_value() ? selfAbsorbFixRaw.value() : true;
		if (!selfAbsorbFixRaw.has_value()) {
			logger::warn("    >Self-Target fix setting somehow not specified in the INI, treating as true."sv);
		}

		auto beneficialFixRaw = iniHolder->GetStoredSetting<bool>("Fixes|bNeverAbsorbBeneficialEffects");
		fixBeneficial = beneficialFixRaw.has_value() ? beneficialFixRaw.value() : true;
		if (!beneficialFixRaw.has_value()) {
			logger::warn("    >Beneficial fix setting somehow not specified in the INI, treating as true."sv);
		}

		auto poisonFixRaw = iniHolder->GetStoredSetting<bool>("Fixes|bNeverAbsorbPoison");
		fixPoison = poisonFixRaw.has_value() ? poisonFixRaw.value() : true;
		if (!poisonFixRaw.has_value()) {
			logger::warn("    >Poison fix setting somehow not specified in the INI, treating as true."sv);
		}

		return Player::Install() && Character::Install();
	}

	bool Listener::IgnoreAbsorb(RE::MagicTarget* a_this,
		RE::Actor* a_actor,
		RE::MagicItem* a_magicItem,
		const RE::Effect* a_effect) {
		(void)a_effect;
		if (!a_magicItem ||
			a_magicItem->effects.empty() ||
			!a_actor ||
			!a_this ||
			!a_this->MagicTargetIsActor()) {
			return false;
		}

		if (!fixBeneficial && !fixPoison && !fixSelfTarget) {
			return false;
		}

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
		for (auto it = effects.begin(); !isHostile && it != effects.end(); ++it) {
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
			return true;
		}
		else if (isPoison && isContact && fixPoison) {
			return true;
		}

		return false;
	}

	// The repository's commonlib does NOT have these vtables specified. If you want to use
	// a different commonlib as a base, they may be defined.

	bool Listener::Character::Install() {
		REL::Relocation<std::uintptr_t> VTABLE{ RE::Offset::PlayerCharacter::VTABLE_PlayerCharacterMagicTarget };
		_func = VTABLE.write_vfunc(0xB, Thunk);
		logger::info("    >Installed Character Magic Target VFunc hook."sv);
		return true;
	}

	bool Listener::Character::Thunk(RE::MagicTarget* a_this,
		RE::Actor* a_actor,
		RE::MagicItem* a_magicItem,
		const RE::Effect* a_effect) {
		auto* listener = Listener::GetSingleton();
		if (!listener) {
			logger::warn("Somehow failed to get internal listener for the Character Absorb Listener."sv);
			return _func(a_this, a_actor, a_magicItem, a_effect);
		}

		if (listener->IgnoreAbsorb(a_this, a_actor, a_magicItem, a_effect)) {
			return false;
		}

		return _func(a_this, a_actor, a_magicItem, a_effect);
	}

	bool Listener::Player::Install() {
		REL::Relocation<std::uintptr_t> VTABLE{ RE::Offset::Character::VTABLE_CharacterMagicTarget };
		_func = VTABLE.write_vfunc(0xB, Thunk);
		logger::info("    >Installed Player Magic Target VFunc hook."sv);
		return true;
	}

	bool Listener::Player::Thunk(RE::MagicTarget* a_this,
		RE::Actor* a_actor,
		RE::MagicItem* a_magicItem,
		const RE::Effect* a_effect) {
		auto* listener = Listener::GetSingleton();
		if (!listener) {
			logger::warn("Somehow failed to get internal listener for the Player Absorb Listener."sv);
			return _func(a_this, a_actor, a_magicItem, a_effect);
		}

		if (listener->IgnoreAbsorb(a_this, a_actor, a_magicItem, a_effect)) {
			return false;
		}

		return _func(a_this, a_actor, a_magicItem, a_effect);
	}
}