#pragma once

#include "Serialization/SerializationManager/SerializationManager.h"

namespace ProjectileManager
{
	class ProjManager : 
		public REX::Singleton<ProjManager>,
		public RE::BSTEventSink<RE::TESFormDeleteEvent>,
		public Serialization::SerializationManager::Serializable
	{
	public:
		bool Initialize();

		float GetLinearVelocityFactor(const RE::Projectile* projectile) const;
		float GetGravityFactor(const RE::Projectile* projectile) const;

		bool AddDeadzone(const RE::TESObjectREFR* ref);
		bool OverrideZoneVelocity(const RE::TESObjectREFR* ref, float newFactor);
		bool OverrideZoneGravity(const RE::TESObjectREFR* ref, float newGrav);
		bool SetZoneAffectsOnlyEnemies(const RE::TESObjectREFR* ref, bool onlyEnemies);

		bool Save(SKSE::SerializationInterface* a_intfc) override;
		bool Load(SKSE::SerializationInterface* a_intfc) override;

	private:
		struct Deadzone
		{
			RE::FormID id = 0;
			float range = -1.0f;
			float velocity = 1.0f;
			float gravity = 1.0f;
			bool onlyEnemies = false;
		};

		std::vector<Deadzone> _zones{};

		RE::BSEventNotifyControl ProcessEvent(const RE::TESFormDeleteEvent* a_event, 
			RE::BSTEventSource<RE::TESFormDeleteEvent>*) override;
	};

	bool InitializeProjectileManager();
}