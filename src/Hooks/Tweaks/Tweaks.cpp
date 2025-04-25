#include "Tweaks.h"

#include "RE/Offset.h"
#include "Settings/INISettings.h"

namespace Hooks::Tweaks
{
	bool Install() {
		auto* listener = Listener::GetSingleton();
		if (!listener) {
			logger::critical("Failed to get Tweaks Listener."sv);
			return false;
		}
		return listener->Install();
	}

	bool Listener::Install() {
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			logger::info("Failed to fetch INI settings holder for Fixes."sv);
			return false;
		}

		auto extendInDialogueRaw = iniHolder->GetStoredSetting<bool>("Tweaks|bExtendEffectsInDialogue");
		extendInDialogue = extendInDialogueRaw.has_value() ? extendInDialogueRaw.value() : false;
		if (!extendInDialogueRaw.has_value()) {
			logger::warn("  >Extend In Dialogue tweak setting somehow not specified in the INI, treating as false."sv);
		}

		bool success = true;
		auto patchCloaks = iniHolder->GetStoredSetting<bool>("Tweaks|bTweakCloaks");
		bool installCloakPatch = patchCloaks.has_value() ? patchCloaks.value() : false;
		if (installCloakPatch && !CloakEffect::Install()) {
			logger::critical("Failed to install Cloak patch."sv);
			success = false;
		}
		if (!patchCloaks.has_value()) {
			logger::warn("  >Cloak tweak setting somehow not specified in the INI, treating as false."sv);
		}

		auto patchSummonEffects = iniHolder->GetStoredSetting<bool>("Tweaks|bTweakSummons");
		bool installSummonPatch = patchSummonEffects.has_value() ? patchSummonEffects.value() : false;
		if (installSummonPatch && !SummonEffect::Install()) {
			logger::critical("Failed to install Summon patch."sv);
			success = false;
		}
		if (!patchCloaks.has_value()) {
			logger::warn("  >Summon tweak setting somehow not specified in the INI, treating as false."sv);
		}

		auto patchBoundWeapons = iniHolder->GetStoredSetting<bool>("Tweaks|bTweakBoundWeapons");
		bool installBoundWeaponPatch = patchBoundWeapons.has_value() ? patchBoundWeapons.value() : false;
		if (installBoundWeaponPatch && !BoundEffect::Install()) {
			logger::critical("Failed to install Bound Weapon patch."sv);
			success = false;
		}
		if (!patchBoundWeapons.has_value()) {
			logger::warn("  >Bound weapon tweak setting somehow not specified in the INI, treating as false."sv);
		}

		return success;
	}

	bool Listener::ShouldUpdate(RE::ActiveEffect* a_effect) {
		using ActiveEffectFlag = RE::EffectSetting::EffectSettingData::Flag;
		auto* ui = RE::UI::GetSingleton();
		if (!ui || !a_effect) {
			return true;
		}

		const auto* caster = a_effect->GetCasterActor().get();
		const auto* target = a_effect->GetTargetActor();
		const auto* base = a_effect->effect ? a_effect->effect->baseEffect : nullptr;
		if (!base || !caster || !target || target != caster) {
			return true;
		}

		if (base->data.flags.any(ActiveEffectFlag::kDetrimental,
			ActiveEffectFlag::kHostile, ActiveEffectFlag::kNoDuration)) {
			return true;
		}

		if (extendInDialogue && ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
			return false;
		}

		return true;
	}

	bool Listener::BoundEffect::Install() {
		REL::Relocation<std::uintptr_t> VTABLE{ RE::Offset::BoundItemEffect::VTABLE_BoundItemEffect };
		_func = VTABLE.write_vfunc(offset, Thunk);
		return true;
	}

	void Listener::BoundEffect::Thunk(RE::ActiveEffect* a_this, 
		float a_delta) {
		auto* listener = Listener::GetSingleton();

		if (listener && !listener->ShouldUpdate(a_this)) {
			a_this->elapsedSeconds -= a_delta;
		}
	}

	bool Listener::SummonEffect::Install() {
		REL::Relocation<std::uintptr_t> VTABLE{ RE::Offset::SummonCreatureEffect::VTABLE_SummonCreatureEffect };
		_func = VTABLE.write_vfunc(offset, Thunk);
		return true;
	}

	void Listener::SummonEffect::Thunk(RE::ActiveEffect* a_this, 
		float a_delta) {
		_func(a_this, a_delta);

		auto* listener = Listener::GetSingleton();
		if (listener && !listener->ShouldUpdate(a_this)) {
			a_this->elapsedSeconds -= a_delta;
		}
	}

	bool Listener::CloakEffect::Install() {
		REL::Relocation<std::uintptr_t> VTABLE{ RE::Offset::CloakEffect::VTABLE_CloakEffect };
		_func = VTABLE.write_vfunc(offset, Thunk);
		return true;
	}

	void Listener::CloakEffect::Thunk(RE::ActiveEffect* a_this, 
		float a_delta) {
		_func(a_this, a_delta);

		auto* listener = Listener::GetSingleton();
		if (listener && !listener->ShouldUpdate(a_this)) {
			a_this->elapsedSeconds -= a_delta;
		}
	}
}