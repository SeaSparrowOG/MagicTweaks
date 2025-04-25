#pragma once

namespace Settings
{
	namespace JSON
	{
		bool Read();

		class Holder : public Utilities::Singleton::ISingleton<Holder>
		{
		public:
			bool Read();
		};
	}
}
