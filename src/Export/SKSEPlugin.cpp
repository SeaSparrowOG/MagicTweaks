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

SKSE_PLUGIN_QUERY(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		REX::CRITICAL("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
#ifdef SKYRIM_AE
	if (ver < SKSE::RUNTIME_SSE_LATEST) {
#else
	if (ver < SKSE::RUNTIME_1_5_39) {
#endif
		REX::CRITICAL("Unsupported runtime version {}", ver.string());
		return false;
	}

	return true;
	}

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

#ifdef SKYRIM_AE
	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_SSE_LATEST) {
		return false;
	}
#endif

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