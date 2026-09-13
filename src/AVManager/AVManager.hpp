#pragma once

#include "API_ActorValueGenerator.h"

namespace AVManager
{
	inline static constexpr std::string_view g_magickaShieldAV = "MagickaShield";

	inline RE::ActorValue GetMagickaShieldAV()
	{
		const auto av = AVG::ExtraValue{ g_magickaShieldAV }.get();

		return av != RE::ActorValue::kNone
			? av
			: RE::ActorValue::kVoiceRate;
	}
}