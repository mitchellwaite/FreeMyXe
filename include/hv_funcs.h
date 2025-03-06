#include <stdint.h>

uint64_t HvxGetVersions(uint32_t magic, int op, uint64_t source, uint64_t dest, uint64_t length);

void SetUsingFreeboot(int val);
uint64_t GetHVTargetAddress(uint32_t address);
void WriteHypervisor(uint32_t hv_address, void *userland_data, size_t length);
void HypervisorClearCache(uint32_t address);
void ReadHypervisor(void *userland_data, uint32_t hv_address, size_t length);
void WriteHypervisorUInt32(uint32_t hv_address, uint32_t value);
void WriteHypervisorUInt64(uint32_t hv_address, uint64_t value);
void WriteHypervisorUInt32_RMCI(uint32_t address, uint32_t value);
void WriteHypervisorUInt64_RMCI(uint32_t address, uint64_t value);
void HypervisorExecute(uint64_t address, void *code, size_t length);
