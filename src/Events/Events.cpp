#include "events.h"

namespace Events
{
	bool Install() {
		logger::info("==========================================================");
		logger::info("Registering Event Listeners..."sv);
		auto* listenerSingleton = ExampleClass::GetSingleton();
		if (!listenerSingleton) {
			logger::error("  >Failed to grab ExampleClass listener."sv);
			return false;
		}

		return listenerSingleton->RegisterListener();
	}

	RE::BSEventNotifyControl ExampleClass::ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*)
	{
		using control = RE::BSEventNotifyControl;
		if (!a_event) {
			return control::kContinue;
		}
		return control::kContinue;
	}
}