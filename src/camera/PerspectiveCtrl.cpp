#include "PerspectiveCtrl.h"

extern void CameraFPS()
{
	CamData* pCamData = &camData;
	IniSettings* pIniSet = &iniSet;

	if (!pCamData->pCamBool->isCamInit)
	{
		return;
	}

	ViewMatrix* dcam_mtx = TraverseSafePtr<ViewMatrix>(dbgCam_ptr);
	ViewMatrix* fcam_mtx = TraverseSafePtr<ViewMatrix>(flwChrCam_ptr);
	ViewMatrix* ccam_mtx = TraverseSafePtr<ViewMatrix>(chrCam_ptr);

	if (TraverseUnsafePtr(CSChrPhysicsModulePtr) != nullptr)
	{
		const float sinPlayerRot_2 = std::clamp(*TraverseSafeOffsetPtr<float>(0x04, 1, playerRot_ptr), -1.0f, 1.0f);
		const float sinPlayerRot = sinPlayerRot_2 * sqrtf(1.0f - sinPlayerRot_2 * sinPlayerRot_2) * 2.0f;
		const float cosPlayerRot = 1.0f - sinPlayerRot_2 * sinPlayerRot_2 * 2.0f;

		V4D var = V4D(-cosPlayerRot, 0.0f, sinPlayerRot, 0.0f);
		V4D vbr = V4D(0.0f, 1.0f, 0.0f);
		V4D vcr = V4D(-sinPlayerRot, 0.0f, -cosPlayerRot, 0.0f);

		VxD::write_vmtx(var, vbr, vcr, dcam_mtx);
		VxD::write_vmtx(var, vbr, vcr, fcam_mtx);
		VxD::write_vmtx(var, vbr, vcr, ccam_mtx);
	}

	SetEventFlag(*reinterpret_cast<void**>(CSEventFlagMan), 92599, false, 0);

	*TraverseSafePtr(isLockedOnMove_ptr) = 0;
	*TraverseSafePtr(isLockedOnAnim_ptr) = 1;
	pCamData->pCamBool->isLockedOnLook = 1;

	*TraverseSafePtr<float>(fovZoomRender_ptr) = RADf(125.0f);

	if (pIniSet->pIniBool->isArmReposEnable)
	{
		memcpy(TraverseSafePtr(upperArmParam_ptr), newUpperArmParam, 80);
	}

	pCamData->pCamBool->isFPS = true;
}

extern void Camera3PS()
{
	CamData* pCamData = &camData;

	if (!pCamData->pCamBool->isCamInit)
	{
		return;
	}

	pCamData->pCamBool->isFPS = false;

	SetEventFlag(*reinterpret_cast<void**>(CSEventFlagMan), 92599, true, 0);

	*TraverseSafePtr(camModeSelect_ptr) = 0;

	*TraverseSafePtr(isLockedOnMove_ptr) = 1;
	*TraverseSafePtr(isLockedOnAnim_ptr) = 0;
	pCamData->pCamBool->isLockedOnLook = 0;

	pCamData->pCamBool->isDisableRunFPS = false;
	pCamData->pCamBool->isFPSAim = false;
	pCamData->pCamBool->RootTOverride = false;
	pCamData->pCamBool->LockTgtOverride = false;
	pCamData->pCamBool->isCrosshairEnable = false;

	pCamData->playerPos_prev[3] = 0.0f;

	*TraverseSafePtr(isEnableCrosshair_ptr) = false;

	*TraverseSafePtr<float>(fov_ptr) = RADf(120.0f);
	*TraverseSafePtr<float>(fov2_ptr) = RADf(48.0f);
	*TraverseSafePtr<float>(fovZoomRender_ptr) = RADf(25.0f);

	*TraverseSafePtr<float>(lockAddAng_ptr) = RADf(30.0f);
	*TraverseSafePtr<float>(lockAng_ptr) = RADf(40.0f);

	std::memset(TraverseSafePtr(upperArmParam_ptr), 0, 80);
}

extern void PerspectiveInit(uint8_t* PlayerIns)
{
	CamData* pCamData = &camData;

	if (!pCamData->pCamBool->isCamInit)
	{
		InitializeCamera(PlayerIns);

		if (pCamData->pCamBool->isFPS)
		{
			CameraFPS();
		}
		else
		{
			Camera3PS();
		}

		return;
	}
}

extern void PerspectiveControl()
{
	CamData* pCamData = &camData;

	if (TraverseUnsafePtr<void>(CSChrPhysicsModulePtr) == nullptr)
	{
		return;
	}

	float _dT = *TraverseSafePtr<float>(dT_ptr);
	pCamData->dT = _dT;

	static float sMenuTimeout = 0.0f;

	if (*TraverseSafePtr(isMenu_ptr) != 0 || *TraverseSafePtr(isMouse_ptr) != 0)
	{
		sMenuTimeout = 0.3f;
	}
	else if (sMenuTimeout > 0.0f)
	{
		sMenuTimeout -= _dT;
	}

	uint8_t* playerCamTgtIns = *TraverseSafePtr<uint8_t*>(playerCam_ptr);
	static uint8_t* playerCamTgtIns_prev = nullptr;

	if (pCamData->pCamBool->isCamInit)
	{
		if (playerCamTgtIns_prev != playerCamTgtIns)
		{
			DeinitializeCamera();

			playerCamTgtIns_prev = playerCamTgtIns;

			return;
		}
	}
	else if (*TraverseSafePtr(REMOState_ptr) == 1)
	{
		PerspectiveInit(playerCamTgtIns);

		playerCamTgtIns_prev = playerCamTgtIns;

		return;
	}

	playerCamTgtIns_prev = playerCamTgtIns;

	if (pCamData->pCamBool->isMimic)
	{
		if (pCamData->pCamBool->isFPS && !pCamData->pCamBool->isFPS_prev)
		{
			Camera3PS();

			pCamData->pCamBool->isFPS_prev = true;
		}

		return;
	}
	else if (pCamData->pCamBool->isFPS_prev)
	{
		CameraFPS();

		pCamData->pCamBool->isFPS_prev = false;

		return;
	}

	uint32_t* plockOnPressOffset = reinterpret_cast<uint32_t*>(TraverseUnsafePtr(lockOnPressOffset_ptr));
	uint32_t lockOnPressOffset;

	if (!ReadUnsafePtr(plockOnPressOffset, lockOnPressOffset))
	{
		return;
	}

	uint8_t* plockOnPress = TraverseUnsafeOffsetPtr<uint8_t>(lockOnPressOffset, 4, lockOnPress_ptr);
	uint8_t lockOnPress;

	if (!ReadUnsafePtr(plockOnPress, lockOnPress))
	{
		return;
	}

	static float sPressTime = 0.0f;
	static bool sIsPressState = false;

	if (lockOnPress == 0 && sMenuTimeout <= 0.0f)
	{
		sPressTime += _dT;
		if (!(pCamData->pCamBool->isFPS || sIsPressState))
		{
			if (sPressTime > 0.3f)
			{
				sIsPressState = true;
				CameraFPS();
				return;
			}
		}
		else if (pCamData->pCamBool->isFPS && !sIsPressState)
		{
			if (sPressTime == _dT)
			{
				bool* pLockMode2 = TraverseSafePtr<bool>(lockModeEnabled_ptr);
				*pLockMode2 = !*pLockMode2;
			}

			if (sPressTime > 0.3f)
			{
				sIsPressState = true;
				Camera3PS();
				return;
			}
		}
	}
	else
	{
		sPressTime = 0.0f;
		sIsPressState = false;
	}
}
