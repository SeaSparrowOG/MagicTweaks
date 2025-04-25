#include "Hooks.h"

#include "Fixes/Fixes.h"
#include "Tweaks/Tweaks.h"

#include "Settings/INISettings.h"

namespace Hooks 
{
	bool Install() {
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

		SKSE::AllocTrampoline(allocSize);
		return Fixes::Install() && Tweaks::Install();
	}
}