#pragma once

namespace ConditionParser
{
	inline static constexpr std::string_view openingTag{ "<font face=\'$EverywhereMediumFont\' size=\'20\' color=\'#FFFFFF\'>" };
	inline static constexpr std::string_view closingTag{ "</font>" };
	inline static std::regex pattern{ R"(<(.*?)>)" };

	std::string GetFormattedDescription(RE::SpellItem* a_spell);
}