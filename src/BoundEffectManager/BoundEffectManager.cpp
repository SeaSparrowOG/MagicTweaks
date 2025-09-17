#include "BoundEffectManager.h"

#include "Data/ModObjectManager.h"
#include "Serialization/Serde.h"

namespace BoundEffectManager
{
	bool InitializeBoundEffectManager() {
		SECTION_SEPARATOR;
		logger::info("Starting up the Bound Effect Manager..."sv);
		auto* manager = BoundEffectManager::GetSingleton();
		if (!manager) {
			logger::info("  >Failed to get internal singleton."sv);
			return false;
		}
		return manager->Initialize();
	}

	bool BoundEffectManager::Initialize() {
		logger::info("  >Caching game forms..."sv);

		bool nominal = true;
		player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			logger::critical("    >Failed to cache the player.");
			nominal = false;
		}

		if (!nominal) {
			logger::critical("  >Failed to cache game forms."sv);
			return false;
		}

		logger::info("  >Caching mod forms..."sv);
		bindHealthKeyword = Data::ModObject<RE::BGSKeyword>(BindHealthKeywordID);
		if (!bindHealthKeyword) {
			logger::critical("    >Failed to cache {}", BindHealthKeywordID);
			nominal = false;
		}
		bindStaminaKeyword = Data::ModObject<RE::BGSKeyword>(BindStaminaKeywordID);
		if (!bindStaminaKeyword) {
			logger::critical("    >Failed to cache {}", BindStaminaKeywordID);
			nominal = false;
		}
		bindMagickaKeyword = Data::ModObject<RE::BGSKeyword>(BindMagickaKeywordID);
		if (!bindMagickaKeyword) {
			logger::critical("    >Failed to cache {}", BindMagickaKeywordID);
			nominal = false;
		}
		if (!nominal) {
			logger::critical("  >Failed to cache mod forms. This means that the Armillary_ModObjectsQuest Quest in Armillary.esm is either overwritten or corrupted."sv);
			return false;
		}

		auto* serdeManager = Serialization::SerializationManager::ObjectManager::GetSingleton();
		if (!serdeManager) {
			logger::critical("  Failed to register self as a serializable form."sv);
			return false;
		}
		serdeManager->RegisterObject(this, RecordType);
		return true;
	}

	bool BoundEffectManager::Save(SKSE::SerializationInterface* a_intfc) {
		logger::info("  >Saving Bound Effect Manager state..."sv);
		if (!a_intfc->OpenRecord(RecordType, Serialization::Version)) {
			logger::error("    >Error serializing Bound Effects!"sv);
			return false;
		}
		if (!a_intfc->WriteRecordData(totalHealthBound)) {
			logger::error("    >Error writing Bound Health."sv);
			return false;
		}
		if (!a_intfc->WriteRecordData(totalStaminaBound)) {
			logger::error("    >Error writing Bound Stamina."sv);
			return false;
		}
		if (!a_intfc->WriteRecordData(totalMagickaBound)) {
			logger::error("    >Error writing Bound Magicka."sv);
			return false;
		}
		logger::info("    >Success."sv);
		return true;
	}

	bool BoundEffectManager::Load(SKSE::SerializationInterface* a_intfc) {
		if (!a_intfc->ReadRecordData(totalHealthBound)) {
			logger::critical("    >Failed to deserialize Bound Health."sv);
			return false;
		}
		if (!a_intfc->ReadRecordData(totalStaminaBound)) {
			logger::critical("    >Failed to deserialize Bound Stamina."sv);
			return false;
		}
		if (!a_intfc->ReadRecordData(totalMagickaBound)) {
			logger::critical("    >Failed to deserialize Bound Magicka."sv);
			return false;
		}
		Revert(a_intfc);
		auto* activeEffects = player->GetActiveEffectList();
		for (auto* effect : *activeEffects) {
			ProcessEffectAdded(effect);
		}
		return true;
	}

	void BoundEffectManager::Revert(SKSE::SerializationInterface* a_intfc) {
		(void)a_intfc;
		player->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kHealth, totalHealthBound);
		player->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kStamina, totalStaminaBound);
		player->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kMagicka, totalMagickaBound);
		totalHealthBound = 0.0f;
		totalStaminaBound = 0.0f;
		totalMagickaBound = 0.0f;
	}
}

