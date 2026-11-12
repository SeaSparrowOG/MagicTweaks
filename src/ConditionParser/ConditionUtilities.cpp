#include "ConditionUtilities.h"

namespace {
	char ascii_lower(const unsigned char chr) { 
		constexpr unsigned char diff = 'a' - 'A';
		if (chr >= 'A' && chr <= 'Z') {
			return static_cast<char>(chr - diff);
		}
		return static_cast<char>(chr);
	};

	bool equal_lower_ascii_str(const std::string_view lhs, const std::string_view rhs) {
		const auto lhsSize = lhs.size();
		if (lhsSize != rhs.size()) {
			return false;
		}
		for (size_t i = 0u; i < lhsSize; ++i) {
			if (ascii_lower(static_cast<unsigned char>(lhs[i])) != static_cast<unsigned char>(rhs[i])) {
				return false;
			}
		}
		return true;
	};

	bool starts_with_lower_ascii_str(const std::string_view lhs, const std::string_view start) {
		const auto startSize = start.size();
		if (lhs.size() < startSize) {
			return false;
		}
		return equal_lower_ascii_str(lhs.substr(0u, startSize), start);
	};

	std::string floored_float_to_str(const float val) {
		const float floored = std::floor(val);
		const int32_t cast = static_cast<int32_t>(floored);
		return std::to_string(cast);
	};

	std::string float_to_str_to_precision(const float val, std::streamsize precision) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precision) << val;
		return oss.str();
	};
}

namespace ConditionParser {
	struct EvalContext {
		bool                       _checkAll = true;
		const RE::Effect*          _effect = nullptr;
		const RE::EffectSetting*   _base = nullptr;
		RE::ConditionCheckParams*  _params = nullptr;
		RE::PlayerCharacter*       _player = RE::PlayerCharacter::GetSingleton();
	};

	[[nodiscard]] static bool evaluate_condition(const RE::TESConditionItem* head, EvalContext& ctx) noexcept {
		using funcID = RE::FUNCTION_DATA::FunctionID;
		const auto& id = head->data.functionData.function;
		const bool supported = id.any(funcID::kGetBaseActorValue, 
			funcID::kGetIsPlayableRace, 
			funcID::kGetIsRace,
			funcID::kGetIsSex, 
			funcID::kGetLevel, 
			funcID::kGetQuestCompleted, 
			funcID::kGetQuestRunning,
			funcID::kGetStageDone, 
			funcID::kGetStage, 
			funcID::kGetVMScriptVariable, 
			funcID::kGetVMQuestVariable, 
			funcID::kHasPerk
		);
		if (!supported) {
			return false;
		}
		if (ctx._checkAll) {
			return true;
		}

		const bool isTargetted = head->data.object.any(RE::CONDITIONITEMOBJECT::kTarget);
		if (isTargetted) {
			return true;
		}

		const auto* runOnRef = head->data.runOnRef.get().get();
		const bool runOnAcceptable = !runOnRef || runOnRef->IsPlayerRef();
		return runOnAcceptable;
	}

	[[nodiscard]] static bool check_all_conditions(const RE::EffectSetting* effect) {
		assert(effect);
		const auto delivery = effect->data.delivery;
		if (delivery == RE::MagicSystem::Delivery::kSelf) {
			return true;
		}

		switch (effect->GetArchetype()) {
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
				return true;
		}
	}

	[[nodiscard]] 
	inline bool evaluate_or_group(RE::TESConditionItem*& head, EvalContext& ctx) {
		auto& params = *ctx._params;
		if (!head) {
			return true;
		}

		bool allOptional = true;
		while (head) {
			const bool evaluate = evaluate_condition(head, ctx);
			allOptional &= !evaluate;
			if (!evaluate) {
				head = head->next;
				continue;
			}

			if (head->IsTrue(params)) {
				bool foundAND = false;
				while (!foundAND) {
					head = head->next;
					foundAND = head ? !head->data.flags.isOR : true;
				}
				return true;
			}
			head = head->next;
		}
		return allOptional;
	}


	template <typename T>
	static bool evaluate_conditions(const T* form, EvalContext& ctx) {
		if (!form) {
			return false;
		}

		auto* head = form->conditions.head;
		auto& params = *ctx._params;
		while (head) {
			const bool evaluate = evaluate_condition(head, ctx);
			const bool isOR = head->data.flags.isOR;

			if (evaluate && !head->IsTrue(params)) {
				return false;
			}

			if (isOR) {
				if (!evaluate_or_group(head, ctx)) {
					return false;
				}
				continue;
			}
			head = head->next;
		}
		return true;
	}

	static bool is_valid(EvalContext& ctx) {
		const bool spellValid = evaluate_conditions(ctx._effect, ctx);
		if (!spellValid) {
			return false;
		}
		return evaluate_conditions(ctx._base, ctx);
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

	static std::string get_formatted_effect_description(const RE::Effect* a_effect) {
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

	[[nodiscard]]
	static bool append_to_description(EvalContext& ctx) noexcept {
		const auto& flags = ctx._base->data.flags;
		if (flags.any(RE::EffectSetting::EffectSettingData::Flag::kHideInUI)) {
			return false;
		}
		return is_valid(ctx);
	}

	[[nodiscard]]
	std::string get_alt_spell_description(RE::SpellItem* a_spell) {
		if (!a_spell || a_spell->effects.empty()) {
			return "";
		}

		std::string newDescription = "";
		EvalContext ctx{};
		auto* player = ctx._player;
		if (!player) {
			REX::ERROR("Failed to get Player Character when constructing conditions."sv);
			return newDescription;
		}

		auto params = RE::ConditionCheckParams(player, player);
		ctx._params = &params; // ctx only lives as long as this function does, so this is fine. maybe.
		const auto& effects = a_spell->effects;
		for (const auto* effect : effects) {
			const auto* base = effect ? effect->baseEffect : nullptr;
			if (!base) {
				continue;
			}

			ctx._effect = effect;
			ctx._base = base;
			ctx._checkAll = check_all_conditions(base);
			if (!append_to_description(ctx)) {
				continue;
			}

			auto proposed = get_formatted_effect_description(effect);
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