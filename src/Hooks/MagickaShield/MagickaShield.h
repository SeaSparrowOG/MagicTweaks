#pragma once

namespace Hooks
{
	namespace MagickaShield
	{
		bool InstallMagickaShield();

		struct MagickaShieldHandler
		{
			static bool InstallHandler();
			static bool ApplyMagickaShield(RE::Actor* a_this, float a_damage, RE::Actor* a_blame, bool a_adjustDifficulty);
			inline static REL::Relocation<decltype(ApplyMagickaShield)> _hitActor;
		};
	}
}