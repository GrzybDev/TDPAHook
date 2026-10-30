#include <cstdio>
#include <filesystem>
#include <format>
#include <string>

#include <windows.h>

#include "redirect.hpp"
#include "version.hpp"

BOOL APIENTRY DllMain(const HMODULE module, const DWORD reason, LPVOID /*reserved*/)
{
	if (reason != DLL_PROCESS_ATTACH)
		return TRUE;

	std::wstring path(MAX_PATH, L'\0');
	DWORD length;
	while ((length = GetModuleFileNameW(module, path.data(), static_cast<DWORD>(path.size()))) == path.size())
		path.resize(path.size() * 2);
	path.resize(length);

	const std::filesystem::path folder = std::filesystem::path(path).parent_path();
	const std::filesystem::path settings = folder / L"TDPAHook.ini";

	// Room for far more than the longest address that fits, so that the log shows a longer one whole.
	std::wstring url(2048, L'\0');
	url.resize(GetPrivateProfileStringW(L"Games", L"ListUrl", tdpahook::kDefaultUrl.data(), url.data(),
	                                    static_cast<DWORD>(url.size()), settings.c_str()));

	// The games' DRM decrypts their code only after this library is loaded, but the read-only data that holds the
	// address is never encrypted, so it is rewritten before any of the game's code runs.
	const tdpahook::Redirect result =
		tdpahook::redirectGamesList(tdpahook::readOnlyData(GetModuleHandleW(nullptr)), url);

	if (GetPrivateProfileIntW(L"Log", L"File", 0, settings.c_str()) == 0)
		return TRUE;

	std::wstring outcome;
	switch (result)
	{
	case tdpahook::Redirect::Replaced:
		outcome = std::format(L"The game downloads the list of games from {}", url);
		break;
	case tdpahook::Redirect::TooLong:
		outcome = std::format(L"{} is longer than {} characters, so the game keeps its own address", url,
		                      tdpahook::kMaxUrlLength);
		break;
	case tdpahook::Redirect::NotFound:
		outcome = L"This executable holds no address of the list of games, so it stays as it is";
		break;
	}

	FILE* log;
	if (_wfopen_s(&log, (folder / L"TDPAHook.log").c_str(), L"w, ccs=UTF-8") == 0)
	{
		fputws(std::format(L"TDPAHook {} by Marek Grzyb (@GrzybDev), https://github.com/GrzybDev/TDPAHook\n{}\n",
		                   tdpahook::kVersion, outcome).c_str(), log);
		fclose(log);
	}

	return TRUE;
}
