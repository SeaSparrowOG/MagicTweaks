#pragma once

namespace Settings
{
    namespace INI
    {
        template <typename T, typename U>
        concept same_underlying = std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>>;

        template <typename T>
        concept is_bool = same_underlying<T, bool>;

        template <typename T>
        concept is_numeric = !is_bool<T> && (
            std::is_integral_v<T> || std::is_floating_point_v<T>
        );

        template <typename T>
        concept is_string = same_underlying<std::string, T> || same_underlying<std::string_view, T>;
        
        template <typename T>
        concept is_ini_setting = is_string<T> || is_numeric<T> || is_bool<T>;

        class INIHolder : public REX::TSingleton<INIHolder>
        {
        public:
            [[nodiscard]] bool Preload() noexcept;

            template <typename T> requires is_ini_setting<T>
            [[nodiscard]]
            std::optional<T> GetSetting(const std::string_view setting) const noexcept {
                if constexpr (is_string<T>) {
                    auto it = _settings.find(setting);
                    if (it == _settings.end()) {
                        return std::nullopt;
                    }
                    return it->second;
                }

                auto it = _settings.find(setting);
                if (it == _settings.end()) {
                    return std::nullopt;
                }

                std::string_view val = it->second;

                if constexpr (is_bool<T>) {
                    inline constexpr std::array<std::string_view, 2u> truthy = { "1", "true" };
                    inline constexpr std::array<std::string_view, 2u> falsy = { "0", "false" };

                    std::ranges::transform(val, val, [](const char chr){
                        constexpr char  diff = 'a' - 'A';
                        if (chr >= 'A' && chr <= 'Z') {
                            return chr + diff;
                        } 
                        return chr;
                    });

                    if (std::ranges::contains(truthy, val)) {
                        return true;
                    }
                    else if (std::ranges::contains(falsy, val)) {
                        return false;
                    }
                    return std::nullopt;
                }

                T out;
                auto [res, ec] = std::from_chars(val.data(), val.data() + val.size(), out);
                if (ec) {
                    return std::nullopt;
                }
                return out;
            }

        private:
            util::istring_map<std::string> _settings{};

            [[nodiscard]] bool ReadINI(const char* path);
            void DumpSettings() const noexcept;
        };

        inline constexpr const std::string_view FIX_POISON     = "Fixes|bNeverAbsorbPoison";
        inline constexpr const std::string_view FIX_SELF       = "Fixes|bNeverAbsorbSelfTargettingEffects";
        inline constexpr const std::string_view FIX_BENEFICIAL = "Fixes|bNeverAbsorbBeneficialEffects";
        inline constexpr const std::string_view FIX_CLOAKS     = "Fixes|bFixCloakArchetype";

        inline constexpr const std::string_view TWEAK_DIALOGUE       = "Tweaks|bExtendEffectsInDialogue";
        inline constexpr const std::string_view TWEAK_DISPEL         = "Tweaks|bDispelOnSheathe";
        inline constexpr const std::string_view TWEAK_EARN_EXP       = "Tweaks|bEarnConjurationHitExp";
        inline constexpr const std::string_view TWEAK_UI_DESCRIPTION = "Dynamic Spell Description|bInstall";

        inline constexpr const std::string_view REDUCTION_ENABLE    = "Tweaks|bTweakCostReduction";
        inline constexpr const std::string_view REDUCTION_WEIGHT    = "Tweaks|fCostReductionSkillWeight";
        inline constexpr const std::string_view REDUCTION_MIN       = "Tweaks|fMinimumSpellSkill";
        inline constexpr const std::string_view REDUCTION_MAX_SKILL = "Tweaks|fMaximumSpellSkill";
        inline constexpr const std::string_view RREDUCTION_MAX_COST = "Tweaks|fMaxSpellCostReduction";

        inline constexpr const std::string_view MECHANICS_CONDITIONS                 = "Additional Conditions|bInstall";
        inline constexpr const std::string_view MECHANICS_SPELL_BINDING              = "Bound Spells|bInstall";
        inline constexpr const std::string_view MECHANICS_SPELL_BINDING_UI           = "Bound Spells|bShowInUI";
        inline constexpr const std::string_view MECHANICS_SPELL_BINDING_EXP          = "Bound Spells|bEarnExperienceInCombat";
        inline constexpr const std::string_view MECHANICS_MAGICKA_SHIELD             = "Magicka Shield|bInstall";
        inline constexpr const std::string_view MECHANICS_CASTING_SPEED              = "Tweaks|bTweakCastingSpeed";
        inline constexpr const std::string_view MECHANICS_CASTING_SPEED_TIER_PENALTY = "Tweaks|fCastingSlowPercentPerTier";
        inline constexpr const std::string_view MECHANICS_CASTING_SPEED_TIER_BLOCK   = "Tweaks|iMaxSkillDifference";

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

        [[nodiscard]] inline bool Preload() noexcept {
            REX::INFO("  - Initializing INI Settings..."sv);

            auto* holder = INIHolder::GetSingleton();
            if (!holder) {
                REX::CRITICAL("    >Failed to retrieve internal INI settings holder and reader."sv);
                return false;
            }
            return holder->Preload();
        }
    }
}