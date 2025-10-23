#include "Fixes.h"

#include "RE/Offset.h"
#include "Settings/INI/INISettings.h"

#include <xbyak.h>

namespace
{
	bool AllowAbsorb(RE::MagicTarget* a_this,
		RE::Actor* a_actor,
		RE::MagicItem* a_magicItem,
		const RE::Effect* a_effect)
	{
		(void)a_effect;
		if (!a_magicItem ||
			a_magicItem->effects.empty() ||
			!a_actor ||
			!a_this ||
			!a_this->MagicTargetIsActor()) {
			return true;
		}

		bool fixBeneficial = Settings::INI::GetSetting<bool>(Settings::INI::FIX_BENEFICIAL).value_or(false);
		bool fixPoison = Settings::INI::GetSetting<bool>(Settings::INI::FIX_POISON).value_or(false);
		bool fixSelfTarget = Settings::INI::GetSetting<bool>(Settings::INI::FIX_SELF).value_or(false);

		using EffectFlag = RE::EffectSetting::EffectSettingData::Flag;
		using CastFlag = RE::MagicSystem::CastingType;
		using DeliveryFlag = RE::MagicSystem::Delivery;

		const auto* targetActor = static_cast<RE::Actor*>(a_this);
		if (!targetActor) {
			return true;
		}

		if (fixSelfTarget && a_actor == targetActor) {
			return true;
		}

		const auto& effects = a_magicItem->effects;
		bool isHostile = false;
		bool isPoison = false;
		bool isContact = a_magicItem->GetDelivery() == DeliveryFlag::kTouch;
		for (auto it = effects.begin(); it != effects.end(); ++it) {
			const auto* effect = *it;
			const auto* base = effect ? effect->baseEffect : nullptr;
			if (!base) {
				continue;
			}

			const auto& effectData = base->data;

			if (effectData.flags.any(EffectFlag::kHostile)) {
				isHostile = true;
			}
			if (effectData.resistVariable == RE::ActorValue::kPoisonResist) {
				isPoison = true;
			}
		}

		if (!isHostile && fixBeneficial) {
			return false;
		}
		else if (isPoison && isContact && fixPoison) {
			return false;
		}

		return true;
	}
}

namespace Hooks {
	namespace Fixes {
		bool InstallFixes() {
			auto installPoison = Settings::INI::GetSetting<bool>(Settings::INI::FIX_POISON);
			auto installSelf = Settings::INI::GetSetting<bool>(Settings::INI::FIX_SELF);
			auto installBeneficial = Settings::INI::GetSetting<bool>(Settings::INI::FIX_BENEFICIAL);

			bool atLeastOnePatch = installPoison.value_or(false) &&
				installSelf.value_or(false) &&
				installBeneficial.value_or(false);

			if (!atLeastOnePatch) {
				return true;
			}

			bool success = true;
			success &= Character::InstallCharacterFixes();
			success &= Player::InstallPlayerFixes();
			success &= CloakArchetypeFix::InstallCloakFix();

			return success;
		}

		bool Character::InstallCharacterFixes() {
			REL::Relocation<std::uintptr_t> VTABLE{ RE::Character::VTABLE[4] };
			_func = VTABLE.write_vfunc(0xB, CharacterThunk);
			logger::info("    >Installed Character Magic Target VFunc hook."sv);
			return true;
		}

		bool Player::InstallPlayerFixes() {
			REL::Relocation<std::uintptr_t> VTABLE{ RE::PlayerCharacter::VTABLE[4] };
			_func = VTABLE.write_vfunc(0xB, PlayerThunk);
			logger::info("    >Installed Player Character Magic Target VFunc hook."sv);
			return true;
		}

		bool Player::PlayerThunk(RE::MagicTarget* a_this, 
			RE::Actor* a_actor, 
			RE::MagicItem* a_magicItem, 
			const RE::Effect* a_effect)
		{
			return AllowAbsorb(a_this, a_actor, a_magicItem, a_effect) && 
				_func(a_this, a_actor, a_magicItem, a_effect);
		}

		bool Character::CharacterThunk(RE::MagicTarget* a_this,
			RE::Actor* a_actor,
			RE::MagicItem* a_magicItem,
			const RE::Effect* a_effect)
		{
			return AllowAbsorb(a_this, a_actor, a_magicItem, a_effect) && 
				_func(a_this, a_actor, a_magicItem, a_effect);
		}

		bool CloakArchetypeFix::InstallCloakFix() {
			static constexpr std::size_t hookSize = 20; // 7 (AND_) + 7 (OR_) + 2 (TEST) + 2 (JZ) + 2 (JMP)

			bool shouldInstall = Settings::INI::GetSetting<bool>(Settings::INI::FIX_CLOAKS).value_or(false);
			if (!shouldInstall) {
				logger::info("    >User chose to not install the Cloak Archetype Fix."sv);
				return true;
			}

			logger::info("    >Installing the Cloak Archetype Fix..."sv);
			REL::Relocation<std::uintptr_t> target{ RE::Offset::ActiveEffect::Restart, 0x85 };
			const std::uintptr_t hookAddr = target.address();
			const std::uintptr_t continuation = hookAddr + hookSize;

			if (!REL::make_pattern<"84 C0 74 09 81 4F 7C 00 10 00 00 EB 07 81 67 7C FF EF FF FF">().match(target.address())) {
				logger::critical("      Failed to validate the hook pattern."sv);
				return false;
			}

			struct Patch : Xbyak::CodeGenerator {
				Patch(std::uintptr_t contAddr) {
					sub(rsp, 0x20);
					mov(rcx, rdi);
					mov(rax, reinterpret_cast<std::uintptr_t>(&AllowDualCastModification));
					call(rax);
					add(rsp, 0x20);

					test(al, al);
					jz("clear_flag");

					or_(dword[rdi + 0x7C], 0x1000);
					jmp(ptr[rip]);
					dq(contAddr);

					L("clear_flag");
					and_(dword[rdi + 0x7C], 0xFFFFEFFF);
					jmp(ptr[rip]);
					dq(contAddr);
				}
			};


			Patch patch{ continuation };
			patch.ready();

			auto& trampoline = SKSE::GetTrampoline();

			REL::safe_fill(hookAddr, REL::NOP, hookSize);
			trampoline.write_branch<5>(hookAddr, trampoline.allocate(patch));
			return true;
		}

		bool CloakArchetypeFix::AllowDualCastModification(RE::ActiveEffect* a_effect)
		{
			bool instant = a_effect->castingSource == RE::MagicSystem::CastingSource::kInstant;
			if (instant) {
				return false;
			}
			if (a_effect->caster.get() && a_effect->caster.get()->IsDualCasting()) {
				return true;
			}
			return false;
		}
	}
}