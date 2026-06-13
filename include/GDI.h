#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>

extern int g_screenW;
extern int g_screenH;
extern std::vector<uint8_t> g_pixels;
extern std::vector<uint8_t> g_scratch;
extern BITMAPINFO g_bmi;

void GDIInit();
void GDI_CaptureScreen(HDC hdcScreen);
void GDI_PresentBuffer(HDC hdcDest);
void cleanup();

void gdihsv(HDC hdc, float t);
