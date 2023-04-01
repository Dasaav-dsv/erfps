#pragma once

extern __declspec(dllexport) bool __cdecl defCheckSpEffect(const void* CSSpecialEffect, const int spEffectID);

extern decltype(&::defCheckSpEffect) CheckSpEffect;