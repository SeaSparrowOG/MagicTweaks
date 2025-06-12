#pragma once

#include "Events/Events.h"

namespace Events
{
	namespace HitEvent
	{
		class ConjurationHitManager : public EventClass<ConjurationHitManager, RE::TESHitEvent>
		{
		public:
			inline static std::string setting{ "Tweaks|bEarnConjurationHitExp" };
			inline static std::string name{ "Conjuration Hit Listener" };

		private:
			RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>*) override;

			float boundWeaponExp{ 1.15f };
			float minionExp{ 1.6f };
		};
	}
}