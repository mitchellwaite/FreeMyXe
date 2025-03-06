#include <xtl.h>
#include <stdint.h>
#include "xboxkrnl.h"
#include "ppcasm.h"

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

static uint8_t icbi_bytecode[] =
{
    0x38, 0x80, 0x00, 0x7F, 0x7C, 0xA3, 0x20, 0x78, 0x7C, 0x00, 0x1F, 0xAC, 0x4E, 0x80, 0x00, 0x20
};

static uint8_t rmci_write_std_bytecode[] =
{
    0x7D, 0x88, 0x02, 0xA6, 0x38, 0x60, 0x00, 0x00, 0x39, 0x60, 0x03, 0x98, 0x7D, 0x69, 0x03, 0xA6, 
    0x4E, 0x80, 0x04, 0x21, 0xF8, 0xC5, 0x00, 0x00, 0x38, 0x60, 0x00, 0x01, 0x39, 0x60, 0x03, 0x98, 
    0x7D, 0x69, 0x03, 0xA6, 0x4E, 0x80, 0x04, 0x21, 0x7D, 0x88, 0x03, 0xA6, 0x4E, 0x80, 0x00, 0x20, 
};

static uint8_t rmci_write_stw_bytecode[] =
{
    0x7D, 0x88, 0x02, 0xA6, 0x38, 0x60, 0x00, 0x00, 0x39, 0x60, 0x03, 0x98, 0x7D, 0x69, 0x03, 0xA6, 
    0x4E, 0x80, 0x04, 0x21, 0x90, 0xC5, 0x00, 0x00, 0x38, 0x60, 0x00, 0x01, 0x39, 0x60, 0x03, 0x98, 
    0x7D, 0x69, 0x03, 0xA6, 0x4E, 0x80, 0x04, 0x21, 0x7D, 0x88, 0x03, 0xA6, 0x4E, 0x80, 0x00, 0x20, 
};

// Freeboot memcpy
uint64_t __declspec(naked) HvxGetVersions(uint32_t magic, int op, uint64_t source, uint64_t dest, uint64_t length) {
    __asm
    {
        li r0, 0x0
        sc
        blr
    }
}

// BadUpdate backdoor, args for glorified Freeboot memcpy
static uint64_t __declspec(naked) HvxPostOutputMemcpy(uint32_t r3, uint64_t shellcode_addy, uint64_t source, uint64_t dest, uint64_t length)
{
    _asm
    {
        li r0, 0xD
        sc
        blr
    }
}

// BadUpdate backdoor, args for flushing cache
static uint64_t __declspec(naked) HvxPostOutputIcbi(uint32_t r3, uint64_t shellcode_addy, uint64_t address)
{
    _asm
    {
        li r0, 0xD
        sc
        blr
    }
}

// BadUpdate backdoor, args for writing with rmci disabled
static uint64_t __declspec(naked) HvxPostOutputRMCIWrite(uint32_t r3, uint64_t shellcode_addy, uint64_t address, uint64_t value)
{
    _asm
    {
        li r0, 0xD
        sc
        blr
    }
}

static int use_freeboot_syscall = 0;

void SetUsingFreeboot(int val)
{
    use_freeboot_syscall = val;
}

uint64_t GetHVTargetAddress(uint32_t address)
{
    if (address >= 0x00000 && address < 0x10000)
        return 0x8000010000000000 |  address;
    else if (address >= 0x10000 && address < 0x20000)
        return 0x8000010200000000 |  address;
    else if (address >= 0x20000 && address < 0x30000)
        return 0x8000010400000000 |  address;
    else if (address >= 0x30000 && address < 0x40000)
        return 0x8000010600000000 |  address;
    else
        return 0x8000030000000000 |  address;
}

void WriteHypervisor(uint32_t hv_address, void *userland_data, size_t length)
{
    // get the hypervisor address to patch
    uint64_t hv_target = GetHVTargetAddress(hv_address);
    // copy the data to write into physical memory
    uint8_t *data_buf = (uint8_t *)XPhysicalAlloc(0x1000, MAXULONG_PTR, 0, PAGE_READWRITE);
    uint64_t data_addr = 0x8000000000000000 | MmGetPhysicalAddress(data_buf);
    memcpy(data_buf, userland_data, length);
    // decide which syscall to use
    if (use_freeboot_syscall)
    {
        HvxGetVersions(0x72627472, 5, data_addr, hv_target, length);
    }
    else
    {
        // allocate a buffer for our freeboot memcpy
        uint8_t *shellcode_buf = (uint8_t *)XPhysicalAlloc(0x1000, MAXULONG_PTR, 0, PAGE_READWRITE);
        uint64_t shellcode_addr = 0x8000000000000000 | MmGetPhysicalAddress(shellcode_buf);
        memcpy(shellcode_buf, freeboot_memcpy_bytecode, sizeof(freeboot_memcpy_bytecode));
        // patch the freeboot memcpy to always have r4 as memcpy (5) and turn the beq after to a b
        *(uint32_t *)(shellcode_buf + 0x8) = LI(4, 5);
        *(uint16_t *)(shellcode_buf + 0xC) = 0x4800;
        // use the hvxpostoutput backdoor to perform the memcpy
        HvxPostOutputMemcpy(0x72627472, shellcode_addr, data_addr, hv_target, length);
        XPhysicalFree(shellcode_buf);
    }
    XPhysicalFree(data_buf);
}

