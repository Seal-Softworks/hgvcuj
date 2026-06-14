#include <windows.h>
#include <vector>

struct reg {
    HKEY root;
    LPCWSTR subKey;
    LPCWSTR valueName;
    DWORD value;
};

bool applyreg(bool block);