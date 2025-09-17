#pragma once

namespace Hooks
{
	namespace MagicCaster
	{
		bool Install();

		// Allows to send a "can't cast" message if the player cannot bind the spell.
		struct CheckCastHook
		{
			static void Install();
			inline static bool CheckCast(RE::ActorMagicCaster* a_this,
				RE::MagicItem* a_spell, 
				bool a_dualCast,
				float* a_effectStrength,
				RE::MagicSystem::CannotCastReason* a_reason,
				bool a_useBaseValueForCost);
			inline static REL::Relocation<decltype(CheckCast)> _checkCast;
		};
	}
}