#include "Tweaks.h"

#include "RE/Offset.h"
#include "RE/Misc.h"
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
		auto* extender = EffectExtender::GetSingleton();
		auto* silencer = CommentSilencer::GetSingleton();
		if (!extender || !silencer) {
			logger::critical("Failed to get internal hook listeners."sv);
			return false;
		}
		return extender->Install() &&
			silencer->Install();
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

	bool CommentSilencer::Install() {
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			logger::critical("Failed to fetch ini settings holder."sv);
			return false;
		}

		auto installRaw = iniHolder->GetStoredSetting<bool>("Tweaks|bSuppressMagicComments");
		bool install = installRaw.has_value() ? installRaw.value() : false;
		if (!installRaw.has_value()) {
			logger::warn("  >Setting {} not found in ini settings, treating as false.", "Tweaks|bSuppressMagicComments");
		}
		if (!install) {
			return true;
		}

		REL::Relocation<std::uintptr_t> target{ RE::Offset::DialogueTopicManager::SayTopic, 0xE2 };
		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			logger::critical("Failed to validate pattern of the Comment Silencer."sv);
			return false;
		}

		auto& trampoline = SKSE::GetTrampoline();
		_func = trampoline.write_call<5>(target.address(), &Thunk);
		return true;
	}

	void CommentSilencer::AppendQuest(const RE::TESQuest* a_candidate) {
		if (blacklistedQuests.contains(a_candidate)) {
			return;
		}
		blacklistedQuests.emplace(a_candidate);
	}

	RE::DialogueItem* CommentSilencer::Thunk(RE::DialogueItem* a_dialogueItem,
		RE::TESQuest* a_quest,
		RE::TESTopic* a_topic,
		RE::TESTopicInfo* a_topicInfo,
		RE::TESObjectREFR* a_speaker) {
		auto* silencer = CommentSilencer::GetSingleton();
		auto* response = _func(a_dialogueItem, a_quest, a_topic, a_topicInfo, a_speaker);
		if (silencer && silencer->blacklistedQuests.contains(a_quest)) {
			delete response;
			return nullptr;
		}

		return RE::ConstructDialogueItem(a_dialogueItem, a_quest, a_topic, a_topicInfo, a_speaker);
	}
}