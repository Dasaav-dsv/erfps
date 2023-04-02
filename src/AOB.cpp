#include "AOB.h"

static void* text;
static size_t text_size;

inline void* AOBScanAddress(const unsigned char* AOBString, const char* AOBMask, const void* region = text, const size_t region_size = text_size)
{
    const auto pattern = mem::pattern(AOBString, AOBMask);
    const auto mregion = mem::region(region, region_size);
    char* result = mem::simd_scanner(pattern).scan(mregion).any();

    if (result == nullptr)
    {
        AllocConsoleOnce();
        size_t AOBStrLen = std::strlen(AOBMask);
        std::cout << "AOB string ";
        for (size_t i = 0; i < AOBStrLen; i++)
        {
            std::cout << std::hex << static_cast<int>(AOBString[i]) << " ";
        }
        std::cout << "not found.\n";
    }

    return reinterpret_cast<void*>(result);
}

inline void* AOBScanCode(const uint8_t* AOBString, const char* AOBMask, const int Offset = 0, const void* region = text, const size_t region_size = text_size)
{
    return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(AOBScanAddress(AOBString, AOBMask, region, region_size)) + Offset);
}

inline void** AOBScanBase(const unsigned char* AOBString, const char* AOBMask, const int InOffset = 7, const int OpOffset = 3)
{
    uint8_t* addr = static_cast<uint8_t*>(AOBScanAddress(AOBString, AOBMask));
    return reinterpret_cast<void**>(addr + *reinterpret_cast<int32_t*>(addr + OpOffset) + InOffset);
}

inline void* AOBScanFuncCall(const unsigned char* AOBString1, const char* AOBMask1, const unsigned char* AOBString2, const char* AOBMask2, const size_t FuncSize, const int CallOffset = 0)
{
    uint8_t* addr = reinterpret_cast<uint8_t*>(AOBScanAddress(AOBString1, AOBMask1));
    addr = addr + *reinterpret_cast<int32_t*>(addr + 1 + CallOffset) + 5 + CallOffset;
    return AOBScanAddress(AOBString2, AOBMask2, addr, FuncSize);
}

extern void GetText()
{
    const char TextstrMatch[] = ".text"; // refused to work with char* string

    const auto pattern = mem::pattern(TextstrMatch, ".....");
    const auto region = mem::region(mInfo.lpBaseOfDll, mInfo.SizeOfImage);
    uint8_t* result = mem::simd_scanner(pattern).scan(region).any();

    text_size = *reinterpret_cast<uint32_t*>(result + 0x10);
    text = result + *reinterpret_cast<uint32_t*>(result + 0x14);
}

