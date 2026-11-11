#include "hooks.h"

#include <cstring>

namespace HookLineAndSinker
{
	typedef RE::LEVELED_OBJECT*(AddLeveledObjectSig)(RE::TESLeveledList*, uint16_t, uint16_t, int8_t, RE::TESForm*, RE::ContainerItemExtra*);
	REL::Relocation<AddLeveledObjectSig> OriginalFunction;

	namespace
	{
		// TESLeveledList::AddScriptAddedLeveledObject (Fallout4VR.exe 1.2.72.0: 0x140164020) calls
		// TESLeveledList::AddLeveledObject (0x140162760) at +0x6C with a 5-byte E8 call, read from raw disassembly
		// and matching the VR address library's names for both IDs.
		constexpr std::uint64_t kAddScriptAddedLeveledObjectId = 860553;
		constexpr std::uint64_t kAddLeveledObjectId = 1163308;
		constexpr std::ptrdiff_t kCallOffset = 0x6C;
		constexpr std::uint8_t kCallOpcode = 0xE8;
	}

	RE::LEVELED_OBJECT* HookedAddLeveledObject(RE::TESLeveledList* a_leveledList, uint16_t a_level, uint16_t a_count, int8_t a_chanceNone, RE::TESForm* a_item, RE::ContainerItemExtra* a_containerExta)
	{
		a_level = 1;
		return OriginalFunction(a_leveledList, a_level, a_count, a_chanceNone, a_item, a_containerExta);
	}

	bool RegisterHook(F4SE::Trampoline& trampoline)
	{
		REL::Relocation<AddLeveledObjectSig> callLocation{ REL::ID(kAddScriptAddedLeveledObjectId), kCallOffset };
		const auto site = callLocation.address();
		const auto expectedTarget = REL::ID(kAddLeveledObjectId).address();

		std::uint8_t opcode = 0;
		std::int32_t displacement = 0;
		std::memcpy(&opcode, reinterpret_cast<const void*>(site), sizeof(opcode));
		std::memcpy(&displacement, reinterpret_cast<const void*>(site + 1), sizeof(displacement));
		const auto target = site + 5 + static_cast<std::intptr_t>(displacement);
		if (opcode != kCallOpcode || target != expectedTarget) {
			logger::error("Injection hook site {:X} holds opcode {:02X} to {:X}, expected a call to {:X}; script-injected entries keep their levels",
				site, opcode, target, expectedTarget);
			return false;
		}

		OriginalFunction = trampoline.write_call<5>(site, &HookedAddLeveledObject);
		return true;
	}
}
