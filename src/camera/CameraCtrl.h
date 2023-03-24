#pragma once

#include "../stdafx.h"
#pragma warning(push)
#pragma warning(disable : 26451)
#include "InitializeCamera.h"
#pragma warning(pop)
#include "../gamefn/Bullet.h"
#include "../gamefn/hk.h"
#include "../dxoverlay/Crosshair/Crosshair.h"
#include "../dxoverlay/AttackIndicator/AttackIndicator.h"
#include "../dxoverlay/Autoaim/Autoaim.h"

inline ViewMatrix sound_mtx = UMTX;

extern void __vectorcall AttachToHead(__m128 headBonePos, __m128 headBoneQ);