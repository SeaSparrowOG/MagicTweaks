#include "papyrus.h"

#include "BoundEffectManager/BoundEffectManager.h"

namespace Papyrus
{
	static bool UnBindAllSpells(STATIC_ARGS) {
		LOG_DEBUG("===[PAPYRUS]===");
		LOG_DEBUG("Called UnBindAllSpells");

		auto* manager = BoundEffectManager::BoundEffectManager::GetSingleton();
		if (!manager) {
			LOG_DEBUG("  >Failed to get internal bound effect manager."sv);
			return false;
		}
		return manager->UnBindAllSpells();
	}

	static int UnBindSpell(STATIC_ARGS, RE::MagicItem* a_spell) {
		LOG_DEBUG("===[PAPYRUS]===");
		LOG_DEBUG("Called UnBindSpell");

		auto* manager = BoundEffectManager::BoundEffectManager::GetSingleton();
		if (!manager) {
			LOG_DEBUG("  >Failed to get internal bound effect manager."sv);
			return -1;
		}
		return manager->UnBindSpell(a_spell);
	}

	static std::vector<int> GetVersion(STATIC_ARGS) {
		return { Plugin::VERSION[0], Plugin::VERSION[1], Plugin::VERSION[2] };
	}

	static void Bind(VM& a_vm) {
		logger::info("  >Binding GetVersion..."sv);
		BIND(GetVersion);
		logger::info("  >Binding UnBindAllSpells..."sv);
		BIND(UnBindAllSpells);
		logger::info("  >Binding UnBindSpell..."sv);
		BIND(UnBindSpell);
	}

	bool RegisterFunctions(VM* a_vm) {
		SECTION_SEPARATOR;
		logger::info("Binding papyrus functions in utility script {}..."sv, script);
		Bind(*a_vm);
		logger::info("Finished binding functions."sv);
		return true;
	}
}
