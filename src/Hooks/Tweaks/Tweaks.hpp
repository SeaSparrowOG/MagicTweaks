#pragma once

namespace Hooks
{
    namespace Tweaks
    {
        // Description:          3 CALL Hooks -> 3 * 14 -> 42
        // Active Effect Update: 1 CALL Hook            -> 14
        // TODO: Figure out if it is worth it to make desc hook size optional.
        inline constexpr size_t ALLOC_SIZE = 14u + 3 * 14u;

        struct ActiveEffectDurationPatch
        {
            [[nodiscard]] static bool Patch();
            static void Update(RE::ActiveEffect* a_this, float delta);
            inline static REL::Relocation<decltype(Update)> _update;
        };

        struct DescriptionPatch
        {
            [[nodiscard]] static bool Patch();

            static void GetSpellDescription(RE::ItemCard* itemCard, RE::SpellItem* spell, RE::BSString& out);
            static void GetSpellTomeDescription(RE::ItemCard* itemCard, RE::SpellItem* spell, RE::BSString& out);
            static void GetEnchantmentDescription(RE::ItemCard* itemCard, RE::SpellItem* spell, RE::BSString& out);

			inline static REL::Relocation<decltype(GetSpellDescription)> _getSpellDescription;
            inline static REL::Relocation<decltype(GetSpellTomeDescription)> _getSpellTomeDescription;
            inline static REL::Relocation<decltype(GetEnchantmentDescription)> _getEnchantmentDescription;
        };

        struct SheathePatch
        {
            static void Patch();
            static void RaiseLowerHands(RE::PlayerCharacter* a_this, bool draw);
            inline static REL::Relocation<decltype(RaiseLowerHands)> _raiseLowerHands;
        };

        [[nodiscard]] inline bool Install() {
            REX::INFO("  - Installing tweak-related hooks..."sv);
            return true;
        }
    }
}