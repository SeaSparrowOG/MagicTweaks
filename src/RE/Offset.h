#pragma once

namespace RE::Offset
{
    namespace ItemCard
    {
        REL::Relocation<std::uintptr_t> GetSpellDescription{ REL::ID(51898), 0x5DC };
        REL::Relocation<std::uintptr_t> GetSpellTomeDescription{ REL::ID(51897), 0xF5C };
        REL::Relocation<std::uintptr_t> GetStaffDescription{ REL::ID(51897), 0x1377 };
    }
    namespace MagicTarget 
    {
        REL::Relocation<std::uintptr_t> UpdateActiveEffect{ REL::ID(34527), 0xC6 };
    }
}