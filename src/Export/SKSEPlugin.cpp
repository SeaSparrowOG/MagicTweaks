#include "BoundEffectManager/BoundEffectManager.h"
#include "ConditionManager/ConditionManager.h"
#include "Data/ModObjectManager.h"
#include "Events/Events.h"
#include "Hooks/Hooks.h"
#include "Papyrus/Papyrus.h"
#include "Serialization/Serde.h"
#include "Settings/INI/INISettings.h"
#include "Settings/JSON/JSONSettings.h"

static void MessageEventCallback(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		SECTION_SEPARATOR;
		if (!Data::PreloadModObjects()) {
			REX::FAIL("Failed to preload mod objects. Check the log for more information."sv);
		}
		SECTION_SEPARATOR;
		if (!Settings::JSON::Read()) {
			REX::FAIL("Failed to read JSON settings. Check the log for more information."sv);
		}
		SECTION_SEPARATOR;
		if (!ConditionManager::Initialize()) {
			REX::FAIL("Failed to initialize the Condition Manager. Check the log for more information."sv);
		}
		SECTION_SEPARATOR;
		if (!Hooks::ReadSettings()) {
			REX::FAIL("Failed to read hook-related settings. Check the log for more information."sv);
		}

		//TODO: actually do this correctly
		Settings::JSON::Reader::GetSingleton()->settings.clear();

		SECTION_SEPARATOR;
		if (!Events::Register()) {
			REX::FAIL("Failed to register events. Check the log for more information."sv);
		}
		SECTION_SEPARATOR;
		if (!BoundEffectManager::InitializeBoundEffectManager()) {
			REX::FAIL("Failed to initialize the Bound Effect Manager. Check the log for more information."sv);
		}
		SECTION_SEPARATOR;
		REX::INFO("Finished startup tasks, enjoy your game!"sv);
		break;
	default:
		break;
	}
}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []()
	{
		SKSE::PluginVersionData v{};

		v.PluginVersion(Plugin::VERSION);
		v.PluginName(Plugin::NAME);
		v.AuthorName("SeaSparrow"sv);
		v.UsesAddressLibrary();
		v.UsesUpdatedStructs();

		return v;
	}();
#endif

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface * a_skse)
{
	constexpr std::size_t allocSize = 14u * 5u + 33u * 2u;
	SKSE::InitInfo info;
	info.hook = true;
	info.log = true;
	info.logLevel = REX::ELogLevel::Trace;
	info.logName = Plugin::NAME.data();
	info.logPattern = "[%T.%e] [%=5t] [%L] %v";
	info.trampoline = true;
	info.trampolineSize = allocSize;

	SKSE::Init(a_skse, info);
	REX::INFO("Author: SeaSparrow"sv);
	SECTION_SEPARATOR;

	const auto ver = a_skse->RuntimeVersion();

#ifdef SKYRIM_GOG
	static constexpr std::array<REL::Version, 4> supported = 
	{
		SKSE::RUNTIME_SSE_1_6_1130,
		SKSE::RUNTIME_SSE_1_6_1170,
		SKSE::RUNTIME_SSE_1_6_1179,
		REL::Version(1, 6, 1179, 1) // no idea what this is still
	};
#else
	static constexpr std::array<REL::Version, 2> supported = 
	{
		SKSE::RUNTIME_SSE_1_7_104,
		SKSE::RUNTIME_SSE_1_7_99
	};	
#endif

	if ((ver < SKSE::RUNTIME_SSE_LATEST) && (!std::ranges::contains(supported, ver))) {
		REX::CRITICAL("Game Version: {}"sv, ver.string());
		REX::CRITICAL("Supported Versions:"sv);
		for (const auto& allowed : supported) {
			REX::CRITICAL("  - {}"sv, allowed.string());
		}
		REX::FAIL(
			fmt::format("You are using a version not supported by this plugin. Check the log at (Documents/My Games/Skyrim Special Edition/{}.log for more information."sv, Plugin::NAME)
		);
	}

	REX::INFO("Performing startup tasks..."sv);

	SECTION_SEPARATOR;
	if (!Settings::INI::Read()) {
		REX::FAIL("Failed to load INI settings. Check the log for more information."sv);
	}
	SECTION_SEPARATOR;
	if (!Hooks::Install()) {
		REX::FAIL("Failed to install hooks. Check the log for more information."sv);
	}
	SECTION_SEPARATOR;

	SKSE::GetPapyrusInterface()->Register(Papyrus::RegisterFunctions);

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(&MessageEventCallback);

	REX::INFO("Setting up serialization system..."sv);
	const auto serialization = SKSE::GetSerializationInterface();
	serialization->SetUniqueID(Serialization::ID);
	serialization->SetSaveCallback(&Serialization::SaveCallback);
	serialization->SetLoadCallback(&Serialization::LoadCallback);
	serialization->SetRevertCallback(&Serialization::RevertCallback);
	REX::INFO("  >Registered necessary functions."sv);
	SECTION_SEPARATOR;

	return true;
}