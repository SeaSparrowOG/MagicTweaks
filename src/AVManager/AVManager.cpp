#include "AVManager.h"

#include <Windows.h> //yell at Noah later
#include "API_ActorValueGenerator.h"

namespace AVManager
{
	RE::ActorValue GetMagickaShieldAV()
	{
		auto avg = REX::W32::GetModuleHandleA(AVG_API_SOURCE);
		if (avg == nullptr) {
			return RE::ActorValue::kVoiceRate;
		}

		RE::ActorValue av = AVG::ExtraValue(g_magickaShieldAV);
		if (av != RE::ActorValue::kNone) {
			return av;
		}
		return RE::ActorValue::kVoiceRate;
	}
}