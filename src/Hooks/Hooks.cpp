#include "Hooks.h"

#include "Fixes/Fixes.h"
#include "Tweaks/Tweaks.h"

// See comment above commented out code.
// #include "Settings/INISettings.h"

namespace Hooks 
{
	bool Install() {
		// Needed only for the Silencer tweak, that didn't quite meet my standards.
		/*
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			logger::critical("Failed to get ini holder."sv);
			return false;
		}

		auto installSielencerRaw = iniHolder->GetStoredSetting<bool>("Tweaks|bSuppressMagicComments");
		bool allocateSilencer = installSielencerRaw.has_value() ? installSielencerRaw.value() : false;
		if (!installSielencerRaw.has_value()) {
			logger::warn("  >Failed to get the Tweaks|bSuppressMagicComments setting from the INI."sv);
		}

		size_t allocSize = 0;
		allocSize += allocateSilencer ? 14 : 0;
		if (allocSize > 0) {
			SKSE::AllocTrampoline(allocSize);
		}
		*/

		size_t allocSize = 128;
		SKSE::AllocTrampoline(allocSize);

		logger::info("Installing necessary hooks..."sv);
		return Fixes::Install() && Tweaks::Install();
	}
}