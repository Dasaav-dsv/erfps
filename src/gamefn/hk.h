#pragma once

extern __declspec(dllexport) void __cdecl defHknpWorldRayCast(const void* CSPhysWorld, const int filterID, const float rayOrigin[4], const float rayVector[4], float (&collision)[4], const void* ChrIns);

extern decltype(&::defHknpWorldRayCast) hknpWorldRayCast;