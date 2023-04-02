#pragma once

#include "stdafx.h"
#include <tuple>

namespace
{
	void* TraverseSafePtrSTEP(void* base, const int offset0)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(base) + offset0);
	}

	void* TraverseSafePtrSTEP(void* base, const int* pOffset0)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(base) + *pOffset0);
	}

	void* TraverseSafePtrSTEP(void** base, const int offset0)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(*base) + offset0);
	}

	void* TraverseSafePtrSTEP(void** base, const int* pOffset0)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(*base) + *pOffset0);
	}

	void* TraverseSafePtrSTEP(void*** base, const int offset0)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(**base) + offset0);
	}

	void* TraverseSafePtrSTEP(void*** base, const int* pOffset0)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(**base) + *pOffset0);
	}

	template <typename... Args> constexpr void* TraverseSafePtrSTEP(void* base, const int offset0, const Args... offsets)
	{
		return TraverseSafePtrSTEP(reinterpret_cast<void*>(*reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(base) + offset0)), offsets...);
	}

	template <typename... Args> constexpr void* TraverseSafePtrSTEP(void** base, const int offset0, const Args... offsets)
	{
		return TraverseSafePtrSTEP(reinterpret_cast<void*>(*reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(*base) + offset0)), offsets...);
	}

	template <typename... Args> constexpr void* TraverseSafePtrSTEP(void*** base, const int offset0, const Args... offsets)
	{
		return TraverseSafePtrSTEP(reinterpret_cast<void*>(*reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(**base) + offset0)), offsets...);
	}

	template <typename... Args> constexpr void* TraverseSafePtrSTEP(void* base, const int* pOffset0, const Args... offsets)
	{
		return TraverseSafePtrSTEP(reinterpret_cast<void*>(*reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(base) + *pOffset0)), offsets...);
	}

	template <typename... Args> constexpr void* TraverseSafePtrSTEP(void** base, const int* pOffset0, const Args... offsets)
	{
		return TraverseSafePtrSTEP(reinterpret_cast<void*>(*reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(*base) + *pOffset0)), offsets...);
	}

	template <typename... Args> constexpr void* TraverseSafePtrSTEP(void*** base, const int* pOffset0, const Args... offsets)
	{
		return TraverseSafePtrSTEP(reinterpret_cast<void*>(*reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(**base) + *pOffset0)), offsets...);
	}
}

template <typename Tin, typename Tout> inline bool ReadUnsafePtr(Tin* baseAddress, Tout& out)
{
	if (reinterpret_cast<uint64_t>(baseAddress) < 0x10000 || reinterpret_cast<uint64_t>(baseAddress) >= 0x800000000000ull)
	{
		out = {};
		return false;
	}

	return ReadProcessMemory(pHandle, reinterpret_cast<void*>(baseAddress), &out, sizeof(out), nullptr);
}

namespace
{
	void* TraverseUnsafePtrSTEP(void* base, const int offset0)
	{
		void* p = nullptr;
		uint8_t* p_ = reinterpret_cast<uint8_t*>(base) + offset0;

		if (ReadUnsafePtr(p_, p))
		{
			return reinterpret_cast<void*>(p_);
		}

		return nullptr;
	}

	void* TraverseUnsafePtrSTEP(void* base, const int* pOffset0)
	{
		void* p = nullptr;
		uint8_t* p_ = reinterpret_cast<uint8_t*>(base) + *pOffset0;

		if (ReadUnsafePtr(p_, p))
		{
			return reinterpret_cast<void*>(p_);
		}

		return nullptr;
	}

	void* TraverseUnsafePtrSTEP(void** base, const int offset0)
	{
		void* p = nullptr;
		uint8_t* p_ = reinterpret_cast<uint8_t*>(*base) + offset0;

		if (ReadUnsafePtr(p_, p))
		{
			return reinterpret_cast<void*>(p_);
		}

		return nullptr;
	}

	void* TraverseUnsafePtrSTEP(void** base, const int* pOffset0)
	{
		void* p = nullptr;
		uint8_t* p_ = reinterpret_cast<uint8_t*>(*base) + *pOffset0;

		if (ReadUnsafePtr(p_, p))
		{
			return reinterpret_cast<void*>(p_);
		}

		return nullptr;
	}

	void* TraverseUnsafePtrSTEP(void*** base, const int offset0)
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(*base);

