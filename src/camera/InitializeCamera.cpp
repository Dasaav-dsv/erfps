#include "InitializeCamera.h"

extern void InitializeCamera(uint8_t* PlayerIns)
{
	CamData* pCamData = &camData;

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

		GetBulletParamEntry(paramDataBuf0, 10000001);
		std::memcpy(paramDataBuf0[0], bulletParam101, sizeof(bulletParam101));

		GetBulletParamEntry(paramDataBuf0, 10000002);
		std::memcpy(paramDataBuf0[0], bulletParam102, sizeof(bulletParam102));

		uint8_t* paramDataBuf1[4];

		GetAttackParamEntry(paramDataBuf1, 1, 301100800);
		*reinterpret_cast<uint32_t*>(paramDataBuf1[2] + 0x18) = 502161;

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