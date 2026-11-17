#include "Tweaks.hpp"

#include "RE/Offset.h"
#include "Settings/INI/INIHolder.hpp"

namespace Hooks::Tweaks {

    [[nodiscard]]
    static std::optional<std::string> get_alt_spell_description(const RE::SpellItem* spell) {
        (void)spell;
        return std::nullopt;
    }

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

    bool DescriptionPatch::Patch() {
        REX::INFO("    - Description Patch..."sv);
        bool shouldInstall = Settings::INI::Get<bool>(Settings::INI::TWEAK_UI_DESCRIPTION).value_or(false);
        if (!shouldInstall) {
            REX::INFO("      >User chose not to install the patch."sv);
            return true;
        }

        bool allGood = true;
        auto spellDescriptionTarget = RE::Offset::ItemCard::GetSpellDescription.address();
        auto spellTomeDescriptionTarget = RE::Offset::ItemCard::GetSpellTomeDescription.address();
        auto staffDescriptionTarget = RE::Offset::ItemCard::GetStaffDescription.address();
		if (!REL::Pattern<"E8">().match(spellDescriptionTarget)) {
			REX::CRITICAL("      >Failed to validate the hook pattern for the spell card."sv);
			allGood = false;
		}
		if (!REL::Pattern<"E8">().match(spellTomeDescriptionTarget)) {
			REX::CRITICAL("      >Failed to validate the hook pattern for the spell tome card."sv);
			allGood = false;
		}
		if (!REL::Pattern<"E8">().match(staffDescriptionTarget)) {
			REX::CRITICAL("      >Failed to validate the hook pattern for the staff card."sv);
			allGood = false;
		}
        if (!allGood) {
            return false;
        }

		auto& trampoline = REL::GetTrampoline();

		_getSpellDescription = 
            trampoline.write_call<5>(spellDescriptionTarget, &GetSpellDescription);
		_getSpellTomeDescription = 
            trampoline.write_call<5>(spellTomeDescriptionTarget, &GetSpellTomeDescription);
		_getEnchantmentDescription = 
            trampoline.write_call<5>(staffDescriptionTarget, &GetEnchantmentDescription);
        return true;
    }

    void DescriptionPatch::GetEnchantmentDescription(RE::ItemCard* itemCard, RE::SpellItem* spell, RE::BSString& out) {
        _getEnchantmentDescription(itemCard, spell, out);
        if (auto alt = get_alt_spell_description(spell); alt && !alt.value().empty()) {
            out = alt.value();
        } 
    }

    void DescriptionPatch::GetSpellDescription(RE::ItemCard* itemCard, RE::SpellItem* spell, RE::BSString& out) {
        _getSpellDescription(itemCard, spell, out);
        if (auto alt = get_alt_spell_description(spell); alt && !alt.value().empty()) {
            out = alt.value();
        } 
    }

    void DescriptionPatch::GetSpellTomeDescription(RE::ItemCard* itemCard, RE::SpellItem* spell, RE::BSString& out) {
        _getSpellTomeDescription(itemCard, spell, out);
        if (auto alt = get_alt_spell_description(spell); alt && !alt.value().empty()) {
            out = alt.value();
        } 
    }

    inline void ActiveEffectDurationPatch::Update(RE::ActiveEffect* a_this, float delta) {
        const float previousElapsed = a_this->elapsedSeconds;

		_update(a_this, delta);

        if (extend_duration(a_this)) {
            a_this->elapsedSeconds = previousElapsed;
        }
    }
}