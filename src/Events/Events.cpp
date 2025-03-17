#include "events.h"

namespace Events
{
	void Install() {
	}

	RE::BSEventNotifyControl ExampleClass::ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*)
	{
		using control = RE::BSEventNotifyControl;
		if (!a_event) {
			return control::kContinue;
		}
		return control::kContinue;
	}
}