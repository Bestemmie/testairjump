#include <iostream>
#include <windows.h>
#include <chrono>
#include <psapi.h>
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
using namespace std;

// OFFSETS

int offsets[7] = { 0x88, 0x1C8, 0x1C8, 0x1D0, 0x178, 0x80, 0xF0 };

// Gets the module base address by specifying the name

unsigned long long getModuleBaseAddress(DWORD pid, TCHAR* mn) {

    // First base address getter working without stupid ass TlHelp32.h

    unsigned long long module_baseaddr = 0;
    HMODULE modules[1024];

    HANDLE p = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
    if (p == NULL) return -1;

    DWORD needed;
    EnumProcessModules(p, modules, sizeof(modules), &needed);
    DWORD n_modules = needed / sizeof(HMODULE);

    TCHAR base_name[MAX_PATH];
    _MODULEINFO module_info;

    for (int i = 0; i < n_modules; i++) {
        if (GetModuleBaseName(p, modules[i], base_name, MAX_PATH)) {
            if (_tcscmp(base_name, mn) == 0) {
                GetModuleInformation(p, modules[i], &module_info, sizeof(module_info));
                module_baseaddr = (unsigned long long)module_info.lpBaseOfDll;
                return module_baseaddr;
            }
        }
    }

    return module_baseaddr;
}

// Writes a value to an addres by using the MBA and it's Offset, then
// calculates the address using the offset array declared at the beginning of the file

void writeIntToAddress(HANDLE p, int base_addr, int base_off, int v) {
    int temp, value = v;
    unsigned long long address;

    while (true) {
        ReadProcessMemory(p, (LPVOID)(base_addr + base_off), &address, sizeof(address), 0);

        for (int i = 0; i < 6; i++) {
            ReadProcessMemory(p, (LPVOID)(address + offsets[i]), &address, sizeof(address), 0);
            cout << address << endl;
        }
        address += offsets[6];

        ReadProcessMemory(p, (LPVOID)(address), &temp, sizeof(int), 0);
        cout << "Value @ " << address << ": " << temp << endl;
        WriteProcessMemory(p, (LPVOID)(address), &value, sizeof(int), 0);
    }
}

int main() {

    HWND window = FindWindowA(("LWJGL"), NULL);
    DWORD pid;

    GetWindowThreadProcessId(window, &pid);
    HANDLE p = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

    TCHAR modulename[8] = "jvm.dll";
    unsigned long long base = getModuleBaseAddress(pid, modulename);

    writeIntToAddress(p, base, 0x007FD7C0, 65537);

    return 0;
}
