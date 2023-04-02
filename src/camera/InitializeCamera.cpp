#include "InitializeCamera.h"

extern void InitializeCamera(uint8_t* PlayerIns)
{
	CamData* pCamData = &camData;
	IniSettings* pIniSet = &iniSet;

	if (reinterpret_cast<uint64_t*>(WorldChrMan) != nullptr)
	{
		uint32_t* pBoneOffset;

		if (PlayerIns != nullptr)
		{
			pCamData->pCamBool->isSpectate = true;

			PlayerInsSpectate = PlayerIns;

			pBoneOffset = TraverseUnsafePtr<uint32_t>(boneLayout_ptr_PIns);
		}
		else
		{
			pCamData->pCamBool->isSpectate = false;

			PlayerInsSpectate = nullptr;

			pBoneOffset = TraverseUnsafePtr<uint32_t>(boneLayout_ptr);
		}

		uint32_t BoneOffset;
		if (!ReadUnsafePtr(pBoneOffset, BoneOffset))
		{
			return;
		}

		pCamData->headBoneOffset = 0x40ull + static_cast<uint64_t>(0xB0 * ((*pBoneOffset) & 0xFFFFFF));

		if (!GetEventFlag(*reinterpret_cast<void**>(CSEventFlagMan), 92599))
		{
			pCamData->pCamBool->isFPS = true;
		}

		uint8_t* paramDataBuf0[2];
		uint8_t* paramDataBuf1[4];

		if (pIniSet->pIniBool->isShowAttacks)
		{
			GetBulletParamEntry(paramDataBuf0, 10000001);
			std::memcpy(paramDataBuf0[0], bulletParam101, sizeof(bulletParam101));

			GetBulletParamEntry(paramDataBuf0, 10000002);
			std::memcpy(paramDataBuf0[0], bulletParam102, sizeof(bulletParam102));
		}

		if (pIniSet->pIniBool->isDMMMKicks)
		{
			GetSpEffectParamEntry(paramDataBuf0, 501180);
			*reinterpret_cast<int*>(paramDataBuf0[0]) = -1;
			*reinterpret_cast<float*>(paramDataBuf0[0] + 0x8) = 2.0f;
			*reinterpret_cast<short*>(paramDataBuf0[0] + 0x13E) = 0;
			*(paramDataBuf0[0] + 0x16C) |= 33;
			*reinterpret_cast<int*>(paramDataBuf0[0] + 0x170) = -1;
			*reinterpret_cast<int*>(paramDataBuf0[0] + 0x18C) = -1;
			*reinterpret_cast<float*>(paramDataBuf0[0] + 0x270) = 1.0f;
			*reinterpret_cast<float*>(paramDataBuf0[0] + 0x284) = 1.0f;

			GetSpEffectParamEntry(paramDataBuf0, 501220);
			*reinterpret_cast<int*>(paramDataBuf0[0]) = -1;
			*reinterpret_cast<float*>(paramDataBuf0[0] + 0x8) = 0.125f;
			*reinterpret_cast<int*>(paramDataBuf0[0] + 0x124) = 501180;
			*reinterpret_cast<short*>(paramDataBuf0[0] + 0x13E) = 0;
			*(paramDataBuf0[0] + 0x16C) |= 33;
			*reinterpret_cast<int*>(paramDataBuf0[0] + 0x170) = -1;
			*reinterpret_cast<int*>(paramDataBuf0[0] + 0x18C) = -1;
			*reinterpret_cast<int*>(paramDataBuf0[0] + 0x208) = 0;

			GetAttackParamEntry(paramDataBuf1, 1, 301100800);
			*reinterpret_cast<uint32_t*>(paramDataBuf1[2] + 0x18) = 501220;
			*(paramDataBuf1[2] + 0x72) = 10;
		}

		GetAttackParamEntry(paramDataBuf1, 1, 999999999);
		*(paramDataBuf1[2] + 0x78) = 6;
		*(paramDataBuf1[2] + 0x7E) |= 12;

		pCamData->pCamBool->isCamInit = true;
	}
}

extern void DeinitializeCamera()
{
	CamData* pCamData = &camData;

	pCamData->pCamBool->isCamInit = false;
}