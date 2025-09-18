#pragma once

namespace ConditionManager
{
	bool Initialize();

	class ConditionManager : public REX::Singleton<ConditionManager>
	{
	public:
		bool Initialize();

		bool SubstituteItemCount(RE::TESBoundObject* a_obj, RE::TESObjectREFR* a_ref, int32_t& a_out);
	
	private:
		RE::TESBoundObject* totalCount{ nullptr };
		RE::TESBoundObject* conjuredCount{ nullptr };
		RE::TESBoundObject* reanimatedCount{ nullptr };
		RE::TESBoundObject* fireCount{ nullptr };
		RE::TESBoundObject* frostCount{ nullptr };
		RE::TESBoundObject* shockCount{ nullptr };
	};
}