#pragma once

namespace Hooks::Fixes {
	class Listener : 
		public Utilities::Singleton::ISingleton<Listener>
	{
	public:
		bool Install();

	private:
		struct Character
		{
			static bool Install();
			static bool Thunk(RE::MagicTarget* a_this,
				RE::Actor* a_actor,
				RE::MagicItem* a_magicItem,
				const RE::Effect* a_effect);

			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		struct Player
		{
			static bool Install();
			static bool Thunk(RE::MagicTarget* a_this,
				RE::Actor* a_actor,
				RE::MagicItem* a_magicItem,
				const RE::Effect* a_effect);

			inline static REL::Relocation<decltype(&Thunk)> _func;
		};

		bool fixPoison{ false };
		bool fixSelfTarget{ false };
		bool fixBeneficial{ false };

		bool IgnoreAbsorb(RE::MagicTarget* a_this,
			RE::Actor* a_actor,
			RE::MagicItem* a_magicItem,
			const RE::Effect* a_effect);
	};

	bool Install();
}