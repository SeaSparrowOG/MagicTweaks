#pragma once

namespace Hooks
{
	namespace Conditions
	{
		bool Install();

		struct GetActorItemCountHook
		{
			inline static bool Install();
			inline static int32_t GetItemCount(RE::InventoryChanges* a_inv, RE::TESBoundObject* a_obj);
			// Not 100% on this. Later on this function we get a second GetCountDelta, but it *appears* to be handling formlists... or keywords?
			inline static int32_t Papyrus_GetItemCount_SingleItem(RE::InventoryChanges* a_inv, RE::TESBoundObject* a_obj);
			inline static int32_t Papyrus_GetItemCount_MultipleForms(RE::InventoryChanges* a_inv, RE::TESBoundObject* a_obj);

			inline static REL::Relocation<decltype(GetItemCount)> _getItemCount;
			inline static REL::Relocation<decltype(GetItemCount)> _Papyrus_GetItemCount_SingleItem;
			inline static REL::Relocation<decltype(GetItemCount)> _Papyrus_GetItemCount_MultipleForms;
		};
	}
}