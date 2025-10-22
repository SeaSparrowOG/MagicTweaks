#pragma once

namespace Settings
{
	namespace INI
	{
		bool Read();

		class Holder :
			public REX::Singleton<Holder>
		{
		public:
			bool StoreSettings();
			void DumpSettings();

			template <typename T>
			std::optional<T> GetStoredSetting(const std::string& a_settingName) {
				if constexpr (std::is_same_v<T, float>) {
					auto it = floatSettings.find(a_settingName);
					if (it != floatSettings.end()) return it->second;
				}
				else if constexpr (std::is_same_v<T, std::string>) {
					auto it = stringSettings.find(a_settingName);
					if (it != stringSettings.end()) return it->second;
				}
				else if constexpr (std::is_same_v<T, long>) {
					auto it = longSettings.find(a_settingName);
					if (it != longSettings.end()) return it->second;
				}
				else if constexpr (std::is_same_v<T, bool>) {
					auto it = boolSettings.find(a_settingName);
					if (it != boolSettings.end()) return it->second;
				}
				else {
					static_assert(always_false<T>, "Called GetStoredSetting with unsupported type.");
				}
				return std::nullopt;
			}

		private:
			std::map<std::string, long>        longSettings;
			std::map<std::string, bool>        boolSettings;
			std::map<std::string, float>       floatSettings;
			std::map<std::string, std::string> stringSettings;

			bool OverrideSettings();
		};

		inline static constexpr const char* FIX_POISON = "Fixes|bNeverAbsorbPoison";
		inline static constexpr const char* FIX_SELF = "Fixes|bNeverAbsorbSelfTargettingEffects";
		inline static constexpr const char* FIX_BENEFICIAL = "Fixes|bNeverAbsorbBeneficialEffects";
		inline static constexpr const char* FIX_CLOAKS = "Fixes|bFixCloakArchetype";

		inline static constexpr const char* TWEAK_DIALOGUE = "Tweaks|bExtendEffectsInDialogue";
		inline static constexpr const char* TWEAK_DIALOGUE_WEAPONS = "Tweaks|bTweakBoundWeapons";
		inline static constexpr const char* TWEAK_DIALOGUE_CLOAKS = "Tweaks|bTweakCloaks";
		inline static constexpr const char* TWEAK_DIALOGUE_LIGHTS = "Tweaks|bTweakLight";
		inline static constexpr const char* TWEAK_DIALOGUE_SCRIPTS = "Tweaks|bTweakScripts";
		inline static constexpr const char* TWEAK_DIALOGUE_SUMMONS = "Tweaks|bTweakSummons";
		inline static constexpr const char* TWEAK_DIALOGUE_VALUES = "Tweaks|bTweakValue";

		inline static constexpr const char* TWEAK_DISPEL = "Tweaks|bDispelOnSheathe";
		inline static constexpr const char* TWEAK_EARN_EXP = "Tweaks|bEarnConjurationHitExp";

		inline static constexpr const char* TWEAK_REDUCTION = "Tweaks|bTweakCostReduction";
		inline static constexpr const char* TWEAK_REDUCTION_WEIGHT = "Tweaks|fCostReductionSkillWeight";
		inline static constexpr const char* TWEAK_REDUCTION_MIN = "Tweaks|fMinimumSpellSkill";
		inline static constexpr const char* TWEAK_REDUCTION_MAX = "Tweaks|fMaximumSpellSkill";
		inline static constexpr const char* TWEAK_REDUCTION_REDUCTION_MAX = "Tweaks|fMaxSpellCostReduction";

		inline static constexpr const char* ADDITIONAL_CONDITIONS = "Additional Conditions|bInstall";
		inline static constexpr const char* DYNAMIC_SPELL_DESCRIPTIONS = "Dynamic Spell Description|bInstall";
		inline static constexpr const char* BOUND_SPELLS = "Bound Spells|bInstall";
		inline static constexpr const char* BOUND_SPELLS_UI = "Bound Spells|bShowInUI";
		inline static constexpr const char* BOUND_SPELLS_EXP = "Bound Spells|bEarnExperienceInCombat";
		inline static constexpr const char* MAGICKA_SHIELD = "Magicka Shield|bInstall";

		inline static constexpr const std::uint8_t EXPECTED_COUNT = 24;
		inline static constexpr const std::array<const char*, EXPECTED_COUNT> EXPECTED_SETTINGS = {
			FIX_POISON,
			FIX_SELF,
			FIX_BENEFICIAL,
			FIX_CLOAKS,
			TWEAK_DIALOGUE,
			TWEAK_DIALOGUE_WEAPONS,
			TWEAK_DIALOGUE_CLOAKS,
			TWEAK_DIALOGUE_LIGHTS,
			TWEAK_DIALOGUE_SCRIPTS,
			TWEAK_DIALOGUE_SUMMONS,
			TWEAK_DIALOGUE_VALUES,
			TWEAK_DISPEL,
			TWEAK_EARN_EXP,
			TWEAK_REDUCTION,
			TWEAK_REDUCTION_WEIGHT,
			TWEAK_REDUCTION_MIN,
			TWEAK_REDUCTION_MAX,
			TWEAK_REDUCTION_REDUCTION_MAX,
			ADDITIONAL_CONDITIONS,
			DYNAMIC_SPELL_DESCRIPTIONS,
			BOUND_SPELLS,
			BOUND_SPELLS_UI,
			BOUND_SPELLS_EXP,
			MAGICKA_SHIELD
		};

		template <typename T>
		std::optional<T> GetSetting(const std::string& a_settingName) {			
			auto* holder = Holder::GetSingleton();
			return holder ? holder->GetStoredSetting<T>(a_settingName) : std::nullopt;
		}
	}
}