extern void PopulateBases()
{
    const uint8_t GameDataManMatch[] = { 0x48, 0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0, 0xE8, 0x0, 0x0, 0x0, 0x0, 0x4C, 0x8D, 0x44, 0x24, 0x0, 0x49, 0x8B, 0xD6 };
    const uint8_t WorldChrManMatch[] = { 0x48, 0x8B, 0x1D, 0x0, 0x0, 0x0, 0x0, 0x0F, 0x28, 0x74, 0x24, 0x0, 0x48, 0x8B, 0x7C, 0x24, 0x0, 0x48, 0x85, 0xDB };
    const uint8_t WorldChrManDbgMatch[] = { 0x48, 0x8B, 0x05, 0x0, 0x0, 0x0, 0x0, 0x8B, 0x48, 0x74, 0x48, 0x8B, 0x85, 0x90, 0x01, 0x0, 0x0, 0x41, 0x89, 0x0C, 0x24 };
    const uint8_t SessionManMatch[] = { 0x41, 0xB8, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8B, 0xD7, 0x48, 0x8B, 0x0D };
    const uint8_t FieldAreaMatch[] = { 0x48, 0x8B, 0x3D, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8D, 0x54, 0x24, 0x0, 0x48, 0x8B, 0xCD, 0xE8, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8B, 0xC8 };
    const uint8_t LockTgtManMatch[] = { 0x48, 0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0, 0x0F, 0x57, 0xD2, 0xF3, 0x0F, 0x10, 0x0 };
    const uint8_t CSFlipperMatch[] = { 0x48, 0x8B, 0x05, 0x0, 0x0, 0x0, 0x0, 0x8B, 0x4B, 0x70, 0x89, 0x48, 0x08, 0x48, 0x83, 0xC4, 0x0, 0x0, 0xC3 };
    const uint8_t CSMenuManMatch[] = { 0x48, 0x8B, 0x05, 0x0, 0x0, 0x0, 0x0, 0x45, 0x0F, 0xB6, 0xE9, 0x4C, 0x63, 0xFA };
    const uint8_t CSRemoMatch[] = { 0x0F, 0xB6, 0xDB, 0x0F, 0x45, 0xDF, 0x48, 0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0, 0x48, 0x85, 0xC9, 0x75 };
    const uint8_t CSEventFlagManMatch[] = { 0x48, 0x8B, 0x35, 0x0, 0x0, 0x0, 0x0, 0x0F, 0x44, 0xC7, 0x89, 0x84, 0x24, 0x0, 0x0, 0x0, 0x0, 0x48, 0x85, 0xF6 };
    const uint8_t CSUserInputManMatch[] = { 0x48, 0x89, 0x1D, 0x0, 0x0, 0x0, 0x0, 0x48, 0x85, 0xDB, 0x74, 0x14, 0x48, 0x8B, 0xCB };
    const uint8_t CSBulletManMatch[] = { 0x48, 0x8B, 0x05, 0x0, 0x0, 0x0, 0x0, 0x0F, 0x28, 0xD4, 0x0F, 0xC6, 0xD2, 0x00, 0x0F, 0x59, 0xD3 };
    const uint8_t CSHavokManMatch[] = { 0x48, 0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0, 0x49, 0x8B, 0xF8, 0x48, 0x85, 0xC9, 0x75 };
    const uint8_t SoloParamRepositoryMatch[] = { 0x45, 0x33, 0xED, 0x45, 0x33, 0xE4, 0x48, 0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0, 0x48, 0x85, 0xC9, 0x75 };
    const uint8_t MsgRepositoryMatch[] = { 0x48, 0x8B, 0x05, 0x0, 0x0, 0x0, 0x0, 0x48, 0x85, 0xC0, 0x48, 0x0F, 0x44, 0xC1, 0x48, 0x89, 0x05, 0x0, 0x0, 0x0, 0x0, 0xE8 };
    const uint8_t CSPcKeyConfigMatch[] = { 0x4C, 0x8B, 0x2D, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8B, 0xF1, 0x4D, 0x85, 0xED, 0x75 };
    const uint8_t CSMouseManMatch[] = { 0x48, 0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0, 0xE8, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8B, 0xCB, 0x0F, 0x28, 0xF0, 0xE8 };
    const uint8_t CSLangMatch[] = { 0x89, 0x2D, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8B, 0x6C, 0x24, 0x0, 0x89, 0x35 };
    const uint8_t MagicPtrMatch[] = { 0x4C, 0x8D, 0x3D, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8D, 0x34, 0x45, 0x01, 0x0, 0x0, 0x0 };

    const uint8_t printProcessEntriesCodeMatch[] = { 0xBA, 0x01, 0x0, 0x0, 0x0, 0xB9, 0x82, 0x0, 0x0, 0x0 };

    const char* GameDataManMask = "...????.????....?...";
    const char* WorldChrManMask = "...????....?....?...";
    const char* WorldChrManDbgMask = "...????..............";
    const char* SessionManMask = "..????......";
    const char* FieldAreaMask = "...????....?....????...";
    const char* LockTgtManMask = "...????.......";
    const char* CSFlipperMask = "...????.........??.";
    const char* CSMenuManMask = "...????.......";
    const char* CSRemoMask = ".........????....";
    const char* CSEventFlagManMask = "...????......????...";
    const char* CSUserInputManMask = "...????........";
    const char* CSBulletManMask = "...????..........";
    const char* CSHavokManMask = "...????.......";
    const char* SoloParamRepositoryMask = ".........????....";
    const char* MsgRepositoryMask = "...????..........????.";
    const char* CSPcKeyConfigMask = "...????.......";
    const char* CSMouseManMask = "...????.????.......";
    const char* CSLangMask = "..????....?..";
    const char* MagicPtrMask = "...????........";

    const char* printProcessEntriesCodeMask = "..........";

    GameDataMan = AOBScanBase(GameDataManMatch, GameDataManMask);
    WorldChrMan = AOBScanBase(WorldChrManMatch, WorldChrManMask);
    WorldChrManDbg = AOBScanBase(WorldChrManDbgMatch, WorldChrManDbgMask);
    SessionMan = AOBScanBase(SessionManMatch, SessionManMask, 16, 12);
    FieldArea = AOBScanBase(FieldAreaMatch, FieldAreaMask);
    LockTgtMan = AOBScanBase(LockTgtManMatch, LockTgtManMask);
    CSFlipper = AOBScanBase(CSFlipperMatch, CSFlipperMask);
    CSMenuMan = AOBScanBase(CSMenuManMatch, CSMenuManMask);
    CSRemo = AOBScanBase(CSRemoMatch, CSRemoMask, 13, 9);
    CSEventFlagMan = AOBScanBase(CSEventFlagManMatch, CSEventFlagManMask);
    CSUserInputMan = AOBScanBase(CSUserInputManMatch, CSUserInputManMask);
    CSBulletMan = AOBScanBase(CSBulletManMatch, CSBulletManMask);
    CSHavokMan = AOBScanBase(CSHavokManMatch, CSHavokManMask);
    SoloParamRepository = AOBScanBase(SoloParamRepositoryMatch, SoloParamRepositoryMask, 13, 9);
    MsgRepository = AOBScanBase(MsgRepositoryMatch, MsgRepositoryMask);
    CSPcKeyConfig = AOBScanBase(CSPcKeyConfigMatch, CSPcKeyConfigMask);
    CSMouseMan = AOBScanBase(CSMouseManMatch, CSMouseManMask);
    MagicPtr = reinterpret_cast<void*>(AOBScanBase(MagicPtrMatch, MagicPtrMask));
    CSLang = reinterpret_cast<int*>(AOBScanBase(CSLangMatch, CSLangMask, 6, 2));

    printProcessEntriesCode = reinterpret_cast<uint8_t*>(AOBScanBase(printProcessEntriesCodeMatch, printProcessEntriesCodeMask, 33, 29));
}

