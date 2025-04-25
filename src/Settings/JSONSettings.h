#pragma once

namespace Settings
{
	namespace JSON
	{
		class Holder : public Utilities::Singleton::ISingleton<Holder>
		{
		public:
			bool Read();
		};
	}
}
