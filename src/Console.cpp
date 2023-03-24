#include "Console.h"

extern void AllocConsoleOnce()
{
	static bool s_IsAlloc = false;

	if (s_IsAlloc)
	{
		return;
	}

	if (AllocConsole())
	{
		FILE* stream;
		s_IsAlloc = !freopen_s(&stream, "CON", "w", stdout);
	}
}