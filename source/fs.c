#include <xtl.h>
#include <stdio.h>

int FSFileExists(char *filename)
{
    DWORD attr = GetFileAttributesA(filename);
    DWORD lastError = GetLastError();
    return !(attr == -1 && (lastError == 2 || lastError == 3 || lastError == 1617));
}

int FSOpenFile(char *filename)
{
    HANDLE handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (handle == INVALID_HANDLE_VALUE)
        return -1;
    return (int)handle;
}

int FSFileSize(int file)
{
    return GetFileSize((HANDLE)file, NULL);
}

int FSReadFile(int file, int offset, void *buffer, int size)
{
    DWORD readBytes;
    SetFilePointer((HANDLE)file, offset, NULL, FILE_BEGIN);
    if (ReadFile((HANDLE)file, buffer, size, &readBytes, NULL) == FALSE)
        return -1;
    return readBytes;
}

void FSCloseFile(int file)
{
    XCloseHandle((HANDLE)file);
}
