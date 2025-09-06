#pragma once

namespace Events
{
	namespace HitEvent
	{
		bool RegisterHitEvent();

		class ConjurationHitManager : 
			public REX::Singleton<ConjurationHitManager>,
			public RE::BSTEventSink<RE::TESHitEvent>
		{
		public:
			bool Register();

		private:
			RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>*) override;

			float boundWeaponExp{ 1.15f };
			float minionExp{ 1.6f };
		};
	}
}