#pragma once

#include "stdafx.h"

#pragma warning(push)
#pragma warning(disable : 6387 26451 26495 26498)
#include "../include/asmjit/asmjit.h"
#include "../include/MCF/VirtualAllocNear.hpp"
#pragma warning(pop)

#include "Console.h"
#include "Trampoline.h"
#include "MsgRepoPatch.h"
#include "camera/Camera.h"

const static void* pInitializeCamera = reinterpret_cast<const void*>(&InitializeCamera);
const static void* pDeinitializeCamera = reinterpret_cast<const void*>(&DeinitializeCamera);
const static void* pPerspectiveControl = reinterpret_cast<const void*>(&PerspectiveControl);
const static void* pPerspectiveInit = reinterpret_cast<const void*>(&PerspectiveInit);
const static void* pCheckPlayerTAE = reinterpret_cast<const void*>(&CheckPlayerTAE);
const static void* pCheckEnemyTAE = reinterpret_cast<const void*>(&CheckEnemyTAE);
const static void* pAttachToHead = reinterpret_cast<const void*>(&AttachToHead);
const static void* pPatchMsgRepo = reinterpret_cast<const void*>(&PatchMsgRepo);

extern void InjectAsm();