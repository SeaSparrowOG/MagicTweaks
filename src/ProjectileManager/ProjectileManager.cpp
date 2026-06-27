#include "ProjectileManager.h"

namespace ProjectileManager
{
	bool ProjManager::Initialize() {
		auto* serde = Serialization::SerializationManager::ObjectManager::GetSingleton();
		if (!serde) {
			logger::critical("  - Failed to get internal Projectile Manager singleton."sv);
			return false;
		}
		serde->RegisterObject(this, 'PRMG');
		logger::info("  - Successfully initialized projectile manager."sv);
		return true;
	}

	float ProjManager::GetLinearVelocityFactor(const RE::Projectile* projectile) const {
		(void)projectile;
		return 1.0f;
	}

	float ProjManager::GetGravityFactor(const RE::Projectile* projectile) const {
		(void)projectile;
		return 1.0f;
	}

	bool ProjManager::AddDeadzone(const RE::TESObjectREFR* ref) {
		(void)ref;
		return false;
	}

	bool ProjManager::OverrideZoneVelocity(const RE::TESObjectREFR* ref, float newFactor) {
		(void)ref; (void)newFactor;
		return false;
	}

	bool ProjManager::OverrideZoneGravity(const RE::TESObjectREFR* ref, float newGrav) {
		(void)ref; (void)newGrav;
		return false;
	}

	bool ProjManager::SetZoneAffectsOnlyEnemies(const RE::TESObjectREFR* ref, bool onlyEnemies) {
		(void)ref; (void)onlyEnemies;
		return false;
	}

	bool ProjManager::Save(SKSE::SerializationInterface* a_intfc) {
		(void)a_intfc;
		return false;
	}

	bool ProjManager::Load(SKSE::SerializationInterface* a_intfc) {
		(void)a_intfc;
		return false;
	}

	RE::BSEventNotifyControl ProjManager::ProcessEvent(const RE::TESFormDeleteEvent* a_event, 
		RE::BSTEventSource<RE::TESFormDeleteEvent>*) 
	{
		if (!a_event) {
			return RE::BSEventNotifyControl::kContinue;
		}

		auto id = a_event->formID;
		auto it = std::ranges::find_if(_zones, [id](const Deadzone& zone) -> bool { return zone.id == id; });
		if (it != _zones.end()) {
			_zones.erase(it);
		}
		return RE::BSEventNotifyControl::kContinue;
	}

	bool InitializeProjectileManager() {
		logger::info("Initializing Projectile Manager..."sv);
		auto* projectileManager = ProjManager::GetSingleton();
		if (!projectileManager) {
			logger::critical("  - Failed to get internal Projectile Manager. Aborting load..."sv);
			return false;
		}
		return projectileManager->Initialize();
	}
}