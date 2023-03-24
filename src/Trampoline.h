#pragma once

#include "stdafx.h"
#include "Console.h"

unsigned const char c_NopBytes[10][9] = 
{
{0x90}, 
{0x90}, 
{0x66, 0x90}, 
{0x0F, 0x1F, 0x00},
{0x0F, 0x1F, 0x40, 0x00}, 
{0x0F, 0x1F, 0x44, 0x00, 0x00},
{0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00}, 
{0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00},
{0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
};

inline void Trampoline(void* injPoint, const void* jmpTgt, int32_t nopSize, const bool isCall = false)
{
	uint8_t* pInj = reinterpret_cast<uint8_t*>(injPoint);
	const uint8_t* pJmp = reinterpret_cast<const uint8_t*>(jmpTgt);

	uint8_t InjBuf[5] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
	InjBuf[0] -= isCall;

	if (std::abs(pJmp - pInj) > MAXINT32)
	{
		AllocConsoleOnce();
		std::cout << "TrampolineJmp: Target " << reinterpret_cast<const void*>(pJmp) << " out of bounds of " << reinterpret_cast<void*>(pInj) << "\n";

		return;
	};

	int Disp = static_cast<int32_t>(pJmp - pInj - 5);

	memcpy(InjBuf + 1, &Disp, 4);
	memcpy(pInj, InjBuf, 5);

	if (nopSize > 0)
	{
		uint8_t* NopPoint = pInj + 5;
		while (nopSize / 9 > 0)
		{
			memcpy(NopPoint, c_NopBytes[9], 9);
			NopPoint += 9;
			nopSize -= 9;
		}

		memcpy(NopPoint, c_NopBytes[nopSize], nopSize);
	};
}