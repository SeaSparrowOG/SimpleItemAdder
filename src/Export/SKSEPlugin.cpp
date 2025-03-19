#include "ContainerManager/ContainerManager.h"
#include "Data/ModObjectManager.h"
#include "Events/Events.h"
#include "Papyrus/Papyrus.h"
#include "Serialization/Serde.h"

namespace
{
	void InitializeLog()
	{
		auto path = logger::log_directory();
		if (!path) {
			util::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("{}.log"sv, Plugin::NAME);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

#ifndef NDEBUG
		const auto level = spdlog::level::debug;
#else 
		const auto level = spdlog::level::info;
#endif

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%^%l%$] %v"s);
	}
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []()
	{
		SKSE::PluginVersionData v{};

		v.PluginVersion(Plugin::VERSION);
		v.PluginName(Plugin::NAME);
		v.AuthorName("SeaSparrow"sv);
		v.UsesAddressLibrary(true);

		return v;
	}();

extern "C" DLLEXPORT bool SKSEAPI
SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_6_1130) {
		return false;
	}

	return true;
}

static void MessageEventCallback(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Data::ModObjectManager::GetSingleton()->RegisterListener();
		ContainerManager::ContainerManager::GetSingleton()->InitializeMembers();
		Events::Install();
		break;
	default:
		break;
	}
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("=================================================");
	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());
	logger::info("Author: SeaSparrow"sv);
	logger::info("=================================================");
	logger::info("Performing startup tasks..."sv);
	const auto then = std::chrono::high_resolution_clock::now();
	SKSE::Init(a_skse);

	logger::info("Checking runtime version..."sv);
	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_6_1130) {
		return false;
	}

	logger::info("Registering Papyrus functions..."sv);
	// Credit to PO3 for the Papyrus binding implementation.
	// Nexus:  https://next.nexusmods.com/profile/powerofthree?gameId=1704
	// Github: https://github.com/powerof3
	SKSE::GetPapyrusInterface()->Register(Papyrus::RegisterFunctions);

	logger::info("Registering messaging callbacks..."sv);
	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(&MessageEventCallback);

	logger::info("Registering serialization callbacks..."sv);
	const auto serialization = SKSE::GetSerializationInterface();
	serialization->SetUniqueID(Serialization::ID);
	serialization->SetSaveCallback(&Serialization::SaveCallback);
	serialization->SetLoadCallback(&Serialization::LoadCallback);
	serialization->SetRevertCallback(&Serialization::RevertCallback);

	const auto now = std::chrono::high_resolution_clock::now();
	const auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count();

	logger::info("Plugin successfully loaded in {}ms"sv, elapsedMilliseconds);
	logger::info("=================================================");
	return true;
}