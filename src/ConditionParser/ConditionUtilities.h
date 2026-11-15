#pragma once

namespace ConditionParser
{
	[[nodiscard]]
	std::string get_alt_spell_description(RE::SpellItem* a_spell);

	[[nodiscard]]
	bool is_effect_valid(const RE::Effect* effect); 
}