#pragma once

#include "REX/W32/OLE32.h"
#include "REX/W32/SHELL32.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <system_error>

namespace LogPath
{
	inline std::filesystem::path Fallout4VRLogDirectoryFromDocuments(const std::filesystem::path& a_documentsDirectory)
	{
		return a_documentsDirectory / "My Games" / "Fallout4VR" / "F4SE";
	}

	inline std::optional<std::filesystem::path> GetFallout4VRLogDirectory()
	{
		wchar_t* buffer{ nullptr };
		const auto result = REX::W32::SHGetKnownFolderPath(REX::W32::FOLDERID_Documents, REX::W32::KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));
		std::unique_ptr<wchar_t[], decltype(&REX::W32::CoTaskMemFree)> documentsPath(buffer, REX::W32::CoTaskMemFree);
		if (!documentsPath || result != 0) {
			return std::nullopt;
		}

		auto path = Fallout4VRLogDirectoryFromDocuments(documentsPath.get());
		std::error_code ec;
		std::filesystem::create_directories(path, ec);
		if (ec) {
			return std::nullopt;
		}

		return path;
	}
}
