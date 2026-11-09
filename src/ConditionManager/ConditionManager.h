#pragma once

namespace ConditionManager
{
	bool Initialize();

	class ConditionManager : public REX::TSingleton<ConditionManager>
	{
	public:
		bool Initialize();

		[[nodiscard]]
		std::optional<int32_t> SubstituteItemCount(RE::TESBoundObject* a_obj, RE::TESObjectREFR* a_ref);
	
	private:
		RE::TESBoundObject* totalCount{ nullptr };
		RE::TESBoundObject* conjuredCount{ nullptr };
		RE::TESBoundObject* reanimatedCount{ nullptr };
		RE::TESBoundObject* fireCount{ nullptr };
		RE::TESBoundObject* frostCount{ nullptr };
		RE::TESBoundObject* shockCount{ nullptr };
	};
}