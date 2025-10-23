#pragma once

namespace Hooks 
{
	namespace Fixes
	{
		bool InstallFixes();

		struct Character
		{
			static bool InstallCharacterFixes();
			static bool CharacterThunk(RE::MagicTarget* a_this,
				RE::Actor* a_actor,
				RE::MagicItem* a_magicItem,
				const RE::Effect* a_effect);

			inline static REL::Relocation<decltype(&CharacterThunk)> _func;
		};

		struct Player
		{
			static bool InstallPlayerFixes();
			static bool PlayerThunk(RE::MagicTarget* a_this,
				RE::Actor* a_actor,
				RE::MagicItem* a_magicItem,
				const RE::Effect* a_effect);

			inline static REL::Relocation<decltype(&PlayerThunk)> _func;
		};

		struct CloakArchetypeFix
		{
			static bool InstallCloakFix();
			static void ResetCloakEffect(RE::ActiveEffect* a_effect);

			inline static REL::Relocation<decltype(&ResetCloakEffect)> _func;
		};
	}
}