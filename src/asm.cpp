#include "asm.h"

#define PUSH a.push(x86::r15);\
			 a.mov(x86::r15, x86::rsp);\
			 a.and_(x86::rsp, -0x10);\
			 a.sub(x86::rsp, 0xC0);\
			 a.movaps(x86::ptr(x86::rsp, 0xB0), x86::xmm0);\
			 a.movaps(x86::ptr(x86::rsp, 0xA0), x86::xmm1);\
			 a.movaps(x86::ptr(x86::rsp, 0x90), x86::xmm2);\
			 a.movaps(x86::ptr(x86::rsp, 0x80), x86::xmm3);\
			 a.movaps(x86::ptr(x86::rsp, 0x70), x86::xmm4);\
			 a.movaps(x86::ptr(x86::rsp, 0x60), x86::xmm5);\
			 a.mov(x86::ptr(x86::rsp, 0x58), x86::rax);\
			 a.mov(x86::ptr(x86::rsp, 0x50), x86::rcx);\
			 a.mov(x86::ptr(x86::rsp, 0x48), x86::rdx);\
			 a.mov(x86::ptr(x86::rsp, 0x40), x86::r8);\
			 a.mov(x86::ptr(x86::rsp, 0x38), x86::r9);\
			 a.mov(x86::ptr(x86::rsp, 0x30), x86::r10);\
			 a.mov(x86::ptr(x86::rsp, 0x28), x86::r11);\
			 a.mov(x86::ptr(x86::rsp, 0x20), x86::r15);\

#define POP  a.mov(x86::r15, x86::ptr(x86::rsp, 0x20));\
			 a.mov(x86::r11, x86::ptr(x86::rsp, 0x28));\
			 a.mov(x86::r10, x86::ptr(x86::rsp, 0x30));\
			 a.mov(x86::r9, x86::ptr(x86::rsp, 0x38));\
			 a.mov(x86::r8, x86::ptr(x86::rsp, 0x40));\
			 a.mov(x86::rdx, x86::ptr(x86::rsp, 0x48));\
			 a.mov(x86::rcx, x86::ptr(x86::rsp, 0x50));\
			 a.mov(x86::rax, x86::ptr(x86::rsp, 0x58));\
			 a.movaps(x86::xmm5, x86::ptr(x86::rsp, 0x60));\
			 a.movaps(x86::xmm4, x86::ptr(x86::rsp, 0x70));\
			 a.movaps(x86::xmm3, x86::ptr(x86::rsp, 0x80));\
			 a.movaps(x86::xmm2, x86::ptr(x86::rsp, 0x90));\
			 a.movaps(x86::xmm1, x86::ptr(x86::rsp, 0xA0));\
			 a.movaps(x86::xmm0, x86::ptr(x86::rsp, 0xB0));\
			 a.mov(x86::rsp, x86::r15);\
			 a.pop(x86::r15)

#define PUSHR a.push(x86::rax)

#define POPR  a.pop(x86::rax)

static CamData* pCamData = &camData;
static IniSettings* pIniSet = &iniSet;

using namespace asmjit;

void InitAsm(CodeHolder& pCode, void* InjPoint, uint8_t*& codeMemFree, const int32_t NopSize = 0, const bool isCall = false)
{
	uint8_t* bcodeMem = codeMemFree;
	codeMemFree = reinterpret_cast<uint8_t*>(reinterpret_cast<uint64_t>(codeMemFree + 0x0F) & ~0x0F);
	const size_t dcodeMem = static_cast<size_t>(codeMemFree - bcodeMem);
	std::memset(bcodeMem, 0xCC, dcodeMem);
	Trampoline(InjPoint, codeMemFree, NopSize, isCall);
	pCode.relocateToBase(reinterpret_cast<uint64_t>(codeMemFree));
	const size_t codeSize = pCode.codeSize();
	pCode.copySectionData(codeMemFree, codeSize, 0, CopySectionFlags::kPadTargetBuffer | CopySectionFlags::kPadSectionBuffer);
	codeMemFree += codeSize;
	pCode.reset();
}

