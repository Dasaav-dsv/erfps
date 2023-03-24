#include "CameraCtrl.h"

void DrawCrosshair(float playerVel)
{
	CamData* pCamData = &camData;
	IniSettings* pIniSet = &iniSet;

	const bool isDrawUI = *TraverseSafePtr(hideHUD_ptr) != 0 && *TraverseSafePtr<uint32_t>(isMenu_ptr) == 0 && *TraverseSafePtr(isMouse_ptr) == 0 && *TraverseSafePtr(aimCamMode_ptr) == 0;

	pCamData->pCamBool->isCrosshairEnable = pIniSet->pIniBool->isCrosshairEnable && isDrawUI;

	if (isDrawUI && pIniSet->pIniBool->isShowAutoaim && *TraverseSafePtr(lockModeEnabled_ptr) == 1)
	{
		if (!!s_Autoaim) s_Autoaim->DrawAA();
	}

	if (pCamData->pCamBool->isCrosshairEnable)
	{
		if (!!s_Crosshair) s_Crosshair->Draw();

		static float playerVel_prev = 0.0f;

		if (pIniSet->pIniBool->isCrosshairDynamic)
		{
			playerVel = (playerVel + playerVel_prev) * 0.5f;

			float playerVelCvt = std::clamp(playerVel, 0.0f, 0.09f);
			playerVelCvt = tanf(std::ceilf(sqrtf(playerVelCvt) * (playerVelCvt - 0.7f / (playerVelCvt + 50.0f)) * 120.0f) * 0.2f) + 0.2f;

			if (playerVel > 0.02f)
			{
				pCamData->crosshairSpread = pIniSet->crosshairSpread * 5.0f * playerVelCvt + pCamData->crosshairSpread * 0.75f;
			}
			else
			{
				pCamData->crosshairSpread = pIniSet->crosshairSpread * 3.5f + pCamData->crosshairSpread * playerVelCvt;
			}
		}
		else
		{
			pCamData->crosshairSpread = pIniSet->crosshairSpread * 4.375f;
		}

		playerVel_prev = playerVel;
	}
}

bool CheckSimpleAttack(const uint32_t animGroup, const uint32_t animType)
{
	switch (animGroup)
	{
	case 7:
		switch (animType)
		{
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 49:
		case 52:
		case 62:
		case 63:
			return true;
		}
		return false;
	case 8:
		switch (animType)
		{
		case 1:
		case 2:
		case 3:
		case 11:
		case 14:
		case 15:
		case 18:
		case 63:
		case 64:
			return true;
		}
		return false;
	case 9:
		switch (animType)
		{
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			return true;
		}
		return false;
	case 12:
		switch (animType)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			return true;
		}
		return false;
	case 15:
		return true;
	default:
		return false;
	}
}

bool CheckJumpState(const uint32_t animGroup, const uint32_t animType)
{
	if (animGroup == 114)
	{
		switch (animType)
		{
		case 55:
		case 56:
		case 57:
		case 70:
			return true;
		}
	}
	return false;
}

bool CastRay(const V4D rayOrigin, const V4D rayVector, V4D& rayCollision, const float rayLength = 0.0f)
{
	const void* CSPhysWorld = reinterpret_cast<void*>(*(*reinterpret_cast<uint64_t**>(CSHavokMan) + 19)); // CSHavokMan + 0x98
	const void* PlayerIns = *TraverseSafePtr<void*>(PlayerIns_ptr);

	const int filterID = 33554520;

	alignas(16) float rayData[8];
	alignas(16) float collision[4] = {};

	VxD::write(rayOrigin, rayData);

	if (rayLength > 0.0f)
	{
		VxD::write(rayVector * (rayLength / rayVector.length()), rayData + 4);
	}
	else
	{
		VxD::write(rayVector, rayData + 4);
	}

	hknpWorldRayCast(CSPhysWorld, filterID, rayData, rayData + 4, collision, PlayerIns);

	if (_mm_testz_si128(*reinterpret_cast<__m128i*>(&collision), *reinterpret_cast<__m128i*>(&collision)))
	{
		return false;
	}

	rayCollision = collision;
	rayCollision[3] = 1.0f;

	return true;
}

void CreateScanBullet()
{
	uint8_t* PlayerIns = *TraverseSafePtr<uint8_t*>(PlayerIns_ptr);
	*reinterpret_cast<uint64_t*>(bulletStData) = reinterpret_cast<uint64_t>(PlayerIns);
	CreateBulletIns(bulletStData, GetChrBulletEmitterSlot(PlayerIns + ChrSlotSysOffset, 1000002, PlayerIns));
}

void RegisterHitTAE()
{
	uint8_t* p;
	V4D enemyXZY;

	int64_t* pHitRBuf = reinterpret_cast<int64_t*>(&enemyHitRBuf);
	uint32_t count = 0;
	while (*pHitRBuf != -1)
	{
		if (!ReadUnsafePtr(reinterpret_cast<uint8_t*>(*pHitRBuf) + 0x58, p)) goto bad_ptr;
		if (!ReadUnsafePtr(p + 0x260, enemyXZY.V4)) goto bad_ptr;

		if (enemyXZY.inRange(TraverseSafePtr<float>(playerPos_ptr), 10.0f) > 8.0f) goto out_of_range;

		if (!ReadUnsafePtr(reinterpret_cast<uint8_t*>(*pHitRBuf) + 0x190, p)) goto bad_ptr;
		if (!ReadUnsafePtr(p + 0x18, p)) goto bad_ptr;

		*(pHitRBuf + 1) = reinterpret_cast<int64_t>(p);
		reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[count] |= 1;
		goto _continue;
	
	bad_ptr:
	out_of_range:
		reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[count] &= 0;

	_continue:
		count++;
		pHitRBuf += 2;
	}
}

