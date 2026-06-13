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
void gdi_glitch(HDC hdc, float t);
void gdi_pixelate(HDC hdc, float t);
void gdi_invert_pulse(HDC hdc, float t);
void gdi_scanlines(HDC hdc, float t);
void gdi_radial_zoom(HDC hdc, float t);
void gdi_abstract_rectangles(HDC hdc, float t);
void gdi_invert(HDC hdc, float t);
void gdi_cursorsymbol(HDC hdc, float t);
void gdi_screenmelt(HDC hdc, float t);
