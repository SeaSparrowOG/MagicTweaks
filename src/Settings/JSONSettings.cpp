#include "JSONSettings.h"

#include "Hooks/Tweaks/Tweaks.h"

namespace Settings::JSON
{
	bool Read() {
		auto* holder = Holder::GetSingleton();
		if (!holder) {
			logger::critical("Failed to get JSON setting holder."sv);
			return false;
		}
		return holder->Read();
	}

	bool Holder::Read() {
		auto* commentSilencer = Hooks::Tweaks::CommentSilencer::GetSingleton();
		if (!commentSilencer) {
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
		}
		catch (const std::exception& e) {
			logger::warn("Caught {} while reading files.", e.what());
			return false;
		}
		if (paths.empty()) {
			logger::info("No settings found");
			return true;
		}

		for (const auto& path : paths) {
			Json::Reader JSONReader;
			Json::Value JSONFile;
			try {
				std::ifstream rawJSON(path);
				JSONReader.parse(rawJSON, JSONFile, false);
				if (!JSONFile.isObject()) {
					logger::warn("  >File {} is not valid. Top level should be an object."sv, path);
					continue;
				}

				const auto& blacklistedQuests = JSONFile["blacklistedQuests"];
				if (!blacklistedQuests) {
					logger::warn("  >File {} does not have blacklisted quests."sv, path);
					continue;
				}
				if (!blacklistedQuests.isArray()) {
					logger::warn("  >File {} has blacklisted quests, but the format is not valid. This field needs to be an array."sv, path);
					continue;
				}

				std::vector<const RE::TESQuest*> candidateQuests{};
				const auto begin = blacklistedQuests.begin();
				const auto end = blacklistedQuests.end();
				bool errored = false;
				for (auto it = begin; !errored && it != end; ++it) {
					const auto& rawQuest = *it;
					if (!rawQuest || !rawQuest.isString()) {
						logger::warn("  >File {} has a non-string blacklisted quest. All quests in this config will be ignored."sv, path);
						errored = true;
						continue;
					}

					const auto* candidate = Utilities::Forms::GetFormFromString<RE::TESQuest>(rawQuest.asString());
					if (!candidate) {
						logger::warn("  >File {} has form {}, but it could not be found."sv, path, rawQuest.asString());
						continue;
					}
					candidateQuests.push_back(candidate);
				}
				if (candidateQuests.empty() || errored) {
					continue;
				}
				for (const auto* candidate : candidateQuests) {
					commentSilencer->AppendQuest(candidate);
				}
			}
			catch (const Json::Exception& e) {
				logger::warn("Caught {} while reading files.", e.what());
				continue;
			}
			catch (const std::exception& e) {
				logger::error("Caught unhandled exception {} while reading files.", e.what());
				continue;
			}
		}
		return true;
	}
}
