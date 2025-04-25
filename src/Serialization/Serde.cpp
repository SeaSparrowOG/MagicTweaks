#include "Serde.h"

#include "Hooks/Hooks.h"

namespace Serialization {
	void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		if (!a_intfc->OpenRecord(RecordType, Version)) {
			logger::error("Failed to open record for RecordType"sv);
			return;
		}

		RE::TESForm* exampleForm = nullptr;
		if (exampleForm) {
			if (!a_intfc->WriteRecordData(exampleForm->formID)) {
				logger::error("Failed to write FormID ({:08X})"sv, exampleForm->formID);
				return;
			}
		}
	}

	void LoadCallback(SKSE::SerializationInterface* a_intfc)
	{
		std::uint32_t type;
		std::uint32_t version;
		std::uint32_t length;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			if (version != Version) {
				logger::error("Loaded data is incompatible with plugin version!"sv);
			}

			if (type == RecordType) {
				RE::FormID storedID;
				if (!a_intfc->ReadRecordData(storedID)) {
					logger::error("Failed to read stored FormID"sv);
					return;
				}

				RE::FormID currentID;
				if (!a_intfc->ResolveFormID(storedID, currentID)) {
					logger::error("Failed to resolve FormID ({:08X})"sv, storedID);
					return;
				}

				auto* resolvedForm = RE::TESForm::LookupByID(currentID);
				if (!resolvedForm) {
					logger::error("Failed to find appropriate form ({:08X})", currentID);
					return;
				}
			}
		}
	}

	void RevertCallback(SKSE::SerializationInterface* a_intfc)
	{
		(void)a_intfc;
	}

	std::string DecodeTypeCode(std::uint32_t a_typeCode)
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
}