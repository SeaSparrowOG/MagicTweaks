#pragma once

namespace Events
{
	namespace CombatEvent
	{
		bool RegisterCombatEvent();

		class CombatChangeManager :
			public REX::Singleton<CombatChangeManager>,
			public RE::BSTEventSink<RE::TESCombatEvent>
		{
		public:
			bool RegisterCombatListener();

		private:
			RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>*) override;
			bool outOfCombat{ true };
		};
	}
}