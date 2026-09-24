#include "Configuration.h"
#include "LogPath.h"
#include "hooks.h"

bool unlevelItems;
bool unlevelCharacters;
bool unlevelInjections;  // used to unlevel script-added things after the save is loaded

void LoadConfigs()
{
	unlevelItems = unlevelCharacters = unlevelInjections = false;
	try {
		const auto path = Configuration::ResolvePath();
		const auto utf8Path = path.u8string();
		const std::string displayPath(utf8Path.begin(), utf8Path.end());
		logger::info("Loading configuration from {}", displayPath);
		const auto result = Configuration::Load(path);
		if (!result.error.empty()) {
			logger::error("Cannot load configuration: {}. Unleveling remains disabled.", result.error);
			return;
		}
		unlevelItems = result.options.unlevelItems;
		unlevelCharacters = result.options.unlevelCharacters;
		unlevelInjections = result.options.unlevelInjections;
		logger::info("UnlevelItems={}, UnlevelCharacters={}, UnlevelInjections={}",
			unlevelItems, unlevelCharacters, unlevelInjections);
	} catch (const std::exception& error) {
		logger::error("Cannot initialize configuration: {}. Unleveling remains disabled.", error.what());
	}
}

void UnlevelStuff()
{
	if (auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		if (unlevelItems) {
			logger::warn("Unleveling item lists");
			for (auto currentItemList : dataHandler->GetFormArray<RE::TESLevItem>()) {
				std::int32_t total = (currentItemList->baseListCount + currentItemList->scriptListCount);

				for (int i = 0; i < total; i++) {
					RE::LEVELED_OBJECT* obj;
					if (i < currentItemList->baseListCount) {
						obj = &currentItemList->leveledLists[i];
					} else {
						obj = currentItemList->scriptAddedLists[i - currentItemList->baseListCount];
					}

					if (obj) {
						obj->level = 1;
					}
				}
			}
		} else {
			logger::warn("Ignoring item lists");
		}

		if (unlevelCharacters) {
			logger::warn("Unleveling character lists");
			for (auto currentCharList : dataHandler->GetFormArray<RE::TESLevCharacter>()) {
				std::int32_t total = (currentCharList->baseListCount + currentCharList->scriptListCount);

				for (int i = 0; i < total; i++) {
					RE::LEVELED_OBJECT* obj;
					if (i < currentCharList->baseListCount) {
						obj = &currentCharList->leveledLists[i];
					} else {
						obj = currentCharList->scriptAddedLists[i - currentCharList->baseListCount];
					}

					if (obj) {
						obj->level = 1;
					}
				}
			}
		} else {
			logger::warn("Ignoring character lists");
		}
	}
}

void ListenerThing(F4SE::MessagingInterface::Message* a_thing)
{
	if (a_thing->type == F4SE::MessagingInterface::kGameDataReady) {
		logger::info("Game data finished loading. Beginning unleveling...");
		UnlevelStuff();
	}
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = LogPath::GetFallout4VRLogDirectory();
	if (!path) {
		return false;
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = "GLXRM_UnleveledWorld";
	a_info->version = 69;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor");
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_VR_1_2_72) {
		logger::critical(FMT_STRING("unsupported runtime v{}"), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se, false);  // false = don't reinitialize logger, keep the one from Query

	LoadConfigs();

	// hook into the function that injects stuff to LLs to force level to 1
	if (unlevelInjections) {
		auto& trampoline = F4SE::GetTrampoline();
		trampoline.create(20);
		HookLineAndSinker::RegisterHook(trampoline);
	}

	F4SE::GetMessagingInterface()->RegisterListener(ListenerThing);

	return true;
}
