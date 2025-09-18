#pragma once

namespace Data
{
	bool PreloadModObjects();

	class ModObjectManager : 
		public REX::Singleton<ModObjectManager>
	{
	public:
		const std::string QuestName = fmt::format("{}_ModObjectsQuest"sv, Plugin::NAME);
		const std::string ScriptName = fmt::format("{}_ModObjectsScript"sv, Plugin::NAME);

		bool PreLoad();

		[[nodiscard]] RE::TESForm* Get(std::string_view a_key) const;
	private:
		bool Verify();

		util::istring_map<RE::TESForm*> objects;
	};

	template <typename T>
	[[nodiscard]] inline T* ModObject(std::string_view a_key)
	{
		if (const auto object = ModObjectManager::GetSingleton()->Get(a_key))
			return object->As<T>();
		return nullptr;
	}

	inline static constexpr const char* MAGIC_BINDS_HEALTH = "MagicBindHealth";
	inline static constexpr const char* MAGIC_BINDS_STAMINA = "MagicBindStamina";
	inline static constexpr const char* MAGIC_BINDS_MAGICKA = "MagicBindMagicka";

	inline static constexpr const char* MAGIC_COUNT_COMMANDED_CONJURATION = "MGT_PRXY_CommandedConjurationCount";
	inline static constexpr const char* MAGIC_COUNT_SUMMONED = "MGT_PRXY_CommandedConjurationSummon";
	inline static constexpr const char* MAGIC_COUNT_REANIMATED = "MGT_PRXY_CommandedConjurationReanimated";
	inline static constexpr const char* MAGIC_COUNT_SUMMONED_FIRE = "MGT_PRXY_CommandedConjurationFire";
	inline static constexpr const char* MAGIC_COUNT_SUMMONED_FROST = "MGT_PRXY_CommandedConjurationFrost";
	inline static constexpr const char* MAGIC_COUNT_SUMMONED_SHOCK = "MGT_PRXY_CommandedConjurationShock";

	inline static constexpr std::array<const char*, 9> EXPECTED_OBJECTS = {
		MAGIC_BINDS_HEALTH,
		MAGIC_BINDS_STAMINA,
		MAGIC_BINDS_MAGICKA,

		MAGIC_COUNT_COMMANDED_CONJURATION,
		MAGIC_COUNT_SUMMONED,
		MAGIC_COUNT_REANIMATED,
		MAGIC_COUNT_SUMMONED_FIRE,
		MAGIC_COUNT_SUMMONED_FROST,
		MAGIC_COUNT_SUMMONED_SHOCK
	};
}