#include "DynamicDescription.h"

#include "ConditionParser/ConditionUtilities.h"
#include "Settings/INI/INISettings.h"

namespace Hooks::DynamicDescription
{
	bool InstallDynamicDescriptionPatch() {
		bool installDynamicDescription = Settings::INI::GetSetting<bool>(Settings::INI::DYNAMIC_SPELL_DESCRIPTIONS).value_or(false);
		if (!installDynamicDescription) {
			return true;
		}

		bool success = true;
		success &= SpellDescriptionPatch::InstallSpellDescriptionPatch();
		success &= SpellTomeDescriptionPatch::InstallSpellTomeDescriptionPatch();
		success &= EnchantmentDescriptionPatch::InstallEnchantmentDescriptionPatch();
		return success;
	}

	bool SpellDescriptionPatch::InstallSpellDescriptionPatch() {
		REL::Relocation<std::uintptr_t> target{ REL::ID(51898), 0x5DC };
		if (!REL::make_pattern<"E8">().match(target.address())) {
			logger::critical("    >Failed to validate the hook pattern."sv);
			return false;
		}
		auto& trampoline = SKSE::GetTrampoline();
		_getSpellDescription = trampoline.write_call<5>(target.address(), &GetSpellDescription);
		return true;
	}

	void SpellDescriptionPatch::GetSpellDescription(RE::ItemCard* a1, 
		RE::SpellItem* a2, 
		RE::BSString& a_out)
	{
		_getSpellDescription(a1, a2, a_out);
		auto proposedDescription = ConditionParser::GetFormattedDescription(a2);
		if (!proposedDescription.empty()) {
			a_out = proposedDescription;
		}
	}

	bool SpellTomeDescriptionPatch::InstallSpellTomeDescriptionPatch() {
		REL::Relocation<std::uintptr_t> target{ REL::ID(51897), 0xF5C };
		if (!REL::make_pattern<"E8">().match(target.address())) {
			logger::critical("    >Failed to validate the hook pattern."sv);
			return false;
		}
		auto& trampoline = SKSE::GetTrampoline();
		_getSpellTomeDescription = trampoline.write_call<5>(target.address(), &GetSpellTomeDescription);
		return true;
	}

	void SpellTomeDescriptionPatch::GetSpellTomeDescription(RE::ItemCard* a1,
		RE::SpellItem* a2,
		RE::BSString& a_out)
	{
		_getSpellTomeDescription(a1, a2, a_out);
		auto proposedDescription = ConditionParser::GetFormattedDescription(a2);
		if (!proposedDescription.empty()) {
			a_out = proposedDescription;
		}
	}

	bool EnchantmentDescriptionPatch::InstallEnchantmentDescriptionPatch() {
		REL::Relocation<std::uintptr_t> target{ REL::ID(51897), 0x1377 };
		if (!REL::make_pattern<"E8">().match(target.address())) {
			logger::critical("    >Failed to validate the hook pattern."sv);
			return false;
		}
		auto& trampoline = SKSE::GetTrampoline();
		_getEnchantmentDescription = trampoline.write_call<5>(target.address(), &GetEnchantmentDescription);
		return true;
	}

	void EnchantmentDescriptionPatch::GetEnchantmentDescription(RE::ItemCard* a1,
		RE::SpellItem* a2,
		RE::BSString& a_out)
	{
		_getEnchantmentDescription(a1, a2, a_out);
		auto proposedDescription = ConditionParser::GetFormattedDescription(a2);
		if (!proposedDescription.empty()) {
			a_out = proposedDescription;
		}
	}
}