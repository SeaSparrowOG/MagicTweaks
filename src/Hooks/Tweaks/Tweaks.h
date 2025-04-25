#pragma once

namespace Hooks::Tweaks {
	class Listener :
		public Utilities::Singleton::ISingleton<Listener>
	{
	public:
		bool Install();

	private:
		struct BoundEffect
		{
			static bool Install();
			static void Thunk(RE::ActiveEffect* a_this, float a_delta);

			inline static REL::Relocation<decltype(&Thunk)> _func;
			inline static size_t offset{ 0x4 };
		};

		bool extendInDialogue{ false };

		bool ShouldUpdate(RE::ActiveEffect* a_effect);
	};

	bool Install();
}