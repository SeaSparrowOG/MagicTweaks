#include "Hooks/hooks.h"

#include "Hooks/Fixes/Fixes.h"
#include "Tweaks/Tweaks.h"
#include "Settings/INI/INISettings.h"

namespace Hooks {
	bool Install() {
		SECTION_SEPARATOR;
		logger::info("Installing hooks..."sv);

		size_t allocSize = 0u;

		auto tweakReduction = Settings::INI::GetSetting<bool>(Settings::INI::TWEAK_REDUCTION);
		if (tweakReduction && tweakReduction.value()) {
			allocSize += 33u;
		}
		if (allocSize > 0u) {
			logger::info("  Allocating trampoline size {}"sv, allocSize);
			SKSE::AllocTrampoline(allocSize);
		}

		bool success = true;
		success &= Hooks::Fixes::InstallFixes();
		success &= Hooks::Tweaks::InstallTweaks();
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