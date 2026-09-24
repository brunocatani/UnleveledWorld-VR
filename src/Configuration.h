#pragma once

#include <filesystem>
#include <string>

namespace Configuration
{
	struct Options
	{
		bool unlevelItems{ false };
		bool unlevelCharacters{ false };
		bool unlevelInjections{ false };
	};

	struct LoadResult
	{
		Options options;
		std::string error;
	};

	std::filesystem::path PathFromGameDirectory(const std::filesystem::path& gameDirectory);
	std::filesystem::path ResolvePath();
	LoadResult Load(const std::filesystem::path& path);
}
