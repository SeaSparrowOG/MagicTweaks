#include "JSONSettings.h"

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
				JSONReader.parse(rawJSON, JSONFile);
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