		if (ReadUnsafePtr(p, p))
		{
			uint8_t* p_ = p + offset0;

			if (ReadUnsafePtr(p_, p))
			{
				return reinterpret_cast<void*>(p_);
			}
		}

		return nullptr;
	}

	void* TraverseUnsafePtrSTEP(void*** base, const int* pOffset0)
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(*base);

		if (ReadUnsafePtr(p, p))
		{
			uint8_t* p_ = p + *pOffset0;

			if (ReadUnsafePtr(p_, p))
			{
				return reinterpret_cast<void*>(p_);
			}
		}

		return nullptr;
	}

	template <typename... Args> constexpr void* TraverseUnsafePtrSTEP(void* base, const int offset0, const Args... offsets)
	{
		void* p = nullptr;

		if (ReadUnsafePtr(reinterpret_cast<uint8_t*>(base) + offset0, p))
		{
			return TraverseUnsafePtrSTEP(p, offsets...);
		}
			
		return nullptr;
	}

	template <typename... Args> constexpr void* TraverseUnsafePtrSTEP(void** base, const int offset0, const Args... offsets)
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(*base);

		if (ReadUnsafePtr(p + offset0, p))
		{
			return TraverseUnsafePtrSTEP(reinterpret_cast<void*>(p), offsets...);
		}
		
		return nullptr;
	}

	template <typename... Args> constexpr void* TraverseUnsafePtrSTEP(void*** base, const int offset0, const Args... offsets)
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(*base);

		if (ReadUnsafePtr(p, p))
		{
			if (ReadUnsafePtr(p + offset0, p))
			{
				return TraverseUnsafePtrSTEP(reinterpret_cast<void*>(p), offsets...);
			}
		}
		
		return nullptr;
	}

	template <typename... Args> constexpr void* TraverseUnsafePtrSTEP(uint8_t* base, const int* pOffset0, const Args... offsets)
	{
		void* p = nullptr;

		if (ReadUnsafePtr(reinterpret_cast<uint8_t*>(base) + *pOffset0, p))
		{
			return TraverseUnsafePtrSTEP(p, offsets...);
		}

		return nullptr;
	}

	template <typename... Args> constexpr void* TraverseUnsafePtrSTEP(void** base, const int* pOffset0, const Args... offsets)
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(*base);

		if (ReadUnsafePtr(p + *pOffset0, p))
		{
			return TraverseUnsafePtrSTEP(reinterpret_cast<void*>(p), offsets...);
		}

		return nullptr;
	}

	template <typename... Args> constexpr void* TraverseUnsafePtrSTEP(void*** base, const int* pOffset0, const Args... offsets)
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(*base);
		if (ReadUnsafePtr(p, p))
		{
			if (ReadUnsafePtr(p + *pOffset0, p))
			{
				return TraverseUnsafePtrSTEP(reinterpret_cast<void*>(p), offsets...);
			}
		}

		return nullptr;
	}
}

template <typename T = uint8_t, typename... Ts> constexpr T* TraverseSafePtr(const Ts... t)
{
	return reinterpret_cast<T*>(std::apply([](auto &&... args) -> void* { return TraverseSafePtrSTEP(args...); }, t...));
}

template <typename T = uint8_t, typename... Ts> constexpr T* TraverseSafeOffsetPtr(const int offset, const int numAdd, const Ts... t)
{
	return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(std::apply([](auto &&... args) -> void* { return TraverseSafePtrSTEP(args...); }, t...)) + offset * numAdd);
}

template <typename T = uint8_t, typename... Ts> constexpr T* TraverseUnsafePtr(const Ts... t)
{
	return reinterpret_cast<T*>(std::apply([](auto &&... args) -> void* { return TraverseUnsafePtrSTEP(args...); }, t...));
}

template <typename T = uint8_t, typename... Ts> constexpr T* TraverseUnsafeOffsetPtr(const int offset, const int numAdd, const Ts... t)
{
	uint8_t* p = reinterpret_cast<uint8_t*>(std::apply([](auto &&... args) -> void* { return TraverseUnsafePtrSTEP(args...); }, t...));

	if (p == nullptr)
	{
		return nullptr;
	}

	return reinterpret_cast<T*>(p + offset * numAdd);
}

inline void* codeMem = nullptr;

