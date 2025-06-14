#pragma once

namespace Settings
{
	namespace INI
	{
		bool Read();

		class Holder : public Utilities::Singleton::ISingleton<Holder>
		{
		public:
			bool Read();

			template <typename T>
			std::optional<T> GetStoredSetting(std::string a_settingName) {
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

			bool VerifySettings();
			bool OverrideSettings();
		};

		inline static constexpr const char* TWEAK_COST_REDUCTION = "Tweaks|bTweakCostReduction";
		inline static constexpr const char* TWEAK_COST_REDUCTION_WEIGHT = "Tweaks|fCostReductionSkillWeight";
		inline static constexpr const char* TWEAK_COST_REDUCTION_MAX = "Tweaks|fMaxSpellCostReduction";
		inline static constexpr const char* TWEAK_COST_REDUCTION_MAX_SKILL = "Tweaks|fMaximumSpellSkill";
		inline static constexpr const char* TWEAK_COST_REDUCTION_MIN_SKILL = "Tweaks|fMinimumSpellSkill";
	}
}
