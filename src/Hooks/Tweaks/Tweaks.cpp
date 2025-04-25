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
		(void)a_effect;
		auto* ui = RE::UI::GetSingleton();

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
		_func(a_this, a_delta);

		auto* listener = Listener::GetSingleton();
		if (!listener) {
			logger::error("Failed to get Tweak Listener."sv);
			return;
		}

		if (!listener->ShouldUpdate(a_this)) {
			a_this->elapsedSeconds -= a_delta;
		}
	}
}