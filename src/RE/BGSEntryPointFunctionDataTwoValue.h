#pragma once

namespace RE
{
	class BGSEntryPointFunctionDataTwoValue : public BGSEntryPointFunctionData
	{
	public:
		float value;
		float av;
	};
	static_assert(sizeof(BGSEntryPointFunctionDataTwoValue) == 0x10);
}