#pragma once

namespace Events
{
	bool Install();

	class ExampleClass : public Utilities::Singleton::EventClass<ExampleClass, RE::TESMagicEffectApplyEvent> {
	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*) override;
	};
}