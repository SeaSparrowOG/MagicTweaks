#pragma once

#include "BoundEffectManager/BoundEffectManager.h"

namespace Hooks::Tweaks {
	inline bool g_extendInDialogue = false;

	bool InstallTweaks();

	struct CastSpeedTweaks
	{
		inline static bool PatchSpeed();
		inline static void SetCastTimer(RE::MagicCaster* a_this);
		inline static REL::Relocation<decltype(SetCastTimer)> _setCastTimer;
	};

	struct EnforceCastLimits
	{
		inline static bool PatchCheckCast();

		inline static bool CheckCast(RE::ActorMagicCaster* a_this, 
			RE::MagicItem* a_spell,
			bool a_dualCast, 
			float* a_effectStrength,
			RE::MagicSystem::CannotCastReason* a_reason,
			bool a_useBaseValueForCost);

		inline static const char* GetCannotCastString(RE::MagicSystem::CannotCastReason a_reason);
		
		inline static REL::Relocation<decltype(CheckCast)> _checkCast;
		inline static REL::Relocation<decltype(GetCannotCastString)> _getCannotCastString;
	};

	struct ModifySpellReduction
	{
		inline static bool PatchSpellReduction();
		inline static float ProcessSpellReduction(RE::MagicItem* a_spell, RE::Actor* a_actor);
		inline static REL::Relocation<decltype(ProcessSpellReduction)> _func;

		inline static float skillFloor{ -45.0f };
		inline static float skillCeilling{ 200.0f };
		inline static float skillWeight{ 0.01f };
		inline static float maxReductionPct{ 0.9f };
	};

	class SpellDispeler :
		public REX::TSingleton<SpellDispeler>
	{
	public:
		bool Install();
		void ClearDispelableSpells(RE::PlayerCharacter* a_player);

		bool LoadJSONSettings();

	private:
		struct PlayerDrawMonitor
		{
			static bool Install();
			static void Thunk(RE::PlayerCharacter* a_this, bool a_draw);

			inline static REL::Relocation<decltype(&Thunk)> _func;

			inline static size_t offset{ 0xA6 };
			inline static std::string setting{ "Tweaks|bDispelOnSheathe" };
		};

		std::unordered_set<const RE::EffectSetting*> dispelEffects{};

		inline static constexpr const char* SheatheArray = "DispelOnSeathe";
	};

	struct ActiveEffectDurationPatch
	{
		[[nodiscard]] static bool Patch();
		inline static void Update(RE::ActiveEffect* a_this, float a_delta);
		inline static REL::Relocation<decltype(Update)> _update;
	};

	struct ProjectilePatch
	{
		[[nodiscard]] static bool Patch();

		static RE::EffectSetting* ProcessProjectile(RE::MagicItem* a_this);
		static RE::EffectSetting* ProcessImpact(RE::MagicItem* a_this);
		inline static REL::Relocation<decltype(ProcessProjectile)> _processProjectile;
		inline static REL::Relocation<decltype(ProcessImpact)> _processImpact;
	};

	/*
	Deprecated since 2.5.0 - I use MagicTarget -> Update instead
	template <typename T>
	struct ArchetypePatch
	{
		inline static bool Patch()
		{
			REL::Relocation<std::uintptr_t> VTABLE{ T::VTABLE[0] };
			_func = VTABLE.write_vfunc(0x4, Update);
			return true;
		}

		inline static void Update(T* a_this, float a_delta)
		{
			_func(a_this, a_delta);
			if (!a_this) {
				return;
			}

			if (auto* boundManager = BoundEffectManager::BoundEffectManager::GetSingleton(); 
				boundManager && boundManager->IsBoundEffect(a_this))
			{
				a_this->elapsedSeconds -= a_delta;
				return;
			}

			if (!g_extendInDialogue) {
				return;
			}

			using ActiveEffectFlag = RE::EffectSetting::EffectSettingData::Flag;
			auto* ui = RE::UI::GetSingleton();
			if (!ui) {
				return;
			}

			if (!ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
				return;
			}

			const auto* caster = a_this->GetCasterActor().get();
			const auto* target = a_this->GetTargetActor();
			const auto* base = a_this->effect ? a_this->effect->baseEffect : nullptr;
			if (!base || !caster || !caster->IsPlayerRef() || !target) {
				return;
			}

			if (base->data.flags.any(ActiveEffectFlag::kDetrimental,
				ActiveEffectFlag::kHostile,
				ActiveEffectFlag::kNoDuration)) {
				return;
			}

			a_this->elapsedSeconds -= a_delta;
		}

		inline static REL::Relocation<decltype(&Update)> _func;
	};
	*/
}