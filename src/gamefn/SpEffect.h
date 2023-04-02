#pragma once

extern __declspec(dllexport) bool __cdecl defGetSpEffectParamEntry(unsigned char* (&paramDataBuf)[2], int paramEntryID);

extern decltype(&::defGetSpEffectParamEntry) GetSpEffectParamEntry;

extern __declspec(dllexport) bool __cdecl defCheckSpEffect(const void* CSSpecialEffect, const int spEffectID);

extern decltype(&::defCheckSpEffect) CheckSpEffect;