namespace BoundEffectManager {
	bool BoundEffectManager::CanCastSpell(RE::MagicItem* a_spell, bool a_dualCast) {
		auto* spellItem = a_spell ? a_spell->As<RE::SpellItem>() : nullptr;
		if (!spellItem || spellItem->effects.empty()) {
			return true;
		}

		auto& spellEffects = spellItem->effects;
		float demandHealth = 0.0f;
		float demandMagicka = 0.0f;
		float demandStamina = 0.0f;
		for (const auto* effect : spellEffects) {
			auto* base = effect ? effect->baseEffect : nullptr;
			if (!base) {
				continue;
			}

			bool bindsHealth = base->HasKeyword(bindHealthKeyword);
			bool bindsStamina = base->HasKeyword(bindStaminaKeyword);
			bool bindsMagicka = base->HasKeyword(bindMagickaKeyword);
			if (!(bindsHealth || bindsStamina || bindsMagicka) ||
				!IsBindingEffectApplicable(base, a_dualCast))
			{
				continue;
			}

			float mag = effect->GetMagnitude();
			if (bindsHealth) {
				demandHealth += mag;
			}
			if (bindsStamina) {
				demandStamina += mag;
			}
			if (bindsMagicka) {
				demandMagicka += mag;
			}
		}

		if (demandHealth == 0.0f &&
			demandStamina == 0.0f &&
			demandMagicka == 0.0f)
		{
			return true;
		}

		return HasEnoughOfAttributeToBind(RE::ActorValue::kHealth, demandHealth) &&
			HasEnoughOfAttributeToBind(RE::ActorValue::kStamina, demandStamina) &&
			HasEnoughOfAttributeToBind(RE::ActorValue::kMagicka, demandMagicka);
	}

	void BoundEffectManager::ProcessEffectAdded(RE::ActiveEffect* a_effect) {
		if (!a_effect || a_effect->flags.any(RE::ActiveEffect::Flag::kInactive)) {
			return;
		}
		auto* appliedBase = a_effect->GetBaseObject();
		auto* appliedSpell = a_effect->spell;
		if (!appliedSpell || !appliedBase) {
			return;
		}

		auto* caster = a_effect->caster.get().get();
		auto* magicTarget = a_effect->target;
		auto* target = magicTarget && magicTarget->MagicTargetIsActor() ?
			magicTarget->GetTargetAsActor() : nullptr;
		if (!caster || !target || caster != target || caster != player) {
			return;
		}

		auto& appliedSpellEffects = appliedSpell->effects;
		float lastCostliestCost = -1.0f;
		float demandHealth = 0.0f;
		float demandStamina = 0.0f;
		float demandMagicka = 0.0f;
		bool hasValidBinding = false;
		RE::EffectSetting* costliestEffect = nullptr;
		for (const auto* effect : appliedSpellEffects) {
			auto* base = effect ? effect->baseEffect : nullptr;
			if (!base) {
				continue;
			}

			bool bindsHealth = base->HasKeyword(bindHealthKeyword);
			bool bindsStamina = base->HasKeyword(bindStaminaKeyword);
			bool bindsMagicka = base->HasKeyword(bindMagickaKeyword);

			bool binds = bindsHealth || bindsStamina || bindsMagicka;
			if (binds && IsBindingEffectApplicable(base, a_effect->flags.any(RE::ActiveEffect::Flag::kDual))) {
				hasValidBinding = true;
				if (bindsHealth) {
					demandHealth += effect->GetMagnitude();
				}
				if (bindsStamina) {
					demandStamina += effect->GetMagnitude();
				}
				if (bindsMagicka) {
					demandMagicka += effect->GetMagnitude();
				}
			}

			if (!costliestEffect) {
				costliestEffect = base;
			}
			if (binds || effect->cost < lastCostliestCost) {
				continue;
			}
			if (!effect->conditions.IsTrue(caster, target) || !base->conditions.IsTrue(caster, target)) {
				continue;
			}
			lastCostliestCost = effect->cost;
			costliestEffect = base;
		}
		if (!hasValidBinding) {
			return;
		}

		if (appliedBase == costliestEffect) {
			auto boundAttributes = std::vector<BoundAttribute>();
			if (demandHealth > 0.0f) {
				auto newDemand = BoundAttribute();
				newDemand.ammount = demandHealth;
				newDemand.attribute = healthBinding;
				newDemand.Bind(player);
				boundAttributes.push_back(std::move(newDemand));
				totalHealthBound += demandHealth;
			}
			if (demandStamina > 0.0f) {
				auto newDemand = BoundAttribute();
				newDemand.ammount = demandStamina;
				newDemand.attribute = staminaBinding;
				newDemand.Bind(player);
				boundAttributes.push_back(std::move(newDemand));
				totalStaminaBound += demandStamina;
			}
			if (demandMagicka > 0.0f) {
				auto newDemand = BoundAttribute();
				newDemand.ammount = demandMagicka;
				newDemand.attribute = magickaBinding;
				newDemand.Bind(player);
				boundAttributes.push_back(std::move(newDemand));
				totalMagickaBound += demandMagicka;
			}
			costliestBindings.emplace(a_effect, std::move(boundAttributes));
		}
		else {
			boundEffects.insert(a_effect);
		}
	}

