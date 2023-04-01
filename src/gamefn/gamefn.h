#pragma once

#include "Bullet.h"
#include "EventFlag.h"
#include "hk.h"
#include "SpEffect.h"

#include "../Pointer.h"

inline void GetGameFunctions()
{
	CreateBulletIns = reinterpret_cast<decltype(&::defCreateBulletIns)>(createBulletInsCode);

	GetChrBulletEmitterSlot = reinterpret_cast<decltype(&::defGetChrBulletEmitterSlot)>(getChrBulletEmitterSlotCode);

	GetBulletParamEntry = reinterpret_cast<decltype(&::defGetBulletParamEntry)>(getBulletParamEntryCode);

	GetAttackParamEntry = reinterpret_cast<decltype(&::defGetAttackParamEntry)>(getAttackParamEntryCode);

	GetEventFlag = reinterpret_cast<decltype(&::defGetEventFlag)>(getEventFlagCode);

	SetEventFlag = reinterpret_cast<decltype(&::defSetEventFlag)>(setEventFlagCode);

	hknpWorldRayCast = reinterpret_cast<decltype(&::defHknpWorldRayCast)>(hknpWorldRayCastCode);

	CheckSpEffect = reinterpret_cast<decltype(&::defCheckSpEffect)>(checkSpEffectCode);
}