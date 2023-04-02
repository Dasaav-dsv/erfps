#pragma warning(disable : 26819)
#include "TAE.h"

static auto TAE_Header_ptr = std::make_tuple(&TAE_Base_ptr, 0x8, 0x18, 0x0);
static auto eventCount_ptr = std::make_tuple(&TAE_Base_ptr, 0x20);

struct varTAE
{
	bool isTrackHead = false;
	bool isLockLook = false;
	bool isOverrideTrackHead = false;
	bool isSyncRot = false;
	bool isSmoothAnim = false;
	bool isObjAct = false;
	bool isAim = true;
	bool isBinocs = false;
	bool isDragon = false;
	bool isCrouch = false;
};

bool animRange500CheckFull(const uint32_t animationID, CamData* pCamData, varTAE* pVars)
{
	switch (animationID / 500)
	{
	case 68065:
	case 814090:
	case 814290:
	case 830090:
	case 844090:
	case 854090:
	case 884090:
	case 884290:
	case 902090:
	case 962090:
	case 974090:
	case 974290:
	case 980090:
	case 1048090:
	case 1048290:
	case 1436080:
	case 1420080:
	case 1638080:
	case 1662061:
	case 1662065:
	case 1662080:
		pVars->isAim = false;
		return true;
	case 900090:
		pCamData->camAdjustFwd = 0.25f;
		pCamData->camAdjustUp = 2.75f;
		pVars->isDragon = true;
		return true;
	case 908090:
		pCamData->camAdjustFwd = 0.0f;
		pCamData->camAdjustUp = 4.0f;
		pVars->isDragon = true;
		return true;
	case 934090:
		pCamData->camAdjustFwd = 0.7f;
		pCamData->camAdjustUp = 2.5f;
		pVars->isDragon = true;
		return true;
	case 910090:
	case 1046090:
	case 1046290:
		pCamData->camAdjustFwd = -2.0f;
		pCamData->camAdjustUp = 2.0f;
		pVars->isDragon = true;
		return true;
	}

	return false;
}

bool animRangeMoveCheck(const uint32_t animationID, varTAE* pVars)
{
	if ((animationID >= 20000 && animationID < 24000) || (animationID >= 120000 && animationID < 121000) || (animationID >= 320000 && animationID < 321000))
	{
		if (animationID >= 22100 && animationID < 22300)
		{
			pVars->isSyncRot = true;
		}

		pVars->isSmoothAnim = true;
		return true;
	}

	return false;
}

bool animRangeIndivCheck(const uint32_t animationID, CamData* pCamData, IniSettings* pIniSet, varTAE* pVars)
{
	switch (animationID)
	{
	case 0:
	case 300000:
		pVars->isSmoothAnim = true;
		pVars->isSyncRot = true;
	case 5000:
	case 5001:
	case 5002:
	case 19022:
	case 19120:
	case 36210:
	case 36215:
	case 38900:
	case 38910:
	case 38920:
	case 39000:
	case 39010:
	case 39020:
	case 39040:
	case 50070:
	case 50110:
	case 50111:
	case 50500:
	case 50210:
		return true;
	case 50090:
	case 50091:
	case 50092:
		pVars->isBinocs = true;
		return true;
	case 50080:
	case 50100:
	case 50170:
		pVars->isAim = false;
		return true;
	case 50180:
	case 50190:
	case 50191:
	case 50520:
		pVars->isTrackHead = true;
		return true;
	case 90006:
		pVars->isLockLook = true;
		pVars->isObjAct = true;
		return true;
	case 90300:
	case 90301:
	case 90302:
		pCamData->camAdjustFwd = -0.03f;
		pCamData->camAdjustUp = -0.03f;
		pVars->isDragon = true;
		pVars->isTrackHead = true;
		pVars->isLockLook = true;
		return true;
	case 19320:
		pVars->isTrackHead = pIniSet->pIniBool->isTrackDodge;
		return true;
	}

	return false;
}

bool animRange50Check(const uint32_t animationID, varTAE* pVars)
{
	switch (animationID / 50) // ranges of 50
	{
	case 100:
	case 101:
	case 102:
	case 103:
	case 104:
	case 105:
	case 2441:
		return true;
	case 1000:
	case 1001:
	case 1004:
	case 1005:
	case 1006:
	case 1011:
	case 1012:
		pVars->isTrackHead = true;
		return true;
	}

	return false;
}

bool animRange500CheckPartial(const uint32_t animationID, IniSettings* pIniSet, varTAE* pVars)
{
	switch (animationID / 500)
	{
	case 10:
	case 11:
		pVars->isTrackHead = pIniSet->pIniBool->isTrackHit;
	case 20:
	case 22:
	case 26:
	case 56:
	case 57:
	case 58:
		return true;
	case 404:
		pVars->isSyncRot = animationID != 202100 && animationID != 202125 && animationID != 202126;
		return true;
	}

	return false;
}

