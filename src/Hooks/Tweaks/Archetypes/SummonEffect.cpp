#include "SummonEffect.h"

#include "Hooks/Tweaks/Tweaks.h"
#include "Settings/INISettings.h"

namespace Hooks::Tweaks::Archetypes
{
	bool SummonEffect::Install() {
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			logger::critical("Failed to fetch ini settings holder."sv);
			return false;
		}

		auto installRaw = iniHolder->GetStoredSetting<bool>(setting);
		bool install = installRaw.has_value() ? installRaw.value() : false;
		if (!installRaw.has_value()) {
			logger::warn("  >Setting {} not found in ini settings, treating as false.", setting);
		}
		if (!install) {
			return true;
		}

		REL::Relocation<std::uintptr_t> VTABLE{ RE::Offset::SummonCreatureEffect::VTABLE_SummonCreatureEffect };
		_func = VTABLE.write_vfunc(offset, Thunk);
		return true;
	}

	void SummonEffect::Thunk(RE::ActiveEffect* a_this,
		float a_delta) {
		_func(a_this, a_delta);

		auto* extenderSingleton = EffectExtender::GetSingleton();
		if (extenderSingleton) {
			extenderSingleton->ExtendEffectIfNecessary(a_this, a_delta);
		}
	}
}