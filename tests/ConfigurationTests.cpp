#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "Configuration.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace
{
	void Check(bool condition, const char* message)
	{
		if (!condition) throw std::runtime_error(message);
	}

	std::string Read(const std::filesystem::path& path)
	{
		std::ifstream input(path, std::ios::binary);
		Check(input.good(), "Cannot read test file");
		return { std::istreambuf_iterator<char>(input), {} };
	}

	void Write(const std::filesystem::path& path, const std::string& contents)
	{
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		output << contents;
		output.close();
		Check(output.good(), "Cannot write test file");
	}
}

int main()
{
	const auto root = std::filesystem::temp_directory_path() /
		("UnleveledWorldConfigTests_" + std::to_string(GetCurrentProcessId()) + "_" + std::to_string(GetTickCount64()));
	try {
		Check(std::filesystem::create_directory(root), "Test directory already exists");
		const auto gameDirectory = root / L"Game_\u00e7\u6e2c";
		const auto path = Configuration::PathFromGameDirectory(gameDirectory);
		Check(path == gameDirectory / "Data" / "F4SE" / "Plugins" / "GLXRM_UnleveledWorld.ini", "Wrong configuration path");

		const auto resolvedPath = Configuration::ResolvePath();
		const auto originalDirectory = std::filesystem::current_path();
		std::filesystem::current_path(root);
		const auto relocatedPath = Configuration::ResolvePath();
		std::filesystem::current_path(originalDirectory);
		Check(resolvedPath.is_absolute() && relocatedPath == resolvedPath, "Configuration path depends on the working directory");

		const auto missing = Configuration::Load(path);
		Check(!missing.error.empty() && !std::filesystem::exists(gameDirectory), "Missing configuration created files or directories");
		Check(!missing.options.unlevelItems && !missing.options.unlevelCharacters && !missing.options.unlevelInjections, "Missing configuration enabled a feature");
		const Configuration::Options defaults;
		std::filesystem::create_directories(path.parent_path());

		const std::string partial = "; Keep this comment\r\n[General]\r\nUnlevelItems=TrUe\r\nUnknownKey=keep\r\n[Other]\r\nValue=42\r\n";
		Write(path, partial);
		const auto stamp = std::filesystem::last_write_time(path);
		const auto existing = Configuration::Load(path);
		Check(existing.error.empty() && existing.options.unlevelItems, "Existing option was not loaded");
		Check(existing.options.unlevelCharacters == defaults.unlevelCharacters && existing.options.unlevelInjections == defaults.unlevelInjections, "Missing keys did not use compiled defaults");
		Check(Read(path) == partial && std::filesystem::last_write_time(path) == stamp, "Existing configuration was modified");

		Write(path, "[General]\nUnlevelItems=false\nUnlevelCharacters=TRUE\nUnlevelInjections=true\n");
		const auto enabled = Configuration::Load(path);
		Check(enabled.error.empty() && !enabled.options.unlevelItems && enabled.options.unlevelCharacters && enabled.options.unlevelInjections, "Explicit feature settings were not loaded");
		Write(path, "[General]\nUnlevelItems=yes\nUnlevelCharacters=1\nUnlevelInjections=invalid\n");
		const auto invalid = Configuration::Load(path);
		Check(invalid.error.empty() && !invalid.options.unlevelItems && !invalid.options.unlevelCharacters && !invalid.options.unlevelInjections, "Original boolean semantics changed");
		Write(path, "");
		const auto empty = Configuration::Load(path);
		Check(empty.error.empty() && Read(path).empty(), "Empty existing configuration was replaced");

		const auto blocked = root / "blocked";
		Write(blocked, "preserve");
		const auto blockedRead = Configuration::Load(blocked / "GLXRM_UnleveledWorld.ini");
		Check(!blockedRead.error.empty() && Read(blocked) == "preserve", "Invalid parent path was not handled safely");
		const auto failedRead = Configuration::Load(root);
		Check(!failedRead.error.empty(), "Unreadable configuration was not reported");
		Check(!failedRead.options.unlevelItems && !failedRead.options.unlevelCharacters && !failedRead.options.unlevelInjections, "Read failure enabled a feature");

		std::filesystem::remove(path);
		const auto deleted = Configuration::Load(path);
		Check(!deleted.error.empty() && std::filesystem::is_empty(path.parent_path()), "Deleted configuration was recreated or temporary files were written");
		std::filesystem::remove_all(root);
		std::cout << "Read-only configuration loading, preservation, Unicode paths, and failure cases passed.\n";
		return 0;
	} catch (const std::exception& error) {
		std::cerr << error.what() << "\nTest files: " << root << '\n';
		return 1;
	}
}
