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
		REX::INFO("  >Installing condition-related hooks..."sv);
		return GetActorItemCountHook::Install();
	}

	bool GetActorItemCountHook::Install() {
		REX::INFO("    >Installing GetItemCount hook."sv);
		REL::Relocation<std::uintptr_t> target{ RE::Offset::TESObjectREFR::GetItemCount, 0xAE };
		REL::Relocation<std::uintptr_t> papyrusTargetSingle{ RE::Offset::Papyrus::GetItemCount, 0x8A };
		REL::Relocation<std::uintptr_t> papyrusTargetMultiple{ RE::Offset::Papyrus::GetItemCount, 0xEC };

		bool success = true;

		if (!REL::Pattern<"E8">().match(papyrusTargetSingle.address())) {
			success = false;
			REX::CRITICAL("      >Failed to match the expected pattern for Single Papyrus GetItemCount."sv);
		}
		if (!REL::Pattern<"E8">().match(papyrusTargetMultiple.address())) {
			success = false;
			REX::CRITICAL("      >Failed to match the expected pattern for Multiple Papyrus GetItemCount."sv);
		}
		if (!REL::Pattern<"E8">().match(target.address())) {
			success = false;
			REX::CRITICAL("      >Failed to match the expected pattern for GetItemCount."sv);
		}
		if (!success) {
			return false;
		}

		auto& trampoline = REL::GetTrampoline();
		_getItemCount = trampoline.write_call<5>(target.address(), GetItemCount);
		_Papyrus_GetItemCount_SingleItem = trampoline.write_call<5>(papyrusTargetSingle.address(), Papyrus_GetItemCount_SingleItem);
		_Papyrus_GetItemCount_MultipleForms = trampoline.write_call<5>(papyrusTargetMultiple.address(), Papyrus_GetItemCount_MultipleForms);
		return success;
	}

	[[nodiscard]]
	static std::optional<int32_t> get_alt_count(RE::InventoryChanges* a_inv, RE::TESBoundObject* a_obj) {
		static auto* manager = ConditionManager::ConditionManager::GetSingleton();
		if (!manager) {
			return std::nullopt;
		}

		auto* owner = a_inv && a_inv->owner ? a_inv->owner : nullptr;
		if (!owner || !a_obj) {
			return std::nullopt;
		}
		return manager->SubstituteItemCount(a_obj, owner);
	}

	inline int32_t GetActorItemCountHook::GetItemCount(RE::InventoryChanges* a_inv, RE::TESBoundObject* a_obj)
	{
		auto altOut = get_alt_count(a_inv, a_obj);
		return altOut.has_value() ? altOut.value() : _getItemCount(a_inv, a_obj);
	}

    inline int32_t GetActorItemCountHook::Papyrus_GetItemCount_SingleItem(RE::InventoryChanges *a_inv, 
		RE::TESBoundObject *a_obj)
    {
        auto altOut = get_alt_count(a_inv, a_obj);
		return altOut.has_value() ? altOut.value() : _Papyrus_GetItemCount_SingleItem(a_inv, a_obj);
    }

    inline int32_t GetActorItemCountHook::Papyrus_GetItemCount_MultipleForms(RE::InventoryChanges *a_inv, 
		RE::TESBoundObject *a_obj)
    {
        auto altOut = get_alt_count(a_inv, a_obj);
		return altOut.has_value() ? altOut.value() : _Papyrus_GetItemCount_MultipleForms(a_inv, a_obj);
    }
}