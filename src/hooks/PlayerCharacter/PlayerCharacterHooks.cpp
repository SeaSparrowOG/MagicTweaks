#include "PlayerCharacterHooks.h"

#include "BoundEffectManager/BoundEffectManager.h"

namespace Hooks::PlayerCharacter
{
	bool Install() {
		logger::info("  >Installing Player Character Hooks..."sv);
		UpdateHook::Install();
		return true;
	}

	void UpdateHook::Install() {
		REL::Relocation<std::uintptr_t> PlayerCharacterVTBL{ RE::PlayerCharacter::VTABLE[0] };
		_update = PlayerCharacterVTBL.write_vfunc(0xAD, &Update);
	}

	void UpdateHook::Update(RE::PlayerCharacter* a_this, float a_delta) {
		_update(a_this, a_delta);
		auto* boundEffectManager = BoundEffectManager::BoundEffectManager::GetSingleton();
		if (boundEffectManager) {
			boundEffectManager->UpdateTimePassed(a_delta);
		}
	}
}