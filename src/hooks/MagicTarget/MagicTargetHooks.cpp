#include "MagicTargetHooks.h"

#include "BoundEffectManager/BoundEffectManager.h"

namespace Hooks::MagicTarget
{
	bool Install() {
		logger::info("  >Installing Magic Target Hooks..."sv);
		EffectAddedHook::Install();
		EffectRemovedHook::Install();
		return true;
	}

	void EffectAddedHook::Install() {
		REL::Relocation<std::uintptr_t> ActorMagicTargetVTBL{ RE::PlayerCharacter::VTABLE[4] };
		_effectAdded = ActorMagicTargetVTBL.write_vfunc(0x8, &EffectAdded);
	}

	void EffectRemovedHook::Install() {
		REL::Relocation<std::uintptr_t> ActorMagicTargetVTBL{ RE::PlayerCharacter::VTABLE[4] };
		_effectRemoved = ActorMagicTargetVTBL.write_vfunc(0x9, &EffectRemoved);
	}

	void EffectRemovedHook::EffectRemoved(RE::MagicTarget* a_this, RE::ActiveEffect* a_effect) {
		_effectRemoved(a_this, a_effect);
		auto* boundEffectManager = BoundEffectManager::BoundEffectManager::GetSingleton();
		if (boundEffectManager) {
			boundEffectManager->ProcessEffectRemoved(a_effect);
		}
	}

	void EffectAddedHook::EffectAdded(RE::MagicTarget* a_this, RE::ActiveEffect* a_effect) {
		_effectAdded(a_this, a_effect);
		auto* boundEffectManager = BoundEffectManager::BoundEffectManager::GetSingleton();
		if (boundEffectManager) {
			boundEffectManager->ProcessEffectAdded(a_effect);
		}
	}
}