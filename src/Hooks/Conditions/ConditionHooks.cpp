#include "ConditionHooks.h"

#include "ConditionManager/ConditionManager.h"
#include "RE/Offset.h"
#include "Settings/INI/INISettings.h"

namespace Hooks::Conditions
{
	bool Install() {
		if (!Settings::INI::GetSetting<bool>(Settings::INI::ADDITIONAL_CONDITIONS).value_or(false)) {
			return true;
		}
		logger::info("  >Installing condition-related hooks..."sv);
		return GetActorItemCountHook::Install();
	}

	bool GetActorItemCountHook::Install() {
		logger::info("    >Installing GetItemCount hook."sv);
		REL::Relocation<std::uintptr_t> target{ RE::Offset::TESObjectREFR::GetItemCount, 0xAE };
		if (!REL::make_pattern<"E8">().match(target.address())) {
			logger::critical("      >Failed to match expected pattern (E8)."sv);
			return false;
		}
		auto& trampoline = SKSE::GetTrampoline();
		_getItemCount = trampoline.write_call<5>(target.address(), GetItemCount);
		return true;
	}

	inline int32_t GetActorItemCountHook::GetItemCount(RE::InventoryChanges* a_inv, RE::TESBoundObject* a_obj)
	{
		int32_t out = 0;
		auto* owner = a_inv && a_inv->owner ? a_inv->owner : nullptr;
		auto* manager = ConditionManager::ConditionManager::GetSingleton();
		if (manager && owner && manager->SubstituteItemCount(a_obj, owner, out)) {
			return out;
		}
		return _getItemCount(a_inv, a_obj);
	}
}