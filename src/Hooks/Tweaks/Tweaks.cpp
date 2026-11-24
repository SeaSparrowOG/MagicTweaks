#include "Tweaks.hpp"

#include "Conditions/Conditions.hpp"
#include "RE/Offset.h"
#include "Settings/Settings.hpp"

namespace {
    [[nodiscard]] char ascii_lower(const char chr) noexcept 
    { 
		constexpr char diff = 'a' - 'A';
		if (chr < 'A' && chr > 'Z') {
			return chr;
		}
		return chr - diff;
	};

	[[nodiscard]] bool ascii_str_equal_lower(const std::string_view lhs, 
        const std::string_view rhs) noexcept
    {
		const auto lSize = lhs.size();
		if (lSize != rhs.size()) {
			return false;
		}
		for (size_t i = 0u; i < lSize; ++i) {
			if (ascii_lower(lhs[i]) != ascii_lower(rhs[i])) {
				return false;
			}
		}
		return true;
	};

	[[nodiscard]] bool ascii_str_starts_with_lower(const std::string_view str, 
        const std::string_view start) noexcept 
    {
		const auto startSize = start.size();
        if (startSize > str.size()) {
            return false;
        }
        for (size_t i = 0u; i < startSize; ++i) {
            if (ascii_lower(str[i]) != ascii_lower(start[i])) {
                return false;
            }
        }
        return true;
	};

	[[nodiscard]] std::string floored_float_to_str(const float val) {
		const float floored = std::floor(val);
		const int32_t cast = static_cast<int32_t>(floored);
		return std::to_string(cast);
	};

    template <typename T> requires std::is_floating_point_v<T>
    [[nodiscard]]
	std::string float_to_str_to_precision(const T val, std::streamsize precision) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precision) << val;
		return oss.str();
	};

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
		if (ascii_str_equal_lower(str, magTag)) {
			out += floored_float_to_str(data.mag);
		}
		else if (ascii_str_equal_lower(str, durTag)) {
			out += std::to_string(data.dur);
		}
		else if (ascii_str_equal_lower(str, areaTag)) {
			out += std::to_string(data.area);
		}
		else if (ascii_str_starts_with_lower(str, globTag)) {
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
    
    [[nodiscard]]
    static std::optional<std::string> format_effect_description(const RE::Effect* effect) {
        constexpr char openingTag = '<';
		constexpr char closingTag = '>';

		const auto baseEffect = effect ? effect->baseEffect : nullptr;
		if (!baseEffect || baseEffect->magicItemDescription.empty()) {
			return std::nullopt;
		}
        if (baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kHideInUI)) {
            return std::nullopt;
        }

		const std::string_view original = baseEffect->magicItemDescription;
        if (original.empty()) {
            return std::nullopt;
        }

		EffectData data{};
		const auto& effectItem = effect->effectItem;
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
    std::optional<std::string> get_alt_spell_description(const RE::SpellItem* spell) {
        if (!spell) {
            return std::nullopt;
        }
        auto effects = Conditions::get_valid_effects(spell);
        if (effects.empty()) {
            return std::nullopt;
        }

        std::string desc{};
        std::optional<std::string> effDesc = std::nullopt;
        for (const auto* effect : effects) {
            effDesc = format_effect_description(effect);
            if (!effDesc) {
                continue;
            }
            if (!desc.empty() && desc.front() != ' ') {
                desc += ' ' ;
            }
            desc += effDesc.value();
        }
        if (desc.empty()) {
            return std::nullopt;
        }
        return desc;
    }
}
namespace Hooks::Tweaks {

    [[nodiscard]] static bool extend_duration(const RE::ActiveEffect* effect) {
        (void)effect;
        return false;
    }

