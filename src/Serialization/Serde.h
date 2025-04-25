#pragma once

namespace Serialization
{
	constexpr std::uint32_t Version = 1;     //Save version. Mismatched version numbers will NOT be read.
	constexpr std::uint32_t ID = 'CEDH';         //Save ID. 4 characters. Must be unique among all installed SKSE plugins. Good luck.
	constexpr std::uint32_t RecordType = 'HDEC'; //Record Type example. Not to be confused with bethesda record types, these are unique to your plugin.

	/*
	* Called when the game saves.
	*/
	void SaveCallback(SKSE::SerializationInterface* a_intfc);

	/*
	* Called when the game loads.
	*/
	void LoadCallback(SKSE::SerializationInterface* a_intfc);

	/*
	* I don't know when it is called lmao.
	*/
	void RevertCallback(SKSE::SerializationInterface* a_intfc);
	
	/*
	* Given an ID or a RecordType, decodes it into a readable string.
	* Useful for debugging unexpected types. Maybe we can know what the fuck is a kilometer.
	* @param a_typeCode The code received from the SerializationInterface.
	* @return A string containing the plaintext code.
	*/
	std::string DecodeTypeCode(std::uint32_t a_typeCode);
}