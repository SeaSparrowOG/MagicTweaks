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

		inline bool CloakArchetypeFix::InstallCloakFix() {
			// Overkill?
			static constexpr std::size_t hookSize = 20; // 7 (AND_) + 7 (OR_) + 2 (TEST) + 2 (JZ) + 2 (JMP)

			bool shouldInstall = Settings::INI::GetSetting<bool>(Settings::INI::FIX_CLOAKS).value_or(false);
			if (!shouldInstall) {
				logger::info("    >User chose to not install the Cloak Archetype Fix."sv);
				return true;
			}
			logger::info("    >Installing the Cloak Archetype Fix..."sv);

			REL::Relocation<std::uintptr_t> target{ RE::Offset::ValueModifierEffect::ApplyEffect, 0x85 };
			// Maybe overkill, potentially problematic for GOG. TEST!!!!
			if (!REL::make_pattern<"84 C0 74 09 81 4F 7C 00 10 00 00 EB 07 81 67 7C FF EF FF FF">().match(target.address())) {
				logger::critical("      Failed to validate the hook pattern."sv);
				return false;
			}

			const std::uintptr_t hookAddr = target.address();
			const std::uintptr_t continuation = hookAddr + hookSize;

			struct Patch : Xbyak::CodeGenerator {
				Patch(std::uintptr_t contAddr) {
					sub(rsp, 0x20);
					mov(rcx, rdi); // In RDI lives a ValueModifierEffect - might be an ActiveEffect pointer though, test.
					mov(rax, reinterpret_cast<std::uintptr_t>(&ShouldClearDualFlag));
					call(rax);
					add(rsp, 0x20);

					// Restore vanilla behavior
					test(al, al);   // Important, al is modified by ShouldModifyEffect. This is stored in
									// bVar3, and if bVar3 DOESN'T uVar5 [*(param_1 + 0x7c) >> 0xc], RDI's
									// effect is modified. So, changing al is intentional. Remove it for fun!
					jz("do_clear");

					or_(dword[rdi + 0x7C], 0x1000);      // Sets the kDualFlag in the effect in RDI
					jmp(ptr[rip]);
					dq(contAddr);

					L("do_clear");
					and_(dword[rdi + 0x7C], 0xFFFFEFFF); // Clears the kDualFlag from the effect in RDI
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

		inline bool CloakArchetypeFix::ShouldClearDualFlag(RE::ActiveEffect* a_effect)
		{
			const bool noDualCast = a_effect->castingSource == RE::MagicSystem::CastingSource::kInstant;
			const bool isDualCasting = a_effect->GetCasterActor() ? a_effect->GetCasterActor()->IsDualCasting() : false;
			return isDualCasting && !noDualCast;
		}
	}
}