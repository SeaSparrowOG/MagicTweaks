#include "Hooks/hooks.h"

#include "Conditions/ConditionHooks.h"
#include "DynamicDescription/DynamicDescription.h"
#include "MagicCaster/MagicCasterHooks.h"
#include "MagickaShield/MagickaShield.h"
#include "MagicTarget/MagicTargetHooks.h"
#include "PlayerCharacter/PlayerCharacterHooks.h"
#include "Hooks/Fixes/Fixes.h"
#include "Tweaks/Tweaks.h"
#include "Settings/INI/INISettings.h"

namespace Hooks {
	bool Install() {
		logger::info("Installing hooks..."sv);

		size_t allocSize = 0u;

		auto tweakReduction = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_REDUCTION);
		if (tweakReduction && tweakReduction.value()) {
			allocSize += 33u; // 2 * 14 + 5
		}
		bool installDynamicDescription = Settings::INI::GetSetting<bool>(Settings::INI::DYNAMIC_SPELL_DESCRIPTIONS).value_or(false);
		if (installDynamicDescription) {
			allocSize += 42u; // 3 * 14
		}
		bool installConditionPatch = Settings::INI::GetSetting<bool>(Settings::INI::ADDITIONAL_CONDITIONS).value_or(false);
		if (installConditionPatch) {
			allocSize += 14u; // 1 * 14
		}
		bool installMagickaShield = Settings::INI::GetSetting<bool>(Settings::INI::MAGICKA_SHIELD).value_or(false);
		if (installMagickaShield) {
			allocSize += 33u; // 2 * 14 + 5
		}
		bool installCloakFix = Settings::INI::GetSetting<bool>(Settings::INI::FIX_CLOAKS).value_or(false);
		if (installCloakFix) {
			allocSize += 20; // 7 + 7 + 2 + 2 + 2
		}

		if (allocSize > 0u) {
			logger::info("  Allocating trampoline size {}"sv, allocSize);
			SKSE::AllocTrampoline(allocSize);
		}

		bool success = true;
		success &= Hooks::Conditions::Install();
		success &= Hooks::DynamicDescription::InstallDynamicDescriptionPatch();
		success &= Hooks::PlayerCharacter::Install();
		success &= Hooks::MagicCaster::Install();
		success &= Hooks::MagicTarget::Install();
		success &= Hooks::Fixes::InstallFixes();
		success &= Hooks::Tweaks::InstallTweaks();
		success &= Hooks::MagickaShield::InstallMagickaShield();
		if (!success) {
			logger::error("Failed to install all hooks, aborting load..."sv);
			return false;
		}

		return true;
	}

	bool ReadSettings() {
		bool success = true;

		auto* effectDispeler = Tweaks::SpellDispeler::GetSingleton();
		if (!effectDispeler) {
			logger::critical("Failed to get internal effect dispeler manager."sv);
			return false;
		}
		success &= effectDispeler->LoadJSONSettings();

		return success;
	}
}