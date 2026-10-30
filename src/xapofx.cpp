#include <filesystem>
#include <string>

#include <windows.h>
#include <unknwn.h>

// The signature of XAPOFX 1.5; the Windows SDK's xapofx.h declares the four parameters of later versions.
extern "C" __declspec(dllexport) HRESULT WINAPI CreateFX(REFCLSID effectId, IUnknown** effect)
{
	// The games import this library only for this function, which the system's copy carries out. It is loaded on the
	// first call rather than in DllMain, where loading a library can deadlock.
	static const auto function = []
	{
		std::wstring folder(MAX_PATH, L'\0');
		folder.resize(GetSystemDirectoryW(folder.data(), MAX_PATH));
		const HMODULE library = LoadLibraryW((std::filesystem::path(folder) / L"XAPOFX1_5.dll").c_str());
		return library ? reinterpret_cast<decltype(&CreateFX)>(GetProcAddress(library, "CreateFX")) : nullptr;
	}();

	return function ? function(effectId, effect) : HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}
