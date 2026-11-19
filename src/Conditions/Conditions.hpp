#pragma once

namespace Conditions
{
    [[nodiscard]]
    std::vector<RE::Effect*> get_valid_effects(const RE::SpellItem* spell);

    [[nodiscard]]
    std::optional<int32_t> get_alt_item_count(const RE::TESBoundObject* form, const RE::TESObjectREFR* ref);
}