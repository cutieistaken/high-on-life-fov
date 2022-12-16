#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>

// GetModuleBaseAddress code borrowed from UnknownCheats (user: onra2) in thread: https://www.unknowncheats.me/forum/programming-for-beginners/315168-please-explain-base-address-automatically-2.html
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!_wcsicmp(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

int main()
{
    // Find window (used to get the hWnd needed for WPM/RPM
    // May cause issues if you have multiple unreal engine games opened but like i dont so im not gonna fix it
    HWND hwnd = FindWindowA("UnrealWindow", NULL);

    
    DWORD procID;

    // Check hwnd is not NULL
    if (!hwnd)
    {
        std::cout << "[!] failed to find window\n";
        return 1;
    }

    std::cout << "found high on life window\n";
    GetWindowThreadProcessId(hwnd, &procID);

    // i had to use OpenProcess in order to get read/write, there is probs a way to do it with FindWindowA but im not sure
    HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, procID);

    if (!proc)
    {
        std::cout << "[!] failed to use OpenProcess to get handle\n";
        return 1;
    }
    
    // Change Oregon-Win64-Shipping.exe to whatever your version is, this is the steam version
    DWORD64 baseAddr = GetModuleBaseAddress(procID, L"Oregon-Win64-Shipping.exe");

    std::cout << "process id is: " << procID << "\n";
    std::cout << "program base is: " << std::hex << baseAddr << "\n";

    // Use basic ptr arithmatic to get to the base of the setFov function
    // Hard-coding this offset may cause a problem in the future if updates break it, but for now (version: 10125262), it works
    DWORD_PTR setFOVBase = baseAddr + 0x34661F7;
    DWORD_PTR otherBase = baseAddr + 0x1634B70;
    
    std::cout << "setFOV() base is: " << std::hex << setFOVBase << "\n";

    // using unsigned char since its 1 byte long instead of 4 byte int
    unsigned char byte[sizeof(BYTE) * 8] = { 144, 144, 144, 144, 144, 144, 144 , 144 }; // 90 in hex
    unsigned char otherBytes[sizeof(BYTE) * 2] = { 195, 144 };

    // Check whether WPM fails
    if (WriteProcessMemory(proc, (BYTE*)(setFOVBase + 9), &byte, sizeof(byte), NULL) == 0)
    {
        std::cout << "[!] WPM failure\n";
        return 1;
    }

    // Check whether WPM fails
    if (WriteProcessMemory(proc, (BYTE*)(otherBase), &otherBytes, sizeof(otherBytes), NULL) == 0)
    {
        std::cout << "[!] WPM failure\n";
        return 1;
    }

    return 0;
    
}
