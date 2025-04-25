#pragma once

namespace RE
{
	namespace Offset
	{
		// Credit for VTABLE Offsets: PO3, Thiaggo
		namespace BoundItemEffect
		{
			constexpr auto VTABLE_BoundItemEffect = REL::ID(205842);
		}

		namespace Character
		{
			constexpr auto VTABLE_CharacterMagicTarget = REL::ID(207894);
		}

		namespace CloakEffect
		{
			constexpr auto VTABLE_CloakEffect = REL::ID(205849);
		}

		namespace DialogueTopicManager
		{
			constexpr auto SayTopic = REL::ID(25541);
		}

		namespace PlayerCharacter
		{
			constexpr auto VTABLE_PlayerCharacterMagicTarget = REL::ID(208048);
		}

		namespace LightEffect
		{
			constexpr auto VTABLE_LightEffect = REL::ID(205894);
		}

		namespace ScriptEffect
		{
			constexpr auto VTABLE_ScriptEffect = REL::ID(206031);
		}

		namespace SummonCreatureEffect
		{
			constexpr auto VTABLE_SummonCreatureEffect = REL::ID(206083);
		}

		namespace ValueModifierEffect
		{
			constexpr auto VTABLE_ValueModifierEffect = REL::ID(206143);
		}
	}
}