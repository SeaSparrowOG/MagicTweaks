#pragma once

#include "ProjectileManager/ProjectileManager.h"

namespace Hooks
{
	namespace Projectiles
	{
		bool Install();

		template <typename T>
		struct GetGravityHook
		{
			inline static float GetGravity(T* a_this)
			{
				float gravity = _getGravity(a_this);
				const static auto* manager = ProjectileManager::ProjManager::GetSingleton();
				if (!manager || gravity <= 0.0f) {
					return gravity;
				}
				const float factor = manager->GetGravityFactor(a_this);
				if (factor < 0.0f) {
					return gravity;
				}
				const float result = gravity * result;
				if (std::isnan(result)) {
					return gravity;
				}
				return std::clamp(result, std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
			}

			inline static REL::Relocation<decltype(GetGravity)> _getGravity;

			static void Install()
			{
				REL::Relocation<std::uintptr_t> VTABLE{ T::VTABLE[0] };
				_getGravity = VTABLE.write_vfunc(0xB5, &GetGravity);
			}
		};

		template <typename T>
		struct GetVelocityHook
		{
			inline static void GetLinearVelocity(T* a_this, RE::NiPoint3& a_velocity)
			{
				_getLinearVelocity(a_this, a_velocity);
				const static auto* manager = ProjectileManager::ProjManager::GetSingleton();
				if (!manager) {
					return;
				}
				float factor = manager->GetGravityFactor(a_this);
				if (std::isnan(factor) || factor < 0.0f) {
					return;
				}
				std::clamp(factor, std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
				a_velocity *= factor;
			}

			inline static REL::Relocation<decltype(GetLinearVelocity)> _getLinearVelocity;

			static void Install()
			{
				REL::Relocation<std::uintptr_t> VTABLE{ T::VTABLE[0] };
				_getLinearVelocity = VTABLE.write_vfunc(0x86, &GetLinearVelocity);
			}
		};
	}
}