void RegisterAttackIndicators(V4D playerCamFwd, const float fov)
{
	float dot;
	uint8_t* p;
	V4D enemyXZY;
	V4D playerCamRight;

	int64_t* pHitRBuf = reinterpret_cast<int64_t*>(&enemyHitRBuf);
	uint32_t count = 0;
	while (*pHitRBuf != -1)
	{
		if (reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[count] != 3) goto _continue;

		if (!ReadUnsafePtr(reinterpret_cast<uint8_t*>(*pHitRBuf) + 0x190, p)) goto bad_ptr;
		if (!ReadUnsafePtr(p + 0x68, p)) goto bad_ptr;
		if (!ReadUnsafePtr(p + 0x70, enemyXZY.V4)) goto bad_ptr;

		playerCamRight = V4D(playerCamFwd[2], 0.0f, -playerCamFwd[0]).normalize();

		enemyXZY = enemyXZY - TraverseSafePtr<float>(playerPos_ptr);

		dot = playerCamFwd * enemyXZY.normalize();

		enemyXZY = enemyXZY.flatten(true);
		enemyXZY = playerCamFwd.flatten(true) * enemyXZY[0] + playerCamRight * enemyXZY[2];

		if (dot < cosf(fov * 0.25f))
		{
			if (!!s_AttackIndicator) s_AttackIndicator->DrawIndicator(*pHitRBuf, enemyXZY);
		}

		goto _continue;

	bad_ptr:
		reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[count] &= 0;

	_continue:
		count++;
		pHitRBuf += 2;
	}
}

alignas(16) static float rbuf[20];
static uint8_t rbuf_numelements = 0;
static float rbuf_numelementsf = 0.0f;
static uint8_t rbuf_offset = 0;

float SmoothZ16(const float CurrZ)
{
	float* rbuf_start = rbuf + 4;

	if (rbuf_offset < 12)
	{
		rbuf_start[rbuf_offset] = CurrZ;

		rbuf_offset++;
	}
	else if (rbuf_offset > 15)
	{
		rbuf_start[0] = CurrZ;

		rbuf_offset = 1;
	}
	else
	{
		rbuf_start[rbuf_offset] = CurrZ;
		rbuf_start[rbuf_offset - 16] = CurrZ;

		rbuf_offset++;
	}

	if (rbuf_numelements < 16)	
	{
		rbuf_numelementsf = static_cast<float>(++rbuf_numelements);
	}

	uint8_t rbuf_read_offset = rbuf_offset;
	__m128 sum = {};

	for (uint8_t i = 0; i < (rbuf_numelements + 3) / 4; i++)
	{
		sum = _mm_add_ps(sum, _mm_loadu_ps(rbuf + rbuf_read_offset));
		rbuf_read_offset = (rbuf_read_offset - 4) & 15;
	}

	return _mm_cvtss_f32(V4D(sum).hadd()) / rbuf_numelementsf;
}

inline void SmoothZ16Clear()
{
	for (int i = 0; i < 5; i++)
	{
		_mm_store_ps(rbuf + i * 4, _mm_setzero_ps());
	}

	rbuf_offset = 0;
	rbuf_numelements = 0;
}

const float cam_adjust_height = 0.09f;
const float cam_adjust_horizontal = 0.0f;
const float cam_adjust_roll = 0.15f;
const float cam_adjust_depth = -0.03f;

