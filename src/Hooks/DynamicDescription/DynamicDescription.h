#pragma once

namespace Hooks
{
	namespace DynamicDescription
	{
		bool InstallDynamicDescriptionPatch();

		struct SpellDescriptionPatch
		{
			static bool InstallSpellDescriptionPatch();
			static void GetSpellDescription(RE::ItemCard* a1, RE::SpellItem* a2, RE::BSString& a_out);
			inline static REL::Relocation<decltype(GetSpellDescription)> _getSpellDescription;
		};

		struct SpellTomeDescriptionPatch
		{
			static bool InstallSpellTomeDescriptionPatch();
			static void GetSpellTomeDescription(RE::ItemCard* a1, RE::SpellItem* a2, RE::BSString& a_out);
			inline static REL::Relocation<decltype(GetSpellTomeDescription)> _getSpellTomeDescription;
		};

		struct EnchantmentDescriptionPatch
		{
			static bool InstallEnchantmentDescriptionPatch();
			static void GetEnchantmentDescription(RE::ItemCard* a1, RE::SpellItem* a2, RE::BSString& a_out);
			inline static REL::Relocation<decltype(GetEnchantmentDescription)> _getEnchantmentDescription;
		};
	}
}