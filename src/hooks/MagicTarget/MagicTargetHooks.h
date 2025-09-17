#pragma once

namespace Hooks
{
	namespace MagicTarget
	{
		bool Install();

		struct EffectAddedHook
		{
			static void Install();
			inline static void EffectAdded(RE::MagicTarget* a_this, RE::ActiveEffect* a_effect);
			inline static REL::Relocation<decltype(EffectAdded)> _effectAdded;
		};

		struct EffectRemovedHook
		{
			static void Install();
			inline static void EffectRemoved(RE::MagicTarget* a_this, RE::ActiveEffect* a_effect);
			inline static REL::Relocation<decltype(EffectRemoved)> _effectRemoved;
		};
	}
}