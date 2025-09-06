#pragma once

namespace Hooks::Tweaks {
	bool InstallTweaks();

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
		public REX::Singleton<SpellDispeler>
	{
	public:
		bool Install();
		void ApendEffect(const RE::EffectSetting* a_candidate);
		void ClearDispelableSpells(RE::PlayerCharacter* a_player);

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
	};

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
			using ActiveEffectFlag = RE::EffectSetting::EffectSettingData::Flag;
			auto* ui = RE::UI::GetSingleton();
			if (!ui || !a_this) {
				return;
			}

			if (ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
				a_this->elapsedSeconds -= a_delta;
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
		}

		inline static REL::Relocation<decltype(&Update)> _func;
	};
}