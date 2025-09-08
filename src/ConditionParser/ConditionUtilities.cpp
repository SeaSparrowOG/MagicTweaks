#include "ConditionUtilities.h"

namespace ConditionParser
{
	std::string tolower(std::string_view a_str)
	{
		std::string result(a_str);
		std::ranges::transform(result, result.begin(), [](unsigned char ch) { return static_cast<unsigned char>(std::tolower(ch)); });
		return result;
	}

	static bool ShouldSkipCondition(RE::TESConditionItem* a_condition,
		RE::PlayerCharacter* a_player,
		bool a_evaluateAll) {
		using funcID = RE::FUNCTION_DATA::FunctionID;
		if (!a_evaluateAll) {
			if (!(a_condition->data.object.any(RE::CONDITIONITEMOBJECT::kTarget))
				&& a_condition->data.runOnRef.get().get() != a_player->AsReference()) {
				return true;
			}
		}

		switch (a_condition->data.functionData.function.get()) {
		case funcID::kGetBaseActorValue:
		case funcID::kGetIsPlayableRace:
		case funcID::kGetIsRace:
		case funcID::kGetIsSex:
		case funcID::kGetLevel:
		case funcID::kGetQuestCompleted:
		case funcID::kGetQuestRunning:
		case funcID::kGetStageDone:
		case funcID::kGetStage:
		case funcID::kGetVMQuestVariable:
		case funcID::kGetVMScriptVariable:
		case funcID::kHasPerk:
			return false;
			break;
		default:
			break;
		}
		return true;
	}

	static bool ShouldCheckAllConditions(RE::EffectSetting* a_effect) {
		const auto delivery = a_effect->data.delivery;
		const auto archetype = a_effect->GetArchetype();

		if (delivery != RE::MagicSystem::Delivery::kSelf) {
			switch (archetype) {
			case RE::EffectSetting::Archetype::kAbsorb:
			case RE::EffectSetting::Archetype::kBanish:
			case RE::EffectSetting::Archetype::kCalm:
			case RE::EffectSetting::Archetype::kConcussion:
			case RE::EffectSetting::Archetype::kDemoralize:
			case RE::EffectSetting::Archetype::kDisarm:
			case RE::EffectSetting::Archetype::kDualValueModifier:
			case RE::EffectSetting::Archetype::kEtherealize:
			case RE::EffectSetting::Archetype::kFrenzy:
			case RE::EffectSetting::Archetype::kGrabActor:
			case RE::EffectSetting::Archetype::kInvisibility:
			case RE::EffectSetting::Archetype::kLight:
			case RE::EffectSetting::Archetype::kLock:
			case RE::EffectSetting::Archetype::kOpen:
			case RE::EffectSetting::Archetype::kParalysis:
			case RE::EffectSetting::Archetype::kPeakValueModifier:
			case RE::EffectSetting::Archetype::kRally:
			case RE::EffectSetting::Archetype::kSoulTrap:
			case RE::EffectSetting::Archetype::kStagger:
			case RE::EffectSetting::Archetype::kTelekinesis:
			case RE::EffectSetting::Archetype::kTurnUndead:
			case RE::EffectSetting::Archetype::kValueAndParts:
			case RE::EffectSetting::Archetype::kValueModifier:
				return false;
				break;
			default:
				break;
			}
		}
		return true;
	}

	static bool AreConditionsValid(RE::Effect* a_effect, RE::ConditionCheckParams& a_params, RE::PlayerCharacter* a_player, bool a_checkAll) {
		auto effectCondHead = a_effect->conditions.head;
		bool matchedOR = false;
		while (effectCondHead) {
			if (ShouldSkipCondition(effectCondHead, a_player, a_checkAll)) {
				effectCondHead = effectCondHead->next;
				continue;
			}

			if (effectCondHead->data.flags.isOR) {
				if (!effectCondHead->IsTrue(a_params)) {
					effectCondHead = effectCondHead->next;
					continue;
				}
				matchedOR = true;
			}
			else {
				if (!matchedOR && !effectCondHead->IsTrue(a_params)) {
					return false;
				}
				matchedOR = false;
			}
			effectCondHead = effectCondHead->next;
		}

		auto baseCondHead = a_effect->baseEffect->conditions.head;
		matchedOR = false;
		while (baseCondHead) {
			if (ShouldSkipCondition(baseCondHead, a_player, a_checkAll)) {
				baseCondHead = baseCondHead->next;
				continue;
			}

			if (baseCondHead->data.flags.isOR) {
				if (!baseCondHead->IsTrue(a_params)) {
					baseCondHead = baseCondHead->next;
					continue;
				}
				matchedOR = true;
			}
			else {
				if (!matchedOR && !baseCondHead->IsTrue(a_params)) {
					return false;
				}
				matchedOR = false;
			}
			baseCondHead = baseCondHead->next;
		}

		return true;
	}

	static std::string GetFormattedEffectDescription(RE::Effect* a_effect)
	{
		const auto baseEffect = a_effect->baseEffect;
		std::string originalDescription = baseEffect->magicItemDescription.c_str();
		if (originalDescription.empty()) {
			return originalDescription;
		}

		std::string::const_iterator searchStart(originalDescription.cbegin());
		std::smatch matches;
		std::string response;

		while (std::regex_search(searchStart, originalDescription.cend(), matches, pattern)) {
			response += matches.prefix().str();
			response += openingTag;
			auto contents = matches[1].str();
			auto contentsLower = tolower(contents);
			if (contentsLower == "mag") {
				int64_t clampedValue = std::clamp(
					static_cast<int64_t>(a_effect->GetMagnitude()),
					std::numeric_limits<int64_t>::min(),
					std::numeric_limits<int64_t>::max()
				);
				response += std::to_string(clampedValue);
			}
			else if (contentsLower == "dur") {
				response += std::to_string(a_effect->GetDuration());
			}
			else if (contentsLower == "area") {
				response += std::to_string(a_effect->GetArea());
			}
			else if (contentsLower.starts_with("global=")) {
				auto globalEDID = contents.substr(7);
				const auto global = RE::TESForm::LookupByEditorID<RE::TESGlobal>(globalEDID);
				if (global) {
					response += std::to_string(global->value);
				}
			}
			else {
				response += contents;
			}
			response += closingTag;
			searchStart = matches.suffix().first;
		}
		response += std::string(searchStart, originalDescription.cend());
		return response;
	}

	std::string GetFormattedDescription(RE::SpellItem* a_spell) {
		if (!a_spell || a_spell->effects.empty()) {
			return "";
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			logger::critical("Failed to get the player or SpellItemDescirptionSingleton. This will likely cause a crash later.");
			return "";
		}

		auto* frontEffect = a_spell->effects.front();
		auto* frontBaseEffect = frontEffect ? frontEffect->baseEffect : nullptr;
		if (!frontBaseEffect) {
			return "";
		}

		auto params = RE::ConditionCheckParams(player, player);
		bool checkAll = ShouldCheckAllConditions(frontBaseEffect);
		std::string newDescription = "";
		for (auto* effect : a_spell->effects) {
			const auto baseEffect = effect ? effect->baseEffect : nullptr;
			if (!baseEffect) {
				continue;
			}
			if (baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kHideInUI)) {
				continue;
			}

			if (AreConditionsValid(effect, params, player, checkAll)) {
				auto proposed = GetFormattedEffectDescription(effect);
				if (proposed.empty()) {
					continue;
				}

				if (proposed.front() != ' ') {
					proposed += " ";
				}
				newDescription += proposed;
			}
		}
		return newDescription;
	}
}