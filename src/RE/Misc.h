#pragma once

#include "offset.h"

namespace RE
{
	RE::DialogueItem* ConstructDialogueItem(
		RE::DialogueItem* a_dialogueItem,
		RE::TESQuest* a_quest,
		RE::TESTopic* a_topic,
		RE::TESTopicInfo* a_topicInfo,
		RE::TESObjectREFR* a_speaker) 
	{
		using func_t = decltype(&ConstructDialogueItem);
		static REL::Relocation<func_t> func{ Offset::DialogueItem::Ctor };
		return func(a_dialogueItem, a_quest, a_topic, a_topicInfo, a_speaker);
	}
}