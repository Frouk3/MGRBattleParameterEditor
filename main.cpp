#include "UI.h"

#include <string>
#include <stdio.h>

extern std::string ConvertWideToUTF8(const std::wstring& wideStr);

#ifdef _WINDLL

HANDLE hCurrentUIThread = nullptr;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDLL);
        UI::hCurrentModule = hinstDLL;
        hCurrentUIThread = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)UI::Render, nullptr, NULL, nullptr);
    }

    if (fdwReason == DLL_PROCESS_DETACH)
        TerminateThread(hCurrentUIThread, 0);

    return TRUE;
}

#else

#pragma warning(push)
#pragma warning(disable : 4996)

FILE* outFile = nullptr, *errFile = nullptr;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	std::string commandLine = ConvertWideToUTF8(lpCmdLine);
    if (strstr(commandLine.c_str(), "-console"))
	{
		AllocConsole();
		outFile = freopen("CONOUT$", "w", stdout);
		errFile = freopen("CONOUT$", "w", stderr);
        printf("Console initialized.\n");
	}
    UI::Render();

	fclose(outFile);
	fclose(errFile);

    FreeConsole();

    return 0;
}
#pragma warning(pop)
#endif