inline void** GameDataMan = nullptr;
inline void** WorldChrMan = nullptr;
inline void** WorldChrManDbg = nullptr;
inline void** SessionMan = nullptr;
inline void** FieldArea = nullptr;
inline void** LockTgtMan = nullptr;
inline void** CSFlipper = nullptr;
inline void** CSMenuMan = nullptr;
inline void** CSRemo = nullptr;
inline void** CSEventFlagMan = nullptr;
inline void** CSUserInputMan = nullptr;
inline void** CSBulletMan = nullptr;
inline void** CSHavokMan = nullptr;
inline void** SoloParamRepository = nullptr;
inline void** MsgRepository = nullptr;
inline void** CSPcKeyConfig = nullptr;
inline void** CSMouseMan = nullptr;
inline void* MagicPtr = nullptr;
inline int* CSLang = nullptr;

inline int32_t PlayerInsOffset = 0x10EF8;
inline int32_t ChrAsmOffset = 0x638;
inline int32_t ChrSlotSysOffset = 0x300;
inline int32_t crosshairOffset = 0x3710;

inline void* PlayerInsSpectate = nullptr;

inline uint8_t* printProcessEntriesCode = nullptr;

inline void* lockStickCode = nullptr;
inline void* lockSwitchCode = nullptr;
inline void* lockAcquireCode = nullptr;
inline void* lockDropCode = nullptr;
inline void* lockAimDropCode = nullptr;
inline void* lockOnCode = nullptr;
inline void* lockOnCodeCall = nullptr;
inline void* saveLoadCode = nullptr;
inline void* dtorStartCode = nullptr;
inline void* playerTAECode = nullptr;
inline void* enemyTAECode = nullptr;
inline void* fallDeathCode = nullptr;
inline void* headSizeCode = nullptr;
inline void* upperHandCode = nullptr;
inline void* padSensCode = nullptr;
inline void* menuDisplayCode = nullptr;
inline void* startScreenCode = nullptr;
inline void* hkRunCode = nullptr;
inline void* RunStateCode = nullptr;
inline void* aimTgtCode = nullptr;
inline void* aimFadeCode = nullptr;
inline void* genInputCode = nullptr;
inline void* menuLogoCode = nullptr;
inline void* chrDitherCode = nullptr;
inline void* AEGDitherCode = nullptr;
inline void* playerDitherCode = nullptr;
inline void* playerLightCode = nullptr;
inline void* playerShadowCode = nullptr;
inline void* VFXCode00 = nullptr;
inline void* VFXCode01 = nullptr;
inline void* crosshairSizeCode = nullptr;
inline void* crosshairSpreadCode = nullptr;
inline void* drawGameCrosshairCode00 = nullptr;
inline void* drawGameCrosshairCode01 = nullptr;
inline void* lockTgtCode = nullptr;
inline void* chunkCode = nullptr;
inline void* rootTransformCode = nullptr;
inline void* stateInfoCode = nullptr;
inline void* ragdollCode = nullptr;

inline void* hitDetectCode = nullptr;
inline void* hitDetectCodeCpy = nullptr;
inline void* hitDetectCodeEnd = nullptr;
inline void* hitRegisterCode = nullptr;
inline void* regBulletCode = nullptr;
inline void* dstBulletCode = nullptr;

inline void* hitRegisterCall = nullptr;

inline void* hvkBoneCode = nullptr;

inline void* CSDrawStepCall = nullptr;
inline void* frameRendCode = nullptr;
inline void* FrameRendCall = nullptr;
inline void* writeRendMCode00 = nullptr;
inline void* cwriteRendMCode00 = nullptr;
inline void* writeRendMCode01 = nullptr;
inline void* writeRendMCode02 = nullptr;

inline void* writeRendMCall01 = nullptr;

inline void* strafeCheckCode00 = nullptr;
inline void* strafeCheckCode01 = nullptr;
inline void* strafeCheckCode02 = nullptr;
inline void* strafeCheckCode03 = nullptr;
inline void* strafeCheckCode04 = nullptr;

inline void* strafeCheckCodeCall = nullptr;