	void BoundEffectManager::ProcessEffectRemoved(RE::ActiveEffect* a_effect) {
		if (!a_effect) {
			return;
		}
		if (costliestBindings.contains(a_effect)) {
			auto& toClear = costliestBindings.at(a_effect);
			for (auto& element : toClear) {
				switch (element.attribute) {
				case RE::ActorValue::kHealth:
					totalHealthBound -= element.ammount;
					break;
				case RE::ActorValue::kStamina:
					totalStaminaBound -= element.ammount;
					break;
				case RE::ActorValue::kMagicka:
					totalMagickaBound -= element.ammount;
					break;
				}
				element.Restore(player);
			}
			toClear.clear();
			costliestBindings.erase(a_effect);

			auto* caster = a_effect->GetCasterActor().get();
			if (caster) {
				auto* activeEffects = caster->GetActiveEffectList();
				if (!activeEffects || activeEffects->empty()) {
					return;
				}

				auto spellEffectsCopy = a_effect->spell ? a_effect->spell->effects : RE::BSTArray<RE::Effect*>();
				if (spellEffectsCopy.empty()) {
					return;
				}

				auto* costliestBase = a_effect->GetBaseObject();
				for (auto it = activeEffects->begin(); it != activeEffects->end(); ++it) {
					auto* applied = *it;
					auto* base = applied ? applied->GetBaseObject() : nullptr;
					if (!base || base == costliestBase || !boundEffects.contains(applied)) {
						continue;
					}

					bool found = false;
					for (auto itCopy = spellEffectsCopy.begin(); !found && itCopy != spellEffectsCopy.end(); ++itCopy) {
						auto* copyBase = *itCopy ? (*itCopy)->baseEffect : nullptr;
						if (!copyBase || copyBase != base) {
							continue;
						}

						applied->Dispel(false);
						found = true;
						spellEffectsCopy.erase(itCopy);
					}
				}
			}
		}
		else if (boundEffects.contains(a_effect)) {
			boundEffects.erase(a_effect);
		}
	}

	bool BoundEffectManager::IsBoundEffect(RE::ActiveEffect* a_effect) {
		return costliestBindings.contains(a_effect) || boundEffects.contains(a_effect);
	}

	void BoundEffectManager::UpdateTimePassed(float a_delta) {
		timeElapsed += abs(a_delta); // this is unecessary, delta is always positive.
		if (timeElapsed >= 0.3f) {
			if (queued) {
				timeElapsed = 0.0f;
				return;
			}
			queued = true;
			auto* taskInterface = SKSE::GetTaskInterface();
			if (taskInterface) {
				taskInterface->AddTask([self = this]() {
					self->UpdateUI();
					});
			}
		}
	}

	bool BoundEffectManager::HasEnoughOfAttributeToBind(RE::ActorValue a_av, float a_demand) {
		float theoreticalMax = player->GetBaseActorValue(a_av) +
			player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, a_av);
		return theoreticalMax >= a_demand;
	}

	bool BoundEffectManager::IsBindingEffectApplicable(const RE::EffectSetting* a_effect, bool a_dualCast) {
		const auto& conditions = a_effect->conditions;
		auto checkParams = RE::ConditionCheckParams(player, player);
		auto* head = conditions.head;
		while (head) {
			bool valid = true;
			auto functionData = head->data.functionData.function;
			if (functionData.any(RE::FUNCTION_DATA::FunctionID::kDualCast)) {
				valid = a_dualCast;
			}
			else {
				valid = head->IsTrue(checkParams);
			}
			if (!valid) {
				return false;
			}
			head = head->next;
		}
		return true;
	}

	void BoundEffectManager::UpdateUI() {
		float baseHealth = player->GetBaseActorValue(RE::ActorValue::kHealth);
		float currentHealth = baseHealth +
			player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kHealth);
		float healthRatio = std::clamp(currentHealth / baseHealth * 100.0f, 0.0f, 100.0f);

		float baseStamina = player->GetBaseActorValue(RE::ActorValue::kStamina);
		float currentStamina = baseStamina +
			player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kStamina);
		float staminaRatio = std::clamp(currentStamina / baseStamina * 100.0f, 0.0f, 100.0f);

		float baseMagicka = player->GetBaseActorValue(RE::ActorValue::kMagicka);
		float currentMagicka = baseMagicka +
			player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, RE::ActorValue::kMagicka);
		float magickaRatio = std::clamp(currentMagicka / baseMagicka * 100.0f, 0.0f, 100.0f);

		auto* healthGlobal = RE::TESForm::LookupByEditorID<RE::TESGlobal>("Survival_ColdAttributePenaltyPercent"sv);
		auto* staminaGlobal = RE::TESForm::LookupByEditorID<RE::TESGlobal>("Survival_HungerAttributePenaltyPercent"sv);
		auto* magickaGlobal = RE::TESForm::LookupByEditorID<RE::TESGlobal>("Survival_ExhaustionAttributePenaltyPercent"sv);

		if (healthGlobal) {
			healthGlobal->value = 100.0f - healthRatio;
		}
		if (staminaGlobal) {
			staminaGlobal->value = 100.0f - staminaRatio;
		}
		if (magickaGlobal) {
			magickaGlobal->value = 100.0f - magickaRatio;
		}
		timeElapsed = 0.0f;
		queued = false;
	}
}