extern void CheckPlayerTAE()
{
	CamData* pCamData = &camData;
	IniSettings* pIniSet = &iniSet;

	auto TAE_EventCurrent_ptr = std::make_tuple(&TAE_Base_ptr, 0x0, 0x0, 0x0);

	varTAE localVars;

	if (*CSLang == 1)
	{
		pIniSet->pIniBool->isTrackDodge = *TraverseSafePtr(trackDodgesSet_ptr) == 1;
	}

	uint32_t currAnimationID = pCamData->animationID;

	void* pTAEHeader = TraverseUnsafePtr<void>(TAE_Header_ptr);

	uint16_t* pEventCount = TraverseUnsafePtr<uint16_t>(eventCount_ptr);
	uint16_t eventCount;

	if (pTAEHeader == nullptr || pEventCount == nullptr || reinterpret_cast<uint64_t>(pTAEHeader) >= reinterpret_cast<uint64_t>(pEventCount))
	{
		goto AnimIDCheckEnd;
	}

	if (animRange500CheckFull(currAnimationID, pCamData, &localVars))
	{
		goto AnimIDCheckEnd;
	}

	currAnimationID -= currAnimationID / 1000000 * 1000000;

	if (currAnimationID >= 300000 && currAnimationID < 400000)
	{
		localVars.isCrouch = currAnimationID != 390001;
	}

	if (animRangeMoveCheck(currAnimationID, &localVars))
	{
		goto AnimIDCheckEnd;
	}

	if (currAnimationID >= 68000 && currAnimationID < 80000) // interact and throw animations
	{
		localVars.isTrackHead = true;
		goto AnimIDCheckEnd;
	}

	if (animRangeIndivCheck(currAnimationID, pCamData, pIniSet, &localVars))
	{
		goto AnimIDCheckEnd;
	}

	if (animRange50Check(currAnimationID, &localVars))
	{
		goto AnimIDCheckEnd;
	}

	if (animRange500CheckPartial(currAnimationID, pIniSet, &localVars))
	{
		goto AnimIDCheckEnd;
	}

	eventCount = *pEventCount;

	for (int16_t i = 0; i < eventCount; i++)
	{
		std::get<2>(TAE_EventCurrent_ptr) = 0x10 + 0x18 * i;

		const uint32_t* pCurrentEventID = TraverseSafePtr<uint32_t>(TAE_EventCurrent_ptr);
		const uint32_t* pTableArg1Arg1 = TraverseSafeOffsetPtr<uint32_t>(0x10, 1, TAE_EventCurrent_ptr);

		if (reinterpret_cast<uint64_t>(pTAEHeader) >= reinterpret_cast<uint64_t>(pCurrentEventID))
		{
			goto AnimIDCheckEnd;
		}

		const uint32_t currentEventID = *pCurrentEventID;
		const uint32_t tableArg1 = *pTableArg1Arg1;

		switch (currentEventID)
		{
		case 0:
			switch (tableArg1)
			{
			case 94:
				localVars.isOverrideTrackHead = true;
				break;
			}
			break;
		case 66:
		case 67:
			switch (tableArg1)
			{
			case 100240:
			case 100360:
				if (pIniSet->pIniBool->isTrackDodge)
				{
					localVars.isTrackHead = true;
					localVars.isLockLook = true;
				}
				break;
			}
			break;
		}
	}

AnimIDCheckEnd:
	if (!pCamData->pCamBool->isFPS)
	{
		return;
	}

	pCamData->pCamBool->isLockLook = localVars.isLockLook;
	pCamData->pCamBool->isSyncRot = localVars.isSyncRot;
	pCamData->pCamBool->isFPSAim = localVars.isAim;
	pCamData->pCamBool->isSmoothAnim = localVars.isSmoothAnim;
	pCamData->pCamBool->isObjAct = localVars.isObjAct;
	pCamData->pCamBool->isBinocs = localVars.isBinocs;
	pCamData->pCamBool->isDragon = localVars.isDragon;
	pCamData->pCamBool->isCrouch = localVars.isCrouch;

	if (localVars.isTrackHead || localVars.isOverrideTrackHead)
	{
		pCamData->camGoalSyncTime = 0.25f;
		pCamData->pCamBool->isTrackHead = true;
	}
	else if (pCamData->camGoalSyncTime > 0.0f)
	{
		pCamData->camGoalSyncTime = std::clamp(pCamData->camGoalSyncTime - pCamData->dT, 0.0f, 0.25f);
		pCamData->pCamBool->isTrackHead = false;
	}
	else
	{
		pCamData->pCamBool->isTrackHead = false;
	}
}

extern void CheckEnemyTAE(const void** pTAEBase, const int enemyIndex)
{
	auto TAE_Header_ptr = std::make_tuple(pTAEBase, 0x8, 0x18, 0x0);
	auto eventCount_ptr = std::make_tuple(pTAEBase, 0x20);

	void* pTAEHeader = TraverseUnsafePtr<void>(TAE_Header_ptr);

	uint16_t* pEventCount = TraverseUnsafePtr<uint16_t>(eventCount_ptr);
	uint16_t eventCount;

	if (pTAEHeader == nullptr || pEventCount == nullptr || reinterpret_cast<uint64_t>(pTAEHeader) >= reinterpret_cast<uint64_t>(pEventCount))
	{
		reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[enemyIndex] = 0;
		return;
	}

	if (!ReadUnsafePtr(pEventCount, eventCount)) 
	{
		reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[enemyIndex] = 0;
		return;
	}

	auto TAE_EventCurrent_ptr = std::make_tuple(pTAEBase, 0x0, 0x0, 0x0);

	for (int16_t i = 0; i < eventCount; i++)
	{
		std::get<2>(TAE_EventCurrent_ptr) = 0x10 + 0x18 * i;
		uint32_t* pCurrentEventID = TraverseUnsafePtr<uint32_t>(TAE_EventCurrent_ptr);

		if (reinterpret_cast<uint64_t>(pTAEHeader) >= reinterpret_cast<uint64_t>(pCurrentEventID))
		{
			reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[enemyIndex] = 0;
			return;
		}

		uint32_t currentEventID;

		if (!ReadUnsafePtr(pCurrentEventID, currentEventID))
		{
			reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[enemyIndex] = 0;
			return;
		}

		switch (currentEventID)
		{
		case 1:
		case 2:
			reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[enemyIndex] |= 2;
			return;
		}
	}

	reinterpret_cast<uint8_t*>(&enemyHitRBuf.structFlags)[enemyIndex] &= 1;
}