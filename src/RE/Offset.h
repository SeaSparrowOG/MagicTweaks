#pragma once

namespace RE::Offset
{
    template <std::size_t SIZE, REX::TStaticString CODE>
    struct HookSite
    {
        REL::ID        id;
        std::ptrdiff_t offset;

        [[nodiscard]]
        std::uintptr_t address() const {
            const REL::Relocation<std::uintptr_t> target{ id, offset };
            return target.address();
        }

        [[nodiscard]] bool match() const {
            return REL::Pattern<CODE>().match(address());
        }

        template <typename FUNC>
        [[nodiscard]] auto hook(FUNC func) const {
            auto& trampoline = REL::GetTrampoline();
            return trampoline.write_call<SIZE>(address(), func);
        }
    };

    namespace ItemCard
    {
        // Calls to get the description of a magic item. 
        // Used in Hooks->Tweaks->DescriptionPatch
        // Arguments: ItemCard*, SpellItem*, BSString&
        // Return: nothing
        inline static HookSite<5u, "E8"> GetSpellDescription{ REL::ID(51898), 0x5DC };
        inline static HookSite<5u, "E8"> GetSpellTomeDescription{ REL::ID(51897), 0xF5C };
        inline static HookSite<5u, "E8"> GetStaffDescription{ REL::ID(51897), 0x1377 };
    }
    namespace MagicTarget 
    {
        // Periodically ran on targets that have active effects running. Eventually calls OnUpdate on each effect.
        // Used in Hooks->Tweaks->ActiveEffectDurationPatch
        // Arguments: ActiveEffect*, float
        // Return: nothing
        inline static HookSite<5u, "E8"> UpdateActiveEffect{ REL::ID(34527), 0xC6 };
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