inline void* camCode00 = nullptr;
inline void* camCode01 = nullptr;
inline void* camCode02 = nullptr;
inline void* camCode03 = nullptr;
inline void* camCode04 = nullptr;
inline void* camCode05 = nullptr;
inline void* camCode06 = nullptr;
inline void* camCode07 = nullptr;
inline void* camCode08 = nullptr;
inline void* camCode09 = nullptr;
inline void* camCode10 = nullptr;
inline void* camCode11 = nullptr;
inline void* camCode12 = nullptr;
inline void* camCode13 = nullptr;
inline void* camCode14 = nullptr;
inline void* camCode15 = nullptr;
inline void* camCode16 = nullptr;
inline void* camCode17 = nullptr;
inline void* camCode18 = nullptr;
inline void* camCode19 = nullptr;
inline void* camCode20 = nullptr;
inline void* camCode21 = nullptr;
inline void* camCode22 = nullptr;
inline void* camCode23 = nullptr;
inline void* camCode24 = nullptr;
inline void* camCode25 = nullptr;
inline void* camCode26 = nullptr;
inline void* camCode27 = nullptr;
inline void* camCode28 = nullptr;
inline void* camCode29 = nullptr;
inline void* camCode30 = nullptr;
inline void* camCode31 = nullptr;

inline void* camCode01Call = nullptr;
inline void* camCode15Call = nullptr;
inline void* camCode26Call = nullptr;

inline void* getEventFlagCode = nullptr;
inline void* setEventFlagCode = nullptr;

inline void* hknpWorldRayCastCode = nullptr;

inline void* createBulletInsCode = nullptr;
inline void* getChrBulletEmitterSlotCode = nullptr;
inline void* getBulletParamEntryCode = nullptr;
inline void* getAttackParamEntryCode = nullptr;
inline void* checkSpEffectCode = nullptr;
inline void* getSpEffectParamCode = nullptr;

inline void* TAE_Base_ptr = nullptr;
inline void* TAE_PtrOffset = nullptr;
inline void* TAE_PtrNameOffset = nullptr;

inline void* GenInput_PtrBase = nullptr;

inline auto PlayerIns_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0);

inline auto flwChrCam_ptr = std::make_tuple(&FieldArea, 0x98, 0x60, 0x10);
inline auto chrCam_ptr = std::make_tuple(&FieldArea, 0x98, 0x10);
inline auto dbgCam_ptr = std::make_tuple(&FieldArea, 0x20, 0xD0, 0x10);
inline auto aimCam_ptr = std::make_tuple(&FieldArea, 0x98, 0x68, 0x10);
inline auto aimCamBase_ptr = std::make_tuple(&FieldArea, 0x98, 0x68, 0xA0);
inline auto camModeSelect_ptr = std::make_tuple(&FieldArea, 0x20, 0xC8);
inline auto aimCamMode_ptr = std::make_tuple(&FieldArea, 0x98, 0x7C);
inline auto trackDodgesSet_ptr = std::make_tuple(&FieldArea, 0x98, 0x60, 0x330);

inline auto soundMtx_ptr = std::make_tuple(&FieldArea, 0x20, 0x18, 0x10);
inline auto aimDirMtx_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x120);

inline auto invertPad_ptr = std::make_tuple(&GameDataMan, 0x58, 0xA);
inline auto invertMouse_ptr = std::make_tuple(&CSPcKeyConfig, 0x831);

inline auto fov_ptr = std::make_tuple(&FieldArea, 0x20, 0xD0, 0x50);
inline auto fov2_ptr = std::make_tuple(&FieldArea, 0x98, 0x60, 0x50);
inline auto fovZoom_ptr = std::make_tuple(&FieldArea, 0x98, 0x68, 0x50);
inline auto fovZoomRender_ptr = std::make_tuple(&FieldArea, 0x98, 0x68, 0x118);

inline auto hideHUD_ptr = std::make_tuple(&GameDataMan, 0x58, 0x9);

inline auto isEnableCrosshair_ptr = std::make_tuple(&FieldArea, 0x98, 0x70, 0x127);
inline auto crosshairSquare_ptr = std::make_tuple(&FieldArea, 0x98, 0x70, 0x114);

inline auto playerCam_ptr = std::make_tuple(&WorldChrManDbg, 0xB8);

inline auto playerPos_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0x68, 0x70);
inline auto playerPos_ptr_PIns = std::make_tuple(&PlayerInsSpectate, 0x190, 0x68, 0x70);
inline auto playerRot_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0x68, 0x50);
inline auto playerRot_ptr_PIns = std::make_tuple(&PlayerInsSpectate, 0x190, 0x68, 0x50);
inline auto playerTgtPos_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0x68, 0xA8, 0x18, 0xD0);
inline auto playerRootTransform_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x58, 0x140);
inline auto playerRootTransformMult_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0xC0, 0x24C);
inline auto playerVel_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0xC0, 0xF00);

inline auto CSChrPhysicsModule_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0x68);

