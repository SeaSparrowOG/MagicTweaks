#include "MagickaShield.h"

#include "AVManager/AVManager.h"
#include "RE/Offset.h"
#include "Settings/INI/INISettings.h"

#include <xbyak.h>

namespace Hooks::MagickaShield
{
	bool Hooks::MagickaShield::InstallMagickaShield()
	{
		logger::info("  Installing Magicka Shield..."sv);
		bool success = true;
		if (!Settings::INI::GetSetting<bool>(Settings::INI::MAGICKA_SHIELD).value_or(false)) {
			logger::info("    >User chose not to install Magicka Shield."sv);
			return success;
		}
		success &= MagickaShieldHandler::InstallHandler();
		return success;
	}

	bool MagickaShieldHandler::InstallHandler()
	{
		// Thank you FromSoft - er, I mean Nukem
		struct Patch : Xbyak::CodeGenerator
		{
			explicit Patch(uintptr_t OriginalFuncAddr, size_t OriginalByteLength)
			{
				for (size_t i = 0; i < OriginalByteLength; i++)
					db(*reinterpret_cast<uint8_t*>(OriginalFuncAddr + i));

				jmp(qword[rip]);
				dq(OriginalFuncAddr + OriginalByteLength);
			}
		};

		auto& trampoline = SKSE::GetTrampoline();
		const REL::Relocation<std::uintptr_t> target{ RE::Offset::Actor::DoDamage };

		if (REL::make_pattern<"E9">().match(target.address())) {
			logger::warn("    >Unexpected match - E9. Writing branch instead."sv);
			_hitActor = trampoline.write_branch<5>(target.address(), ApplyMagickaShield);
			return true;
		}

		Patch p(target.address(), 5);
		p.ready();

		trampoline.write_branch<5>(target.address(), ApplyMagickaShield);

		auto alloc = trampoline.allocate(p.getSize());
		memcpy(alloc, p.getCode(), p.getSize());

		_hitActor = reinterpret_cast<uintptr_t>(alloc);
		return true;
	}

	bool MagickaShieldHandler::ApplyMagickaShield(RE::Actor* a_this, float a_damage, RE::Actor* a_blame, bool a_adjustDifficulty) {
		if (!a_this) {
			return _hitActor(a_this, a_damage, a_blame, a_adjustDifficulty);
		}

		auto av = AVManager::GetMagickaShieldAV();
		const float shieldPower = std::clamp(a_this->GetActorValue(av), 0.0f, 100.0f) / 100.0f;
		const float magicka = a_this->GetActorValue(RE::ActorValue::kMagicka);
		const float blocked = std::clamp(shieldPower * a_damage, 0.0f, magicka);
		a_damage = a_damage - blocked;

		if (blocked > 0.0f) {
			a_this->DamageActorValue(RE::ActorValue::kMagicka, blocked);
		}
		return _hitActor(a_this, a_damage, a_blame, a_adjustDifficulty);
	}
}