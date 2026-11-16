#include "papyrus.hpp"

namespace Papyrus {
	

	static std::vector<int> GetVersion(STATIC_ARGS) {
		return { Plugin::VERSION[0], Plugin::VERSION[1], Plugin::VERSION[2] };
	}

	static void Bind(VM& a_vm) {
		REX::INFO("    >Binding GetVersion..."sv);
		BIND(GetVersion);
	}

	static bool Register(VM* a_vm) {
		REX::INFO("  - Binding papyrus functions in utility script {}..."sv, script);
		Bind(*a_vm);
		REX::INFO("  - Finished binding functions."sv);
		return true;
	}

    bool RegisterFunctions() {
		REX::INFO("Binding Papyrus Functions..."sv);
		const auto* intfc = SKSE::GetPapyrusInterface();
		if (!intfc) {
			REX::CRITICAL("  - Failed to get SKSE's Papyrus Interface, aborting load..."sv);
			return false;
		}
		intfc->Register(&Register);
        return true;
    }
}