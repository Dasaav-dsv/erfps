#pragma once

extern __declspec(dllexport) bool __cdecl defGetEventFlag(const void* Manager, const unsigned int FlagID);

extern __declspec(dllexport) void __cdecl defSetEventFlag(const void* Manager, const unsigned int FlagID, const bool State, const char Param_4);

extern decltype(&::defGetEventFlag) GetEventFlag;

extern decltype(&::defSetEventFlag) SetEventFlag;