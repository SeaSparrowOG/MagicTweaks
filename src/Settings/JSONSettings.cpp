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
		auto* dispelManager = Hooks::Tweaks::SpellDispeler::GetSingleton();
		if (!dispelManager) {
			logger::critical("Failed to get internal singletons."sv);
			return false;
		}

		dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler) {
			logger::critical("Failed to get game's data handler."sv);
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

			const auto* foundForm = GetFormFromString<RE::EffectSetting>(entry.asString());
			if (foundForm && !foundForm->data.flags.any(ActiveEffectFlag::kNoDuration)) {
				foundForms.push_back(foundForm);
			}
			else {
				const auto* alternateForm = GetFormFromString<RE::SpellItem>(entry.asString());
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
}

namespace Settings::JSON
{
	std::string Holder::GetEditorID(const RE::TESForm* a_form)
	{
		switch (a_form->GetFormType()) {
		case RE::FormType::Keyword:
		case RE::FormType::LocationRefType:
		case RE::FormType::Action:
		case RE::FormType::MenuIcon:
		case RE::FormType::Global:
		case RE::FormType::HeadPart:
		case RE::FormType::Race:
		case RE::FormType::Sound:
		case RE::FormType::Script:
		case RE::FormType::Navigation:
		case RE::FormType::Cell:
		case RE::FormType::WorldSpace:
		case RE::FormType::Land:
		case RE::FormType::NavMesh:
		case RE::FormType::Dialogue:
		case RE::FormType::Quest:
		case RE::FormType::Idle:
		case RE::FormType::AnimatedObject:
		case RE::FormType::ImageAdapter:
		case RE::FormType::VoiceType:
		case RE::FormType::Ragdoll:
		case RE::FormType::DefaultObject:
		case RE::FormType::MusicType:
		case RE::FormType::StoryManagerBranchNode:
		case RE::FormType::StoryManagerQuestNode:
		case RE::FormType::StoryManagerEventNode:
		case RE::FormType::SoundRecord:
			return a_form->GetFormEditorID();
		default:
		{
			static auto tweaks = REX::W32::GetModuleHandleW(L"po3_Tweaks");
			static auto func = reinterpret_cast<_GetFormEditorID>(REX::W32::GetProcAddress(tweaks, "GetFormEditorID"));
			if (func) {
				return func(a_form->formID);
			}
			return {};
		}
		}
	}

	inline std::vector<std::string> Holder::split(const std::string& a_str, const std::string& a_delimiter)
	{
		std::vector<std::string> result;
		size_t start = 0;
		size_t end = a_str.find(a_delimiter);

		while (end != std::string::npos) {
			result.push_back(a_str.substr(start, end - start));
			start = end + a_delimiter.length();
			end = a_str.find(a_delimiter, start);
		}

		result.push_back(a_str.substr(start));
		return result;
	}

	inline bool Holder::is_only_hex(std::string_view a_str, bool a_requirePrefix)
	{
		if (!a_requirePrefix) {
			return std::ranges::all_of(a_str, [](unsigned char ch) {
				return std::isxdigit(ch);
				});
		}
		else if (a_str.compare(0, 2, "0x") == 0 || a_str.compare(0, 2, "0X") == 0) {
			return a_str.size() > 2 && std::all_of(a_str.begin() + 2, a_str.end(), [](unsigned char ch) {
				return std::isxdigit(ch);
				});
		}
		return false;
	}

	inline std::string Holder::tolower(std::string_view a_str)
	{
		std::string result(a_str);
		std::ranges::transform(result, result.begin(), [](unsigned char ch) { return static_cast<unsigned char>(std::tolower(ch)); });
		return result;
	}
}