#include "Papyrus/Papyrus.h"

#include "RE/Misc.h"
#include "Settings/INISettings.h"

namespace Papyrus
{
	std::vector<int> GetVersion(STATIC_ARGS) {
		return { Plugin::VERSION[0], Plugin::VERSION[1], Plugin::VERSION[2] };
	}

	void Verify(STATIC_ARGS, RE::BGSMessage* a_verificationMessage) {
		if (!a_verificationMessage) {
			a_vm->TraceStack("NONE message passed.", a_stackID);
			return;
		}

		auto* settings = Settings::INI::Holder::GetSingleton();
		if (!settings) {
			a_vm->TraceStack("Failed to get settings singleton.", a_stackID);
			return;
		}

		auto response = settings->GetStoredSetting<bool>("Debug|bEnableDebugMessages");
		if (!response.has_value() || !response.value()) {
			return;
		}

		RE::BSString out = "";
		a_verificationMessage->GetDescription(out, nullptr);
		if (out.empty()) {
			a_vm->TraceStack("Tried to display empty message.", a_stackID);
			return;
		}

		if (a_verificationMessage->flags.any(RE::BGSMessage::MessageFlag::kMessageBox)) {
			RE::DebugMessageBox(out.c_str());
		}
		else {
			RE::DebugNotification(out.c_str(), "", true);
		}
	}

	void Bind(VM& a_vm) {
		BIND(GetVersion);
		BIND(Verify);
	}

	bool RegisterFunctions(VM* a_vm) {
		Bind(*a_vm);
		return true;
	}
}