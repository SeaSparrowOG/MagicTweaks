#include "Hooks.h"

namespace Hooks 
{
	void Install()
	{
		logger::info("==========================================================");
		logger::info("Installing Hooks...");
		// SKSE::AllocTrampoline(1024);
		// HookClass::GetSingleton()->Install();
	}

	void HookClass::Install()
	{
		Hook1::Install();
	}

	void HookClass::Hook1::Install()
	{
		logger::info("  >Installing hook 1"sv);
		auto& trampoline = SKSE::GetTrampoline();
		REL::Relocation<std::uintptr_t> target{ addressID, offset };

		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			SKSE::stl::report_and_fail("Failed to match executable's pattern for Hook1, aborting load.");
		}
		_hook = trampoline.write_call<5>(target.address(), &Hook);
	}

	void HookClass::Hook1::Hook()
	{
		_hook();
	}
}