#pragma once

namespace Serialization
{
	namespace SerializationManager
	{
		class ObjectManager;

		class Serializable
		{
		public:
			virtual bool Save(SKSE::SerializationInterface* a_intfc) = 0;
			virtual bool Load(SKSE::SerializationInterface* a_intfc) = 0;
			virtual void Revert(SKSE::SerializationInterface* a_intfc) = 0;

			template <typename T>
			bool RegisterForSerialization(T* object, uint32_t type) {
				logger::info("  >Registering for Save/Load events..."sv);
				auto* manager = ObjectManager::GetSingleton();
				if (!manager) {
					logger::critical("    >Failed to get Serialization Object Manager."sv);
					return false;
				}

				auto pointer = std::unique_ptr<Serializable>(object);
				manager->RegisterObject(pointer, type);
				return true;
			}
		};

		class ObjectManager : public REX::Singleton<ObjectManager>
		{
		public:
			bool Save(SKSE::SerializationInterface* a_intfc);
			bool Load(SKSE::SerializationInterface* a_intfc);
			bool Revert(SKSE::SerializationInterface* a_intfc);

			void RegisterObject(Serializable* a_newObject, uint32_t a_recordType);

		private:
			std::unordered_map<uint32_t, Serializable*> recordObjectMap{};

			/// <summary>
			/// Debug tool. When encountering unexpected RecordTypes, converts them to a readable string (HDEC, STEN, etc).
			/// </summary>
			/// <param name="a_typeCode">The unexpected record type.</param>
			/// <returns>The unexpected record type as a string.</returns>
			inline std::string DecodeTypeCode(std::uint32_t a_typeCode)
			{
				constexpr std::size_t SIZE = sizeof(std::uint32_t);

				std::string sig;
				sig.resize(SIZE);
				char* iter = reinterpret_cast<char*>(&a_typeCode);
				for (std::size_t i = 0, j = SIZE - 2; i < SIZE - 1; ++i, --j) {
					sig[j] = iter[i];
				}
				return sig;
			}
		};
	}
}