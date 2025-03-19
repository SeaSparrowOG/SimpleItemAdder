#include "Serde.h"

#include "ContainerManager/ContainerManager.h"
#include "ContainerManager/Filter.h" // Probably comes with the ContainerManager.h, but #pragma once allows me not to care.

namespace
{
	bool write_string(SKSE::SerializationInterface* a_intfc, const std::string& a_str)
	{
		size_t size = a_str.length();
		return a_intfc->WriteRecordData(size) && a_intfc->WriteRecordData(a_str.data(), size);
	}

	bool read_string(SKSE::SerializationInterface* a_intfc, std::string& a_str)
	{
		size_t size;
		if (!a_intfc->ReadRecordData(size)) {
			return false;
		}
		a_str.resize(size);
		return a_intfc->ReadRecordData(a_str.data(), size);
	}
}

namespace Serialization {
	void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		if (!a_intfc->OpenRecord(StoredRecordType, Version)) {
			logger::error("Failed to open record for StoredRecordType"sv);
			return;
		}

		const auto manager = ContainerManager::ContainerManager::GetSingleton();
		assert(manager);
		if (!manager) {
			logger::error("Failed to get the container manager. Save aborted."sv);
			return;
		}

		const auto definitions = manager->GetPapyrusRulesDefinitions();
		if (definitions.empty()) {
			return;
		}

		const auto definitionsCount = static_cast<int>(definitions.size());
		if (!a_intfc->WriteRecordData(definitionsCount)) {
			logger::error("Failed to write record data for definitions count."sv);
			return;
		}

		for (const auto& [index, definition] : definitions) {
			if (!a_intfc->WriteRecordData(index) || !write_string(a_intfc, definition)) {
				logger::error("Failed to write record data for definition {} -> {}"sv, index, definition);
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
				logger::error("Loaded data is incompatible with plugin version."sv);
			}

			const auto manager = ContainerManager::ContainerManager::GetSingleton();
			assert(manager);
			if (!manager) {
				logger::error("Failed to get the container manager. Save aborted."sv);
				return;
			}

			manager->ResetPapyrusRules();

			if (type == StoredRecordType) {
				size_t size = 0;
				if (!a_intfc->ReadRecordData(size)) {
					logger::error("Failed to read record data for definitions count."sv);
					return;
				}

				for (size_t i = 0; i < size; ++i) {
					std::string storedString;
					if (read_string(a_intfc, storedString)) {
						logger::info("Loaded rule: {}"sv, storedString);
					}
					else {
						logger::error("Failed to read stored string."sv);
						return;
					}
				}
			}
			else {
				logger::warn("Unknown record type: {}"sv, DecodeTypeCode(type));
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