#include "JSONSettings.h"

#include "Hooks/Tweaks/Tweaks.h"

namespace Settings::JSON
{
	bool Read() {
		logger::info("Reading JSON configs..."sv);
		auto* holder = Holder::GetSingleton();
		if (!holder) {
			logger::critical("  >Failed to get JSON setting holder."sv);
			return false;
		}
		return holder->Read();
	}

	bool Holder::Read() {
		auto* commentSilencer = Hooks::Tweaks::CommentSilencer::GetSingleton();
		auto* dispelManager = Hooks::Tweaks::SpellDispeler::GetSingleton();
		if (!commentSilencer || !dispelManager) {
			logger::critical("Failed to get internal singletons."sv);
			return false;
		}

		std::string jsonFolder = fmt::format(R"(.\Data\SKSE\Plugins\{})"sv, Plugin::NAME);
		std::vector<std::string> paths{};
		try {
			for (const auto& entry : std::filesystem::directory_iterator(jsonFolder)) {
				if (entry.is_regular_file() && entry.path().extension() == ".json") {
					paths.push_back(entry.path().string());
				}
			}
			std::sort(paths.begin(), paths.end());
			logger::info("  >Found {} configuration files."sv, paths.size());
		}
		catch (const std::exception& e) {
			logger::warn("  >Caught {} while reading files.", e.what());
			return false;
		}
		if (paths.empty()) {
			return true;
		}

		for (const auto& path : paths) {
			Json::Reader JSONReader;
			Json::Value JSONFile;
			try {
				std::ifstream rawJSON(path);
				JSONReader.parse(rawJSON, JSONFile, false);
				
				std::string name = path.substr(jsonFolder.size() + 1, path.size() - 1);
				ParseEntry(JSONFile, name);
			}
			catch (const Json::Exception& e) {
				logger::warn("  >Caught {} while reading files.", e.what());
				continue;
			}
			catch (const std::exception& e) {
				logger::error("  >Caught unhandled exception {} while reading files.", e.what());
				continue;
			}
		}

		if (!effectsToDispel.empty()) {
			logger::info("  >Found {} effects to dispel."sv, effectsToDispel.size());
			for (const auto* effect : effectsToDispel) {
				dispelManager->ApendEffect(effect);
			}
		}
		if (!questsToIgnore.empty()) {
			logger::info("  >Found {} quests to ignore."sv, questsToIgnore.size());
			for (const auto* quest : questsToIgnore) {
				commentSilencer->AppendQuest(quest);
			}
		}
		return true;
	}

	void Holder::ParseEntry(const Json::Value& a_entry, 
		const std::string& a_fileName) 
	{
		logger::info("    >Parsing {}..."sv, a_fileName);

		if (a_entry.empty()) {
			logger::warn("      >Empty");
			return;
		}
		else if (!a_entry.isObject()) {
			logger::warn("      >Invalid structure: Top level is not an object."sv);
			return;
		}

		const auto& dispelTargets = a_entry[DISPEL_ON_SEATHE_FIELD];
		if (dispelTargets) {
			if (!dispelTargets.isArray()) {
				logger::warn("      >Invalid {} field. Expected an array.", DISPEL_ON_SEATHE_FIELD);
			}
			else {
				ParseDispelEntry(dispelTargets);
			}
		}

		const auto& ignoreQuestDialogue = a_entry[IGNORE_QUEST_DIALOGUE];
		if (ignoreQuestDialogue) {
			if (!ignoreQuestDialogue.isArray()) {
				logger::warn("      >Invalid {} field. Expected an array.", IGNORE_QUEST_DIALOGUE);
			}
			else {
				ParseQuestEntry(dispelTargets);
			}
		}
	}

	void Holder::ParseDispelEntry(const Json::Value& a_entry) {
		using ActiveEffectFlag = RE::EffectSetting::EffectSettingData::Flag;
		if (a_entry.empty()) {
			return;
		}
		
		std::vector<const RE::EffectSetting*> foundForms{};
		foundForms.reserve(a_entry.size());

		for (const auto& entry : a_entry) {
			if (!entry.isString()) {
				logger::warn("      >{} contains non-string entries, aborting reading."sv, DISPEL_ON_SEATHE_FIELD);
				return;
			}
			
			const auto* foundForm = Utilities::Forms::GetFormFromString<RE::EffectSetting>(entry.asString());
			if (foundForm && !foundForm->data.flags.any(ActiveEffectFlag::kNoDuration)) {
				foundForms.push_back(foundForm);
			}
			else {
				const auto* alternateForm = Utilities::Forms::GetFormFromString<RE::SpellItem>(entry.asString());
				if (alternateForm && !alternateForm->effects.empty()) {
					const auto& effects = alternateForm->effects;
					for (const auto* effect : effects) {
						const auto* base = effect ? effect->baseEffect : nullptr;
						if (!base || 
							base->data.flags.any(ActiveEffectFlag::kNoDuration) ||
							effect->effectItem.duration < 1) {
							continue;
						}
						foundForms.push_back(base);
					}
				}
			}
		}

		if (foundForms.empty()) {
			logger::warn("      >No effects resolved in {}.", DISPEL_ON_SEATHE_FIELD);
			return;
		}
		
		for (const auto* spell : foundForms) {
			effectsToDispel.push_back(spell);
		}
	}

	void Holder::ParseQuestEntry(const Json::Value& a_entry) {
		if (a_entry.empty()) {
			return;
		}

		std::vector<const RE::TESQuest*> foundForms{};
		foundForms.reserve(a_entry.size());

		for (const auto& entry : a_entry) {
			if (!entry.isString()) {
				logger::warn("      >{} contains non-string entries, aborting reading."sv, DISPEL_ON_SEATHE_FIELD);
				return;
			}

			const auto* foundForm = Utilities::Forms::GetFormFromString<RE::TESQuest>(entry.asString());
			if (foundForm) {
				foundForms.push_back(foundForm);
			}
		}

		if (foundForms.empty()) {
			logger::warn("      >No spells resolved in {}.", DISPEL_ON_SEATHE_FIELD);
			return;
		}

		for (const auto* form : foundForms) {
			questsToIgnore.push_back(form);
		}
	}
}
