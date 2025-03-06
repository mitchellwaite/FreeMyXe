#include <xtl.h>
#include <stdint.h>
#include "xboxkrnl.h"
#include "ppcasm.h"
#include "hv_funcs.h"

static LPWSTR buttons[1] = {L"OK"};
static MESSAGEBOX_RESULT result;
static XOVERLAPPED overlapped;
static wchar_t dialog_text_buffer[256];

void MessageBox(wchar_t *text)
{
    if (XShowMessageBoxUI(0, L"FreeMyXe beta1", text, 1, buttons, 0, XMB_ALERTICON, &result, &overlapped) == ERROR_IO_PENDING)
    {
        while (!XHasOverlappedIoCompleted(&overlapped))
            Sleep(50);
    }
}

// the Freeboot syscall 0 backdoor for 17559
static uint8_t freeboot_memcpy_bytecode[] =
{
    0x3D, 0x60, 0x72, 0x62, 0x61, 0x6B, 0x74, 0x72, 0x7F, 0x03, 0x58, 0x40, 0x41, 0x9A, 0x00, 0x08, 
    0x48, 0x00, 0x1C, 0xCA, 0x2B, 0x04, 0x00, 0x04, 0x41, 0x99, 0x00, 0x94, 0x41, 0x9A, 0x00, 0x44, 
    0x38, 0xA0, 0x15, 0x4C, 0x3C, 0xC0, 0x38, 0x80, 0x2B, 0x04, 0x00, 0x02, 0x40, 0x9A, 0x00, 0x0C, 
    0x60, 0xC6, 0x00, 0x07, 0x48, 0x00, 0x00, 0x0C, 0x2B, 0x04, 0x00, 0x03, 0x40, 0x9A, 0x00, 0x1C, 
    0x38, 0x00, 0x00, 0x00, 0x90, 0xC5, 0x00, 0x00, 0x7C, 0x00, 0x28, 0x6C, 0x7C, 0x00, 0x2F, 0xAC, 
    0x7C, 0x00, 0x04, 0xAC, 0x4C, 0x00, 0x01, 0x2C, 0x38, 0x60, 0x00, 0x01, 0x4E, 0x80, 0x00, 0x20, 
    0x7D, 0x88, 0x02, 0xA6, 0xF9, 0x81, 0xFF, 0xF8, 0xF8, 0x21, 0xFF, 0xF1, 0x7C, 0xA8, 0x03, 0xA6, 
    0x7C, 0xE9, 0x03, 0xA6, 0x80, 0x86, 0x00, 0x00, 0x90, 0x85, 0x00, 0x00, 0x7C, 0x00, 0x28, 0x6C, 
    0x7C, 0x00, 0x2F, 0xAC, 0x7C, 0x00, 0x04, 0xAC, 0x4C, 0x00, 0x01, 0x2C, 0x38, 0xA5, 0x00, 0x04, 
    0x38, 0xC6, 0x00, 0x04, 0x42, 0x00, 0xFF, 0xE0, 0x4E, 0x80, 0x00, 0x20, 0x38, 0x21, 0x00, 0x10, 
    0xE9, 0x81, 0xFF, 0xF8, 0x7D, 0x88, 0x03, 0xA6, 0x4E, 0x80, 0x00, 0x20, 0x2B, 0x04, 0x00, 0x05, 
    0x40, 0x9A, 0x00, 0x14, 0x7C, 0xC3, 0x33, 0x78, 0x7C, 0xA4, 0x2B, 0x78, 0x7C, 0xE5, 0x3B, 0x78, 
    0x48, 0x00, 0xA8, 0x82, 0x38, 0x60, 0x00, 0x02, 0x4E, 0x80, 0x00, 0x20, 
};

// the memory protection patches for 17559
static uint8_t memory_protection_bytecode[] =
{
    0x38, 0x80, 0x00, 0x07, 0x7C, 0x21, 0x20, 0x78, 0x7C, 0x35, 0xEB, 0xA6, 0x48, 0x00, 0x11, 0xC2
};

