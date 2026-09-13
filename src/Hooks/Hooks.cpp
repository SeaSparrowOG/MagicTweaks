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
		REX::INFO("Installing hooks..."sv);

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
			REX::ERROR("Failed to install all hooks, aborting load..."sv);
			return false;
		}

		return true;
	}

	bool ReadSettings() {
		bool success = true;

		auto* effectDispeler = Tweaks::SpellDispeler::GetSingleton();
		if (!effectDispeler) {
			REX::CRITICAL("Failed to get internal effect dispeler manager."sv);
			return false;
		}
		success &= effectDispeler->LoadJSONSettings();

		return success;
	}
}