#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

#include "Data/ModObjects.hpp"
#include "Events/Events.hpp"
#include "Hooks/Hooks.hpp"
#include "Papyrus/Papyrus.hpp"
#include "Serialization/Serialization.hpp"
#include "Settings/Settings.hpp"

static void ReportAndFail(const std::string reason) {
    REX::FAIL(
        fmt::format("{} Failed to initialize - {}. Check the log at {}/{}.log for more information.",
            Plugin::NAME,
            reason,
            "Documents/My Games/Skyrim Special Edition/SKSE",
            Plugin::NAME
        )
    );
}

static void InitializeLog() {
#ifndef NDEBUG
	#define SPD_LOG_LEVEL spdlog::level::debug
#else
	#define SPD_LOG_LEVEL spdlog::level::info
#endif

	auto path = SKSE::log::log_directory();
	if (!path)
		return;

	*path /= std::format("{}.log", Plugin::NAME);

	std::vector<spdlog::sink_ptr> sinks{
		std::make_shared<spdlog::sinks::msvc_sink_mt>()
	};
	sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));

	auto logger = std::make_shared<spdlog::logger>("global", sinks.begin(), sinks.end());
	logger->set_level(SPD_LOG_LEVEL);
	logger->flush_on(SPD_LOG_LEVEL);

	spdlog::set_default_logger(std::move(logger));
	spdlog::set_pattern("[%T.%e] [%=5t] [%L] %v");

	REX::INFO("{} v{}", Plugin::NAME, Plugin::VERSION.string());
}

static void MessageEventCallback(SKSE::MessagingInterface::Message* a_msg) {
	switch (a_msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
			SECTION_SEPARATOR;
			if (!Data::Initialize()) {
				ReportAndFail("Failed to preload game objects");
			}
			SECTION_SEPARATOR;
			if (!Events::Initialize()) {
				ReportAndFail("Failed to initiliaze event listeners");
			}
			SECTION_SEPARATOR;
			REX::INFO("Finished startup tasks. Enjoy your game!"sv);
			SECTION_SEPARATOR;
            break;
        default:
            break;
	}
}

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

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface * a_skse) {
	InitializeLog();

	SECTION_SEPARATOR;
	if (!Settings::Preload()) {
		ReportAndFail("Failed to preload settings");
	}

	SECTION_SEPARATOR;
	SKSE::InitInfo info;
	info.log = false;
	info.hook = Hooks::plugin_hooks;
	info.trampoline = Hooks::plugin_hooks;
	info.trampolineSize = Hooks::alloc_size;

	SKSE::Init(a_skse, info);
	REX::INFO("Author: SeaSparrow"sv);

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

	SECTION_SEPARATOR;
	if (!Hooks::Install()) {
		ReportAndFail("Failed to install hooks");
	}
	SECTION_SEPARATOR;
	if (!Papyrus::RegisterFunctions()) {
		ReportAndFail("Failed to register papyrus functions");
	}
	SECTION_SEPARATOR;
	if (!Serialization::Initialize()) {
		ReportAndFail("Serialization failed to initialize");
	}
	
	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(&MessageEventCallback);

	return true;
}
