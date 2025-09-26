#pragma once

#include "Serialization/SerializationManager/SerializationManager.h"

namespace BoundEffectManager
{
	bool InitializeBoundEffectManager();

	class BoundEffectManager final :
		public REX::Singleton<BoundEffectManager>,
		public Serialization::SerializationManager::Serializable
	{
	public:
		bool Initialize();

		bool CanCastSpell(RE::MagicItem* a_spell, bool a_dualCast);

		void ProcessEffectAdded(RE::ActiveEffect* a_effect);
		void ProcessEffectRemoved(RE::ActiveEffect* a_effect);

		bool IsBoundEffect(RE::ActiveEffect* a_effect); 

		void UpdateTimePassed(float a_delta);

		int UnBindSpell(RE::MagicItem* a_spell);
		bool UnBindAllSpells();

		void GainExperience();

		bool Save(SKSE::SerializationInterface* a_intfc) override;
		bool Load(SKSE::SerializationInterface* a_intfc) override;
		void Revert(SKSE::SerializationInterface* a_intfc) override;

	private:
		bool HasEnoughOfAttributeToBind(RE::ActorValue a_av, float a_demand);
		bool IsBindingEffectApplicable(const RE::EffectSetting* a_effect, bool a_dualCast);
		void UpdateUI();


		struct BoundAttribute
		{
			float ammount{ 0.0f };
			RE::ActorValue attribute{ RE::ActorValue::kNone };

			void Restore(RE::PlayerCharacter* a_player) {
				a_player->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, attribute, ammount);
				attribute = RE::ActorValue::kNone ;
				ammount = 0.0f;
			}

			void Bind(RE::PlayerCharacter* a_player) const {
				a_player->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, attribute, ammount);
				a_player->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, attribute, -ammount);
			}
		};

		bool loading{ false };
		bool enabled{ false };
		bool queued{ false };
		float timeElapsed{ 0.0f };
		float totalHealthBound{ 0.0f };
		float totalStaminaBound{ 0.0f };
		float totalMagickaBound{ 0.0f };
		std::unordered_map<RE::ActiveEffect*, std::vector<BoundAttribute>> costliestBindings{};
		std::unordered_set<RE::ActiveEffect*> boundEffects{};
		
		RE::BGSKeyword* bindHealthKeyword{ nullptr };
		RE::BGSKeyword* bindStaminaKeyword{ nullptr };
		RE::BGSKeyword* bindMagickaKeyword{ nullptr };

		RE::PlayerCharacter* player{ nullptr };
		inline static constexpr uint32_t RecordType{ 'BEFM' };

		inline static constexpr const RE::ActorValue magickaBinding = RE::ActorValue::kMagicka;
		inline static constexpr const RE::ActorValue staminaBinding = RE::ActorValue::kStamina;
		inline static constexpr const RE::ActorValue healthBinding = RE::ActorValue::kHealth;
	};

	inline static constexpr const char* BindHealthKeywordID = "MagicBindHealth";
	inline static constexpr const char* BindStaminaKeywordID = "MagicBindStamina";
	inline static constexpr const char* BindMagickaKeywordID = "MagicBindMagicka";
}