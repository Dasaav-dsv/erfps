#include "HookMain.h"

static Logger logger{ "DllMain" };

HMODULE LoadDllFromSystemFolder(std::string dllName)
{
	std::string systemFolderPath = "";
	char dummy[1];
	UINT pathLength = GetSystemDirectoryA(dummy, 1);
	systemFolderPath.resize(pathLength);
	LPSTR lpSystemFolderPath = const_cast<char*>(systemFolderPath.c_str());
	GetSystemDirectoryA(lpSystemFolderPath, systemFolderPath.size());
	systemFolderPath = lpSystemFolderPath;

	logger.Log("System folder path: %s", systemFolderPath.c_str());

	HMODULE dll = LoadLibraryA(std::string(systemFolderPath + "\\" + dllName).c_str());
	return dll;
}

DWORD WINAPI HookThread(LPVOID lpParam)
{
	static Renderer renderer;
	static DirectXHook dxHook(&renderer);

	if (iniSet.pIniBool->isCrosshairEnable)
	{
		s_Crosshair = new Crosshair;
		dxHook.AddRenderCallback(s_Crosshair);
	}

	if (iniSet.pIniBool->isShowAttacks)
	{
		s_AttackIndicator = new AttackIndicator;
		dxHook.AddRenderCallback(s_AttackIndicator);
	}

	if (iniSet.pIniBool->isShowAutoaim)
	{
		s_Autoaim = new Autoaim;
		dxHook.AddRenderCallback(s_Autoaim);
	}

	dxHook.Hook();
	return S_OK;
}

BOOL WINAPI DirectXHookMain()
{
	CreateThread(0, 0, &HookThread, 0, 0, NULL);

	return 1;
}