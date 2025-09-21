#pragma once

namespace RE
{
	namespace Misc
	{
		static void DamageAV(Character* a_character,
			ACTOR_VALUE_MODIFIER a_modifier,
			ActorValue a_av,
			float a_amount,
			Actor* a_blame)
		{
			using func_t = decltype(&DamageAV);
			static REL::Relocation<func_t> func{ REL::ID(38468) };
			return func(a_character, a_modifier, a_av, a_amount, a_blame);
		}
	}
}