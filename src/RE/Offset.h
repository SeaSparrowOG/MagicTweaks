#pragma once

namespace RE::Offset
{
    namespace ItemCard
    {
        // Calls to get the description of a magic item. 
        // Used in Hooks->Tweaks->DescriptionPatch
        // Arguments: ItemCard*, SpellItem*, BSString&
        // Return: nothing
        inline static REL::Relocation<std::uintptr_t> GetSpellDescription{ REL::ID(51898), 0x5DC };
        inline static REL::Relocation<std::uintptr_t> GetSpellTomeDescription{ REL::ID(51897), 0xF5C };
        inline static REL::Relocation<std::uintptr_t> GetStaffDescription{ REL::ID(51897), 0x1377 };
    }
    namespace MagicTarget 
    {
        // Periodically ran on targets that have active effects running. Eventually calls OnUpdate on each effect.
        // Used in Hooks->Tweaks->ActiveEffectDurationPatch
        // Arguments: ActiveEffect*, float
        // Return: nothing
        inline static REL::Relocation<std::uintptr_t> UpdateActiveEffect{ REL::ID(34527), 0xC6 };
    }
}

namespace RE::VFuncs
{
    template <typename VTABLE>
    struct TargetVFunc
    {
        VTABLE      vtable{};
        std::size_t index{};
        std::size_t offset{};
    };
    
    namespace PlayerCharacter
    {
        inline static constexpr TargetVFunc DrawLowerHands{ RE::PlayerCharacter::VTABLE, 0u, 0xA6 };
    }
}