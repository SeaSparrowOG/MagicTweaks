#pragma once

namespace Hooks 
{
	namespace Fixes
	{
		bool InstallFixes();

		struct Character
		{
			static bool InstallCharacterFixes();
			static bool CharacterThunk(RE::MagicTarget* a_this,
				RE::Actor* a_actor,
				RE::MagicItem* a_magicItem,
				const RE::Effect* a_effect);

			inline static REL::Relocation<decltype(&CharacterThunk)> _func;
		};

		struct Player
		{
			static bool InstallPlayerFixes();
			static bool PlayerThunk(RE::MagicTarget* a_this,
				RE::Actor* a_actor,
				RE::MagicItem* a_magicItem,
				const RE::Effect* a_effect);

			inline static REL::Relocation<decltype(&PlayerThunk)> _func;
		};

		struct CloakArchetypeFix
		{
			static bool InstallCloakFix();
			static void ResetCloakEffect(RE::ActiveEffect* a_effect);

			inline static REL::Relocation<decltype(&ResetCloakEffect)> _func;

			class CloakEffectMagnitudeVisitor : public RE::PerkEntryVisitor
			{
			public:
				RE::BSContainer::ForEachResult Visit(RE::BGSPerkEntry* a_perkEntry);
				bool                           CanRun();
				void                           Finalize();

				CloakEffectMagnitudeVisitor(RE::ActiveEffect* a_effect)
				{
					effect = a_effect;
					spell = a_effect->spell ? a_effect->spell->As<RE::SpellItem>() : nullptr;
					target = a_effect->target;
					caster = a_effect->caster.get().get();
					targetAsActor = target ? target->GetTargetAsActor() : nullptr;

					bool foundMatch = false;
					if (spell && !spell->effects.empty()) {
						const auto* base = a_effect->GetBaseObject();
						auto begin = spell->effects.begin();
						auto end = spell->effects.end();

						for (auto it = begin; !foundMatch && it != end; ++it) {
							const auto* effectItem = *it;
							const auto* spellEffectBase = effectItem ? effectItem->baseEffect : nullptr;
							if (!spellEffectBase) {
								continue;
							}

							foundMatch |= spellEffectBase == base;
							if (foundMatch) {
								result = effectItem->GetMagnitude();
							}
						}
					}
					if (!foundMatch) {
						spell = nullptr;
						effect = nullptr;
						result = 0.0f;
					}
				}
			private:
				float             result{ 1.0f };
				RE::Actor*        caster{ nullptr };
				RE::Actor*        targetAsActor{ nullptr };
				RE::SpellItem*    spell{ nullptr };
				RE::MagicTarget*  target{ nullptr };
				RE::ActiveEffect* effect{ nullptr };
			};
		};
	}
}