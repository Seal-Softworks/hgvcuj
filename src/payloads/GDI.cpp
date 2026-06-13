#include "GDI.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")


int g_screenW = 0;
int g_screenH = 0;
std::vector<uint8_t> g_pixels;
std::vector<uint8_t> g_scratch;
BITMAPINFO g_bmi = {};

static HDC     s_hdcMem = nullptr;
static HBITMAP s_hbm    = nullptr;

void GDIInit() {
    g_screenW = GetSystemMetrics(SM_CXSCREEN);
    g_screenH = GetSystemMetrics(SM_CYSCREEN);

    const int bytes = g_screenW * g_screenH * 4;
    g_pixels.resize(bytes, 0);
    g_scratch.resize(bytes, 0);

    g_bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    g_bmi.bmiHeader.biWidth       =  g_screenW;
    g_bmi.bmiHeader.biHeight      = -g_screenH;
    g_bmi.bmiHeader.biPlanes      = 1;
    g_bmi.bmiHeader.biBitCount    = 32;
    g_bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdcScreen = GetDC(nullptr);
    s_hdcMem = CreateCompatibleDC(hdcScreen);
    s_hbm    = CreateCompatibleBitmap(hdcScreen, g_screenW, g_screenH);
    SelectObject(s_hdcMem, s_hbm);
    ReleaseDC(nullptr, hdcScreen);
}

void GDI_CaptureScreen(HDC hdcScreen) {
    DwmFlush();

    BitBlt(s_hdcMem, 0, 0, g_screenW, g_screenH,
           hdcScreen, 0, 0, SRCCOPY | CAPTUREBLT);

    GetDIBits(s_hdcMem, s_hbm, 0, (UINT)g_screenH,
              g_pixels.data(), &g_bmi, DIB_RGB_COLORS);
}

void GDI_PresentBuffer(HDC hdcDest) {
    SetDIBitsToDevice(
        hdcDest,
        0, 0,
        (DWORD)g_screenW,
        (DWORD)g_screenH,
        0, 0,
        0, (UINT)g_screenH,
        g_pixels.data(),
        &g_bmi,
        DIB_RGB_COLORS
    );
}

void cleanup() {
    if (s_hbm)    { DeleteObject(s_hbm);    s_hbm    = nullptr; }
    if (s_hdcMem) { DeleteDC(s_hdcMem);     s_hdcMem = nullptr; }
    g_pixels.clear();
    g_scratch.clear();
}
