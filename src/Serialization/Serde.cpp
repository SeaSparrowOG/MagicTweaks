#include "Serde.h"

#include "SerializationManager/SerializationManager.h"

namespace Serialization {
	void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		REX::INFO("Starting save..."sv);
		auto* serdeManager = SerializationManager::ObjectManager::GetSingleton();
		if (!serdeManager) {
			REX::CRITICAL("  >Failed to get internal serialization manager."sv);
			return;
		}
		if (!serdeManager->Save(a_intfc)) {
			REX::CRITICAL("  >Failed to save!"sv);
			return;
		}
		REX::INFO("  >Save successful."sv);
	}

	void LoadCallback(SKSE::SerializationInterface* a_intfc)
	{
		REX::INFO("Starting load..."sv);
		auto* serdeManager = SerializationManager::ObjectManager::GetSingleton();
		if (!serdeManager) {
			REX::CRITICAL("  >Failed to get internal serialization manager."sv);
			return;
		}
		if (!serdeManager->Load(a_intfc)) {
			REX::CRITICAL("  >Failed to load!"sv);
			return;
		}
		REX::INFO("  >Load successful."sv);
	}

	void RevertCallback(SKSE::SerializationInterface* a_intfc)
	{
		REX::INFO("Starting revert..."sv);
		auto* serdeManager = SerializationManager::ObjectManager::GetSingleton();
		if (!serdeManager) {
			REX::CRITICAL("  >Failed to get internal serialization manager."sv);
			return;
		}
		serdeManager->Revert(a_intfc);
		REX::INFO("  >Revert done."sv);
	}
}