#pragma once

namespace Hooks
{
	namespace PlayerCharacter
	{
		bool Install();

		struct UpdateHook
		{
			static void Install();
			inline static void Update(RE::PlayerCharacter* a_this, float a_delta);
			inline static REL::Relocation<decltype(Update)> _update;
		};
	}
}