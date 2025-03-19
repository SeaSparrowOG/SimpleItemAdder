#pragma once

namespace Serialization
{
	constexpr std::uint32_t Version = 1;               //Save version. Mismatched version numbers will NOT be read.
	constexpr std::uint32_t ID = 'SIAD';               //Save ID. 4 characters. Must be unique among all installed SKSE plugins. Good luck.
	constexpr std::uint32_t StoredRecordType = 'STPF'; //Record Type to be stored. Represent papyrus plugins. Stands for "Stored Papyrus Filter".

	/// <summary>
	/// Save callback. Called by SKSE when the game is saved. Stores the papyrus filters in the interface so they remain consistent across saves.
	/// </summary>
	/// <param name="a_intfc">The SKSE interface.</param>
	void SaveCallback(SKSE::SerializationInterface* a_intfc);

	/// <summary>
	/// Load callback. Called by SKSE when the game is loaded. Restores the papyrus filters from the interface.
	/// </summary>
	/// <param name="a_intfc"></param>
	void LoadCallback(SKSE::SerializationInterface* a_intfc);

	/// <summary>
	/// Revert callback. Unsure when this is called, but it's here for completeness.
	/// </summary>
	/// <param name="a_intfc"></param>
	void RevertCallback(SKSE::SerializationInterface* a_intfc);
	
	/// <summary>
	/// Given an ID or a RecordType, decodes it into a readable string. Useful for debugging unexpected types.Maybe we can know what the fuck is a kilometer.
	/// </summary>
	/// <param name="a_typeCode">The typecode provided by the interface. Typically, within our ID, we can only have our own RecordTypes.</param>
	/// <returns>The record type as a string.</returns>
	std::string DecodeTypeCode(std::uint32_t a_typeCode);
}