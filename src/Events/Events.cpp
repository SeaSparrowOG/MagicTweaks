#include "Events.h"

#include "CombatEvent/CombatEvent.h"
#include "HitEvent/HitEvent.h"

namespace Events
{
	bool Register() {
		bool success = HitEvent::RegisterHitEvent();
		success &= CombatEvent::RegisterCombatEvent();

		return success;
	}
}