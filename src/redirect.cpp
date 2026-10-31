#include "redirect.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>

namespace tdpahook
{
	std::span<std::byte> readOnlyData(const HMODULE module)
	{
		const auto base = reinterpret_cast<std::uintptr_t>(module);
		const auto* const headers =
			reinterpret_cast<const IMAGE_NT_HEADERS*>(base + reinterpret_cast<const IMAGE_DOS_HEADER*>(module)->e_lfanew);

		for (const IMAGE_SECTION_HEADER& section :
		     std::span(IMAGE_FIRST_SECTION(headers), headers->FileHeader.NumberOfSections))
		{
			if (std::memcmp(section.Name, ".rdata", sizeof(".rdata")) == 0)
				return {reinterpret_cast<std::byte*>(base + section.VirtualAddress), section.Misc.VirtualSize};
		}

		return {};
	}

	Redirect redirectGamesList(const std::span<std::byte> data, const std::wstring_view url)
	{
		if (url.size() > kMaxUrlLength)
			return Redirect::TooLong;

		// With its terminator, so that only the whole string matches and not the start of a longer one.
		const std::span<const std::byte> original = std::as_bytes(std::span(kOriginalUrl));
		const auto found =
			std::search(data.begin(), data.end(), std::boyer_moore_horspool_searcher(original.begin(), original.end()));
		if (found == data.end())
			return Redirect::NotFound;

		const std::span<std::byte> room(found, original.size());
		DWORD protection;
		VirtualProtect(room.data(), room.size(), PAGE_READWRITE, &protection);
		std::ranges::fill(room, std::byte{0});
		std::memcpy(room.data(), url.data(), url.size() * sizeof(wchar_t));
		VirtualProtect(room.data(), room.size(), protection, &protection);
		return Redirect::Replaced;
	}
}