extern void InjectAsm()
{
	const size_t alloc_size = 0x2000;
	auto pVirtualAlloc2 = reinterpret_cast<decltype(&::VirtualAlloc2)>(GetProcAddress(LoadLibrary(L"kernelbase.dll"), "VirtualAlloc2"));
	SYSTEM_INFO systemInfo = {};
	GetSystemInfo(&systemInfo);
	if (pVirtualAlloc2 != nullptr && pIniSet->pIniBool->useVAlloc2)
	{
		MEM_ADDRESS_REQUIREMENTS memAddressRequirements = {};
		const uint64_t align = ~(static_cast<uint64_t>(systemInfo.dwAllocationGranularity) - 1);
		memAddressRequirements.LowestStartingAddress = reinterpret_cast<void*>((reinterpret_cast<uint64_t>(mInfo.lpBaseOfDll) + mInfo.SizeOfImage) & align);
		memAddressRequirements.HighestEndingAddress = reinterpret_cast<void*>(((reinterpret_cast<uint64_t>(mInfo.lpBaseOfDll) - alloc_size + INT_MAX) & align) - 1);
		memAddressRequirements.Alignment = 0;
		MEM_EXTENDED_PARAMETER extendedParameters = {};
		extendedParameters.Type = MemExtendedParameterAddressRequirements;
		extendedParameters.Pointer = &memAddressRequirements;
		codeMem = pVirtualAlloc2(GetCurrentProcess(), nullptr, alloc_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, &extendedParameters, 1);
	}
	else
	{
		codeMem = reinterpret_cast<void*>((reinterpret_cast<uint64_t>(mInfo.lpBaseOfDll) + mInfo.SizeOfImage));
		MCF::VirtualAllocNear(MCF::MemRegion("eldenring.exe"), alloc_size, PAGE_EXECUTE_READWRITE, codeMem, systemInfo.dwAllocationGranularity);
	}
	static uint8_t* codeMemFree = reinterpret_cast<uint8_t*>(codeMem);

	if (pIniSet->pIniBool->isDbgOut)
	{
		AllocConsoleOnce();
		std::cout << "ERFPS > Trampoline allocation base: " << codeMem << "\n";
	}

	JitRuntime rt;
	CodeHolder code;
	Environment env = rt.environment();

	*printProcessEntriesCode = 0xC3; // ret

	code.init(env);
	x86::Assembler a(&code);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(lockStickCode) + 6);
	a.embed(reinterpret_cast<uint8_t*>(lockStickCode), 6);
	a.jmp(reinterpret_cast<uint8_t*>(lockStickCode) + 6);
	a.int3();
	InitAsm(code, lockStickCode, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(lockSwitchCode) + 6);
	a.embed(reinterpret_cast<uint8_t*>(lockSwitchCode), 6);
	a.jmp(reinterpret_cast<uint8_t*>(lockSwitchCode) + 6);
	a.int3();
	InitAsm(code, lockSwitchCode, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(lockDropCode) + 7);
	a.embed(reinterpret_cast<uint8_t*>(lockDropCode), 7);
	a.jmp(reinterpret_cast<uint8_t*>(lockDropCode) + 7);
	a.int3();
	InitAsm(code, lockDropCode, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(lockAimDropCode), 5);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.jne(reinterpret_cast<uint8_t*>(lockAimDropCode) + 5);
	a.xorps(x86::xmm0, x86::xmm0);
	a.jmp(reinterpret_cast<uint8_t*>(lockAimDropCode) + 5);
	a.int3();
	InitAsm(code, lockAimDropCode, codeMemFree);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(dtorStartCode), 11);
	PUSH;
	a.mov(x86::rax, reinterpret_cast<uint64_t>(&pDeinitializeCamera));
	a.call(x86::qword_ptr(x86::rax));
	POP;
	a.jmp(reinterpret_cast<uint8_t*>(dtorStartCode) + 11);
	a.int3();
	InitAsm(code, dtorStartCode, codeMemFree, 6);

	code.init(env);
	code.attach(&a);
	Label PTAE_retjmp = a.newLabel();
	a.embed(reinterpret_cast<uint8_t*>(playerTAECode), 6);
	PUSH;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(PTAE_retjmp);
	a.mov(x86::rax, WorldChrMan);
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.mov(x86::rax, x86::ptr(x86::rax, PlayerInsOffset));
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.mov(x86::rax, x86::ptr(x86::rax, 0x190));
	a.mov(x86::rax, x86::ptr(x86::rax, 0x18));
	a.cmp(x86::rax, x86::rbx);
	a.jne(PTAE_retjmp);
	a.mov(x86::ecx, x86::dword_ptr(x86::rsi));
	a.mov(x86::rax, &pCamData->animationID);
	a.mov(x86::dword_ptr(x86::rax), x86::ecx);
	a.mov(x86::rax, &TAE_Base_ptr);
	a.mov(x86::qword_ptr(x86::rax), x86::rdi);
	a.mov(x86::rax, reinterpret_cast<uint64_t>(&pCheckPlayerTAE));
	a.call(x86::qword_ptr(x86::rax));
	a.bind(PTAE_retjmp);
	POP;
	a.jmp(reinterpret_cast<uint8_t*>(playerTAECode) + 6);
	a.int3();
	InitAsm(code, playerTAECode, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	Label ETAE_loop = a.newLabel();
	Label ETAE_retjmp = a.newLabel();
	const uint8_t TAE_inst0_buf[] = { 0x49, 0x3B, 0x7C, 0xD1, 0x08 };
	a.embed(reinterpret_cast<uint8_t*>(enemyTAECode), 6);
	PUSH;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(ETAE_retjmp);
	a.test(x86::qword_ptr(x86::r14, 0x8), -1);
	a.je(ETAE_retjmp);
	a.test(x86::dword_ptr(x86::r14, 0x20), -1);
	a.je(ETAE_retjmp);
	a.xor_(x86::ecx, x86::ecx);
	a.xor_(x86::edx, x86::edx);
	a.mov(x86::cl, 8);
	a.mov(x86::r9, reinterpret_cast<uint64_t>(&enemyHitRBuf));
	a.lea(x86::rax, x86::ptr(x86::r9, 0x88));
	a.bind(ETAE_loop);
	a.test(x86::byte_ptr(x86::rax), 1);
	a.lea(x86::rax, x86::ptr(x86::rax, 1));
	a.loope(ETAE_loop);
	a.je(ETAE_retjmp);
	a.mov(x86::dl, 7);
	a.sub(x86::dl, x86::cl);
	a.shl(x86::dl, 1);
	a.embedDataArray(TypeId::kUInt8, TAE_inst0_buf, sizeof(TAE_inst0_buf)); // cmp rdi,[r9+rdx*8+8]
	a.lea(x86::rcx, x86::ptr(x86::rcx, 1));
	a.loopne(ETAE_loop);
	a.jne(ETAE_retjmp);
	a.mov(x86::rax, reinterpret_cast<uint64_t>(&pCheckEnemyTAE));
	a.shr(x86::dl, 1);
	a.mov(x86::rcx, x86::r14);
	a.call(x86::qword_ptr(x86::rax));
	a.bind(ETAE_retjmp);
	POP;
	a.jmp(reinterpret_cast<uint8_t*>(enemyTAECode) + 6);
	a.int3();
	InitAsm(code, enemyTAECode, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(fallDeathCode) + 6);
	a.embed(reinterpret_cast<uint8_t*>(fallDeathCode), 6);
	a.jmp(reinterpret_cast<uint8_t*>(fallDeathCode) + 6);
	a.int3();
	InitAsm(code, fallDeathCode, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	Label HSC_spectate = a.newLabel();
	a.embed(reinterpret_cast<uint8_t*>(headSizeCode), 5);
	a.test(x86::rax, x86::rax);
	a.jne(reinterpret_cast<uint8_t*>(headSizeCode) + 5);
	a.mov(x86::rax, &pCamData->pCamBool->isCamInit);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(reinterpret_cast<uint8_t*>(headSizeCode) + 5);
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(reinterpret_cast<uint8_t*>(headSizeCode) + 5);
	a.mov(x86::rax, &PlayerInsSpectate);
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.test(x86::rax, x86::rax);
	a.jne(HSC_spectate);
	a.mov(x86::rax, WorldChrMan);
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.mov(x86::rax, x86::ptr(x86::rax, PlayerInsOffset));
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.bind(HSC_spectate);
	a.lea(x86::rax, x86::ptr(x86::rax, ChrAsmOffset + 0x18));
	a.cmp(x86::rax, x86::rdx);
	a.jne(reinterpret_cast<uint8_t*>(headSizeCode) + 5);
	a.mov(x86::eax, 0xC1200000); // -10.0f
	a.movd(x86::xmm1, x86::eax);
	a.jmp(reinterpret_cast<uint8_t*>(headSizeCode) + 5);
	a.int3();
	InitAsm(code, headSizeCode, codeMemFree);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(upperHandCode), 5);
	a.mov(x86::r8, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::r8), 1);
	a.jne(reinterpret_cast<uint8_t*>(upperHandCode) + 5);
	a.mov(x86::r8, WorldChrMan);
	a.mov(x86::r8, x86::ptr(x86::r8));
	a.mov(x86::r8, x86::ptr(x86::r8, PlayerInsOffset));
	a.mov(x86::r8, x86::ptr(x86::r8));
	a.mov(x86::r8, x86::ptr(x86::r8, ChrAsmOffset));
	a.cmp(x86::r8, x86::ptr(x86::rsp, 0xA0));
	a.jne(reinterpret_cast<uint8_t*>(upperHandCode) + 5);
	a.xor_(x86::eax, x86::eax);
	a.mov(x86::edx, 0x64);
	a.jmp(reinterpret_cast<uint8_t*>(upperHandCode) + 5);
	a.int3();
	InitAsm(code, upperHandCode, codeMemFree, 0);

	code.init(env);
	code.attach(&a);
	Label kMul = a.newLabel();
	a.embed(reinterpret_cast<uint8_t*>(padSensCode), 8);
	a.mov(x86::rax, CSLang);
	a.mov(x86::eax, x86::dword_ptr(x86::rax));
	a.cmp(x86::eax, 1);
	a.jne(reinterpret_cast<uint8_t*>(padSensCode) + 8);
	a.mov(x86::rax, CSPcKeyConfig);
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.movzx(x86::eax, x86::byte_ptr(x86::rax, 0x830));
	a.cvtsi2ss(x86::xmm1, x86::eax);
	a.mulss(x86::xmm1, x86::dword_ptr(kMul));
	a.jmp(reinterpret_cast<uint8_t*>(padSensCode) + 8);
	a.int3();
	a.align(AlignMode::kZero, 0x4);
	a.bind(kMul);
	a.embedFloat(0.1);
	InitAsm(code, padSensCode, codeMemFree, 3);

	code.init(env);
	code.attach(&a);
	uint64_t MagicData[] = { 0x33273D6874003E54, 0x6E656D65766F6D20, 0x3D68746469003E54, 0x3E544E4F462F3C2E, 0x776569003E54414D, 0x7765697620666F20 }; //Don't ask me. It works.
	Label MagicCode = a.newLabel();
	Label MagicCodeSpecial = a.newLabel();
	Label MagicInt64 = a.newLabel();
	a.embed(reinterpret_cast<uint8_t*>(menuDisplayCode), 6);
	a.mov(x86::rax, CSLang);
	a.mov(x86::eax, x86::dword_ptr(x86::rax));
	a.cmp(x86::eax, 1);
	a.jne(reinterpret_cast<uint8_t*>(menuDisplayCode) + 6);
	a.mov(x86::rax, MagicPtr);
	a.mov(x86::rax, x86::qword_ptr(x86::rax, 0x48));
	a.cmp(x86::qword_ptr(MagicInt64), x86::rax);
	a.je(MagicCode);
	a.cmp(x86::qword_ptr(MagicInt64, 0x8), x86::rax);
	a.je(MagicCode);
	a.cmp(x86::qword_ptr(MagicInt64, 0x10), x86::rax);
	a.je(MagicCode);
	a.cmp(x86::qword_ptr(MagicInt64, 0x18), x86::rax);
	a.je(MagicCodeSpecial);
	a.jmp(reinterpret_cast<uint8_t*>(menuDisplayCode) + 6);
	a.bind(MagicCodeSpecial);
	a.mov(x86::rax, MagicPtr);
	a.mov(x86::rax, x86::qword_ptr(x86::rax, 0x40));
	a.cmp(x86::qword_ptr(MagicInt64, 0x20), x86::rax);
	a.je(MagicCode);
	a.cmp(x86::qword_ptr(MagicInt64, 0x28), x86::rax);
	a.je(MagicCode);
	a.jmp(reinterpret_cast<uint8_t*>(menuDisplayCode) + 6);
	a.bind(MagicCode);
	a.mov(x86::rax, reinterpret_cast<uint64_t>(&pIniSet->fpFOV));
	a.movzx(x86::eax, x86::byte_ptr(x86::rax));
	a.shl(x86::edx, 1);
	a.add(x86::edx, x86::eax);
	a.sub(x86::edx, 0xA);
	a.jmp(reinterpret_cast<uint8_t*>(menuDisplayCode) + 6);
	a.int3();
	a.align(AlignMode::kZero, 0x8);
	a.bind(MagicInt64);
	a.embedDataArray(TypeId::kUInt64, MagicData, 6);
	InitAsm(code, menuDisplayCode, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	Label StringPatchState = a.newLabel();
	Label SPS_retjmp = a.newLabel();
	PUSH;
	a.cmp(x86::byte_ptr(StringPatchState), 0);
	a.jne(SPS_retjmp);
	a.inc(x86::byte_ptr(StringPatchState));
	a.mov(x86::rax, reinterpret_cast<uint64_t>(&pPatchMsgRepo));
	a.call(x86::qword_ptr(x86::rax));
	a.bind(SPS_retjmp);
	POP;
	a.embed(reinterpret_cast<uint8_t*>(startScreenCode), 7);
	a.jmp(reinterpret_cast<uint8_t*>(startScreenCode) + 7);
	a.int3();
	a.bind(StringPatchState);
	a.embedInt8(0x0);
	InitAsm(code, startScreenCode, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	Label HvkStr = a.newLabel();
	a.mov(x86::rcx, x86::rdi);
	a.movzx(x86::edx, x86::ax);
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(reinterpret_cast<uint8_t*>(hkRunCode) + 6);
	a.mov(x86::rax, &pCamData->pCamBool->isDisableRunFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(reinterpret_cast<uint8_t*>(hkRunCode) + 6);
	a.cmp(x86::dl, 2);
	a.jne(reinterpret_cast<uint8_t*>(hkRunCode) + 6);
	a.mov(x86::rax, x86::ptr(x86::rdi, 0x20));
	a.mov(x86::rax, x86::ptr(x86::rax, 0x4));
	a.cmp(x86::qword_ptr(HvkStr), x86::rax);
	a.jne(reinterpret_cast<uint8_t*>(hkRunCode) + 6);
	a.xor_(x86::edx, x86::edx);
	a.inc(x86::edx);
	a.jmp(reinterpret_cast<uint8_t*>(hkRunCode) + 6);
	a.int3();
	a.align(AlignMode::kZero, 0x8);
	a.bind(HvkStr);
	a.embedUInt64(0x646E496465657053);
	InitAsm(code, hkRunCode, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	a.mov(x86::rax, &pCamData->pCamBool->isFPSAim);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.je(reinterpret_cast<uint8_t*>(aimTgtCode) + 11);
	a.embed(reinterpret_cast<uint8_t*>(aimTgtCode), 11);
	a.jmp(reinterpret_cast<uint8_t*>(aimTgtCode) + 11);
	a.int3();
	InitAsm(code, aimTgtCode, codeMemFree, 6);

	code.init(env);
	code.attach(&a);
	Label AFC_retjmp = a.newLabel();
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(AFC_retjmp);
	a.mov(x86::rax, WorldChrMan);
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.mov(x86::rax, x86::ptr(x86::rax, PlayerInsOffset));
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.cmp(x86::rax, x86::rdi);
	a.jne(AFC_retjmp);
	a.mov(x86::rax, &pCamData->pCamBool->isBinocs);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.je(AFC_retjmp);
	a.mov(x86::eax, 0x3F800000); // 1.0f
	a.movd(x86::xmm0, x86::eax);
	a.bind(AFC_retjmp);
	POPR;
	a.embed(reinterpret_cast<uint8_t*>(aimFadeCode), 8);
	a.jmp(reinterpret_cast<uint8_t*>(aimFadeCode) + 8);
	a.int3();
	InitAsm(code, aimFadeCode, codeMemFree, 3);

	if (pIniSet->pIniBool->isDisableLogos && menuLogoCode != nullptr)
	{
		*reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(menuLogoCode) + 9) = 0x9066; // nop 2
	}

	*reinterpret_cast<uint32_t*>(chrDitherCode) = 0x90FFCA83; // or edx,-1 nop

	*reinterpret_cast<uint64_t*>(AEGDitherCode) = 0x0000441F0FFFC883; // or eax,-1 nop dword ptr [rax+rax+00]

	*reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(playerDitherCode) + 6) = 0x0;
	*reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(playerDitherCode) + 13) = 0x0;

	code.init(env);
	code.attach(&a);
	Label PLC_bytes = a.newLabel();
	uint8_t PLC_buf[] = { 0x0, 0x0, 0xC0, 0x3F, 0xCD, 0xCC, 0x4C, 0x3E, 0xCD, 0xCC, 0x4C, 0x3E, 0x01, 0x0, 0x0, 0x0 };
	a.embed(reinterpret_cast<uint8_t*>(playerLightCode), 5);
	a.mov(x86::rcx, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rcx), 1);
	a.jne(reinterpret_cast<uint8_t*>(playerLightCode) + 5);
	a.mov(x86::rcx, x86::ptr(x86::r14, 0x10));
	a.test(x86::rcx, x86::rcx);
	a.je(reinterpret_cast<uint8_t*>(playerLightCode) + 5);
	a.movups(x86::xmm0, x86::ptr(x86::rcx, 0x10));
	a.movaps(x86::xmm1, x86::ptr(PLC_bytes));
	a.ptest(x86::xmm0, x86::xmm1);
	a.xorps(x86::xmm6, x86::xmm6);
	a.jc(reinterpret_cast<uint8_t*>(playerLightCode) + 8);
	a.jmp(reinterpret_cast<uint8_t*>(playerLightCode) + 5);
	a.int3();
	a.align(AlignMode::kZero, 0x10);
	a.bind(PLC_bytes);
	a.embedDataArray(TypeId::kUInt8, PLC_buf, sizeof(PLC_buf));
	InitAsm(code, playerLightCode, codeMemFree);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(playerShadowCode), 5);
	a.mov(x86::rcx, WorldChrMan);
	a.mov(x86::rcx, x86::ptr(x86::rcx));
	a.mov(x86::rcx, x86::ptr(x86::rcx, PlayerInsOffset));
	a.test(x86::rcx, x86::rcx);
	a.je(reinterpret_cast<uint8_t*>(playerShadowCode) + 5);
	a.mov(x86::rcx, x86::ptr(x86::rcx));
	a.mov(x86::rcx, x86::ptr(x86::rcx, ChrAsmOffset + 0x10));
	a.cmp(x86::rcx, x86::rdi);
	a.jne(reinterpret_cast<uint8_t*>(playerShadowCode) + 5);
	a.and_(x86::byte_ptr(x86::rbx), 0b11111101);
	a.jmp(reinterpret_cast<uint8_t*>(playerShadowCode) + 5);
	a.int3();
	InitAsm(code, playerShadowCode, codeMemFree);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(VFXCode00), 6);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.jne(reinterpret_cast<uint8_t*>(VFXCode00) + 6);
	a.or_(x86::eax, -1);
	a.cvtsi2ss(x86::xmm7, x86::eax);
	a.jmp(reinterpret_cast<uint8_t*>(VFXCode00) + 6);
	a.int3();
	InitAsm(code, VFXCode00, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(VFXCode01), 7);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.jne(reinterpret_cast<uint8_t*>(VFXCode01) + 7);
	a.or_(x86::eax, -1);
	a.cvtsi2ss(x86::xmm11, x86::eax);
	a.jmp(reinterpret_cast<uint8_t*>(VFXCode01) + 7);
	a.int3();
	InitAsm(code, VFXCode01, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->LockTgtOverride);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(lockTgtCode) + 8);
	a.embed(reinterpret_cast<uint8_t*>(lockTgtCode), 8);
	a.jmp(reinterpret_cast<uint8_t*>(lockTgtCode) + 8);
	a.int3();
	InitAsm(code, lockTgtCode, codeMemFree, 3);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(chunkCode), 7);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isCrossChunk);
	a.mov(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.jmp(reinterpret_cast<uint8_t*>(chunkCode) + 7);
	a.int3();
	InitAsm(code, chunkCode, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	Label RMC_skip = a.newLabel();
	a.embed(reinterpret_cast<uint8_t*>(rootTransformCode), 3);
	a.mov(x86::rcx, WorldChrMan);
	a.mov(x86::rcx, x86::ptr(x86::rcx));
	a.mov(x86::rcx, x86::ptr(x86::rcx, PlayerInsOffset));
	a.mov(x86::rcx, x86::ptr(x86::rcx));
	a.mov(x86::rcx, x86::ptr(x86::rcx, 0x58));
	a.cmp(x86::rcx, x86::rbx);
	a.jne(RMC_skip);
	a.mov(x86::rcx, &pCamData->rootTransform);
	a.movaps(x86::ptr(x86::rcx), x86::xmm0);
	a.mov(x86::dword_ptr(x86::rcx, 0xC), 0);
	a.mov(x86::rcx, &pCamData->pCamBool->RootTOverride);
	a.cmp(x86::byte_ptr(x86::rcx), 1);
	a.je(reinterpret_cast<uint8_t*>(rootTransformCode) + 6);
	a.bind(RMC_skip);
	a.embed(reinterpret_cast<uint8_t*>(rootTransformCode) + 3, 3);
	a.jmp(reinterpret_cast<uint8_t*>(rootTransformCode) + 6);
	a.int3();
	InitAsm(code, rootTransformCode, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	Label HDC_rbuf_count = a.newLabel();
	Label HDC_rbuf_check = a.newLabel();
	Label HDC_rbuf_nfull = a.newLabel();
	const uint8_t HDC_inst0_buf[] = { 0x4A, 0x83, 0x4C, 0x01, 0x10, 0xFF };
	const uint8_t HDC_inst1_buf[] = { 0x4A, 0x83, 0xA4, 0x01, 0x88, 0x00, 0x00, 0x00, 0x00 };
	a.embed(reinterpret_cast<uint8_t*>(hitDetectCodeCpy), 7);
	a.mov(x86::rcx, &BulletIns);
	a.cmp(x86::ptr(x86::rcx), x86::r12);
	a.jne(reinterpret_cast<uint8_t*>(hitDetectCodeCpy) + 7);
	a.cmp(x86::ebx, 1);
	a.jne(reinterpret_cast<uint8_t*>(hitDetectCodeEnd));
	a.movd(x86::xmm0, x86::edx);
	a.shufps(x86::xmm0, x86::xmm0, 0);
	a.movaps(x86::xmm1, x86::xmmword_ptr(HDC_rbuf_check));
	a.pcmpeqd(x86::xmm1, x86::xmm0);
	a.movaps(x86::xmm2, x86::xmmword_ptr(HDC_rbuf_check, 0x10));
	a.pcmpeqd(x86::xmm2, x86::xmm0);
	a.por(x86::xmm1, x86::xmm2);
	a.ptest(x86::xmm1, x86::xmm1);
	a.jne(reinterpret_cast<uint8_t*>(hitDetectCodeEnd));
	a.movzx(x86::ecx, x86::byte_ptr(HDC_rbuf_count));
	a.mov(x86::r8, reinterpret_cast<uint64_t>(&enemyHitRBuf));
	a.and_(x86::cl, 7);
	a.jne(HDC_rbuf_nfull);
	a.movaps(x86::xmmword_ptr(HDC_rbuf_check), x86::xmm1);
	a.movaps(x86::xmmword_ptr(HDC_rbuf_check, 0x10), x86::xmm1);
	a.or_(x86::qword_ptr(x86::r8, 0x10), -1);
	a.xor_(x86::cl, x86::cl);
	a.bind(HDC_rbuf_nfull);
	a.shl(x86::cl, 2);
	a.lea(x86::r9, x86::qword_ptr(HDC_rbuf_check));
	a.mov(x86::dword_ptr(x86::rcx, x86::r9), x86::edx);
	a.shl(x86::cl, 2);
	a.mov(x86::ptr(x86::rcx, x86::r8), x86::rdx);
	a.embedDataArray(TypeId::kUInt8, HDC_inst0_buf, sizeof(HDC_inst0_buf)); // or qword ptr [rcx+r8+10],-1
	a.shr(x86::cl, 4);
	a.embedDataArray(TypeId::kUInt8, HDC_inst1_buf, sizeof(HDC_inst1_buf)); // and qword ptr [rcx+r8+88],0
	a.inc(x86::cl);
	a.mov(x86::byte_ptr(HDC_rbuf_count), x86::cl);
	a.jmp(reinterpret_cast<uint8_t*>(hitDetectCodeEnd));
	a.int3();
	a.bind(HDC_rbuf_count);
	a.embedUInt8(0x0);
	a.align(AlignMode::kZero, 0x10);
	a.bind(HDC_rbuf_check);
	a.embedUInt32(0x0, 8);
	InitAsm(code, hitDetectCodeCpy, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	a.mov(x86::rax, &BulletIns);
	a.cmp(x86::ptr(x86::rax), x86::r15);
	a.je(reinterpret_cast<uint8_t*>(hitRegisterCode) + 5);
	a.call(hitRegisterCall);
	a.jmp(reinterpret_cast<uint8_t*>(hitRegisterCode) + 5);
	a.int3();
	InitAsm(code, hitRegisterCode, codeMemFree);

	code.init(env);
	code.attach(&a);
	Label CBIC_retjmp = a.newLabel();
	Label CBIC_loop = a.newLabel();
	a.embed(reinterpret_cast<uint8_t*>(createBulletInsCode), 5);
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(reinterpret_cast<uint8_t*>(createBulletInsCode) + 5);
	a.cmp(x86::byte_ptr(x86::rcx, 0x14), 1);
	a.jne(reinterpret_cast<uint8_t*>(createBulletInsCode) + 5);
	a.mov(x86::rax, SessionMan);
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.test(x86::rax, x86::rax);
	a.je(reinterpret_cast<uint8_t*>(createBulletInsCode) + 5);
	a.mov(x86::eax, x86::dword_ptr(x86::rax, 0x0C));
	a.test(x86::eax, x86::eax);
	a.jne(reinterpret_cast<uint8_t*>(createBulletInsCode) + 5);
	a.push(x86::rcx);
	a.push(x86::rdx);
	a.sub(x86::rsp, 0x30);
	a.mov(x86::rax, WorldChrMan);
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.mov(x86::rax, x86::ptr(x86::rax, PlayerInsOffset));
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.cmp(x86::ptr(x86::rcx), x86::rax);
	a.jne(CBIC_retjmp);
	a.mov(x86::eax, x86::dword_ptr(x86::rcx, 0x0C));
	a.test(x86::eax, x86::eax);
	a.js(CBIC_retjmp);
	a.shl(x86::eax, 0x10);
	a.mov(x86::ax, x86::word_ptr(x86::rcx, 0x0C));
	a.movd(x86::xmm0, x86::eax);
	a.shufps(x86::xmm0, x86::xmm0, 0);
	a.mov(x86::rbx, fixMagicID);
	a.mov(x86::eax, sizeof(fixMagicID));
	a.bind(CBIC_loop);
	a.sub(x86::eax, sizeof(__m128));
	a.js(CBIC_retjmp);
	a.movaps(x86::xmm1, x86::xmm0);
	a.pcmpeqw(x86::xmm1, x86::xmmword_ptr(x86::rax, x86::rbx));
	a.ptest(x86::xmm1, x86::xmm1);
	a.je(CBIC_loop);
	a.mov(x86::ebx, x86::dword_ptr(x86::rcx, 0x18));
	a.mov(x86::edx, 0x989681); // 10000001
	a.mov(x86::dword_ptr(x86::rcx, 0x18), x86::edx);
	a.lea(x86::rcx, x86::ptr(x86::rsp, 0x20));
	a.call(getBulletParamEntryCode);
	a.mov(x86::rax, x86::ptr(x86::rsp, 0x20));
	a.mov(x86::dword_ptr(x86::rax, 0x68), x86::ebx);
	a.mov(x86::edx, x86::ebx);
	a.lea(x86::rcx, x86::ptr(x86::rsp, 0x20));
	a.mov(x86::rbx, x86::rax);
	a.call(getBulletParamEntryCode);
	a.mov(x86::rax, x86::ptr(x86::rsp, 0x20));
	a.test(x86::rax, x86::rax);
	a.je(CBIC_retjmp);
	a.mov(x86::eax, x86::dword_ptr(x86::rax, 0x14));
	a.mov(x86::dword_ptr(x86::rbx, 0x14), x86::eax);
	a.bind(CBIC_retjmp);
	a.add(x86::rsp, 0x30);
	a.pop(x86::rdx);
	a.pop(x86::rcx);
	a.jmp(reinterpret_cast<uint8_t*>(createBulletInsCode) + 5);
	a.int3();
	InitAsm(code, createBulletInsCode, codeMemFree);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(regBulletCode), 7);
	a.mov(x86::rax, &pCamData->pCamBool->isGetChrAttack);
	a.cmp(x86::byte_ptr(x86::rax), 0);
	a.jbe(reinterpret_cast<uint8_t*>(regBulletCode) + 7);
	a.dec(x86::byte_ptr(x86::rax));
	a.mov(x86::rax, &BulletIns);
	a.mov(x86::ptr(x86::rax), x86::rdx);
	a.jmp(reinterpret_cast<uint8_t*>(regBulletCode) + 7);
	a.int3();
	InitAsm(code, regBulletCode, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(dstBulletCode), 5);
	a.mov(x86::rcx, &BulletIns);
	a.cmp(x86::ptr(x86::rcx), x86::rbx);
	a.jne(reinterpret_cast<uint8_t*>(dstBulletCode) + 5);
	a.mov(x86::rdx, &pCamData->pCamBool->isGetChrAttack);
	a.dec(x86::byte_ptr(x86::rdx));
	a.jns(reinterpret_cast<uint8_t*>(dstBulletCode) + 5);
	a.mov(x86::byte_ptr(x86::rdx), 0);
	a.or_(x86::qword_ptr(x86::rcx), -1);
	a.jmp(reinterpret_cast<uint8_t*>(dstBulletCode) + 5);
	a.int3();
	InitAsm(code, dstBulletCode, codeMemFree);

	code.init(env);
	code.attach(&a);
	Label NewFrame = a.newLabel();
	Label FrameCount = a.newLabel();
	Label MimicState = a.newLabel();
	Label SIC_spectate = a.newLabel();
	a.embed(reinterpret_cast<uint8_t*>(stateInfoCode), 7);
	a.mov(x86::r8, &PlayerInsSpectate);
	a.mov(x86::r8, x86::ptr(x86::r8));
	a.test(x86::r8, x86::r8);
	a.jne(SIC_spectate);
	a.mov(x86::r8, WorldChrMan);
	a.mov(x86::r8, x86::ptr(x86::r8));
	a.mov(x86::r8, x86::ptr(x86::r8, PlayerInsOffset));
	a.test(x86::r8, x86::r8);
	a.je(reinterpret_cast<uint8_t*>(stateInfoCode) + 7);
	a.mov(x86::r8, x86::ptr(x86::r8));
	a.bind(SIC_spectate);
	a.cmp(x86::r8, x86::rsi);
	a.jne(reinterpret_cast<uint8_t*>(stateInfoCode) + 7);
	a.mov(x86::r8, CSFlipper);
	a.mov(x86::r8, x86::ptr(x86::r8));
	a.mov(x86::r8d, x86::dword_ptr(x86::r8, 0x268));
	a.cmp(x86::ptr(FrameCount), x86::r8d);
	a.mov(x86::ptr(FrameCount), x86::r8d);
	a.jne(NewFrame);
	a.cmp(x86::edx, 0x5F); // Chameleon
	a.jne(reinterpret_cast<uint8_t*>(stateInfoCode) + 7);
	a.sete(x86::byte_ptr(MimicState));
	a.jmp(reinterpret_cast<uint8_t*>(stateInfoCode) + 7);
	a.bind(NewFrame);
	a.xor_(x86::r8b, x86::r8b);
	a.xchg(x86::byte_ptr(MimicState), x86::r8b);
	a.mov(x86::r9, &pCamData->pCamBool->isMimic);
	a.mov(x86::byte_ptr(x86::r9), x86::r8b);
	a.jmp(reinterpret_cast<uint8_t*>(stateInfoCode) + 7);
	a.int3();
	a.bind(MimicState);
	a.embedUInt8(0);
	a.align(AlignMode::kZero, 0x4);
	a.bind(FrameCount);
	a.embedUInt32(0);
	InitAsm(code, stateInfoCode, codeMemFree, 2);

	if (pIniSet->pIniBool->isDMMMKicks)
	{
		code.init(env);
		code.attach(&a);
		Label RdC_retjmp = a.newLabel();
		Label RdC_SpCheck = a.newLabel();
		a.embed(reinterpret_cast<uint8_t*>(ragdollCode), 7);
		PUSHR;
		a.mov(x86::rax, x86::ptr(x86::rbx, 0x10));
		a.cmp(x86::byte_ptr(x86::rbx, 0x128), 2);
		a.je(RdC_SpCheck);
		a.mov(x86::rcx, x86::ptr(x86::rax, 0x190));
		a.mov(x86::rcx, x86::ptr(x86::rcx, 0x8));
		a.cmp(x86::byte_ptr(x86::rcx, 0x45), 2);
		a.jne(RdC_retjmp);
		a.bind(RdC_SpCheck);
		a.mov(x86::rcx, x86::ptr(x86::rax, 0x58));
		a.mov(x86::rcx, x86::ptr(x86::rcx, 0xC8));
		a.test(x86::byte_ptr(x86::rcx, 0x24), -1);
		a.jne(RdC_retjmp);
		a.mov(x86::rcx, x86::ptr(x86::rax, 0x178));
		a.mov(x86::edx, 0x7A5BC); // 501180
		a.call(CheckSpEffect);
		a.shl(x86::al, 1);
		a.movzx(x86::edx, x86::al);
		a.mov(x86::byte_ptr(x86::rbx, 0x128), x86::al);
		a.bind(RdC_retjmp);
		POPR;
		a.jmp(reinterpret_cast<uint8_t*>(ragdollCode) + 7);
		a.int3();
		InitAsm(code, ragdollCode, codeMemFree, 2);
	}

	code.init(env);
	code.attach(&a);
	Label HvkB_retjmp = a.newLabel();
	a.embed(reinterpret_cast<uint8_t*>(hvkBoneCode), 7);
	PUSH;
	a.mov(x86::rax, &pCamData->pCamBool->isCamInit);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(HvkB_retjmp);
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	a.jne(HvkB_retjmp);
	a.mov(x86::rax, &pCamData->headBoneOffset);
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.mov(x86::rcx, x86::ptr(x86::rsi, 0x40));
	a.mov(x86::eax, x86::dword_ptr(x86::rax, x86::rcx));
	a.cmp(x86::eax, x86::ebx);
	a.jne(HvkB_retjmp);
	a.movaps(x86::xmm1, x86::xmm6);
	a.mov(x86::rax, reinterpret_cast<uint64_t>(&pAttachToHead));
	a.call(x86::qword_ptr(x86::rax));
	a.bind(HvkB_retjmp);
	POP;
	a.jmp(reinterpret_cast<uint8_t*>(hvkBoneCode) + 7);
	a.int3();
	InitAsm(code, hvkBoneCode, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(strafeCheckCode00) + 6);
	a.mov(x86::byte_ptr(x86::rcx, 0x199), x86::dl);
	a.jmp(reinterpret_cast<uint8_t*>(strafeCheckCode00) + 6);
	a.int3();
	InitAsm(code, strafeCheckCode00, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(strafeCheckCode01) + 6);
	a.mov(x86::byte_ptr(x86::rcx, 0xC9), x86::dl);
	a.jmp(reinterpret_cast<uint8_t*>(strafeCheckCode01) + 6);
	a.int3();
	InitAsm(code, strafeCheckCode01, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(strafeCheckCode02), 10);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isLockedOnLook);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.jne(strafeCheckCode03);
	a.jmp(strafeCheckCode04);
	a.int3();
	InitAsm(code, strafeCheckCode02, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	a.mov(x86::rcx, x86::ptr(x86::rbx, 0xF8));
	PUSH;
	a.mov(x86::rax, reinterpret_cast<uint64_t>(&pPerspectiveControl));
	a.call(x86::qword_ptr(x86::rax));
	POP;
	a.jmp(reinterpret_cast<uint8_t*>(camCode00) + 7);
	a.int3();
	InitAsm(code, camCode00, codeMemFree, 2);
	
	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode01) + 5);
	a.call(camCode01Call);
	a.jmp(reinterpret_cast<uint8_t*>(camCode01) + 5);
	a.int3();
	InitAsm(code, camCode01, codeMemFree, 0);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode05) + 30);
	a.movaps(x86::ptr(x86::rdi, 0x10), x86::xmm0);
	a.movaps(x86::ptr(x86::rdi, 0x20), x86::xmm1);
	a.movaps(x86::xmm0, x86::ptr(x86::rdi, 0x440));
	a.movaps(x86::xmm1, x86::ptr(x86::rdi, 0x450));
	a.movaps(x86::ptr(x86::rdi, 0x30), x86::xmm0);
	a.movaps(x86::ptr(x86::rdi, 0x40), x86::xmm1);
	a.jmp(reinterpret_cast<uint8_t*>(camCode05) + 30);
	a.int3();
	InitAsm(code, camCode05, codeMemFree, 25);

	code.init(env);
	code.attach(&a);
	a.movaps(x86::xmm0, x86::xmm3);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode09) + 7);
	a.movaps(x86::ptr(x86::rdx), x86::xmm7);
	a.jmp(reinterpret_cast<uint8_t*>(camCode09) + 7);
	a.int3();
	InitAsm(code, camCode09, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	a.mulps(x86::xmm2, x86::xmm3);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode10) + 7);
	a.movaps(x86::ptr(x86::rbx, 0x10), x86::xmm5);
	a.jmp(reinterpret_cast<uint8_t*>(camCode10) + 7);
	a.int3();
	InitAsm(code, camCode10, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	a.movaps(x86::xmm1, x86::xmm2);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode11) + 7);
	a.movaps(x86::ptr(x86::rbx, 0x40), x86::xmm6);
	a.jmp(reinterpret_cast<uint8_t*>(camCode11) + 7);
	a.int3();
	InitAsm(code, camCode11, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	a.orps(x86::xmm1, x86::xmm2);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode12) + 7);
	a.movaps(x86::ptr(x86::rbx, 0x20), x86::xmm1);
	a.jmp(reinterpret_cast<uint8_t*>(camCode12) + 7);
	a.int3();
	InitAsm(code, camCode12, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	a.movaps(x86::xmm1, x86::xmm0);
	a.movaps(x86::xmm0, x86::ptr(x86::rsi, 0xA0));
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode13) + 11);
	a.movaps(x86::ptr(x86::rsi, 0x40), x86::xmm1);
	a.jmp(reinterpret_cast<uint8_t*>(camCode13) + 11);
	a.int3();
	InitAsm(code, camCode13, codeMemFree, 6);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(camCode14), 7);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode14) + 11);
	a.movaps(x86::ptr(x86::rdx, 0x40), x86::xmm1);
	a.jmp(reinterpret_cast<uint8_t*>(camCode14) + 11);
	a.int3();
	InitAsm(code, camCode14, codeMemFree, 6);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode15) + 31);
	a.movaps(x86::ptr(x86::rbx, 0x10), x86::xmm1);
	a.movaps(x86::ptr(x86::rbx, 0x20), x86::xmm0);
	a.call(camCode15Call);
	a.movaps(x86::xmm1, x86::ptr(x86::rbx, 0x90));
	a.movaps(x86::xmm0, x86::ptr(x86::rax));
	a.movaps(x86::ptr(x86::rbx, 0x30), x86::xmm1);
	a.movaps(x86::ptr(x86::rbx, 0x40), x86::xmm0);
	a.jmp(reinterpret_cast<uint8_t*>(camCode15) + 31);
	a.int3();
	InitAsm(code, camCode15, codeMemFree, 26);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode19) + 7);
	a.movaps(x86::ptr(x86::rbx, 0xA0), x86::xmm1);
	a.jmp(reinterpret_cast<uint8_t*>(camCode19) + 7);
	a.int3();
	InitAsm(code, camCode19, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode20) + 7);
	a.movaps(x86::ptr(x86::rbx, 0xA0), x86::xmm0);
	a.jmp(reinterpret_cast<uint8_t*>(camCode20) + 7);
	a.int3();
	InitAsm(code, camCode20, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(camCode21), 9);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(reinterpret_cast<uint8_t*>(camCode21) + 13);
	a.movaps(x86::ptr(x86::rbx, 0x40), x86::xmm0);
	a.jmp(reinterpret_cast<uint8_t*>(camCode21) + 13);
	a.int3();
	InitAsm(code, camCode21, codeMemFree, 8);

	code.init(env);
	code.attach(&a);
	a.embed(reinterpret_cast<uint8_t*>(camCode22), 4);
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.jne(reinterpret_cast<uint8_t*>(camCode22) + 7);
	a.mov(x86::rcx, FieldArea);
	a.mov(x86::rcx, x86::ptr(x86::rcx));
	a.mov(x86::rcx, x86::ptr(x86::rcx, 0x20));
	a.mov(x86::rcx, x86::ptr(x86::rcx, 0xD0));
	a.jmp(reinterpret_cast<uint8_t*>(camCode22) + 7);
	a.int3();
	InitAsm(code, camCode22, codeMemFree, 2);

	code.init(env);
	code.attach(&a);
	Label CC24_retjmp = a.newLabel();
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.mov(x86::r15b, x86::byte_ptr(x86::rax));
	a.mov(x86::rax, CSRemo);
	a.mov(x86::rax, x86::ptr(x86::rax));
	a.test(x86::rax, x86::rax);
	a.je(CC24_retjmp);
	a.mov(x86::rax, x86::ptr(x86::rax, 0x8));
	a.mov(x86::al, x86::byte_ptr(x86::rax, 0x40));
	a.cmp(x86::r15b, x86::al);
	a.sete(x86::al);
	a.mov(x86::r15, FieldArea);
	a.mov(x86::r15, x86::ptr(x86::r15));
	a.test(x86::r15, x86::r15);
	a.je(CC24_retjmp);
	a.mov(x86::r15, x86::ptr(x86::r15, 0x20));
	a.mov(x86::r15, x86::ptr(x86::r15, 0xD0));
	a.test(x86::al, x86::al);
	a.bind(CC24_retjmp);
	POPR;
	a.cmovne(x86::rax, x86::r15);
	a.movaps(x86::xmm0, x86::ptr(x86::rax, 0x10));
	a.movaps(x86::xmm1, x86::ptr(x86::rax, 0x20));
	a.movaps(x86::ptr(x86::rcx, 0x10), x86::xmm0);
	a.movaps(x86::ptr(x86::rcx, 0x20), x86::xmm1);
	a.movaps(x86::xmm0, x86::ptr(x86::rax, 0x30));
	a.movaps(x86::xmm1, x86::ptr(x86::rax, 0x40));
	a.movaps(x86::ptr(x86::rcx, 0x30), x86::xmm0);
	a.movaps(x86::ptr(x86::rcx, 0x40), x86::xmm1);
	a.jmp(reinterpret_cast<uint8_t*>(camCode24) + 32);
	a.int3();
	InitAsm(code, camCode24, codeMemFree, 27);
	
	code.init(env);
	code.attach(&a);
	Label CC25_retjmp = a.newLabel();
	a.pushf();
	PUSHR;
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);
	POPR;
	a.je(CC25_retjmp);
	a.embed(reinterpret_cast<uint8_t*>(camCode25), 6);
	a.bind(CC25_retjmp);
	a.popf();
	a.jmp(reinterpret_cast<uint8_t*>(camCode25) + 6);
	a.int3();
	InitAsm(code, camCode25, codeMemFree, 1);

	code.init(env);
	code.attach(&a);
	a.mov(x86::rax, &pCamData->pCamBool->isFPS);
	a.cmp(x86::byte_ptr(x86::rax), 1);	
	a.mov(x86::r8, reinterpret_cast<uint64_t>(&sound_mtx) - 0x10);
	a.cmove(x86::rdx, x86::r8);
	a.call(camCode26Call);
	a.jmp(reinterpret_cast<uint8_t*>(camCode26) + 5);
	a.int3();
	InitAsm(code, camCode26, codeMemFree);
}