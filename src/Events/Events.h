#pragma once

#include "Settings/INISettings.h"

namespace Events
{template <class T, class E>
	class EventClass : public RE::BSTEventSink<E>
	{
	public:
		static T* GetSingleton() {
			static T singleton;
			return std::addressof(singleton);
		}

		bool RegisterListener() {
			logger::info("  >Initializing {}..."sv, T::name);
			auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
			if (!eventHolder) {
				logger::critical("    >Failed to get manager singleton."sv);
			}

			auto* iniHolder = Settings::INI::Holder::GetSingleton();
			if (!iniHolder) {
				logger::critical("    >Failed to fetch ini settings holder."sv);
				return false;
			}

			auto installRaw = iniHolder->GetStoredSetting<bool>(T::setting);
			bool install = installRaw.has_value() ? installRaw.value() : false;
			if (!installRaw.has_value()) {
				logger::warn("    >Setting {} not found in ini settings, treating as false.", T::setting);
			}
			if (!install) {
				return true;
			}

			eventHolder->AddEventSink(this);
			return true;
		}

		EventClass(const EventClass&) = delete;
		EventClass(EventClass&&) = delete;
		EventClass& operator=(const EventClass&) = delete;
		EventClass& operator=(EventClass&&) = delete;
	protected:
		EventClass() = default;
		~EventClass() = default;
	};

	bool InitializeListeners();
}