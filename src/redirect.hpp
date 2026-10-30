#pragma once

#include <cstddef>
#include <iterator>
#include <span>
#include <string_view>

#include <windows.h>

namespace tdpahook
{
	/// The address the games download the list of the anthology's games from.
	inline constexpr wchar_t kOriginalUrl[] = L"https://media.prod.tdp-tech.eu/DP_Anthology_Games.JSON";

	/// The most characters a new address may have: it is written over the original, the only place the game reads.
	inline constexpr std::size_t kMaxUrlLength = std::size(kOriginalUrl) - 1;

	/// The address the games download the list from when TDPAHook.ini names none.
	inline constexpr std::wstring_view kDefaultUrl = L"https://tdpa.grzyb.app/DP_Anthology_Games.json";

	/// What became of the original address.
	enum class Redirect
	{
		Replaced,
		TooLong,
		NotFound,
	};

	/// Gives the read-only data section of a loaded module, where the games keep the address.
	/// @param module The module.
	[[nodiscard]] std::span<std::byte> readOnlyData(HMODULE module);

	/// Writes a new address over the original one, in place, filling the rest of its room with zeros.
	/// @param data The memory that holds the original, which may be read-only.
	/// @param url The new address, of at most kMaxUrlLength characters.
	[[nodiscard]] Redirect redirectGamesList(std::span<std::byte> data, std::wstring_view url);
}
