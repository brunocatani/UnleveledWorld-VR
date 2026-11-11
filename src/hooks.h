#pragma once

namespace HookLineAndSinker
{
	RE::LEVELED_OBJECT* HookedAddLeveledObject(RE::TESLeveledList* a_leveledList, uint16_t a_level, uint16_t a_count, int8_t a_chanceNone, RE::TESForm* a_item, RE::ContainerItemExtra* a_containerExta);

	// False when the call site does not hold the expected call; nothing is patched then.
	[[nodiscard]] bool RegisterHook(F4SE::Trampoline& trampoline);
}
