#include <iostream>
#include <windows.h>
#include <conio.h>
#include <io.h>
#include <tchar.h>

#define CRITICAL_SECTION_NAME TEXT("BRUH_SECTION")
#define SHARED_MEMORY_NAME TEXT("BRUH_MEMORY")
#define N 3

void freeResources(HANDLE* mapping, LPCTSTR* buff, CRITICAL_SECTION* section);

int main()
{
    size_t SectionSize = sizeof(CRITICAL_SECTION);
    size_t MemorySize = sizeof(int);

    int* pSto;

    LPCTSTR csBuffer, smBuffer;
    HANDLE csMapping, smMapping;
    CRITICAL_SECTION* cs;

    csMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, CRITICAL_SECTION_NAME);
    if (csMapping == NULL) {
        return -1;
    }
    cs = (CRITICAL_SECTION*)MapViewOfFile(csMapping, FILE_MAP_ALL_ACCESS, 0, 0, SectionSize);
    if (cs == NULL) {
        freeResources(&csMapping, NULL, NULL);
        return -1;
    }

    smMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);
    if (smMapping == NULL) {
        freeResources(&csMapping, NULL, NULL);
        UnmapViewOfFile(cs);
        return -1;
    }

    pSto = (int*)MapViewOfFile(smMapping, FILE_MAP_ALL_ACCESS, 0, 0, MemorySize);
    if (pSto == NULL) {
        freeResources(&csMapping, NULL, NULL);
        UnmapViewOfFile(cs);
        freeResources(&smMapping, NULL, NULL);
        return -1;
    }

    bool flag = true;
    while (flag) {
        if (TryEnterCriticalSection(cs)) {
            if (*pSto != 0) {
                std::cout << "Proccess #2 read " << *pSto << std::endl << std::flush;
                *pSto -= 1;
                std::cout << "Proccess #2 wrote " << *pSto << std::endl << std::flush;
            }
            else
                flag = false;
            LeaveCriticalSection(cs);
        }
    }

    freeResources(&csMapping, NULL, NULL);
    UnmapViewOfFile(cs);
    freeResources(&smMapping, &smBuffer, NULL);
    return 0;
}

void freeResources(HANDLE* mapping, LPCTSTR* buff, CRITICAL_SECTION* section) {
    if (buff != NULL)
        UnmapViewOfFile(*buff);
    if (mapping != NULL)
        CloseHandle(*mapping);
    if (section != NULL)
        DeleteCriticalSection(section);
}