    bool ActiveEffectDurationPatch::Patch() {
        REX::INFO("    - Active Effect Duration Patch..."sv);

        auto target = RE::Offset::MagicTarget::UpdateActiveEffect.address();
		if (!REL::Pattern<"E8">().match(target)) {
			REX::CRITICAL("      >Failed to validate the hook pattern."sv);
			return false;
		}
		auto& trampoline = REL::GetTrampoline();
		_update = trampoline.write_call<5>(target, &Update);
        return true;
    }

    bool DescriptionPatch::PostLoad() {
        return false;
    }

    bool DescriptionPatch::Patch() {
        REX::INFO("    - Description Patch..."sv);
        bool shouldInstall = Settings::INI::Get<bool>(Settings::INI::TWEAK_UI_DESCRIPTION).value_or(false);
        if (!shouldInstall) {
            REX::INFO("      >User chose not to install the patch."sv);
            return true;
        }

        bool allGood = true;
        const auto spellDescriptionTarget{ RE::Offset::ItemCard::GetSpellDescription };
        const auto spellTomeDescriptionTarget{ RE::Offset::ItemCard::GetSpellTomeDescription };
        const auto staffDescriptionTarget{ RE::Offset::ItemCard::GetStaffDescription };

		if (!spellDescriptionTarget.match()) {
			REX::CRITICAL("      >Failed to validate the hook pattern for the spell card."sv);
			allGood = false;
		}
		if (!spellTomeDescriptionTarget.match()) {
			REX::CRITICAL("      >Failed to validate the hook pattern for the spell tome card."sv);
			allGood = false;
		}
		if (!staffDescriptionTarget.match()) {
			REX::CRITICAL("      >Failed to validate the hook pattern for the staff card."sv);
			allGood = false;
		}
        if (!allGood) {
            return false;
        }

		_getSpellDescription = spellDescriptionTarget.hook(&GetSpellDescription);
		_getSpellTomeDescription = spellDescriptionTarget.hook(&GetSpellTomeDescription);
		_getEnchantmentDescription = spellDescriptionTarget.hook(&GetEnchantmentDescription);
        return true;
    }

    void DescriptionPatch::GetEnchantmentDescription(RE::ItemCard* itemCard, RE::SpellItem* spell, RE::BSString& out) {
        _getEnchantmentDescription(itemCard, spell, out);
        if (auto alt = get_alt_spell_description(spell); alt) {
            out = alt.value();
        } 
    }

    void DescriptionPatch::GetSpellDescription(RE::ItemCard* itemCard, RE::SpellItem* spell, RE::BSString& out) {
        _getSpellDescription(itemCard, spell, out);
        if (auto alt = get_alt_spell_description(spell); alt) {
            out = alt.value();
        } 
    }

    void DescriptionPatch::GetSpellTomeDescription(RE::ItemCard* itemCard, RE::SpellItem* spell, RE::BSString& out) {
        _getSpellTomeDescription(itemCard, spell, out);
        if (auto alt = get_alt_spell_description(spell); alt) {
            out = alt.value();
        }
    }

    void ActiveEffectDurationPatch::Update(RE::ActiveEffect* a_this, float delta) {
        const float previousElapsed = a_this->elapsedSeconds;

		_update(a_this, delta);

        if (extend_duration(a_this)) {
            a_this->elapsedSeconds = previousElapsed;
        }
    }

