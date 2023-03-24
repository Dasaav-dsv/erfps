#include "gamefn.h"

extern decltype(&::defCreateBulletIns) CreateBulletIns = nullptr;

extern decltype(&::defGetChrBulletEmitterSlot) GetChrBulletEmitterSlot = nullptr;

extern decltype(&::defGetBulletParamEntry) GetBulletParamEntry = nullptr;

extern decltype(&::defGetAttackParamEntry) GetAttackParamEntry = nullptr;

extern decltype(&::defGetEventFlag) GetEventFlag = nullptr;

extern decltype(&::defSetEventFlag) SetEventFlag = nullptr;

extern decltype(&::defHknpWorldRayCast) hknpWorldRayCast = nullptr;