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
			default:
				break;
			}
		}
		return true;
	}

	static bool AreConditionsValid(const RE::Effect* a_effect, 
		RE::ConditionCheckParams& a_params, 
		RE::PlayerCharacter* a_player, 
		bool a_checkAll) 
	{
		auto* effectCondHead = a_effect->conditions.head;
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

	struct EffectData {
		uint32_t area;
		uint32_t dur;
		float mag;
	};

	static void modify_tag(const std::string_view str,
		std::string& out,
		const EffectData& data) 
	{
		constexpr std::string_view magTag = "mag";
		constexpr std::string_view durTag = "dur";
		constexpr std::string_view areaTag = "area";
		constexpr std::string_view globTag = "global=";
		constexpr std::string_view boldOpening = "<font face=\'$EverywhereMediumFont\' size=\'20\' color=\'#FFFFFF\'>";
		constexpr std::string_view boldClosing = "</font>";
		
		constexpr auto ascii_lower = [](const unsigned char chr) { 
			constexpr unsigned char diff = 'a' - 'A';
			if (chr >= 'A' && chr <= 'Z') {
				return static_cast<char>(chr - diff);
			}
			return static_cast<char>(chr);
		};

		constexpr auto equal_lower_ascii_str = [&](const std::string_view lhs, const std::string_view rhs) {
			const auto lhsSize = lhs.size();
			if (lhsSize != rhs.size()) {
				return false;
			}
			for (size_t i = 0u; i < lhsSize; ++i) {
				if (ascii_lower(lhs[i]) != ascii_lower(rhs[i])) {
					return false;
				}
			}
			return true;
		};

		constexpr auto starts_with_lower_ascii_str = [&](const std::string_view lhs, const std::string_view start) {
			const auto startSize = start.size();
			if (lhs.size() < startSize) {
				return false;
			}
			return equal_lower_ascii_str(lhs.substr(0u, startSize), start);
		};

		constexpr auto floored_float_to_str = [](const float val) {
			const float floored = std::floor(val);
			const int32_t cast = static_cast<int32_t>(floored);
			return std::to_string(cast);
		};

		constexpr auto float_to_str_to_precision = [](const float val, std::streamsize precision) {
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(precision) << val;
			return oss.str();
		};

		// Note - technically, only <mag>, <dur>, and <area> should be in effects.
		// <global=EDID> is also accepted, and is used. Nested substitution is not
		// possible with these constraints, but I might choose to support this in
		// a possible next update - but it is unlikely.
		out.append(boldOpening); 
		if (equal_lower_ascii_str(str, magTag)) {
			out += floored_float_to_str(data.mag);
		}
		else if (equal_lower_ascii_str(str, durTag)) {
			out += std::to_string(data.dur);
		}
		else if (equal_lower_ascii_str(str, areaTag)) {
			out += std::to_string(data.area);
		}
		else if (starts_with_lower_ascii_str(str, globTag)) {
			constexpr size_t size_of_prefix = globTag.size();
			const auto substr = str.substr(size_of_prefix);
			if (const auto* glob = RE::TESForm::LookupByEditorID<RE::TESGlobal>(substr); glob) {
				const bool isIntegral = glob->type != RE::TESGlobal::Type::kFloat;

				if (isIntegral) {
					out += floored_float_to_str(glob->value);
				}
				else {
					out += float_to_str_to_precision(glob->value, 1);
				}
			}
			else {
				out += "MISSING GLOB";
			}
		}
		else {
			out += str;
		}
		out.append(boldClosing);
	}

	static std::string GetFormattedEffectDescription(const RE::Effect* a_effect) {
		constexpr unsigned char openingTag = '<';
		constexpr unsigned char closingTag = '>';

		const auto baseEffect = a_effect->baseEffect;
		if (!baseEffect || baseEffect->magicItemDescription.empty()) {
			return "";
		}

		const std::string_view original = baseEffect->magicItemDescription;

		EffectData data{};
		const auto& effectItem = a_effect->effectItem;
		data.area = effectItem.area;
		data.dur = effectItem.duration;
		data.mag = effectItem.magnitude;

		size_t cursor = 0u;
		size_t len = original.size();
		std::string res{};
		res.reserve(len * 2u);

		while (cursor < len) {
			const auto opening = original.find(openingTag, cursor);
			if (opening == std::string_view::npos) {
				res += original.substr(cursor);
				break;
			}

			res += original.substr(cursor, opening - cursor);

			const auto closing = original.find(closingTag, opening + 1u);
			if (closing == std::string_view::npos) {
				res += original.substr(opening);
				break;
			}

			const std::string_view tagContents =
            	original.substr(opening + 1u, closing - opening - 1u);
			modify_tag(tagContents, res, data);
			cursor = closing + 1u; // fine if we exceed len
		}
		return res;
	}

	std::string GetFormattedDescription(RE::SpellItem* a_spell) {
		if (!a_spell || a_spell->effects.empty()) {
			return "";
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			REX::CRITICAL("Failed to get the player or SpellItemDescirptionSingleton. This will likely cause a crash later.");
			return "";
		}

		auto* frontEffect = a_spell->effects.front();
		auto* frontBaseEffect = frontEffect ? frontEffect->baseEffect : nullptr;
		if (!frontBaseEffect) {
			return "";
		}

		std::string newDescription = "";

		using ArraySize = RE::BSTArrayBase::size_type;
		auto params = RE::ConditionCheckParams(player, player);
		bool checkAll = ShouldCheckAllConditions(frontBaseEffect);
		const auto& effects = a_spell->effects;
		const auto len = effects.size();
		for (ArraySize i = 0u; i < len; ++i) {
			const auto* effect = effects[i];
			const auto baseEffect = effect ? effect->baseEffect : nullptr;
			if (!baseEffect || 
				baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kHideInUI) ||
				!AreConditionsValid(effect, params, player, checkAll))
			{
				continue;
			}

			auto proposed = GetFormattedEffectDescription(effect);
			if (proposed.empty()) {
				continue;
			}

			if (proposed.front() != ' ') {
				proposed += " ";
			}
			newDescription += proposed;
		}
		return newDescription;
	}
}