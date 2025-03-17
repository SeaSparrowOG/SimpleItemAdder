#pragma once

namespace Events
{
	void Install();

	class ExampleClass : public Utilities::Singleton::EventClass<ExampleClass, RE::TESMagicEffectApplyEvent> {
	public:
		ExampleClass(const ExampleClass&) = delete;
		ExampleClass(ExampleClass&&) = delete;
	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*) override;
	};
}