void WriteHypervisorUInt64_RMCI(uint32_t address, uint64_t value)
{
    uint64_t hv_target = GetHVTargetAddress(address);
    // allocate a buffer for our write 
    uint8_t *shellcode_buf = (uint8_t *)XPhysicalAlloc(0x1000, MAXULONG_PTR, 0, PAGE_READWRITE);
    uint64_t shellcode_addr = 0x8000000000000000 | MmGetPhysicalAddress(shellcode_buf);
    memcpy(shellcode_buf, rmci_write_std_bytecode, sizeof(rmci_write_std_bytecode));
    // use the hvxpostoutput backdoor to perform the memcpy
    HvxPostOutputRMCIWrite(0x72627472, shellcode_addr, hv_target, value);
    XPhysicalFree(shellcode_buf);
}

void HypervisorExecute(uint64_t address, void *code, size_t length)
{
    uint64_t hv_target = GetHVTargetAddress(address);
    // allocate a buffer for our write 
    uint8_t *payload_buf = (uint8_t *)XPhysicalAlloc(0x40000, MAXULONG_PTR, 0, PAGE_READWRITE);
    uint64_t payload_addr = 0x8000000000000000 | MmGetPhysicalAddress(payload_buf);
    memcpy(payload_buf, code, length);
    {
        // allocate a buffer for our freeboot memcpy
        uint8_t *shellcode_buf = (uint8_t *)XPhysicalAlloc(0x1000, MAXULONG_PTR, 0, PAGE_READWRITE);
        uint64_t shellcode_addr = 0x8000000000000000 | MmGetPhysicalAddress(shellcode_buf);
        memcpy(shellcode_buf, freeboot_memcpy_bytecode, sizeof(freeboot_memcpy_bytecode));
        // patch the freeboot memcpy to always have r4 as hv execute (4) and turn the beq after to a b
        *(uint32_t *)(shellcode_buf + 0x8) = LI(4, 4);
        *(uint16_t *)(shellcode_buf + 0xC) = 0x4800;
        // use the hvxpostoutput backdoor to perform the memcpy
        HvxPostOutputMemcpy(0x72627472, shellcode_addr, address, payload_addr, 0x40000);
        XPhysicalFree(shellcode_buf);
    }
    XPhysicalFree(payload_buf);
}

void WriteHypervisorUInt32_RMCI(uint32_t address, uint32_t value)
{
    uint64_t hv_target = GetHVTargetAddress(address);
    // allocate a buffer for our write 
    uint8_t *shellcode_buf = (uint8_t *)XPhysicalAlloc(0x1000, MAXULONG_PTR, 0, PAGE_READWRITE);
    uint64_t shellcode_addr = 0x8000000000000000 | MmGetPhysicalAddress(shellcode_buf);
    memcpy(shellcode_buf, rmci_write_stw_bytecode, sizeof(rmci_write_stw_bytecode));
    // use the hvxpostoutput backdoor to perform the memcpy
    HvxPostOutputRMCIWrite(0x72627472, shellcode_addr, hv_target, value);
    XPhysicalFree(shellcode_buf);
}

void HypervisorClearCache(uint32_t address)
{
    uint64_t hv_target = GetHVTargetAddress(address);
    // allocate a buffer for our write 
    uint8_t *shellcode_buf = (uint8_t *)XPhysicalAlloc(0x1000, MAXULONG_PTR, 0, PAGE_READWRITE);
    uint64_t shellcode_addr = 0x8000000000000000 | MmGetPhysicalAddress(shellcode_buf);
    memcpy(shellcode_buf, icbi_bytecode, sizeof(icbi_bytecode));
    // use the hvxpostoutput backdoor to perform the memcpy
    HvxPostOutputIcbi(0x72627472, shellcode_addr, hv_target);
    XPhysicalFree(shellcode_buf);
}

void ReadHypervisor(void *userland_data, uint32_t hv_address, size_t length)
{
    // get the hypervisor address to patch
    uint64_t hv_target = GetHVTargetAddress(hv_address);
    // allocate some physical memory for the memcpy to copy to
    uint8_t *data_buf = (uint8_t *)XPhysicalAlloc(0x1000, MAXULONG_PTR, 0, PAGE_READWRITE);
    uint64_t data_addr = 0x8000000000000000 | MmGetPhysicalAddress(data_buf);
    // decide which syscall to use
    if (use_freeboot_syscall)
    {
        HvxGetVersions(0x72627472, 5, hv_target, data_addr, length);
    }
    else
    {
        // allocate a buffer for our freeboot memcpy
        uint8_t *shellcode_buf = (uint8_t *)XPhysicalAlloc(0x1000, MAXULONG_PTR, 0, PAGE_READWRITE);
        uint64_t shellcode_addr = 0x8000000000000000 | MmGetPhysicalAddress(shellcode_buf);
        memcpy(shellcode_buf, freeboot_memcpy_bytecode, sizeof(freeboot_memcpy_bytecode));
        // patch the freeboot memcpy to always have r4 as memcpy (5) and turn the beq after to a b
        *(uint32_t *)(shellcode_buf + 0x8) = LI(4, 5);
        *(uint16_t *)(shellcode_buf + 0xC) = 0x4800;
        // use the hvxpostoutput backdoor to perform the memcpy
        HvxPostOutputMemcpy(0x72627472, shellcode_addr, hv_target, data_addr, length);
        XPhysicalFree(shellcode_buf);
    }
    memcpy(userland_data, data_buf, length);
    XPhysicalFree(data_buf);
}

void WriteHypervisorUInt32(uint32_t hv_address, uint32_t value)
{
    uint32_t val = value;
    WriteHypervisor(hv_address, &val, sizeof(uint32_t));
}

void WriteHypervisorUInt64(uint32_t hv_address, uint64_t value)
{
    uint64_t val = value;
    WriteHypervisor(hv_address, &val, sizeof(uint64_t));
}
