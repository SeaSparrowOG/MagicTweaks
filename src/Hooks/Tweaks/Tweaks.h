#pragma once

#include <unordered_set>

namespace Hooks::Tweaks {
	class EffectExtender :
		public Utilities::Singleton::ISingleton<EffectExtender>
	{
	public:
		bool Install();
		void ExtendEffectIfNecessary(RE::ActiveEffect* a_effect, float a_extension);

	private:
		bool extendInDialogue{ false };
	};

	class CommentSilencer :
		public Utilities::Singleton::ISingleton<CommentSilencer>
	{
	public:
		bool Install();
		void AppendQuest(const RE::TESQuest* a_candidate);

	private:
		inline static RE::DialogueItem* Thunk(
			RE::DialogueItem* a_dialogueItem,
			RE::TESQuest* a_quest,
			RE::TESTopic* a_topic,
			RE::TESTopicInfo* a_topicInfo,
			RE::TESObjectREFR* a_speaker);

		inline static REL::Relocation<decltype(&CommentSilencer::Thunk)> _func;

		std::unordered_set<const RE::TESQuest*> blacklistedQuests{};
	};

	class SpellDispeler :
		public Utilities::Singleton::ISingleton<SpellDispeler>
	{
	public:
		bool Install();
		void ApendEffect(const RE::EffectSetting* a_candidate);
		void ClearDispelableSpells(RE::PlayerCharacter* a_player);

	private:
		struct PlayerDrawMonitor
		{
			static bool Install();
			static void Thunk(RE::PlayerCharacter* a_this, bool a_draw);

			inline static REL::Relocation<decltype(&Thunk)> _func;

			inline static size_t offset{ 0xA6 };
			inline static std::string setting{ "Tweaks|bDispelOnSheathe" };
		};

		std::unordered_set<const RE::EffectSetting*> dispelEffects{};
	};

	bool Install();
}