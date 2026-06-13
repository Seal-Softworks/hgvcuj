#include "GDI.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstring>

void gdi_glitch(HDC hdc, float t) {
    GDI_CaptureScreen(hdc);

    uint8_t* src = g_pixels.data();
    uint8_t* dst = g_scratch.data();
    std::memcpy(dst, src, g_screenW * g_screenH * 4);

    int bands = 12 + (int)(std::sinf(t * 3.f) * 8.f);
    int bandH = g_screenH / bands;

    for (int b = 0; b < bands; b++) {
        float glitchAmt = std::sinf(t * 17.f + b * 2.3f);
        if (std::fabsf(glitchAmt) < 0.4f) continue;

        int shift = (int)(glitchAmt * g_screenW * 0.08f);
        int y0 = b * bandH;
        int y1 = std::min(y0 + bandH, g_screenH);

        for (int y = y0; y < y1; y++) {
            for (int x = 0; x < g_screenW; x++) {
                int sx = (x + shift + g_screenW) % g_screenW;
                int di = (y * g_screenW + x)  * 4;
                int si = (y * g_screenW + sx) * 4;
                dst[di+0] = src[si+0];
                dst[di+1] = src[si+1];
                dst[di+2] = src[si+2];
                dst[di+3] = src[si+3];
            }
        }
    }

    std::memcpy(src, dst, g_screenW * g_screenH * 4);
    GDI_PresentBuffer(hdc);
}

void gdi_pixelate(HDC hdc, float t) {
    GDI_CaptureScreen(hdc);

    int block = 4 + (int)(std::fabsf(std::sinf(t * 0.8f)) * 28.f);
    uint8_t* px = g_pixels.data();

    for (int y = 0; y < g_screenH; y += block) {
        for (int x = 0; x < g_screenW; x += block) {
            int idx = (y * g_screenW + x) * 4;
            uint8_t b = px[idx+0], g = px[idx+1], r = px[idx+2];

            for (int dy = 0; dy < block && y+dy < g_screenH; dy++) {
                for (int dx = 0; dx < block && x+dx < g_screenW; dx++) {
                    int i = ((y+dy) * g_screenW + (x+dx)) * 4;
                    px[i+0] = b; px[i+1] = g; px[i+2] = r;
                }
            }
        }
    }

    GDI_PresentBuffer(hdc);
}

void gdi_invert_pulse(HDC hdc, float t) {
    GDI_CaptureScreen(hdc);

    float strength = (std::sinf(t * 2.f) + 1.f) * 0.5f; // 0..1
    uint8_t* px = g_pixels.data();
    int total = g_screenW * g_screenH;

    for (int i = 0; i < total; i++, px += 4) {
        px[0] = (uint8_t)(px[0] + (255 - 2*px[0]) * strength);
        px[1] = (uint8_t)(px[1] + (255 - 2*px[1]) * strength);
        px[2] = (uint8_t)(px[2] + (255 - 2*px[2]) * strength);
    }

    GDI_PresentBuffer(hdc);
}

void gdi_scanlines(HDC hdc, float t) {
    GDI_CaptureScreen(hdc);

    int   spacing = 4;
    int   offset  = (int)(t * 60.f) % spacing;
    float dark    = 0.35f;
    uint8_t* px   = g_pixels.data();

    for (int y = 0; y < g_screenH; y++) {
        if ((y + offset) % spacing != 0) continue;
        for (int x = 0; x < g_screenW; x++) {
            int i = (y * g_screenW + x) * 4;
            px[i+0] = (uint8_t)(px[i+0] * dark);
            px[i+1] = (uint8_t)(px[i+1] * dark);
            px[i+2] = (uint8_t)(px[i+2] * dark);
        }
    }

    GDI_PresentBuffer(hdc);
}

