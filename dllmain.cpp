#include "pch.h"
#include <windows.h>
#include <string>

using namespace std;

extern "C" __declspec(dllexport) const char* GetComputerNameX() {
    static char buffer[256];
    DWORD size = sizeof(buffer);
    GetComputerNameA(buffer, &size);
    return buffer;
}

extern "C" __declspec(dllexport) const char* GetCPU() {
    HKEY hKey;
    static char cpu[256];
    DWORD size = sizeof(cpu);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)cpu, &size);
        RegCloseKey(hKey);
        return cpu;
    }
    return "Unknown";
}

extern "C" __declspec(dllexport) const char* GetRAM() {
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    static string ram = to_string(mem.ullTotalPhys / (1024 * 1024 * 1024)) + " GB";
    return ram.c_str();
}

extern "C" __declspec(dllexport) const char* GetOS() {
    HKEY hKey;
    static char os[256];
    DWORD size = sizeof(os);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)os, &size);
        RegCloseKey(hKey);
        return os;
    }
    return "Unknown";
}

extern "C" __declspec(dllexport) const char* GetDisk() {
    ULARGE_INTEGER total;
    GetDiskFreeSpaceExA("C:\\", NULL, &total, NULL);
    static string disk = to_string(total.QuadPart / (1024 * 1024 * 1024)) + " GB";
    return disk.c_str();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
