#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "Configuration.h"
#include "SimpleIni.h"

#include <array>
#include <stdexcept>

namespace Configuration
{
	namespace
	{
		struct Setting
		{
			const char* key;
			bool Options::* member;
		};

		constexpr std::array settings{
			Setting{ "UnlevelItems", &Options::unlevelItems },
			Setting{ "UnlevelCharacters", &Options::unlevelCharacters },
			Setting{ "UnlevelInjections", &Options::unlevelInjections }
		};
	}

	std::filesystem::path PathFromGameDirectory(const std::filesystem::path& gameDirectory)
	{
		return gameDirectory / "Data" / "F4SE" / "Plugins" / "GLXRM_UnleveledWorld.ini";
	}

	std::filesystem::path ResolvePath()
	{
		std::array<wchar_t, 32768> executable{};
		const auto length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
		if (length == 0 || length == executable.size()) {
			throw std::runtime_error("Cannot resolve the game directory for Unleveled World configuration");
		}
		return PathFromGameDirectory(std::filesystem::path(executable.data()).parent_path());
	}

	LoadResult Load(const std::filesystem::path& path)
	{
		LoadResult result;
		try {
			CSimpleIniA ini(true, false, false);
			if (ini.LoadFile(path.c_str()) < 0) {
				throw std::runtime_error("Cannot read GLXRM_UnleveledWorld.ini supplied by the required original mod");
			}
			Options options;
			for (const auto& setting : settings) {
				const auto value = ini.GetValue("General", setting.key, options.*(setting.member) ? "true" : "false");
				// Preserve the original parser: only case-insensitive 'true' enables a feature.
				options.*(setting.member) = _stricmp(value, "true") == 0;
			}
			result.options = options;
		} catch (const std::exception& error) {
			result.error = error.what();
		}
		return result;
	}
}