void gdi_radial_zoom(HDC hdc, float t) {
    GDI_CaptureScreen(hdc);

    uint8_t* src = g_pixels.data();
    uint8_t* dst = g_scratch.data();

    float zoom   = 1.f + 0.05f * std::sinf(t * 1.5f);
    float cx     = g_screenW * 0.5f;
    float cy     = g_screenH * 0.5f;

    for (int y = 0; y < g_screenH; y++) {
        for (int x = 0; x < g_screenW; x++) {
            float sx = cx + (x - cx) / zoom;
            float sy = cy + (y - cy) / zoom;

            int ix = std::clamp((int)sx, 0, g_screenW - 1);
            int iy = std::clamp((int)sy, 0, g_screenH - 1);

            int di = (y  * g_screenW + x)  * 4;
            int si = (iy * g_screenW + ix) * 4;

            dst[di+0] = src[si+0];
            dst[di+1] = src[si+1];
            dst[di+2] = src[si+2];
            dst[di+3] = src[si+3];
        }
    }

    std::memcpy(src, dst, g_screenW * g_screenH * 4);
    GDI_PresentBuffer(hdc);
}

void gdi_abstract_rectangles(HDC hdc, float t)
{
    static COLORREF rndclr[] = { 0xFF0000, 0xFF00BC, 0x00FF33, 0xFFF700, 0x00FFEF };

    int cx = g_screenW / 2;
    int cy = g_screenH / 2;

    int layers = 12;
    for (int i = 0; i < layers; i++) {
        float depth   = (float)i / layers;
        float angle   = t * 1.5f + depth * 3.14159f * 2.f;
        float scale   = 1.f - depth * 0.85f;             

        int hw = (int)(g_screenW * 0.5f * scale);
        int hh = (int)(g_screenH * 0.5f * scale);

        float cosA = std::cosf(angle);
        float sinA = std::sinf(angle);

        float corners[4][2] = {
            { -hw * cosA - (-hh) * sinA, -hw * sinA + (-hh) * cosA },
            {  hw * cosA - (-hh) * sinA,  hw * sinA + (-hh) * cosA },
            {  hw * cosA -   hh  * sinA,  hw * sinA +   hh  * cosA },
            { -hw * cosA -   hh  * sinA, -hw * sinA +   hh  * cosA },
        };

        POINT pts[5];
        for (int j = 0; j < 4; j++) {
            pts[j] = { cx + (int)corners[j][0], cy + (int)corners[j][1] };
        }
        pts[4] = pts[0]; 

        COLORREF clr = rndclr[i % 5];
        HPEN pen = CreatePen(PS_SOLID, 2, clr);
        HGDIOBJ old = SelectObject(hdc, pen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Polyline(hdc, pts, 5);

        SelectObject(hdc, old);
        DeleteObject(pen);

        if (i == 0) {
            POINT lp[3];
            float warpAngle = t * 0.8f;
            float wc = std::cosf(warpAngle) * 30.f;
            float ws = std::sinf(warpAngle) * 30.f;
            lp[0] = { (int)(cx - hw + wc), (int)(cy - hh + ws) };
            lp[1] = { (int)(cx + hw + wc), (int)(cy - hh - ws) };
            lp[2] = { (int)(cx - hw - wc), (int)(cy + hh + ws) };
            PlgBlt(hdc, lp, hdc, 0, 0, g_screenW, g_screenH, nullptr, 0, 0);
        }
    }
}

void gdi_invert(HDC hdc, float t) {
    if ((int)t % 2 == 0) return;

    GDI_CaptureScreen(hdc);

    uint8_t* px    = g_pixels.data();
    int      total = g_screenW * g_screenH;

    for (int i = 0; i < total; i++, px += 4) {
        px[0] = 255 - px[0];
        px[1] = 255 - px[1];
        px[2] = 255 - px[2];
    }

    GDI_PresentBuffer(hdc);
}

struct ActiveSymbol {
    POINT position;
    HICON iconHandle;
};

void gdi_cursorsymbol(HDC hdc, float t) {
    static std::vector<ActiveSymbol> s_activeSymbols;
    
    static const LPCWSTR icons[] = {
        IDI_WARNING, IDI_ERROR, IDI_INFORMATION,
        IDI_QUESTION, IDI_SHIELD, IDI_APPLICATION
    };
    
    static float s_lastPick = -999.f;

    if (t - s_lastPick >= 0.15f) {
        s_lastPick = t;

        POINT currentCursor;
        GetCursorPos(&currentCursor);

        HWND hwnd = WindowFromDC(hdc);
        if (hwnd) {
            ScreenToClient(hwnd, &currentCursor);
        }

        int randomIdx = rand() % 6;
        HICON newIcon = (HICON)LoadImageW(
            nullptr, 
            MAKEINTRESOURCEW((ULONG_PTR)icons[randomIdx]), 
            IMAGE_ICON, 64, 64, LR_SHARED
        );

        if (newIcon) {
            ActiveSymbol symbolData = { currentCursor, newIcon };
            s_activeSymbols.push_back(symbolData);
        }
    }

    while (s_activeSymbols.size() > 20) {
        s_activeSymbols.erase(s_activeSymbols.begin());
    }

    for (const auto& symbol : s_activeSymbols) {
        DrawIconEx(
            hdc, 
            symbol.position.x - 32,
            symbol.position.y - 32,
            symbol.iconHandle, 
            64, 64, 0, nullptr, DI_NORMAL
        );
    }
}

void gdi_screenmelt(HDC hdc, float t)
{
    static int s_scrW = GetSystemMetrics(SM_CXSCREEN);
    static int s_scrH = GetSystemMetrics(SM_CYSCREEN);
    
    const int stripWidth = 4; 
    static int totalStrips = s_scrW / stripWidth;

    static HDC s_hMemDC = nullptr;
    static HBITMAP s_hBitmap = nullptr;
    static std::vector<int> s_stripOffsets; 
    static std::vector<int> s_stripSpeeds;  
    static float s_lastUpdate = -999.f;

    if (!s_hMemDC) {
        s_hMemDC = CreateCompatibleDC(hdc);
        s_hBitmap = CreateCompatibleBitmap(hdc, s_scrW, s_scrH);
        SelectObject(s_hMemDC, s_hBitmap);

        HDC hdcScreen = GetDC(nullptr);
        BitBlt(s_hMemDC, 0, 0, s_scrW, s_scrH, hdcScreen, 0, 0, SRCCOPY);
        ReleaseDC(nullptr, hdcScreen);

        s_stripOffsets.resize(totalStrips, 0);
        s_stripSpeeds.resize(totalStrips, 0);
        for (int i = 0; i < totalStrips; ++i) {
            s_stripSpeeds[i] = (rand() % 5) + 2; 
        }
    }

    if (s_lastUpdate < 0) s_lastUpdate = t;
    if (t - s_lastUpdate < 0.016f) { 
        BitBlt(hdc, 0, 0, s_scrW, s_scrH, s_hMemDC, 0, 0, SRCCOPY);
        return; 
    }
    s_lastUpdate = t;

    for (int i = 0; i < totalStrips; ++i) {
        int xPos = i * stripWidth;
        
        if (rand() % 20 == 0) {
            s_stripSpeeds[i] += (rand() % 3) - 1;
            if (s_stripSpeeds[i] < 1) s_stripSpeeds[i] = 1;
        }

        int currentDrop = s_stripSpeeds[i];
        s_stripOffsets[i] += currentDrop;

        BitBlt(
            s_hMemDC, 
            xPos, s_stripOffsets[i],
            stripWidth, s_scrH - s_stripOffsets[i], 
            s_hMemDC, 
            xPos, s_stripOffsets[i] - currentDrop, 
            SRCCOPY
        );

        RECT topGap = { xPos, s_stripOffsets[i] - currentDrop, xPos + stripWidth, s_stripOffsets[i] };
        HBRUSH hOozeBrush = CreateSolidBrush(RGB(10, 10, 10));
        FillRect(s_hMemDC, &topGap, hOozeBrush);
        DeleteObject(hOozeBrush);
    }

    BitBlt(hdc, 0, 0, s_scrW, s_scrH, s_hMemDC, 0, 0, SRCCOPY);
}