	[[nodiscard]]
	static bool apply_changes(const Json::Value& strObj) {
		namespace MO = Settings::ModObjects;
		assert(strObj.isString());

		auto* kwd = MO::GetObject<RE::BGSKeyword>(MO::DISPEL_KEYWORD);
		if (!kwd) {
			REX::WARN("Failed to lookup keyword {} in MagicTweaks.esl"sv, MO::DISPEL_KEYWORD);
			return false;
		}

		const std::string_view str = strObj.asString();

		auto spellRes = FormUtils::get_form_from_string<RE::SpellItem>(str);
		if (!spellRes) {
			switch (spellRes.error()) {
				case FormUtils::FailureReason::FormNotInPlugin:
				case FormUtils::FailureReason::PluginNotLoaded:
				case FormUtils::FailureReason::None:
					break;
				default:
					REX::WARN("Query form failed with: {}"sv, 
						FormUtils::failure_to_string(spellRes.error())
					);
					break;
			}
		}
		auto* val = spellRes.value();
		if (!val) {
			return true;
		}

		auto& effects = val->effects;
		if (effects.empty()) {
			return true;
		}

		bool added = true;
		for (auto* effect : effects) {
			auto* base = effect ? effect->baseEffect : nullptr;
			if (!base || base->HasKeyword(kwd)) {
				continue;
			}
			if (!base->AddKeyword(kwd)) {
				std::string failedForm = FormUtils::get_form_name(base);
				REX::WARN("Failed to add keyword {} to {}"sv, MO::DISPEL_KEYWORD, failedForm);
				added = false;
			}
		}
		return added;
	}

	[[nodiscard]]
	static bool apply_config(const Json::Value& val) {
		if (!val.isObject()) {
			return true;
		}
		const auto& dispel = val["dispel"];
		if (!dispel) {
			return true;
		}

		if (dispel.isString()) {
			return apply_changes(dispel);
		}
		if (dispel.isArray()) {
			bool success = true;
			for (const auto& element : dispel) {
				if (!element.isString()) {
					REX::WARN("      - Non-string value in array."sv);
					success = false;
					continue;
				}
				success &= apply_changes(element);
			}
			return success;
		}
		REX::WARN("      - Non object or array item in config."sv);
		return false;
	}

    bool SheathePatch::PostLoad() {
		namespace MO = Settings::ModObjects;
		REX::INFO("  - Sheathe patch..."sv);
		auto* kwd = MO::GetObject<RE::BGSKeyword>(MO::DISPEL_KEYWORD);
		if (!kwd) {
			REX::CRITICAL("    >Failed to find the appropriate keyword within MagicTweaks.esl - plugin likely not active or out of date."sv);
			return false;
		}
		bool success = true;
		const auto& configs = Settings::JSON::Get();
		for (const auto& [name, config] : configs) {
			if (!apply_config(config)) {
				REX::WARN("    >Failed to apply {}", name);
			}
		}
        return success;
    }

    void SheathePatch::Patch() {
        namespace INI = Settings::INI;

		REX::INFO("  - Installing the Spell Dispelling tweak..."sv);
		bool doInstall = INI::Get<bool>(INI::TWEAK_DISPEL).value_or(false);
		if (!doInstall) {
			REX::INFO("    >User chose not to install."sv);
			return;
		}

		static constexpr auto desc{ RE::VFuncs::PlayerCharacter::DrawLowerHands };
		REL::Relocation<std::uintptr_t> target{ desc.vtable[desc.index] };
		_raiseLowerHands = target.write_vfunc(desc.offset, RaiseLowerHands);
    }

    void SheathePatch::RaiseLowerHands(RE::PlayerCharacter* a_this, bool draw) {
		namespace MO = Settings::ModObjects;

		_raiseLowerHands(a_this, draw);
		if (draw) {
			return;
		}
		
		static auto* dispelKwd = MO::GetObject<RE::BGSKeyword>(MO::DISPEL_KEYWORD);
		if (!dispelKwd) {
			return;
		}

		auto* target = a_this->GetMagicTarget();
		if (!target) {
			return;
		}

		// This is... probably not needed. We make a copy of the list instead of
		// just lazily iterating over the ames to collect "hits" and then dispel them.
		auto activeEffects = *target->GetActiveEffectList();
		for (auto* ame : activeEffects) {
			if (!ame || ame->caster.get().get() != a_this) {
				continue;
			}
			const auto* base = ame->GetBaseObject();
			const bool hasKwd = base ? base->HasKeyword(dispelKwd) : false;
			if (!hasKwd) {
				continue;
			}
			ame->Dispel(false);
		}
    }
}