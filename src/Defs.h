#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Psapi.h>

#include <atomic>
#include <map>

#pragma warning(push)
#pragma warning(disable : 4302 4311 6294 6387)
#include "VxD.h"
#include "Pointer.h"
#pragma warning(pop)

#include "dxoverlay/AttackIndicator/IndicatorType.h"

#define M_PIf 3.14159265358979323846f
#define M_PI_180f 0.0174532923847436904f

#define RADf(deg) (deg * M_PI_180f)

inline HANDLE pHandle = nullptr;

inline MODULEINFO mInfo = {};

inline char dllDir[MAX_PATH];

struct CamBool
{
	bool isFPS = false;
	char isGetChrAttack = false;
	bool isTrackHead = false;
	bool isLockLook = false;
	bool isSyncRot = false;
	bool isFPS_prev = false;
	bool isCamInit = false;
	bool isCrosshairEnable = false;
	char FrameCounter = false;
	bool isCrossChunk = false;
	bool isLockedOnLook = false;
	bool isDisableRunFPS = false;
	bool isFPSAim = false;
	bool RootTOverride = false;
	bool LockTgtOverride = false;
	bool isObjAct = false;
	bool isSpectate = false;
	bool isSmoothAnim = false;
	bool isBinocs = false;
	bool isMimic = false;
	bool isDragon = false;
	bool isCrouch = false;
};

struct CamData
{
	CamBool* pCamBool;

	uint32_t animationID = 0;
	float camGoalSyncTime = 0.0f;
	float camAdjustFwd = 0.0f;
	float camAdjustUp = 0.0f;
	float crosshairSize = 0.0f;
	float crosshairSpread = 0.0f;
	__m128 rootTransform = {};
	V4D playerPos_prev = {};
	uint64_t headBoneOffset = 0;
	float aaScale = 1.0f;
	float dT = 0.0f;
};

struct IniBool
{
	bool isArmReposEnable = true;
	bool isDMMMKicks = true;
	bool isTrueFPS = true;
	bool isAirCtrl = true;
	bool isTrackDodge = true;
	bool isTrackHit = true;
	bool isShowAttacks = true;
	bool isShowAutoaim = true;
	bool isCrosshairEnable = true;
	bool isCrosshairDynamic = true;
	bool isCrosshairDefault = false;
	bool isDisableLogos = false;
	bool isDbgOut = false;
	bool useVAlloc2 = true;
	bool isDXHook = true;
};

struct IniSettings
{
	IniBool* pIniBool;

	uint32_t fpFOV = 90;
	float smoothingStr = 1.0f;
	float aimAssistAng = 8.5f;
	float sensScalesistAng = 1.0f;
	char crosshairType = *"A";
	IndType* indicatorType = new IndType("LARGE");
	float crosshairSize = 0.9375f;
	float crosshairSpread = 1.0f;
};

inline CamBool camBoolData = {};
inline CamData camData = { .pCamBool = &camBoolData };

inline IniBool iniBoolSet = {};
inline IniSettings iniSet = { .pIniBool = &iniBoolSet };

inline const uint8_t newUpperArmParam[] = { 0xFB, 0xFF, 0x2D, 0x00, 0x00, 0x00, 0x28, 0x00, 0xF9, 0xFF, 0xFB, 0xFF, 0x00, 0x00, 0x00, 0x00,
											0x05, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0xFB, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

inline const short fixMagicID[] = { 4000, 4001, 4010, 4020, 4021, 4030, 4060, 4080,
									4090, 4400, 4431, 4500, 4610, 4620, 4720, 4800,
									4820, 4830, 4910, 5100, 5110, 6020, 6100, 6210, 
									6310, 6700, 6701, 6810, 7200, 7210, 7320, 7320 };