void __cdecl main()
{
    uint8_t cpu_key[0x10];
    HANDLE hThread;
    DWORD threadId;

    memset(cpu_key, 0, sizeof(cpu_key));

    DbgPrint("FreeMyXe!\n");

    if (HvxGetVersions(0x72627472, 1, 0, 0, 0) == 1)
    {
        DbgPrint("Freeboot detected, installing POST out backdoor\n");
        // downgrade to badupdate backdoor for testing
        // set HvxPostOutput syscall to point to mtctr r4; bctr; pair
        SetUsingFreeboot(1);
        WriteHypervisorUInt32(0x00015FD0 + (0xD * 4), 0x00000354);
        SetUsingFreeboot(0);
    }
    else
    {
        DbgPrint("using BadUpdate POST out backdoor\n");
    }

    // read out the CPU key
    ReadHypervisor(cpu_key, 0x20, sizeof(cpu_key));

    DbgPrint("CPU key: %08X%08X%08X%08X\n",
        *(uint32_t *)(cpu_key + 0x0), *(uint32_t *)(cpu_key + 0x4), *(uint32_t *)(cpu_key + 0x8), *(uint32_t *)(cpu_key + 0xC));

    wsprintfW(dialog_text_buffer, L"About to start patching HV and kernel...\n\nYour CPU key is:\n%08X%08X%08X%08X\n\ngithub.com/InvoxiPlayGames/FreeMyXe", *(uint32_t *)(cpu_key + 0x0), *(uint32_t *)(cpu_key + 0x4), *(uint32_t *)(cpu_key + 0x8), *(uint32_t *)(cpu_key + 0xC));
    MessageBox(dialog_text_buffer);

    DbgPrint("Writing syscall 0 backdoor...\n");
    // install the syscall 0 backdoor at a spare place in memory
    WriteHypervisor(0x0000B564, freeboot_memcpy_bytecode, sizeof(freeboot_memcpy_bytecode));
    // set syscall 0 to point to our backdoor
    WriteHypervisorUInt32(0x00015FD0, 0x0000B564);

    DbgPrint("Writing memory protection patches...\n");
    // write the patched memory protection instructions
    WriteHypervisor(0x0000154C, memory_protection_bytecode, sizeof(memory_protection_bytecode));
	// jump to the above shellcode
    WriteHypervisorUInt32(0x000011BC, 0x4800154E);
    HypervisorClearCache(0x0000154C);

    DbgPrint("Writing expansion signature patches...\n");
    WriteHypervisorUInt32(0x00030894, LI(3, 1)); // call to XeCryptBnQwBeSigVerify
    WriteHypervisorUInt32(0x00030914, LI(3, 0)); // call to memcmp

    DbgPrint("HV patched! Patching kernel\n");

    {
        uint64_t returnTrue = (((uint64_t)LI(3, 1)) << 32) | (BLR);
        uint64_t returnZero = (((uint64_t)LI(3, 0)) << 32) | (BLR);
        uint64_t valTo = 0;
        HANDLE hKernel = NULL;
        PDWORD pdwFunction = NULL;

        XexGetModuleHandle("xboxkrnl.exe", &hKernel);

        // patch XeKeysVerifyRSASignature
        XexGetProcedureAddress(hKernel, 600, &pdwFunction);
        WriteHypervisorUInt64_RMCI(MmGetPhysicalAddress(pdwFunction), returnTrue);
        HypervisorClearCache(MmGetPhysicalAddress(pdwFunction));

        // patch XeKeysVerifyPIRSSignature
        XexGetProcedureAddress(hKernel, 862, &pdwFunction);
        WriteHypervisorUInt64_RMCI(MmGetPhysicalAddress(pdwFunction), returnTrue);
        HypervisorClearCache(MmGetPhysicalAddress(pdwFunction));

        // patch UsbdIsDeviceAuthenticated
        XexGetProcedureAddress(hKernel, 745, &pdwFunction);
        WriteHypervisorUInt64_RMCI(MmGetPhysicalAddress(pdwFunction), returnTrue);
        HypervisorClearCache(MmGetPhysicalAddress(pdwFunction));

        // patch XexpVerifyMediaType
        pdwFunction = (PDWORD)0x80078ed0;
        WriteHypervisorUInt64_RMCI(MmGetPhysicalAddress(pdwFunction), returnTrue);
        HypervisorClearCache(MmGetPhysicalAddress(pdwFunction));

        // patch XexpVerifyDeviceId
        pdwFunction = (PDWORD)0x80079e10;
        WriteHypervisorUInt64_RMCI(MmGetPhysicalAddress(pdwFunction), returnZero);
        HypervisorClearCache(MmGetPhysicalAddress(pdwFunction));

        // patch XexpConvertError
        pdwFunction = (PDWORD)0x8007b920;
        WriteHypervisorUInt64_RMCI(MmGetPhysicalAddress(pdwFunction), returnZero);
        HypervisorClearCache(MmGetPhysicalAddress(pdwFunction));

        // patch a hash check in XexpVerifyXexHeaders
        pdwFunction = (PDWORD)0x8007c034;
        valTo = (((uint64_t)NOP) << 32) | NOP;
        WriteHypervisorUInt64_RMCI(MmGetPhysicalAddress(pdwFunction), valTo);
        HypervisorClearCache(MmGetPhysicalAddress(pdwFunction));

        // patch XexpVerifyMinimumVersion
        pdwFunction = (PDWORD)0x8007af08;
        WriteHypervisorUInt64_RMCI(MmGetPhysicalAddress(pdwFunction), returnZero);
        HypervisorClearCache(MmGetPhysicalAddress(pdwFunction));

        // patch XexpTranslateAndVerifyBoundPath to always return true
        pdwFunction = (PDWORD)0x80078eb0;
        valTo = (((uint64_t)LI(3, 1)) << 32) | ADDI(1, 1, 0x190);
        WriteHypervisorUInt64_RMCI(MmGetPhysicalAddress(pdwFunction), valTo);
        HypervisorClearCache(MmGetPhysicalAddress(pdwFunction));
    }

    DbgPrint("Done\n");

    Sleep(500);

    buttons[0] = L"Yay!";
    wsprintfW(dialog_text_buffer, L"Hypervisor and kernel have been patched!\n\nYour CPU key is:\n%08X%08X%08X%08X\n\ngithub.com/InvoxiPlayGames/FreeMyXe", *(uint32_t *)(cpu_key + 0x0), *(uint32_t *)(cpu_key + 0x4), *(uint32_t *)(cpu_key + 0x8), *(uint32_t *)(cpu_key + 0xC));
    MessageBox(dialog_text_buffer);
}
