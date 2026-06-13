#include "GDI.h"
#include <cstdlib>
#include <ctime>
#include <shellapi.h>
#include <string>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  char tmpDir[MAX_PATH] = {};
  char tmpPath[MAX_PATH] = {};
  GetTempPathA(MAX_PATH, tmpDir);
  lstrcatA(tmpDir, "note.txt");
  lstrcpyA(tmpPath, tmpDir);

  std::string msg = "hgvcuj\nts a test note\ni lvoe seals";
  DWORD wb = 0;
  HANDLE note = CreateFileA(tmpPath, GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  
  if (note == INVALID_HANDLE_VALUE)
    ExitProcess(4);
  if (!WriteFile(note, msg.c_str(), (DWORD)msg.size(), &wb, nullptr))
    ExitProcess(5);
  CloseHandle(note);
  ShellExecuteA(nullptr, "open", "notepad.exe", tmpPath, nullptr, SW_SHOWDEFAULT);

  srand((unsigned)time(nullptr));
  GDIInit();

  HDC hdc = GetDC(nullptr);
  DWORD start = GetTickCount();

  while (true) {
    float t = (GetTickCount() - start) / 1000.f;
    gdihsv(hdc, t);
    Sleep(8);
  }

  RedrawWindow(nullptr, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
  ReleaseDC(nullptr, hdc);
  cleanup();
  return 0;
}
