#include "INIHolder.hpp"

#include "ClibUtil/simpleINI.hpp"

namespace Settings::INI {
    bool INIHolder::Preload() noexcept {
        std::filesystem::path base{
            fmt::format(R"(.\Data\SKSE\Plugins\{}.ini)"sv, Plugin::NAME)
        };
        if (!std::filesystem::exists(base)) {
            REX::CRITICAL("    >Failed to look up base INI!"sv);
            return false;
        }

        std::filesystem::path custom{
            fmt::format(R"(.\Data\SKSE\Plugins\{}_Custom.ini)"sv, Plugin::NAME)
        };

        bool success = true;
        REX::INFO("    >Reading Base INI..."sv);
        success &= ReadINI(base.string().data());
        REX::INFO("    >Reading Custom INI..."sv);
        success &= std::filesystem::exists(custom) ? ReadINI(custom.string().data()) : true;

        if (success) {
            REX::INFO("    >Finished reading INI settings:"sv);
            DumpSettings();
        }
        return success;
    }

    bool INIHolder::ReadINI(const char* path) {
        CSimpleIniA ini{};
        try {
            ini.SetUnicode();
			ini.LoadFile(path);

			std::list<CSimpleIniA::Entry> sections{};
			ini.GetAllSections(sections);

			if (sections.empty()) {
				return true;
			}

            std::string name{};
            std::string val{};
			for (const auto& section : sections) {
				std::list<CSimpleIniA::Entry> sectionKeys{};
				ini.GetAllKeys(section.pItem, sectionKeys);

				if (sectionKeys.empty()) {
					REX::WARN("  >INI section {} has no settings. This MAY be normal.", section.pItem);
					continue;
				}

				for (const auto& key : sectionKeys) {
					name = std::string(section.pItem) + "|" + std::string(key.pItem);
					val = std::string(ini.GetValue(section.pItem, key.pItem));
                    _settings.insert({name, val});
                }
            }
        }
        catch (std::exception& e) {
            REX::CRITICAL("    >Caught exception {} while iterating over {}."sv, e.what(), path);
            return false;
        }
        return true;
    }

    void INIHolder::DumpSettings() const noexcept {
        std::vector<std::pair<std::string, std::string>> parsed{};
        parsed.reserve(_settings.size());

        size_t longest = 0u;
        std::string nameStr{};
        std::string valStr{};
        for (const auto& [name, val] : _settings) {
            nameStr = std::string(name);
            if (nameStr.size() > longest) {
                longest = nameStr.size();
            }

            valStr = std::string(val);
            parsed.emplace_back(std::make_pair(nameStr, valStr));
        }
        std::ranges::sort(parsed, [](const auto& lhs, const auto& rhs){
            return lhs.first < rhs.first;
        });

        for (const auto& [name, val] : parsed) {
            REX::INFO("      {}{}: {}"sv, name, std::string(longest - name.size(), ' '), val);
        }
    }
}