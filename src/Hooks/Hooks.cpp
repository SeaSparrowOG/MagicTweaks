#include "Hooks.h"

#include "Fixes/Fixes.h"
#include "Tweaks/Tweaks.h"

namespace Hooks 
{
	bool Install() {
		return Fixes::Install() && Tweaks::Install();
	}
}