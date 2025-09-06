#include "Events.h"

#include "HitEvent/HitEvent.h"

namespace Events
{
	bool Register() {
		return HitEvent::RegisterHitEvent();
	}
}