extern void ScanAOBs()
{
    const uint8_t PlayerInsOffsetMatch[] = { 0x48, 0x8B, 0x86, 0x0, 0x0, 0x0, 0x0, 0x45, 0x33, 0xC9, 0x4C, 0x8D, 0x86, 0x0, 0x0, 0x0, 0x0, 0x8B, 0x90, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8B, 0xCE };
    const uint8_t ChrAsmOffsetMatch[] = { 0x48, 0x8B, 0x8E, 0x0, 0x0, 0x0, 0x0, 0xE8, 0x0, 0x0, 0x0, 0x0, 0x44, 0x8B, 0xE0, 0x83, 0xCA, 0xFF };

    const uint8_t lockStickCodeMatch[] = { 0x0F, 0xB6, 0x86, 0x30, 0x28, 0x0, 0x0, 0x88, 0x86, 0x31, 0x28, 0x0, 0x0 };
    const uint8_t lockSwitchCodeMatch[] = { 0x48, 0x8B, 0x0D, 0x0, 0x0, 0x0, 0x0, 0x80, 0xB9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0F, 0x94, 0xC0, 0x88, 0x81 };
    const uint8_t lockAcquireCodeMatch[] = { 0x88, 0x86, 0x0, 0x0, 0x0, 0x0, 0x44, 0x38, 0xAE };
    const uint8_t lockDropCodeMatch[] = { 0xC6, 0x80, 0x0, 0x0, 0x0, 0x0, 0x00, 0x4C, 0x89, 0x74, 0x24, 0x0, 0x48, 0x89, 0x74, 0x24 };
    const uint8_t lockOnCodeMatch[] = { 0x0F, 0x28, 0xCE, 0xE8, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8B, 0x8D, 0x0, 0x0, 0x0, 0x0, 0xE8 };
    const uint8_t lockAimDropCodeMatch_call[] = { 0x48, 0x8D, 0x4D, 0x0, 0xE8, 0x0, 0x0, 0x0, 0x0, 0xF3, 0x0F, 0x11, 0x86, 0x74, 0x29, 0x0, 0x0, 0x48 };
    const uint8_t lockAimDropCodeMatch_func[] = { 0xF3, 0x0F, 0x10, 0x40, 0x48 };
    const uint8_t saveLoadCodeMatch[] = { 0x48, 0x89, 0x05, 0x0, 0x0, 0x0, 0x0, 0x45, 0x0F, 0xB6, 0xC6, 0x33, 0xD2, 0x83, 0xC9, 0xFF, 0xE8 };
    const uint8_t dtorStartCodeMatch[] = { 0xC7, 0x84, 0x24, 0x0, 0x0, 0x0, 0x0, 0xD1, 0x07, 0x0, 0x0, 0x48 };
    const uint8_t playerTAECodeMatch[] = { 0x48, 0x8B, 0x53, 0x18, 0x8B, 0x08, 0x89, 0x4A, 0x10, 0x48, 0x8B, 0xCE };
    const uint8_t enemyTAECodeMatch[] = { 0x4C, 0x8B, 0x47, 0x18, 0x8B, 0x08, 0x41, 0x89, 0x48, 0x10, 0x48, 0x8B, 0xCD };
    const uint8_t fallDeathCodeMatch[] = { 0x80, 0xE2, 0x01, 0xC0, 0xE2, 0x02, 0x08, 0x91, 0xC8, 0x01, 0x0, 0x0 };
    const uint8_t headSizeCodeMatch[] = { 0x83, 0xF8, 0x07, 0x73, 0x0, 0xF3, 0x0F, 0x10, 0x0C, 0x82 };
    const uint8_t upperHandCodeMatch[] = { 0x0F, 0xB6, 0x80, 0x27, 0x01, 0x0, 0x0, 0xEB, 0x03, 0x41, 0x8B, 0xC6, 0x6B, 0xD6, 0x64 };
    const uint8_t padSensCodeMatch[] = { 0x0F, 0x28, 0x86, 0x80, 0x02, 0x0, 0x0, 0xF3, 0x0F, 0x10, 0x8E, 0xD0, 0x02, 0x0, 0x0 };
    const uint8_t menuDisplayCodeMatch[] = { 0x48, 0x8D, 0x48, 0x08, 0x8B, 0x97, 0x28, 0x0D, 0x0, 0x0, 0xE8 };
    const uint8_t startScreenCodeMatch[] = { 0x48, 0x8D, 0x96, 0x0, 0x0, 0x0, 0x0, 0x41, 0xB8, 0x0A, 0x0, 0x0, 0x0, 0x48, 0x8D, 0x8E, 0x0, 0x0, 0x0, 0x0, 0xE8 };
    const uint8_t hkRunCodeMatch[] = { 0xFF, 0x50, 0x20, 0x0F, 0xB7, 0xD0, 0x48, 0x8B, 0xCF };
    const uint8_t aimTgtCodeMatch[] = { 0x0F, 0xB6, 0x86, 0x0, 0x0, 0x0, 0x0, 0x24, 0x06, 0x3C, 0x06, 0x75 };
    const uint8_t aimFadeCodeMatch[] = { 0xF3, 0x0F, 0x5C, 0xC2, 0x0F, 0x28, 0xCF, 0xE8, 0x0, 0x0, 0x0, 0x0, 0xF3, 0x0F, 0x11, 0x87 };
    const uint8_t genInputCodeMatch[] = { 0x48, 0x3B, 0x5C, 0x24, 0x0, 0x49, 0x0F, 0x47, 0xFF, 0x48, 0x85, 0xFF };
    const uint8_t menuLogoCodeMatch[] = { 0x33, 0xF6, 0x89, 0x75, 0x0, 0x40, 0x38, 0x75 };
    const uint8_t chrDitherCodeMatch[] = { 0x0F, 0xBF, 0x50, 0x06, 0x49, 0x8B, 0x4C, 0x24, 0x50, 0xE8 };
    const uint8_t AEGDitherCodeMatch[] = { 0x0F, 0xBE, 0x80, 0xAF, 0x0, 0x0, 0x0, 0xC3 };
    const uint8_t playerDitherCodeMatch[] = { 0x41, 0xF6, 0x47, 0x0, 0x01, 0xBA, 0x0, 0x0, 0x0, 0x0, 0x75, 0x05, 0xBA, 0x0, 0x0, 0x0, 0x0, 0xE8 };
    const uint8_t playerLightCodeMatch[] = { 0x42, 0xFF, 0x54, 0xC8, 0x10, 0x0F, 0x28, 0x30, 0x0F, 0x29, 0x75, 0xE0 };
    const uint8_t playerShadowCodeMatch[] = { 0xE8, 0x0, 0x0, 0x0, 0x0, 0xB8, 0x0, 0x0, 0x0, 0x0, 0x83, 0xFE };
    const uint8_t VFXCode00Match[] = { 0xF3, 0x0F, 0x10, 0x7C, 0x8A, 0x0, 0xF3, 0x0F, 0x10, 0x74, 0x8A, 0x0 };
    const uint8_t VFXCode01Match[] = { 0xF3, 0x44, 0x0F, 0x10, 0x5C, 0x9E, 0x0, 0xF3, 0x44, 0x0F, 0x10, 0x54, 0x9E, 0x0 };
    const uint8_t lockTgtCodeMatch[] = { 0x41, 0x0F, 0x11, 0x87, 0xD0, 0x0, 0x0, 0x0, 0x48, 0x8B, 0x8F, 0x88, 0x0, 0x0, 0x0 };
    const uint8_t chunkCodeMatch[] = { 0x0F, 0x11, 0x83, 0x0, 0x0, 0x0, 0x0, 0xC6, 0x83, 0x0, 0x0, 0x0, 0x0, 0x01, 0x0F, 0x28, 0xBC, 0x24 };
    const uint8_t rootTransformCodeMatch[] = { 0x48, 0x8D, 0xBB, 0x40, 0x01, 0x0, 0x0, 0x0F, 0x28, 0x0, 0x0F, 0x29, 0x07 };
    const uint8_t stateInfoCodeMatch_call[] = { 0x48, 0x8B, 0x89, 0x0, 0x0, 0x0, 0x0, 0x66, 0xBA, 0x0F, 0x0, 0x32, 0xDB };
    const uint8_t stateInfoCodeMatch_func[] = { 0x0F, 0xB7, 0x91, 0x0, 0x0, 0x0, 0x0, 0xEB, 0x0, 0x33, 0xD2, 0x66, 0x41, 0x3B, 0xD2 };
    const uint8_t ragdollCodeMatch[] = { 0xE8, 0x0, 0x0, 0x0, 0x0, 0x0F, 0xB6, 0x93, 0x0, 0x0, 0x0, 0x0, 0x3C, 0x03 };

    const uint8_t hitDetectCodeMatch[] = { 0xFF, 0x50, 0x10, 0x8B, 0xD8, 0xBE, 0x0, 0x0, 0x0, 0x0, 0x83, 0xF8, 0x08 };
    const uint8_t hitDetectCodeCpyMatch[] = { 0x48, 0x8B, 0x95, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8B, 0x52, 0x08, 0x48, 0x8B, 0xCF, 0xE8 };
    const uint8_t hitDetectCodeEndMatch[] = { 0x48, 0x8B, 0x8D, 0x0, 0x0, 0x0, 0x0, 0x48, 0x33, 0xCC, 0xE8 };
    const uint8_t hitRegisterCodeMatch[] = { 0x4D, 0x8B, 0xCE, 0x4D, 0x8B, 0xC5, 0x48, 0x8B, 0xD7, 0x48, 0x8B, 0x49, 0x40, 0xE8 };
    const uint8_t regBulletCodeMatch[] = { 0xFF, 0x43, 0x18, 0xFF, 0x43, 0x1C, 0x48, 0x8B, 0x4B, 0x08, 0x48, 0x89, 0x8A };
    const uint8_t dstBulletCodeMatch[] = { 0x0F, 0x28, 0x7C, 0x24, 0x0, 0xF3, 0x0F, 0x11, 0xB3, 0x0, 0x0, 0x0, 0x0, 0xF3, 0x0F, 0x11, 0xB3 };

    const uint8_t hvkBoneCodeMatch[] = { 0x0F, 0x28, 0x45, 0x0, 0x0F, 0x29, 0x07, 0xE8 };

    const uint8_t strafeCheckCode00Match[] = { 0x88, 0x91, 0x99, 0x01, 0x0, 0x0, 0xC3 };
    const uint8_t strafeCheckCode01Match[] = { 0x88, 0x91, 0xC9, 0x0, 0x0, 0x0, 0x41, 0xB1, 0x01 };
    const uint8_t strafeCheckCode02Match[] = { 0xF3, 0x44, 0x0F, 0x5C, 0x55, 0x0, 0xF3, 0x44, 0x0F, 0x11, 0x55, 0x0 };
    const uint8_t strafeCheckCode03Match[] = { 0xF3, 0x0F, 0x5C, 0x55, 0x0, 0xF3, 0x0F, 0x11, 0x55, 0x0, 0x48, 0x8D, 0x55, 0x0, 0x48, 0x8D, 0x4C };
    const uint8_t strafeCheckCode04Match[] = { 0x0F, 0x28, 0x0, 0x66, 0x0F, 0x7F, 0x45, 0x0, 0x48, 0x8D, 0x55, 0x0, 0x48, 0x8B, 0xCB };

    const uint8_t camCode00Match[] = { 0x0F, 0x28, 0xCE, 0x48, 0x8B, 0x8B, 0xF8, 0x0, 0x0, 0x0 };
    const uint8_t camCode01Match[] = { 0x0F, 0x29, 0x54, 0x24, 0x0, 0x0F, 0x29, 0x64, 0x24, 0x0, 0x0F, 0x29, 0x5C, 0x24, 0x0, 0x48, 0x8D, 0x54, 0x24, 0x0, 0x48, 0x8D, 0x4E, 0x10 };
    const uint8_t camCode05Match[] = { 0x0F, 0x29, 0x47, 0x10, 0x0F, 0x28, 0x87, 0x40, 0x04, 0x0, 0x0 };
    const uint8_t camCode09Match[] = { 0x66, 0x0F, 0x6F, 0xC3, 0x0F, 0x29, 0x3A, 0x0F, 0x28, 0xBC, 0x24 };
    const uint8_t camCode13Match[] = { 0x0F, 0x29, 0x46, 0x40, 0x0F, 0x28, 0x86, 0xA0, 0x0, 0x0, 0x0 };
    const uint8_t camCode14Match[] = { 0x0F, 0x58, 0x8A, 0xD0, 0x0, 0x0, 0x0, 0x0F, 0x29, 0x4A, 0x40, 0xC3 };
    const uint8_t camCode15Match[] = { 0x0F, 0x29, 0x43, 0x20, 0x0F, 0x28, 0x83, 0x90, 0x0, 0x0, 0x0, 0x0F, 0x29, 0x43, 0x30 };
    const uint8_t camCode19Match[] = { 0x0F, 0x29, 0x8B, 0xA0, 0x0, 0x0, 0x0, 0xE8 };
    const uint8_t camCode21Match[] = { 0x44, 0x0F, 0x28, 0x8C, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0F, 0x29, 0x43, 0x40 };
    const uint8_t camCode22Match[] = { 0xF3, 0x0F, 0x11, 0x8F, 0x0, 0x0, 0x0, 0x0, 0x48, 0x8D, 0x55, 0x0, 0x48, 0x8B, 0xCB };
    const uint8_t camCode23Match[] = { 0x0F, 0x28, 0x49, 0x40, 0x0F, 0x29, 0x4F, 0x40, 0xE9 };
    const uint8_t camCode24Match[] = { 0x0F, 0x29, 0x49, 0x40, 0x41, 0xBF, 0x01, 0x0, 0x0, 0x0, 0xF3, 0x0F, 0x10, 0x35 };
    const uint8_t camCode25Match[] = { 0xF3, 0x44, 0x0F, 0x11, 0x63, 0x50, 0xF3, 0x44, 0x0F, 0x11, 0x9B };
    const uint8_t camCode26Match[] = {  0x48, 0x8B, 0x86, 0xE8, 0x0, 0x0, 0x0, 0x48, 0x8B, 0x58, 0x18, 0x48, 0x85, 0xC9 };

    const uint8_t getEventFlagMatch[] = { 0x44, 0x8B, 0x41, 0x0, 0x44, 0x8B, 0xDA, 0x33, 0xD2, 0x41, 0x8B, 0xC3, 0x41, 0xF7, 0xF0 };
    const uint8_t setEventFlagMatch[] = { 0x48, 0x89, 0x5C, 0x24, 0x0, 0x44, 0x8B, 0x49, 0x0, 0x44, 0x8B, 0xD2, 0x33, 0xD2, 0x41, 0x8B, 0xC2, 0x41, 0xF7, 0xF1 };

    const uint8_t hknpWorldRayCastCodeMatch[] = { 0x48, 0x8D, 0x44, 0x24, 0x40, 0x48, 0x89, 0x7C, 0x24, 0x28, 0x4C, 0x8D, 0x4C, 0x24, 0x30, 0x4C, 0x8D, 0x44, 0x24, 0x50, 0x48, 0x89, 0x44, 0x24, 0x20, 0xBA, 0x58, 0x0, 0x0, 0x02, 0xE8 };

    const uint8_t createBulletInsCodeMatch[] = { 0x53, 0x48, 0x83, 0xEC, 0x0, 0x0F, 0xB6, 0x41, 0x14, 0x48, 0x8B, 0xD9, 0x3C, 0x01, 0x75, 0x0, 0x4C, 0x8D, 0x44, 0x24 };
    const uint8_t getChrBulletEmitterSlotCodeMatch[] = { 0x4C, 0x8B, 0xC3, 0x8B, 0x10, 0x48, 0x8D, 0x8B, 0x0, 0x0, 0x0, 0x0, 0xE8, 0x0, 0x0, 0x0, 0x0, 0x48, 0x89, 0x44, 0x24, 0x0, 0x48, 0x85, 0xC0 };
    const uint8_t getBulletParamEntryCodeMatch[] = { 0x78, 0x0, 0x48, 0x8D, 0x4C, 0x24, 0x0, 0x48, 0xC7, 0x44, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0xC7, 0x44, 0x24, 0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0xC6, 0x44, 0x24, 0x0, 0x0, 0xE8 };
    const uint8_t getAttackParamEntryCodeMatch[] = { 0xE8, 0x0, 0x0, 0x0, 0x0, 0x8D, 0x47, 0xFF, 0x83, 0xF8, 0x0E, 0x77, 0x1B };
    const uint8_t getSpEffectParamEntryCodeMatch[] = { 0x44, 0x8B, 0xF2, 0x40, 0x88, 0x78, 0xF4 };
    const uint8_t checkSpEffectCodeMatch[] = { 0xE8, 0x0, 0x0, 0x0, 0x0, 0x44, 0x0F, 0xB6, 0xE8, 0x0F, 0xB6, 0xC0 };

    const char* PlayerInsOffsetMask = "...????......????..????...";
    const char* ChrAsmOffsetMask = "...????.????......";

    const char* lockStickCodeMask = ".............";
    const char* lockSwitchCodeMask = "...????..????......";
    const char* lockAcquireCodeMask = "..????...";
    const char* lockDropCodeMask = "..????.....?....";
    const char* lockOnCodeMask = "....????...????.";
    const char* lockAimDropCodeMask_call = "...?.????.........";
    const char* lockAimDropCodeMask_func = ".....";
    const char* saveLoadCodeMask = "...????..........";
    const char* dtorStartCodeMask = "...????.....";
    const char* playerTAECodeMask = "............";
    const char* enemyTAECodeMask = ".............";
    const char* fallDeathCodeMask = "............";
    const char* headSizeCodeMask = "....?.....";
    const char* upperHandCodeMask = "...............";
    const char* padSensCodeMask = "...............";
    const char* menuDisplayCodeMask = "...........";
    const char* startScreenCodeMask = "...????.........????.";
    const char* hkRunCodeMask = ".........";
    const char* aimTgtCodeMask = "...????.....";
    const char* aimFadeCodeMask = "........????....";
    const char* genInputCodeMask = "....?.......";
    const char* menuLogoCodeMask = "....?...";
    const char* chrDitherCodeMask = "..........";
    const char* AEGDitherCodeMask = "........";
    const char* playerDitherCodeMask = "...?..????...????.";
    const char* playerLightCodeMask = "............";
    const char* playerShadowCodeMask = ".????.????..";
    const char* VFXCode00Mask = ".....?.....?";
    const char* VFXCode01Mask = "......?......?";
    const char* lockTgtCodeMask = "...............";
    const char* chunkCodeMask = "...????..????.....";
    const char* rootTransformCodeMask = ".............";
    const char* stateInfoCodeMask_call = "...????......";
    const char* stateInfoCodeMask_func = "...????.?......";
    const char* ragdollCodeMask = ".????...????..";

    const char* hitDetectCodeMask = "......????...";
    const char* hitDetectCodeCpyMask = "...????........";
    const char* hitDetectCodeEndMask = "...????....";
    const char* hitRegisterCodeMask = "..............";
    const char* regBulletCodeMask = ".............";
    const char* dstBulletCodeMask = "....?....????....";

    const char* hvkBoneCodeMask = "...?....";

    const char* strafeCheckCode00Mask = ".......";
    const char* strafeCheckCode01Mask = ".........";
    const char* strafeCheckCode02Mask = ".....?.....?";
    const char* strafeCheckCode03Mask = "....?....?...?...";
    const char* strafeCheckCode04Mask = ".......?...?...";

    const char* camCode00Mask = "..........";
    const char* camCode01Mask = "....?....?....?....?....";
    const char* camCode05Mask = "...........";
    const char* camCode09Mask = "...........";
    const char* camCode13Mask = "...........";
    const char* camCode14Mask = "............";
    const char* camCode15Mask = "...............";
    const char* camCode19Mask = "........";
    const char* camCode21Mask = ".....????....";
    const char* camCode22Mask = "....????...?...";
    const char* camCode23Mask = ".........";
    const char* camCode24Mask = "..............";
    const char* camCode25Mask = "...........";
    const char* camCode26Mask = "..............";

    const char* getEventFlagMask = "...?...........";
    const char* setEventFlagMask = "....?...?...........";

    const char* hknpWorldRayCastCodeMask = "...............................";

    const char* createBulletInsCodeMask = "....?..........?....";
    const char* getChrBulletEmitterSlotCodeMask = "........????.????....?...";
    const char* getBulletParamEntryCodeMask = ".?....?....?.......?.......?..";
    const char* getAttackParamEntryCodeMask = ".????........";
    const char* getSpEffectParamEntryCodeMask = ".......";
    const char* checkSpEffectCodeMask = ".????.......";

    uint8_t* pPlayerInsOffset = reinterpret_cast<uint8_t*>(AOBScanCode(PlayerInsOffsetMatch, PlayerInsOffsetMask));
    PlayerInsOffset = pPlayerInsOffset != nullptr ? *reinterpret_cast<int32_t*>(pPlayerInsOffset + 13) + 0x18 : PlayerInsOffset;

    uint8_t* pChrAsmOffset = reinterpret_cast<uint8_t*>(AOBScanCode(ChrAsmOffsetMatch, ChrAsmOffsetMask));
    ChrAsmOffset = pChrAsmOffset != nullptr ? *reinterpret_cast<int32_t*>(pChrAsmOffset + 3) : ChrAsmOffset;

    lockStickCode = AOBScanCode(lockStickCodeMatch, lockStickCodeMask, 7);
    lockSwitchCode = AOBScanCode(lockSwitchCodeMatch, lockSwitchCodeMask, 17);
    lockAcquireCode = AOBScanCode(lockAcquireCodeMatch, lockAcquireCodeMask);
    lockDropCode = AOBScanCode(lockDropCodeMatch, lockDropCodeMask);
    lockOnCode = AOBScanCode(lockOnCodeMatch, lockOnCodeMask, 3);
    lockOnCodeCall = lockOnCode != nullptr ? reinterpret_cast<uint8_t*>(lockOnCode) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(lockOnCode) + 1) + 5 : nullptr;
    lockAimDropCode = AOBScanFuncCall(lockAimDropCodeMatch_call, lockAimDropCodeMask_call, lockAimDropCodeMatch_func, lockAimDropCodeMask_func, 0x100, 4);
    saveLoadCode = AOBScanCode(saveLoadCodeMatch, saveLoadCodeMask, 11);
    dtorStartCode = AOBScanCode(dtorStartCodeMatch, dtorStartCodeMask);
    playerTAECode = AOBScanCode(playerTAECodeMatch, playerTAECodeMask);
    enemyTAECode = AOBScanCode(enemyTAECodeMatch, enemyTAECodeMask);
    fallDeathCode = AOBScanCode(fallDeathCodeMatch, fallDeathCodeMask, 6);
    headSizeCode = AOBScanCode(headSizeCodeMatch, headSizeCodeMask, 5);
    upperHandCode = AOBScanCode(upperHandCodeMatch, upperHandCodeMask, 15);
    padSensCode = AOBScanCode(padSensCodeMatch, padSensCodeMask, 7);
    menuDisplayCode = AOBScanCode(menuDisplayCodeMatch, menuDisplayCodeMask, 4);
    startScreenCode = AOBScanCode(startScreenCodeMatch, startScreenCodeMask);
    hkRunCode = AOBScanCode(hkRunCodeMatch, hkRunCodeMask, 3);
    aimTgtCode = AOBScanCode(aimTgtCodeMatch, aimTgtCodeMask);
    aimFadeCode = AOBScanCode(aimFadeCodeMatch, aimFadeCodeMask, 12);
    genInputCode = AOBScanCode(genInputCodeMatch, genInputCodeMask, 3);
    menuLogoCode = AOBScanCode(menuLogoCodeMatch, menuLogoCodeMask);
    chrDitherCode = AOBScanCode(chrDitherCodeMatch, chrDitherCodeMask);
    AEGDitherCode = AOBScanCode(AEGDitherCodeMatch, AEGDitherCodeMask);
    playerDitherCode = AOBScanCode(playerDitherCodeMatch, playerDitherCodeMask);
    playerLightCode = AOBScanCode(playerLightCodeMatch, playerLightCodeMask);
    playerShadowCode = AOBScanCode(playerShadowCodeMatch, playerShadowCodeMask, 5);
    VFXCode00 = AOBScanCode(VFXCode00Match, VFXCode00Mask);
    VFXCode01 = AOBScanCode(VFXCode01Match, VFXCode01Mask);
    lockTgtCode = AOBScanCode(lockTgtCodeMatch, lockTgtCodeMask);
    chunkCode = AOBScanCode(chunkCodeMatch, chunkCodeMask, 7);
    rootTransformCode = AOBScanCode(rootTransformCodeMatch, rootTransformCodeMask, 7);
    stateInfoCode = AOBScanFuncCall(stateInfoCodeMatch_call, stateInfoCodeMask_call, stateInfoCodeMatch_func, stateInfoCodeMask_func, 1000, 13);
    ragdollCode = AOBScanCode(ragdollCodeMatch, ragdollCodeMask, 5);

    hitDetectCode = AOBScanCode(hitDetectCodeMatch, hitDetectCodeMask, 5);
    hitDetectCodeCpy = AOBScanCode(hitDetectCodeCpyMatch, hitDetectCodeCpyMask);
    hitDetectCodeEnd = AOBScanCode(hitDetectCodeEndMatch, hitDetectCodeEndMask, 0, hitDetectCodeCpy, 0x1000);
    hitRegisterCode = AOBScanCode(hitRegisterCodeMatch, hitRegisterCodeMask, 13);
    regBulletCode = AOBScanCode(regBulletCodeMatch, regBulletCodeMask, 10);
    dstBulletCode = AOBScanCode(dstBulletCodeMatch, dstBulletCodeMask);

    hitRegisterCall = reinterpret_cast<uint8_t*>(hitRegisterCode) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(hitRegisterCode) + 1) + 5;

    hvkBoneCode = AOBScanCode(hvkBoneCodeMatch, hvkBoneCodeMask, 0);

    strafeCheckCode00 = AOBScanCode(strafeCheckCode00Match, strafeCheckCode00Mask);
    strafeCheckCode01 = AOBScanCode(strafeCheckCode01Match, strafeCheckCode01Mask);
    strafeCheckCode02 = AOBScanCode(strafeCheckCode02Match, strafeCheckCode02Mask, 6);
    strafeCheckCode03 = AOBScanCode(strafeCheckCode03Match, strafeCheckCode03Mask, 14);
    strafeCheckCode04 = AOBScanCode(strafeCheckCode04Match, strafeCheckCode04Mask, 3);

    camCode00 = AOBScanCode(camCode00Match, camCode00Mask, 3);
    camCode01 = AOBScanCode(camCode01Match, camCode01Mask, 24); // 1 - 4
    camCode05 = AOBScanCode(camCode05Match, camCode05Mask); // 5 - 8
    camCode06 = reinterpret_cast<uint8_t*>(camCode05) + 0x0B;
    camCode07 = reinterpret_cast<uint8_t*>(camCode05) + 0x16;
    camCode08 = reinterpret_cast<uint8_t*>(camCode05) + 0x1A;
    camCode09 = AOBScanCode(camCode09Match, camCode09Mask); // 9 - 12
    camCode10 = reinterpret_cast<uint8_t*>(camCode09) + 0x1B;
    camCode11 = reinterpret_cast<uint8_t*>(camCode09) + 0x22;
    camCode12 = reinterpret_cast<uint8_t*>(camCode09) + 0x6F;
    camCode13 = AOBScanCode(camCode13Match, camCode13Mask);
    camCode14 = AOBScanCode(camCode14Match, camCode14Mask);
    camCode15 = AOBScanCode(camCode15Match, camCode15Mask); // 15 - 18
    camCode16 = reinterpret_cast<uint8_t*>(camCode15) + 0x0B;
    camCode17 = reinterpret_cast<uint8_t*>(camCode15) + 0x0F;
    camCode18 = reinterpret_cast<uint8_t*>(camCode15) + 0x1B;
    camCode19 = AOBScanCode(camCode19Match, camCode19Mask); // 19 - 20
    camCode20 = reinterpret_cast<uint8_t*>(camCode19) + 0x10E;
    camCode21 = AOBScanCode(camCode21Match, camCode21Mask);
    camCode22 = AOBScanCode(camCode22Match, camCode22Mask, 8);
    camCode23 = AOBScanCode(camCode23Match, camCode23Mask, -24);
    camCode24 = AOBScanCode(camCode24Match, camCode24Mask, -28);
    camCode25 = AOBScanCode(camCode25Match, camCode25Mask);
    camCode26 = AOBScanCode(camCode26Match, camCode26Mask, 65);

    camCode01Call = reinterpret_cast<uint8_t*>(camCode01) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(camCode01) + 1) + 5;
    camCode15Call = reinterpret_cast<uint8_t*>(camCode15) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(camCode15) + 20) + 24;
    camCode26Call = reinterpret_cast<uint8_t*>(camCode26) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(camCode26) + 1) + 5;

    getEventFlagCode = AOBScanCode(getEventFlagMatch, getEventFlagMask);
    setEventFlagCode = AOBScanCode(setEventFlagMatch, setEventFlagMask);

    hknpWorldRayCastCode = AOBScanCode(hknpWorldRayCastCodeMatch, hknpWorldRayCastCodeMask, 30);
    hknpWorldRayCastCode = hknpWorldRayCastCode != nullptr ? reinterpret_cast<uint8_t*>(hknpWorldRayCastCode) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(hknpWorldRayCastCode) + 1) + 5 : nullptr;

    createBulletInsCode = AOBScanCode(createBulletInsCodeMatch, createBulletInsCodeMask);

    getChrBulletEmitterSlotCode = AOBScanCode(getChrBulletEmitterSlotCodeMatch, getChrBulletEmitterSlotCodeMask, 12);
    ChrSlotSysOffset = getChrBulletEmitterSlotCode != nullptr ? *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(getChrBulletEmitterSlotCode) - 4) : ChrSlotSysOffset;
    getChrBulletEmitterSlotCode = getChrBulletEmitterSlotCode != nullptr ? reinterpret_cast<uint8_t*>(getChrBulletEmitterSlotCode) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(getChrBulletEmitterSlotCode) + 1) + 5 : nullptr;

    getBulletParamEntryCode = AOBScanCode(getBulletParamEntryCodeMatch, getBulletParamEntryCodeMask, 29);
    getBulletParamEntryCode = getBulletParamEntryCode != nullptr ? reinterpret_cast<uint8_t*>(getBulletParamEntryCode) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(getBulletParamEntryCode) + 1) + 5 : nullptr;

    getAttackParamEntryCode = AOBScanCode(getAttackParamEntryCodeMatch, getAttackParamEntryCodeMask);
    getAttackParamEntryCode = getAttackParamEntryCode != nullptr ? reinterpret_cast<uint8_t*>(getAttackParamEntryCode) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(getAttackParamEntryCode) + 1) + 5 : nullptr;

    getSpEffectParamEntryCode = AOBScanCode(getSpEffectParamEntryCodeMatch, getSpEffectParamEntryCodeMask);
    getSpEffectParamEntryCode = getSpEffectParamEntryCode != nullptr ? reinterpret_cast<uint8_t*>(getSpEffectParamEntryCode) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(getSpEffectParamEntryCode) + 8) + 12 : nullptr;

    checkSpEffectCode = AOBScanCode(checkSpEffectCodeMatch, checkSpEffectCodeMask);
    checkSpEffectCode = checkSpEffectCode != nullptr ? reinterpret_cast<uint8_t*>(checkSpEffectCode) + *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(checkSpEffectCode) + 1) + 5 : nullptr;
}
