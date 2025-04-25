#pragma once

namespace Hooks::Tweaks {
	class EffectExtender :
		public Utilities::Singleton::ISingleton<EffectExtender>
	{
	public:
		bool Install();
		void ExtendEffectIfNecessary(RE::ActiveEffect* a_effect, float a_extension);

	private:
		bool extendInDialogue{ false };
	};

	bool Install();
}