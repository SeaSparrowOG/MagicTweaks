#include "MagicCasterHooks.h"

#include "BoundEffectManager/BoundEffectManager.h"

namespace Hooks::MagicCaster
{
	bool Install() {
		logger::info("  >Installing Magic Caster Hooks..."sv);
		CheckCastHook::Install();
		return true;
	}

	void CheckCastHook::Install() {
		REL::Relocation<std::uintptr_t> ActorMagicCasterVTBL{ RE::ActorMagicCaster::VTABLE[0] };
		_checkCast = ActorMagicCasterVTBL.write_vfunc(0xA, &CheckCast);
	}

	bool CheckCastHook::CheckCast(RE::ActorMagicCaster* a_this,
		RE::MagicItem* a_spell,
		bool a_dualCast,
		float* a_effectStrength,
		RE::MagicSystem::CannotCastReason* a_reason,
		bool a_useBaseValueForCost)
	{
		bool canCast = _checkCast(a_this, a_spell, a_dualCast, a_effectStrength, a_reason, a_useBaseValueForCost);
		auto* boundEffectManager = BoundEffectManager::BoundEffectManager::GetSingleton();
		if (canCast && boundEffectManager) {
			canCast = boundEffectManager->CanCastSpell(a_spell, a_dualCast);
			if (!canCast) {
				*a_reason = RE::MagicSystem::CannotCastReason::kOK;
				RE::SendHUDMessage::ShowHUDMessage("You cannot Bind this spell.");
			}
		}
		return canCast;
	}
}