#include "Events.h"

#include "HitEvent/HitEvent.h"

namespace Events
{
	bool InitializeListeners() {
		logger::info("Initializing Events if necessary..."sv);

		bool nominal = true;
		auto* conjurationHitManager = HitEvent::ConjurationHitManager::GetSingleton();
		if (!conjurationHitManager) {
			logger::critical("  >Failed to get the Conjuration Hit Manager."sv);
			nominal = false;
		}

		if (!nominal) {
			return false;
		}

		return conjurationHitManager->RegisterListener();
	}
}