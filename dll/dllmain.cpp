#include "dllmain.h"

DWORD WINAPI InitERFPS()
{
	pHandle = GetCurrentProcess();

	GetModuleInformation(pHandle, GetModuleHandleA("eldenring.exe"), &mInfo, sizeof(mInfo));

	DWORD OldProtect = PAGE_EXECUTE_READ;
	VirtualProtect(mInfo.lpBaseOfDll, mInfo.SizeOfImage, PAGE_EXECUTE_READWRITE, &OldProtect);

	GetText();

	PopulateBases();

	ScanAOBs();

	GetGameFunctions();

	InjectAsm();

	return S_OK;
}

void OnAttach(HINSTANCE hinstDLL)
{
	char dllFile[MAX_PATH];

	GetModuleFileNameA(hinstDLL, dllFile, MAX_PATH);
	_splitpath_s(dllFile, NULL, 0, dllDir, MAX_PATH, NULL, 0, NULL, 0);

	ParseFPSIni();

	if (iniSet.pIniBool->isDbgOut)
	{
		AllocConsoleOnce();
	}

	InitERFPS();

	if (iniSet.pIniBool->isDXHook)
	{
		DirectXHookMain();
	}
}

void OnDetach()
{
	VirtualFree(codeMem, 0, MEM_RELEASE);
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		OnAttach(hinstDLL);
		break;
	case DLL_PROCESS_DETACH:
		OnDetach();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

