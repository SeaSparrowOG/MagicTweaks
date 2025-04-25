#pragma once

#include "RE/Offset.h"

namespace Hooks::Tweaks::Archetypes
{
	struct SummonEffect
	{
		static bool Install();
		static void Thunk(RE::ActiveEffect* a_this, float a_delta);

		inline static REL::Relocation<decltype(&Thunk)> _func;

		inline static size_t offset{ 0x4 };
		inline static std::string setting{ "Tweaks|bTweakSummons" };
	};
}