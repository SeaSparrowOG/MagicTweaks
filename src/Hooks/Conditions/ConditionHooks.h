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
			inline static REL::Relocation<decltype(GetItemCount)> _getItemCount;
		};
	}
}