extern void __vectorcall AttachToHead(__m128 headBonePos, __m128 headBoneQ)
{
	CamData* pCamData = &camData;
	IniSettings* pIniSet = &iniSet;

	if (!pCamData->pCamBool->isCamInit)
	{
		return;
	}

	if (*TraverseSafePtr(REMOState_ptr) != 1)
	{
		pCamData->pCamBool->isFPS = false;

		pCamData->pCamBool->isFPS_prev = true;

		DeinitializeCamera();

		*TraverseSafePtr(camModeSelect_ptr) = 0;

		return;
	}

	*TraverseSafePtr(camModeSelect_ptr) = 3;

	const bool isEngLang = *CSLang == 1;
	const bool isSpectateLocal = pCamData->pCamBool->isSpectate;
	const bool isNotAim = *TraverseSafePtr(aimCamMode_ptr) == 0;

	float* fov = TraverseSafePtr<float>(fov_ptr);
	float* fov2 = TraverseSafePtr<float>(fov2_ptr);
	float* fov_zoom = TraverseSafePtr<float>(fovZoom_ptr);

	const float fov_base = (20.0f * (*TraverseSafePtr<float>(padSens_ptr) - 0.5f) + static_cast<float>(pIniSet->fpFOV)) * M_PI_180f;
	float _fov = *fov;

	if (isNotAim)
	{
		if (isEngLang)
		{
			_fov = _fov * 0.9f + fov_base * 0.1f;
		}
		else
		{
			_fov = static_cast<float>(pIniSet->fpFOV) * M_PI_180f;
		}
	}
	else
	{
		_fov = _fov * 0.75f + *fov_zoom * 0.09f;
	}

	*fov = _fov;
	*fov2 = _fov;

	if (*TraverseSafePtr(lockModeEnabled_ptr) == 1)
	{
		*TraverseSafePtr<float>(lockAddAng_ptr) = 0.0f;
		*TraverseSafePtr<float>(lockAng_ptr) = std::clamp(pIniSet->aimAssistAng, 2.5f, 10.0f) * M_PI_180f;
	}

	ViewMatrix* dcam_mtx = TraverseSafePtr<ViewMatrix>(dbgCam_ptr);
	ViewMatrix* fcam_mtx = TraverseSafePtr<ViewMatrix>(flwChrCam_ptr);
	ViewMatrix* ccam_mtx = TraverseSafePtr<ViewMatrix>(chrCam_ptr);
	ViewMatrix* aim_mtx = TraverseSafePtr<ViewMatrix>(aimCam_ptr);

	ViewMatrix* adir_mtx = TraverseSafePtr<ViewMatrix>(aimDirMtx_ptr);

	float* pPlayerRot;

	if (isSpectateLocal)
	{
		pPlayerRot = TraverseUnsafePtr<float>(playerRot_ptr_PIns);

		if (pPlayerRot == nullptr)
		{
			goto DefaultRot;
		}
	}
	else
	{
	DefaultRot:
		pPlayerRot = TraverseSafePtr<float>(playerRot_ptr);
	}

	const float sinPlayerRot_2 = std::clamp(*(pPlayerRot + 1), -1.0f, 1.0f);
	float sinPlayerRot_base = sinPlayerRot_2 * sqrtf(1.0f - sinPlayerRot_2 * sinPlayerRot_2) * 2.0f;
	float cosPlayerRot_base = 1.0f - sinPlayerRot_2 * sinPlayerRot_2 * 2.0f;

	float playerRot = asinf(sinPlayerRot_2) * 2.0f;

	const bool isTrackHead = pCamData->pCamBool->isTrackHead && !pCamData->pCamBool->isSpectate;

	const bool isSmoothAnim = pCamData->pCamBool->isSmoothAnim;

	bool isPressRunState = *TraverseSafePtr<float>(runPressTime_ptr) > 0.0f;
	uint8_t* pIsRunState = TraverseSafePtr(hkbRunState_ptr);
	bool isRunState = *pIsRunState == 2;

	if (isRunState && !isPressRunState)
	{
		*pIsRunState = 1;
		isRunState = false;
	}

	static bool isRunStateB = false;

	isRunState = isRunState && isRunStateB ? true : isRunState && !isRunStateB;

	const bool isMountState = *TraverseSafePtr(isMount_ptr) == 1;

	bool isDisableRunFPSLocal = true;

	const bool isObjAct = pCamData->pCamBool->isObjAct || *TraverseSafePtr(isObjAct_ptr) != 0;

	const bool isNotElevator = (*TraverseSafePtr(isElevator_ptr) & 8) != 0;

	const uint32_t animGroup = *TraverseSafePtr<uint32_t>(animGroup_ptr);
	const uint32_t animType = *TraverseSafePtr<uint32_t>(animType_ptr);

	const bool isSimpleAttack = CheckSimpleAttack(animGroup, animType);
	const bool isAttackAim = isSimpleAttack && *TraverseSafePtr(lockModeEnemy_ptr) == 1;

	pCamData->pCamBool->LockTgtOverride = !isAttackAim;

	const bool isJump = CheckJumpState(animGroup, animType);
	const bool isJumpCtrl = isJump && pIniSet->pIniBool->isAirCtrl;

	bool isInteract = false;
	static float isInteractTime = 0.0f;
	static float interactSyncTime = 0.0f;

	uint32_t* pIsInteractOffset = TraverseUnsafePtr<uint32_t>(interactPressOffset_ptr);
	uint32_t isInteractOffset;

	if (pIniSet->pIniBool->isArmReposEnable)
	{
		uint64_t* pUpperArmParam = TraverseSafePtr<uint64_t>(upperArmParam_ptr);

		if (animGroup != 38 && animGroup != 39)
		{
			*pUpperArmParam |= *reinterpret_cast<const uint64_t*>(newUpperArmParam);
		}
		else
		{
			*pUpperArmParam &= 0xFFFF0000FFFFull;
		}
	}

	if (ReadUnsafePtr(pIsInteractOffset, isInteractOffset))
	{
		uint8_t* pIsInteract = TraverseUnsafeOffsetPtr(isInteractOffset, 4, interactPress_ptr);
		uint8_t _isInteract;

		if (ReadUnsafePtr(pIsInteract, _isInteract))
		{
			if (_isInteract == 0 && !isTrackHead)
			{
				if (isInteractTime < 0.35f)
				{
					isInteractTime += pCamData->dT;
				}
				else
				{
					isInteract = true;
					interactSyncTime = 0.25f;
				}
			}
			else
			{
				isInteractTime = 0.0f;
			}
		}
	}

	static float fwdLookBonus = 0.0f;
	static float runTimeout = 0.0f;

	V4D va;
	V4D vb;
	V4D vc;

	V4D movementInput;

	V4D q_input_v = V4D(umtx.r[3]);
	static V4D q_hb_prev = umtx.r[3];

	if (!isSpectateLocal)
	{
		float movementInputL = 0.0f;
		float movementInputR = 0.0f;
		float movementInputU = 0.0f;
		float movementInputD = 0.0f;

		uint32_t* pMovementInputLOffset = TraverseUnsafeOffsetPtr<uint32_t>(0x38, 1, movementInputOffsetPtr);
		uint32_t* pMovementInputROffset = TraverseUnsafeOffsetPtr<uint32_t>(0x54, 1, movementInputOffsetPtr);
		uint32_t* pMovementInputUOffset = TraverseUnsafeOffsetPtr<uint32_t>(0x0, 1, movementInputOffsetPtr);
		uint32_t* pMovementInputDOffset = TraverseUnsafeOffsetPtr<uint32_t>(0x1C, 1, movementInputOffsetPtr);

		uint32_t movementInputLOffset;
		uint32_t movementInputROffset;
		uint32_t movementInputUOffset;
		uint32_t movementInputDOffset;

		if (ReadUnsafePtr(pMovementInputLOffset, movementInputLOffset) &&
			ReadUnsafePtr(pMovementInputROffset, movementInputROffset) &&
			ReadUnsafePtr(pMovementInputUOffset, movementInputUOffset) &&
			ReadUnsafePtr(pMovementInputDOffset, movementInputDOffset))
		{
			float* pMovementInputL = TraverseUnsafeOffsetPtr<float>(movementInputLOffset, 4, movementInputL_ptr);
			float* pMovementInputR = TraverseUnsafeOffsetPtr<float>(movementInputROffset, 4, movementInputR_ptr);
			float* pMovementInputU = TraverseUnsafeOffsetPtr<float>(movementInputUOffset, 4, movementInputU_ptr);
			float* pMovementInputD = TraverseUnsafeOffsetPtr<float>(movementInputDOffset, 4, movementInputD_ptr);

			float _movementInputL;
			float _movementInputR;
			float _movementInputU;
			float _movementInputD;

			if (ReadUnsafePtr(pMovementInputL, _movementInputL) &&
				ReadUnsafePtr(pMovementInputR, _movementInputR) &&
				ReadUnsafePtr(pMovementInputU, _movementInputU) &&
				ReadUnsafePtr(pMovementInputD, _movementInputD))
			{
				movementInputL = _movementInputL;
				movementInputR = _movementInputR;
				movementInputU = _movementInputU;
				movementInputD = _movementInputD;
			}
		}

		movementInput = V4D(movementInputU + movementInputD, 0.0, movementInputL + movementInputR);
		const float movementInputLen = movementInput.length();

		if (pIniSet->pIniBool->isTrueFPS && (isSimpleAttack || isJumpCtrl))
		{
			pCamData->pCamBool->RootTOverride = true;
			fwdLookBonus = isJumpCtrl ? fwdLookBonus * 0.5f + 0.15f : fwdLookBonus * 0.5f;

			V4D rootTransformLocal = *const_cast<__m128*>(&pCamData->rootTransform);
			float* pPlayerRootTransform = TraverseSafePtr<float>(playerRootTransform_ptr);

			if (movementInputLen > 0.15f)
			{
				float rootTransformLocalLen = rootTransformLocal.length();

				V4D movementInput = V4D(movementInputL + movementInputR, 0.0f, movementInputU + movementInputD);
				movementInput = movementInput.normalize();
				movementInput *= rootTransformLocalLen * -0.625f;

				V4D playerRootTransform = pPlayerRootTransform;
				playerRootTransform = playerRootTransform.flatten<V4D::W>();
				playerRootTransform += movementInput;

				if (!isJump)
				{
					rootTransformLocalLen *= 1.25f;
				}

				const float ProotTransformLenNew = playerRootTransform.length();

				if (ProotTransformLenNew > rootTransformLocalLen)
				{
					playerRootTransform *= rootTransformLocalLen / ProotTransformLenNew;
				}

				playerRootTransform[1] = rootTransformLocal[1];
				playerRootTransform[3] = 1.0f;

				VxD::write(playerRootTransform, pPlayerRootTransform);
			}
			else
			{
				const float rootZ = rootTransformLocal[1];
				rootTransformLocal *= 0.25f;
				rootTransformLocal[1] = rootZ;
				rootTransformLocal[3] = 1.0f;

				VxD::write(rootTransformLocal, pPlayerRootTransform);
			}

			runTimeout = 0.1f;
		}
		else
		{
			pCamData->pCamBool->RootTOverride = false;
			fwdLookBonus *= 0.5f;
		}

		if (!isMountState)
		{
			if ((!isRunState && isPressRunState) || (!isPressRunState && isRunState))
			{
				runTimeout = 0.1f;
			}

			if (runTimeout > 0.0f)
			{
				isRunState = false;
				isRunStateB = false;

				runTimeout -= pCamData->dT;
			}
		}

		*TraverseSafePtr(isLockedOnMove_ptr) = isRunState || isMountState;
		*TraverseSafePtr(isLockedOnAnim_ptr) = !(isRunState && isObjAct);
		pCamData->pCamBool->isLockedOnLook = isRunState || isTrackHead || interactSyncTime > 0.0f;

		V4D var = V4D(-cosPlayerRot_base, 0.0f, sinPlayerRot_base);
		V4D vbr = V4D(0.0f, 1.0f, 0.0f);
		V4D vcr = V4D(-sinPlayerRot_base, 0.0f, -cosPlayerRot_base);

		const V4D q_headbone = headBoneQ;

		if (isTrackHead && !isRunState)
		{
			const float q0 = q_headbone[0];
			const float q1 = q_headbone[1];
			const float q2 = q_headbone[2];
			const float q3 = q_headbone[3];

			var = { 2.0f * q3 * q2 - 2.0f * q0 * q1, 2.0f * q0 * q0 + 2.0f * q2 * q2 -1.0f, -2.0f * q2 * q1 - 2.0f * q3 * q0 };
			vbr = { 1.0f - 2.0f * q2 * q2 - 2.0f * q1 * q1, 2.0f * q0 * q1 + 2.0f * q3 * q2, 2.0f * q0 * q2 - 2.0f * q3 * q1 };
			vcr = { 2.0f * q0 * q2 + 2.0f * q3 * q1, 2.0f * q2 * q1 - 2.0f * q3 * q0, 1.0f - 2.0f * q0 * q0 - 2.0f * q1 * q1 };
		}

		V4D vco = dcam_mtx->mtx[2];
		vc = vco;
		V4D vao = vbr.cross(vco).normalize();

		const V4D vbo = vco.cross(var).normalize();

		if (!pCamData->pCamBool->isLockLook)
		{
			const V4D vcob = vco.projectOnto(vbr);
			const float vcoPL = (vco - vcob).length();

			float mouseInputLR;
			float mouseInputUD;

			float maxRotationHdegs = *TraverseSafePtr<float>(turnSpeed_ptr);
			float sens_coeff = pIniSet->sensScalesistAng * tanf(_fov * 0.5f) / tanf(M_PIf * 0.25f);

			float sens_cam = 1.0f;
			float sens_mouse = 1.0f;
			float sens_pad = 1.0f;

			if (isEngLang)
			{
				sens_cam = static_cast<float>(*TraverseSafePtr(mouseSens_ptr)) * 0.1f + 0.8f;
			}
			else
			{
				sens_mouse = static_cast<float>(*TraverseSafePtr(mouseSens_ptr)) * 0.1f + 0.8f;
				sens_pad = *TraverseSafePtr<float>(padSens_ptr) + 0.8f;
			}

			if (maxRotationHdegs != -1.0f)
			{
				sens_coeff *= 1.0f - 120.0f / (maxRotationHdegs + 360.0f);
			}

			if (*TraverseSafePtr<uint32_t>(isMenu_ptr) == 0)
			{
				mouseInputLR = *TraverseSafePtr<float>(mouseInputLR_ptr) * vcoPL * 0.0010f * sens_coeff * sens_mouse * sens_cam;
				mouseInputUD = *TraverseSafePtr<float>(mouseInputUD_ptr) * vcoPL * 0.0013f * sens_coeff * sens_mouse * sens_cam;

				const uint16_t invertMouseData = *TraverseSafePtr<uint16_t>(invertMouse_ptr);
				if ((invertMouseData & 1) == 0) mouseInputLR *= -1.0f;
				if ((invertMouseData & 256) == 0) mouseInputUD *= -1.0f;
			}
			else
			{
				mouseInputLR = 0.0f;
				mouseInputUD = 0.0f;
			}

			float rStickInputLR = *TraverseSafePtr<float>(rStickInputLR_ptr);
			float rStickInputUD = *TraverseSafePtr<float>(rStickInputUD_ptr);

			const uint16_t invertPadData = *TraverseSafePtr<uint16_t>(invertPad_ptr);
			if ((invertPadData & 1) == 0) rStickInputLR *= -1.0f;
			if ((invertPadData & 256) == 0) rStickInputUD *= -1.0f;

			if (sqrtf(rStickInputLR * rStickInputLR + rStickInputUD * rStickInputUD) < 0.15f)
			{
				rStickInputLR = 0.0f;
				rStickInputUD = 0.0f;
			}
			else
			{
				rStickInputLR *= sign(rStickInputLR) * rStickInputLR * vcoPL * 0.075f * sens_coeff * sens_pad * sens_cam;
				rStickInputUD *= sign(rStickInputUD) * rStickInputUD * vcoPL * 0.100f * sens_coeff * sens_pad * sens_cam;
			}

			const float vRot = tanf(mouseInputUD - rStickInputUD);
			q_input_v = V4D(vao, vRot);

			vc = vco + vao * (mouseInputLR + rStickInputLR);
			vc = vc.normalize().qTransform(q_input_v);
		}

		if (!isTrackHead)
		{
			static V4D vc_goal_track = {};
			static V4D vc_goal_interact = {};

			static float vc_goal_track_dot = 0.999f;
			static float vc_goal_interact_dot = 0.999f;

			if (pCamData->camGoalSyncTime > 0.0f)
			{
				float k = vc * vco;
				k *= k;
				vc_goal_track_dot *= k * k;

				V4D vc_goal = vc_goal_track;

				if (vc * vc_goal < vc_goal_track_dot * vc_goal_track_dot)
				{
					V4D Q_Goal = V4D(vc, vc_goal).qSlerp(V4D(umtx.r[3]), pCamData->camGoalSyncTime / 0.25f);
					vc = vc.qTransform(Q_Goal);
				}
				else
				{
					pCamData->camGoalSyncTime = 0.0f;
				}
			}
			else
			{
				vc_goal_track = vc;
				vc_goal_track_dot = 0.999f;
			}

			if (!isInteract)
			{
				if (interactSyncTime > 0.0f)
				{
					float k = vc * vco;
					k = k * k;
					vc_goal_interact_dot *= k * k;

					interactSyncTime = std::clamp(interactSyncTime - pCamData->dT, 0.0f, 0.25f);

					V4D vc_goal = vc_goal_interact;

					if (vc * vc_goal <= vc_goal_interact_dot * vc_goal_interact_dot)
					{
						V4D Q_Goal = V4D(vc, vc_goal).qSlerp(V4D(umtx.r[3]), interactSyncTime / 0.25f);
						vc = vc.qTransform(Q_Goal);
					}
					else
					{
						interactSyncTime = 0.0f;
					}
				}
				else if (isInteractTime <= 0.0f)
				{
					vc_goal_interact = vc;
					vc_goal_interact_dot = 0.999f;
				}
			}
		}

		va = vbr.cross(vc).normalize();
		vb = vc.cross(va).normalize();

		float max_look_h = isTrackHead ? RADf(120.0f) : RADf(120.0f);

		if (isTrackHead)
		{
			const V4D q_delta = q_headbone.qDiv(q_hb_prev).normalize();

			vc = vc.qTransform(q_delta);
			va = vbr.cross(vc);
			va = va.normalize();
		}

		if (isPressRunState && !isMountState)
		{
			if (movementInputLen > 0.37f)
			{
				V4D miLocal = movementInput;
				miLocal = miLocal.normalize();

				V4D miLocalV = va * miLocal[0] + vc * miLocal[2];
				miLocalV = miLocalV.flatten(true);

				const float relAng = atan2(va[0] * miLocalV[2] - va[2] * miLocalV[0], va[0] * miLocalV[0] + va[2] * miLocalV[2]);
				const float maxRelAng = RADf(105.0f);

				if (std::fabs(relAng) > maxRelAng)
				{
					isRunState = false || isRunStateB;
					isDisableRunFPSLocal = true;
					runTimeout = 0.1f;
				}
				else
				{
					isRunState = true;
					isDisableRunFPSLocal = false;
				}
			}
		}

		pCamData->pCamBool->isDisableRunFPS = isDisableRunFPSLocal;

		V4D vca = vc.projectOnto(var);
		V4D vcb = vc.projectOnto(vbr);
		V4D vcc = vc.projectOnto(vcr);

		V4D vcproj = V4D(vc[0] - vcb[0], vc[1] - vcb[1], vc[2] - vcb[2]);
		float vcprojL = vcproj.length();
		const float cosMLV = cos(RADf(70.0f));

		if (vcprojL == 0.0f)
		{
			vcproj = vcc;
			vcproj *= cosMLV;
			vcprojL = vcproj.length();
		}
		else if (vcprojL < cosMLV)
		{
			vcproj *= cosMLV / vcprojL;
			vcprojL = cosMLV;
		}

		vca = vcproj.projectOnto(var);
		vcc = vcproj.projectOnto(vcr);
		vcb /= vcprojL;

		const V4D vcprojcy = vcproj / vcprojL;
		vca = vcprojcy.projectOnto(var);
		vcc = vcprojcy.projectOnto(vcr);

		const float vcaL = vca.length();
		const float sinMLH = sinf(max_look_h);

		if (runTimeout <= 0.0f && !isJump)
		{
			if (max_look_h < M_PIf * 0.5f)
			{
				if (vcaL > sinMLH)
				{
					const float k = sinMLH * vca.sign2v(var);
					vca = var * k;

					const float cosMLH = cosf(max_look_h);
					vcc = vcr * cosMLH;
				}
			}
			else if (vcc.sign2v(vcr) < 0.0f)
			{
				if (vcaL < sinMLH)
				{
					const float k = sinMLH * vca.sign2v(var);
					vca = var * k;

					const float cosMLH = cosf(RADf(120.0f));
					vcc = vcr * cosMLH;
				}
			}
		}

		vc = vca + vcb + vcc;
		vc = vc.normalize();
		va = vbr.cross(vc).normalize();
		vb = vc.cross(va).normalize();
	}

	float sinPlayerRot = sinPlayerRot_base;
	float cosPlayerRot = cosPlayerRot_base;

	if (!isTrackHead && !isSpectateLocal)
	{
		playerRot = -atan2f(-va[2], -va[0]);
		sinPlayerRot = sinf(playerRot);
		cosPlayerRot = cosf(playerRot);
	}

	float* pPlayerPos;

	if (isSpectateLocal)
	{
		pPlayerPos = TraverseUnsafePtr<float>(playerPos_ptr_PIns);

		if (pPlayerPos == nullptr)
		{
			goto DefaultPos;
		}
	}
	else
	{
	DefaultPos:
		pPlayerPos = TraverseSafePtr<float>(playerPos_ptr);
	}

	const V4D playerPos = pPlayerPos;
	const V4D playerFwd = V4D(-sinPlayerRot, 0.0f, -cosPlayerRot);

	V4D va2 = V4D(-cosPlayerRot, 0.0f, sinPlayerRot);
	V4D vb2 = V4D(0.0f, 1.0f, 0.0f);
	V4D vc2 = playerFwd;
	V4D vd2 = playerPos;

	V4D vcs = V4D(vc[0], 0.0f, vc[2]);
	const float vcsL = vcs.length();

	if (vcsL < 0.01f)
	{
		vcs = vc2;
	}
	else
	{
		vcs /= vcsL;
	}

	V4D vas = vb2.cross(vcs).normalize();

	const float PRot_s = -atan2f(-vas[2], -vas[0]);

	V4D vds = playerPos;
	vds[1] += 1.5f;

	V4D vds_dir = V4D(sinf(PRot_s), 0.0f, cosf(PRot_s));
	V4D vds_ray;

	if (!CastRay(vds, vds_dir, vds_ray, 4.0f))
	{
		vds_ray = vds + vds_dir * 4.0f;
	}

	VxD::write_vmtx(vas, vb2, vcs, vds_ray, &sound_mtx);

	const float max_vAng = RADf(70.0f) * 0.8f;
	const float vAng = std::clamp(-atan2f(vc[1], sqrtf(vc[0] * vc[0] + vc[2] * vc[2])), -max_vAng, max_vAng);

	if (!isSpectateLocal)
	{
		V4D Q_V = V4D(va2, vAng);

		vc2 = vc2.qTransform(Q_V);
		vb2 = vc2.cross(va2).normalize();
	}

	static float s_DragonTime = 0.0f;

	if (pCamData->pCamBool->isDragon)
	{
		s_DragonTime = std::clamp(s_DragonTime + pCamData->dT, 0.0f, 1.0f);
	}
	else
	{
		s_DragonTime = std::clamp(s_DragonTime - 2.0f * pCamData->dT, 0.0f, 1.0f);
	}

	const float dragonTime = 2.0f * s_DragonTime;
	const float dragonTime_ = 1.0f / (dragonTime + 1.0f);

	const float k0 = dragonTime * dragonTime_;
	const float k1 = pCamData->camAdjustFwd * k0;
	const float k2 = pCamData->camAdjustUp * k0;
	const float k3 = k2 * 0.5f;

	if (isSpectateLocal)
	{
		va = va2;
		vb = vb2;
		vc = vc2;
	}

	V4D vdo = headBonePos;
	vdo[3] = 1.0f;
	static float walkSmoothTimeout = 0.0f;

	if (isSmoothAnim)
	{
		const float cWalkSmooth = std::clamp(pIniSet->smoothingStr, 0.0f, 1.0f);

		if (cWalkSmooth > 0.0f)
		{
			const float k1 = walkSmoothTimeout * 5.0f;
			const float k2 = isRunState ? k1 * std::clamp(movementInput[0], 0.0f, 1.0f) : k1;

			V4D vdv = vdo - playerPos;

			V4D vdo_smooth = (playerPos + vdv.projectOnto(playerFwd)) * k2 + vdo * (1.0f - k2);
			vdo_smooth[1] = (playerPos[1] + SmoothZ16(*(reinterpret_cast<float*>(&headBonePos) + 1) - pPlayerPos[1])) * k1 + vdo[1] * (1.0f - k1);

			vdo = vdo_smooth * cWalkSmooth + vdo * (1.0f - cWalkSmooth);

			walkSmoothTimeout = std::clamp(walkSmoothTimeout + pCamData->dT, 0.0f, 0.2f);
		}
	}
	else if (pCamData->pCamBool->isSyncRot && walkSmoothTimeout > 0.0f)
	{
		const float k = walkSmoothTimeout * 5.0f;

		vdo[1] = (playerPos[1] + SmoothZ16(*(reinterpret_cast<float*>(&headBonePos) + 1) - pPlayerPos[1])) * k + vdo[1] * (1.0f - k);

		walkSmoothTimeout -= pCamData->dT;
	}
	else
	{
		SmoothZ16Clear();

		walkSmoothTimeout = 0.0f;
	}

	V4D vd = V4D(vdo[0] + va[0] * cam_adjust_horizontal * dragonTime_ + vc[0] * (cam_adjust_depth + fwdLookBonus + k1) + vb[0] * (cam_adjust_roll * dragonTime_ + k2),
				 vdo[1] + cam_adjust_height + vc[1] * k1 + vb[1] * k3 + k3,
				 vdo[2] + va[2] * cam_adjust_horizontal * dragonTime_ + vc[2] * (cam_adjust_depth + fwdLookBonus + k1) + vb[2] * (cam_adjust_roll * dragonTime_ + k2), 1.0f);

	if (pCamData->playerPos_prev[3] != 0.0f)
	{
		static V4D playerDeltaPos_prev = {};
		const float prevZ = playerDeltaPos_prev[1];
		static bool isCrossChunk = false;

		V4D dPos = pCamData->pCamBool->isCrossChunk || isCrossChunk ? playerDeltaPos_prev : playerPos - pCamData->playerPos_prev;

		playerDeltaPos_prev = dPos;

		dPos[1] = dPos[1] < prevZ ? prevZ : dPos[1];

		const float playerVel = dPos.length();

		if (playerVel > 4.0f)
		{
			dPos *= 4.0f / playerVel;
		}

		if (isNotElevator && !isSpectateLocal)
		{
			const float dPosZ = dPos[1];
			V4D dPosproj = dPos.flatten();

			float k = std::clamp(expf(dPosproj.normalize() * playerFwd - 1.0f), 0.0f, 1.0f);

			dPos = dPosproj * k;
			dPos[1] = dPosZ;

			vd += dPos;
		}

		DrawCrosshair(playerVel);

		if (isCrossChunk)
		{
			pCamData->pCamBool->isCrossChunk = false;
		}

		isCrossChunk = pCamData->pCamBool->isCrossChunk;
	}

	if (!isSpectateLocal)
	{
		if (!isTrackHead && (isSmoothAnim || pCamData->pCamBool->isSyncRot || isSimpleAttack || isJump))
		{
			bool isCamCorrect = false;
			float camCorrectCoeff = 0.0f;
			const float vdZ = vd[1];

			V4D ray;
			V4D rayf;

			V4D vec_coll;
			V4D vec_collf = {};

			float vec_collL = 0.0f;
			float vec_collfL = 0.0f;

			V4D orig = playerPos;
			orig[1] = vdZ;

			V4D vec = vc;
			V4D vec_add = {};
			V4D vecf = vec.flatten(true);
			V4D vecf_add = (vd - orig).projectOnto(vecf);

			if (CastRay(orig, vecf + vecf_add, rayf))
			{
				if ((rayf - orig).length() > 0.4f)
				{
					vec_collf = rayf - vd;
					vec_collfL = vec_collf.length() * sign(vec_collf * (vecf + vecf_add));

					if (vec_collfL < 0.45f)
					{
						if (vec_collfL < 0.15f)
						{
							vd += vec_collf - vecf * 0.15f;
						}

						isCamCorrect = true;
						camCorrectCoeff = std::clamp((0.45f - vec_collfL) / 0.3f, 0.0f, 1.0f) * 0.45f;
					}
				}
			}

			vecf_add = (vd - orig).projectOnto(vecf);

			const float vecPL = vec.flatten().length();
			if (vecPL > 0.0f)
			{
				vec_add = vecf_add;
				vec_add[1] = vec[1] / vecPL * vecf_add.length();

				if (vec_add[1] > 0.1f)
				{
					vec_add *= 0.1f / vec_add[1];
				}
			}

			if (CastRay(vd - vec_add, vec + vec_add, ray))
			{
				if ((ray - orig).flatten().length() > 0.4f)
				{
					vec_coll = ray - vd;
					vec_collL = vec_coll.length() * sign(vec_coll * (vec + vec_add));

					if (vec_collL < 0.45f)
					{
						if (vec_collL < 0.15f)
						{
							vd += vec_coll - vec * 0.15f;
						}

						isCamCorrect = true;
						camCorrectCoeff = std::clamp((0.45f - vec_collL) / 0.3f, 0.0f, 1.0f) * 0.45f;
					}
				}
			}

			if (isCamCorrect)
			{
				const float vcZ = vc[1];

				if (vcZ < sinf(RADf(-15.0f)))
				{
					V4D vc_correct = vc.flatten(true) * cosf(RADf(17.5f)) * cosf(RADf(17.5f));
					vc_correct[1] = sinf(RADf(-17.5f));

					V4D q_correct_cam = q_input_v.qConjugate();

					if (vcZ < sin(RADf(-20.0)))
					{
						q_correct_cam = q_correct_cam.qMul(V4D(umtx.r[3]).qSlerp(V4D(vc, vc_correct), std::clamp(1.0f + vcZ / sinf(RADf(70.0f)), 0.1f, 1.0f) * camCorrectCoeff));
					}

					vc = vc.qTransform(q_correct_cam);
					vb = vc.cross(va).normalize();
				}
			}
		}

		if (!isMountState && isInteractTime > 0.0f)
		{
			const float k = std::clamp((isInteractTime - 0.1f) * 4.0f, 0.0f, 1.0f);

			vd += vc.projectOnto(V4D(-cosPlayerRot_base, 0.0f, sinPlayerRot_base)) * 0.4f * k;

			V4D playerBck = vc.projectOnto(playerFwd);

			if (playerFwd.sign2v(playerBck) < 0.0f)
			{
				vd += playerBck * 0.8f * k;
			}
		}

		if (pIniSet->pIniBool->isShowAttacks 
			&& pIniSet->pIniBool->isDXHook 
			&& *TraverseSafePtr(hideHUD_ptr) != 0 
			&& *TraverseSafePtr<uint32_t>(isMenu_ptr) == 0 
			&& *TraverseSafePtr(isMouse_ptr) == 0)
		{
			static int scanCount = 0;

			if (scanCount == 0)
			{
				pCamData->pCamBool->isGetChrAttack = 2;
				CreateScanBullet();
			}

			RegisterHitTAE();

			RegisterAttackIndicators(vc, _fov);

			scanCount = ++scanCount & 31;
		}
	}

	VxD::write_vmtx(va, vb, vc, vd, dcam_mtx);
	VxD::write_vmtx(va, vb, vc, vd, aim_mtx);

	if (isNotAim && pCamData->pCamBool->isFPSAim)
	{
		VxD::write_vmtx(va, vb, vc, vd, adir_mtx);
	}

	VxD::write(vd2, TraverseSafePtr<float>(aimCamBase_ptr));

	vd2 += vc2 * -0.04f;

	V4D PTgt = playerPos;
	PTgt[1] += 1.5f;

	if (isInteractTime > 0.0f)
	{
		goto SkipRot;
	}

	if (!isTrackHead)
	{
		VxD::write_vmtx(va, vb, vc, vd, ccam_mtx);
		VxD::write_vmtx(va2, vb2, vc2, vd, fcam_mtx);

		if (pCamData->pCamBool->isSyncRot && isNotElevator && !isObjAct)
		{
			VxD::write(V4D(0.0f, sinf(playerRot * 0.5f), 0.0f, cosf(playerRot * 0.5f)), pPlayerRot);
		}

		PTgt += vc2 * 6.0f;
	}
	else
	{
		VxD::write(vd2, fcam_mtx->mtx[3]);

		PTgt += V4D(-sinPlayerRot_base * sqrtf(18.0f), 0.0f, -cosPlayerRot_base * sqrtf(18.0f));
	}

	if (!isAttackAim)
	{
		VxD::write(PTgt, TraverseSafePtr<float>(playerTgtPos_ptr));
	}

SkipRot:
	isRunStateB = isRunState;
	pCamData->playerPos_prev = playerPos;
	q_hb_prev = headBoneQ;
}
