#include <algorithm>
#include <cstring>
#include <ranges>
#include <span>
#include <string>

#include <gtest/gtest.h>
#include <windows.h>

#include "redirect.hpp"

namespace tdpahook::test
{
	namespace
	{
		using namespace std::string_literals;

		// The address between two other strings.
		std::wstring stored(const std::wstring_view url)
		{
			return L"GET\0"s + std::wstring(url) + L"\0No Connection\0"s;
		}

		std::span<std::byte> bytes(std::wstring& text)
		{
			return std::as_writable_bytes(std::span(text));
		}
	}

	TEST(RedirectTest, WritesTheNewAddressOverTheOriginalInReadOnlyMemory)
	{
		const std::wstring original = stored(kOriginalUrl);
		const std::size_t size = original.size() * sizeof(wchar_t);
		auto* const page =
			static_cast<std::byte*>(VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		ASSERT_NE(page, nullptr);
		std::memcpy(page, original.data(), size);
		DWORD protection;
		ASSERT_TRUE(VirtualProtect(page, size, PAGE_READONLY, &protection));

		EXPECT_EQ(redirectGamesList(std::span(page, size), kDefaultUrl), Redirect::Replaced);

		const std::wstring padding(kMaxUrlLength - kDefaultUrl.size(), L'\0');
		EXPECT_EQ(std::wstring_view(reinterpret_cast<const wchar_t*>(page), original.size()),
		          stored(std::wstring(kDefaultUrl) + padding));
		MEMORY_BASIC_INFORMATION memory;
		ASSERT_NE(VirtualQuery(page, &memory, sizeof(memory)), 0u);
		EXPECT_EQ(memory.Protect, static_cast<DWORD>(PAGE_READONLY));
		VirtualFree(page, 0, MEM_RELEASE);
	}

	TEST(RedirectTest, TakesAnAddressAsLongAsTheOriginalButNoLonger)
	{
		const std::wstring longest(kMaxUrlLength, L'x');
		std::wstring data = stored(kOriginalUrl);
		EXPECT_EQ(redirectGamesList(bytes(data), longest), Redirect::Replaced);
		EXPECT_EQ(data, stored(longest));

		data = stored(kOriginalUrl);
		EXPECT_EQ(redirectGamesList(bytes(data), longest + L'x'), Redirect::TooLong);
		EXPECT_EQ(data, stored(kOriginalUrl));
	}

	TEST(RedirectTest, LeavesAStringThatOnlyStartsWithTheAddressAlone)
	{
		const std::wstring longer = std::wstring(kOriginalUrl) + L"?v=2";
		std::wstring data = stored(longer);

		EXPECT_EQ(redirectGamesList(bytes(data), kDefaultUrl), Redirect::NotFound);
		EXPECT_EQ(data, stored(longer));
	}

	// Checks the executables named in TDPAHOOK_GAME_EXES, separated by semicolons, such as the four games'
	// SMG0xx\Binaries\Win64\*-Win64-Shipping.exe.
	TEST(GameTest, RedirectsTheListOfEveryGame)
	{
		std::wstring paths(32767, L'\0');
		paths.resize(GetEnvironmentVariableW(L"TDPAHOOK_GAME_EXES", paths.data(), static_cast<DWORD>(paths.size())));
		if (paths.empty())
			GTEST_SKIP() << "Set TDPAHOOK_GAME_EXES to the games' executables, separated by semicolons, to check them";

		for (const auto part : std::views::split(paths, L';'))
		{
			const std::wstring path(part.begin(), part.end());
			// The loader maps the image without running any of it or loading what it imports, and writes to it stay in
			// this process.
			const HMODULE module = LoadLibraryExW(path.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
			ASSERT_NE(module, nullptr) << path;

			const std::span<std::byte> data = readOnlyData(module);
			EXPECT_EQ(redirectGamesList(data, kDefaultUrl), Redirect::Replaced) << path;
			const std::wstring written = std::wstring(kDefaultUrl) + L'\0';
			EXPECT_FALSE(std::ranges::search(data, std::as_bytes(std::span(written))).empty()) << path;
			FreeLibrary(module);
		}
	}
}
