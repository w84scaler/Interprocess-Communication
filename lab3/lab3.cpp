#include <iostream>
#include <windows.h>
#include <conio.h>
#include <io.h>
#include <tchar.h>

#define CRITICAL_SECTION_NAME TEXT("BRUH_SECTION")
#define SHARED_MEMORY_NAME TEXT("BRUH_MEMORY")
#define N 3

LPCTSTR InitializeMapping(HANDLE* mapping, size_t size, LPCWSTR name);
void freeResources(HANDLE* mapping, LPCTSTR* buff, CRITICAL_SECTION* section);

int main()
{
    size_t SectionSize = sizeof(CRITICAL_SECTION);
    size_t MemorySize = sizeof(int);

    int sto = 100;

    LPCTSTR csBuffer, smBuffer;
    HANDLE csMapping, smMapping;
    CRITICAL_SECTION cs;

    STARTUPINFO si1 = { sizeof(STARTUPINFO) };
    STARTUPINFO si2 = { sizeof(STARTUPINFO) };
    STARTUPINFO si3 = { sizeof(STARTUPINFO) };
    STARTUPINFO startupInfo[] = { si1, si2, si3 };

    PROCESS_INFORMATION processInfo[N];

    std::wstring CmdLine1(L"Process1.exe");
    std::wstring CmdLine2(L"Process2.exe");
    std::wstring CmdLine3(L"Process3.exe");

    LPWSTR lpwCmdLine1 = &CmdLine1[0];
    LPWSTR lpwCmdLine2 = &CmdLine2[0];
    LPWSTR lpwCmdLine3 = &CmdLine3[0];
    LPWSTR lpwCmdLines[] = { lpwCmdLine1, lpwCmdLine2, lpwCmdLine3 };

    InitializeCriticalSection(&cs);

    csBuffer = InitializeMapping(&csMapping, SectionSize, CRITICAL_SECTION_NAME);
    if (csBuffer == NULL) {
        return -1;
    }

    smBuffer = InitializeMapping(&smMapping, MemorySize, SHARED_MEMORY_NAME);
    if (smBuffer == NULL) {
        freeResources(&csMapping, &csBuffer, &cs);
        return -1;
    }

    CopyMemory((PVOID)csBuffer, &cs, SectionSize);
    CopyMemory((PVOID)smBuffer, &sto, MemorySize);

    for (int i = 0; i < 3; i++) {
        BOOL res = CreateProcess(NULL, lpwCmdLines[i], NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo[i], &processInfo[i]);
        if (!res) {
            freeResources(&smMapping, &smBuffer, NULL);
            freeResources(&csMapping, &csBuffer, &cs);
            return -1;
        }
    }

    for (int i = 0;i < 3;i++) {
        WaitForSingleObject(processInfo[i].hProcess, INFINITE);
        CloseHandle(processInfo[i].hThread);
        CloseHandle(processInfo[i].hProcess);
    }

    freeResources(&smMapping, &smBuffer, NULL);
    freeResources(&csMapping, &csBuffer, &cs);
    return 0;
}

LPCTSTR InitializeMapping(HANDLE* mapping, size_t size, LPCWSTR name) {
    *mapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
    if (*mapping == NULL) {
        std::cout << "Failed to create mapping\n";
        return NULL;
    }

    LPCTSTR buffer = (LPCTSTR)MapViewOfFile(*mapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (buffer == NULL) {
        return NULL;
    }

    return buffer;
}

void freeResources(HANDLE* mapping, LPCTSTR* buff, CRITICAL_SECTION* section) {
    if (buff != NULL)
        UnmapViewOfFile(*buff);
    if (mapping != NULL)
        CloseHandle(*mapping);
    if (section != NULL)
        DeleteCriticalSection(section);
}