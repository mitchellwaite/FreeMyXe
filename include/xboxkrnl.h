#include <xtl.h>

extern void DbgPrint(const char *s, ...);
extern unsigned int MmGetPhysicalAddress(void *buffer);
extern unsigned int ExCreateThread(PHANDLE pHandle, DWORD dwStackSize, LPDWORD lpThreadId, PVOID apiThreadStartup, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlagsMod);
extern void XapiThreadStartup(void (__cdecl *StartRoutine)(void *), void * StartContext);
extern unsigned int XexGetModuleHandle(PCSTR moduleName, PHANDLE hand);
extern unsigned int XexGetProcedureAddress(HANDLE hand, DWORD dwOrdinal, PVOID Address);
extern unsigned int XexLoadImage(LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle);
