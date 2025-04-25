#include "Tweaks.h"

#include "RE/Offset.h"
#include "Settings/INISettings.h"

#include "Hooks/Tweaks/Archetypes/BoundEffect.h"
#include "Hooks/Tweaks/Archetypes/CloakEffect.h"
#include "Hooks/Tweaks/Archetypes/LightEffect.h"
#include "Hooks/Tweaks/Archetypes/ScriptEffect.h"
#include "Hooks/Tweaks/Archetypes/SummonEffect.h"
#include "Hooks/Tweaks/Archetypes/ValueModifierEffect.h"

namespace Hooks::Tweaks
{
	bool Install() {
		auto* listener = EffectExtender::GetSingleton();
		if (!listener) {
			logger::critical("Failed to get Tweaks Listener."sv);
			return false;
		}
		return listener->Install();
	}

	bool EffectExtender::Install() {
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			logger::info("Failed to fetch INI settings holder for Fixes."sv);
			return false;
		}

		auto extendInDialogueRaw = iniHolder->GetStoredSetting<bool>("Tweaks|bExtendEffectsInDialogue");
		extendInDialogue = extendInDialogueRaw.has_value() ? extendInDialogueRaw.value() : false;
		if (!extendInDialogueRaw.has_value()) {
			logger::warn("  >Extend In Dialogue tweak setting somehow not specified in the INI, treating as false."sv);
		}

		return Archetypes::BoundEffect::Install() &&
			Archetypes::CloakEffect::Install() &&
			Archetypes::LightEffect::Install() &&
			Archetypes::ScriptEffect::Install() &&
			Archetypes::SummonEffect::Install() &&
			Archetypes::ValueEffect::Install();
	}

	void EffectExtender::ExtendEffectIfNecessary(RE::ActiveEffect* a_effect, float a_extension) {
		using ActiveEffectFlag = RE::EffectSetting::EffectSettingData::Flag;
		if (!extendInDialogue) {
			return;
		}

		auto* ui = RE::UI::GetSingleton();
		if (!ui || !a_effect) {
			return;
		}

		const auto* caster = a_effect->GetCasterActor().get();
		const auto* target = a_effect->GetTargetActor();
		const auto* base = a_effect->effect ? a_effect->effect->baseEffect : nullptr;
		if (!base || !caster || !target || target != caster) {
			return;
		}

		if (base->data.flags.any(ActiveEffectFlag::kDetrimental,
			ActiveEffectFlag::kHostile, ActiveEffectFlag::kNoDuration)) {
			return;
		}

		if (ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
			a_effect->elapsedSeconds -= a_extension;
		}
	}
}