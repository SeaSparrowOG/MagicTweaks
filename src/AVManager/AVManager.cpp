#include "AVManager.h"

#include <Windows.h> //yell at Noah later
#include "API_ActorValueGenerator.h"

namespace AVManager
{
	RE::ActorValue GetMagickaShieldAV()
	{
		auto* intfc = AVG::API::RequestInterface<AVG::API::CurrentInterface>();
		if (intfc) {
			auto extraVal = AVG::ExtraValue(g_magickaShieldAV);
			auto av =  intfc->ResolveExtraValue(extraVal);
			if (av != RE::ActorValue::kNone && av != RE::ActorValue::kTotal) {
				return av;
			}
		}
		return RE::ActorValue::kVoiceRate;
	}
}