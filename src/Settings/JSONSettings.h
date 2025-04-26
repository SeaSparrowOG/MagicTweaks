#pragma once

namespace Settings
{
	namespace JSON
	{
		bool Read();

		class Holder : public Utilities::Singleton::ISingleton<Holder>
		{
		public:
			bool Read();

		private:
			void ParseEntry(const Json::Value& a_entry, const std::string& a_fileName);
			void ParseDispelEntry(const Json::Value& a_entry);
			void ParseQuestEntry(const Json::Value& a_entry);

			const std::string DISPEL_ON_SEATHE_FIELD{ "DispelOnSeathe" };
			const std::string IGNORE_QUEST_DIALOGUE{ "IgnoreQuestDialogue" };

			std::vector<const RE::EffectSetting*> effectsToDispel{};
			std::vector<const RE::TESQuest*> questsToIgnore{};
		};
	}
}
