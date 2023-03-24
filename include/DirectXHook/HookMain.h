#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "DirectXHook.h"
#include "Logger.h"
#include "MemoryUtils.h"
#include "../../src/dxoverlay/Crosshair/Crosshair.h"
#include "../../src/dxoverlay/AttackIndicator/AttackIndicator.h"
#include "../../src/dxoverlay/Autoaim/Autoaim.h"

BOOL WINAPI DirectXHookMain();