inline auto isLockedOnMove_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x58, 0x199);
inline auto isLockedOnAnim_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0xC9);
inline auto REMOState_ptr = std::make_tuple(&CSRemo, 0x8, 0x40);

inline auto animID_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0x80, 0x90);
inline auto animGroup_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0x28, 0x138);
inline auto animType_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0x28, 0x13C);
inline auto turnSpeed_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0x8, 0x84);

inline auto dT_ptr = std::make_tuple(&CSFlipper, 0x270);

inline auto lockModeEnemy_ptr = std::make_tuple(&LockTgtMan, 0x2830);
inline auto lockModeEnabled_ptr = std::make_tuple(&LockTgtMan, 0x2831);
inline auto lockModeDrop_ptr = std::make_tuple(&LockTgtMan, 0x2832);
inline auto lockTgtPos_ptr = std::make_tuple(&LockTgtMan, 0x10, 0x80, 0x0);

inline auto lockAddAng_ptr = std::make_tuple(&LockTgtMan, 0x295C);
inline auto lockAng_ptr = std::make_tuple(&LockTgtMan, 0x292C);

inline auto isMenu_ptr = std::make_tuple(&CSMenuMan, 0x1C);
inline auto isMouse_ptr = std::make_tuple(&CSMouseMan, 0x3);
inline auto isObjAct_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x58, 0xE9);
inline auto isElevator_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x2DF);
inline auto isPressRunState_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x58, 0x18, 0x1D0);
inline auto runPressTime_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x58, 0x18, 0x120);

inline auto isMount_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0xE8, 0x163);

inline auto lockOnPress_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x8, 0x770, 0x8, 0x20, 0x0, 0x0, 0x8, 0x8, 0x38, 0x18, 0x0);
inline auto lockOnPressOffset_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x8, 0x770, 0x8, 0x20, 0x0, 0x0, 0x0, 0x10, 0x8, 0x48, 0x38, 0x10, 0x44);

inline auto interactPress_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x8, 0x770, 0x8, 0x20, 0x0, 0x0, 0x8, 0x8, 0x38, 0x18, 0x0);
inline auto interactPressOffset_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x8, 0x770, 0x8, 0x20, 0x0, 0x0, 0x0, 0x10, 0x8, 0x48, 0x38, 0x10, 0x104);

inline auto boneLayout_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x58, 0x28, 0x78, 0x28, 0x38);
inline auto boneLayout_ptr_PIns = std::make_tuple(&PlayerInsSpectate, 0x58, 0x28, 0x78, 0x28, 0x38);
inline auto headBonePos_ptr = std::make_tuple(&CSHavokMan, 0x98, 0x8, 0x20, 0x8, 0x0);
inline auto headBoneQ_ptr = std::make_tuple(&CSHavokMan, 0x98, 0x8, 0x20, 0x8, 0x0);

inline auto MsgRepoMenuItems_ptr = std::make_tuple(&MsgRepository, 0x8, 0x0, 0x640, 0x0);
inline auto MsgRepoMenuDescr_ptr = std::make_tuple(&MsgRepository, 0x8, 0x0, 0x648, 0x0);

inline auto mouseInputLR_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x40, 0x800);
inline auto mouseInputUD_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x40, 0x804);
inline auto mouseSens_ptr = std::make_tuple(&CSPcKeyConfig, 0x830);

inline auto rStickInputLR_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x10, 0x8A8);
inline auto rStickInputUD_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x10, 0x8AC);
inline auto padSens_ptr = std::make_tuple(&FieldArea, 0x98, 0x60, 0x2D0);

inline auto movementInputL_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x8, 0x7A0, 0x0);
inline auto movementInputR_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x8, 0x7A0, 0x0);
inline auto movementInputU_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x8, 0x7A0, 0x0);
inline auto movementInputD_ptr = std::make_tuple(&CSUserInputMan, 0x18, 0x8, 0x7A0, 0x0);

inline auto movementInputOffsetPtr = std::make_tuple(&CSUserInputMan, 0x78, 0x228, 0x48, 0x20, 0x10, 0x0);

inline auto hkbRunState_ptr = std::make_tuple(&WorldChrMan, &PlayerInsOffset, 0x0, 0x190, 0x28, 0x60, 0x30, 0x98, 0x118, 0x18, 0x174);

inline auto upperArmParam_ptr = std::make_tuple(&SoloParamRepository, 0x3298, 0x80, 0x80, 0x160 + 0x50 * 4);