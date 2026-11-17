#pragma once

#include "INI/INIHolder.hpp"
#include "JSON/JSONHolder.hpp"
#include "ModObjects/ModObjects.hpp"

namespace Settings
{
    [[nodiscard]] inline bool Preload() {
        REX::INFO("Preloading settings..."sv);
        bool success = true;
        success &= INI::Preload();
        success &= JSON::Preload();
        return success;
    }

    [[nodiscard]] inline bool PostLoad() {
        return ModObjects::Load();
    }

#define STATIC_STRING inline constexpr const std::string_view 

    STATIC_STRING FIX_POISON     = "Fixes|bNeverAbsorbPoison";
    STATIC_STRING FIX_SELF       = "Fixes|bNeverAbsorbSelfTargettingEffects";
    STATIC_STRING FIX_BENEFICIAL = "Fixes|bNeverAbsorbBeneficialEffects";
    STATIC_STRING FIX_CLOAKS     = "Fixes|bFixCloakArchetype";

    STATIC_STRING TWEAK_DIALOGUE       = "Tweaks|bExtendEffectsInDialogue";
    STATIC_STRING TWEAK_DISPEL         = "Tweaks|bDispelOnSheathe";
	STATIC_STRING TWEAK_EARN_EXP       = "Tweaks|bEarnConjurationHitExp";
    STATIC_STRING TWEAK_UI_DESCRIPTION = "Dynamic Spell Description|bInstall";

    STATIC_STRING REDUCTION_ENABLE    = "Tweaks|bTweakCostReduction";
    STATIC_STRING REDUCTION_WEIGHT    = "Tweaks|fCostReductionSkillWeight";
    STATIC_STRING REDUCTION_MIN       = "Tweaks|fMinimumSpellSkill";
    STATIC_STRING REDUCTION_MAX_SKILL = "Tweaks|fMaximumSpellSkill";
    STATIC_STRING RREDUCTION_MAX_COST = "Tweaks|fMaxSpellCostReduction";

    STATIC_STRING MECHANICS_CONDITIONS                 = "Additional Conditions|bInstall";
    STATIC_STRING MECHANICS_SPELL_BINDING              = "Bound Spells|bInstall";
    STATIC_STRING MECHANICS_SPELL_BINDING_UI           = "Bound Spells|bShowInUI";
    STATIC_STRING MECHANICS_SPELL_BINDING_EXP          = "Bound Spells|bEarnExperienceInCombat";
    STATIC_STRING MECHANICS_MAGICKA_SHIELD             = "Magicka Shield|bInstall";
    STATIC_STRING MECHANICS_CASTING_SPEED              = "Tweaks|bTweakCastingSpeed";
    STATIC_STRING MECHANICS_CASTING_SPEED_TIER_PENALTY = "Tweaks|fCastingSlowPercentPerTier";
    STATIC_STRING MECHANICS_CASTING_SPEED_TIER_BLOCK   = "Tweaks|iMaxSkillDifference";

    inline constexpr std::array<std::string_view, 20u> INI_SETTINGS = {
        FIX_POISON,
        FIX_SELF,
        FIX_BENEFICIAL,

        TWEAK_DIALOGUE,
        TWEAK_DISPEL,
        TWEAK_EARN_EXP,
        TWEAK_UI_DESCRIPTION,

        REDUCTION_ENABLE,
        REDUCTION_WEIGHT,
        REDUCTION_MIN,
        REDUCTION_MAX_SKILL,
        RREDUCTION_MAX_COST,
        
        MECHANICS_CONDITIONS,
        MECHANICS_SPELL_BINDING,
        MECHANICS_SPELL_BINDING_UI,
        MECHANICS_SPELL_BINDING_EXP,
        MECHANICS_MAGICKA_SHIELD,
        MECHANICS_CASTING_SPEED,
        MECHANICS_CASTING_SPEED_TIER_PENALTY,
        MECHANICS_CASTING_SPEED_TIER_BLOCK
    };

    template <typename T> requires INI::is_ini_setting<T>
    [[nodiscard]]
    inline std::optional<T> GetINISetting(const std::string_view setting) noexcept {
        static_assert(std::ranges::contains(INI_SETTINGS, setting), "Called GetINI setting with an unknown setting.");
        
        static const auto* ini = INI::INIHolder::GetSingleton();
        if (!ini) {
            REX::ERROR("Failed to get internal INI holder during setting request."sv);
            return std::nullopt;
        }
        return ini->GetSetting<T>(setting);
    }

    STATIC_STRING SPELL_BINDING_BINDS_HEALTH =  "MagicBindHealth";
	STATIC_STRING SPELL_BINDING_BINDS_STAMINA = "MagicBindStamina";
	STATIC_STRING SPELL_BINDING_BINDS_MAGICKA = "MagicBindMagicka";

	STATIC_STRING PROXY_COUNT_COMMANDED_CONJURATION = "MGT_PRXY_CommandedConjurationCount";
	STATIC_STRING PROXY_COUNT_SUMMONED = "MGT_PRXY_CommandedConjurationSummon";
	STATIC_STRING PROXY_COUNT_REANIMATED = "MGT_PRXY_CommandedConjurationReanimated";
	STATIC_STRING PROXY_COUNT_SUMMONED_FIRE = "MGT_PRXY_CommandedConjurationFire";
	STATIC_STRING PROXY_COUNT_SUMMONED_FROST = "MGT_PRXY_CommandedConjurationFrost";
	STATIC_STRING PROXY_COUNT_SUMMONED_SHOCK = "MGT_PRXY_CommandedConjurationShock";

    inline constexpr std::array<std::string_view, 9u> GAME_OBJECTS = {
        SPELL_BINDING_BINDS_HEALTH,
        SPELL_BINDING_BINDS_STAMINA,
        SPELL_BINDING_BINDS_MAGICKA,

        PROXY_COUNT_COMMANDED_CONJURATION,
        PROXY_COUNT_SUMMONED,
        PROXY_COUNT_REANIMATED,
        PROXY_COUNT_SUMMONED_FIRE,
        PROXY_COUNT_SUMMONED_FROST,
        PROXY_COUNT_SUMMONED_SHOCK
    };

    template <typename T>
    [[nodiscard]]
    T* GetModObject(const std::string_view edid) noexcept {
        static_assert(std::ranges::contains(GAME_OBJECTS, edid), "Called GetObject with an unknown EditorID");
        return ModObjects::GetObject<T>(edid);
    }
}