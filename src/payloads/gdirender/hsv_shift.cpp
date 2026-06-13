#include "GDI.h"

static void RGBtoHSV(uint8_t r, uint8_t g, uint8_t b,
                     float& h, float& s, float& v)
{
    float rf = r / 255.f, gf = g / 255.f, bf = b / 255.f;
    float cmax  = std::max({ rf, gf, bf });
    float cmin  = std::min({ rf, gf, bf });
    float delta = cmax - cmin;

    v = cmax;
    s = (cmax > 0.f) ? delta / cmax : 0.f;

    if (delta < 1e-6f) { h = 0.f; return; }

    if      (cmax == rf) h = 60.f * std::fmod((gf - bf) / delta, 6.f);
    else if (cmax == gf) h = 60.f * ((bf - rf) / delta + 2.f);
    else                 h = 60.f * ((rf - gf) / delta + 4.f);

    if (h < 0.f) h += 360.f;
}

static void HSVtoRGB(float h, float s, float v,
                     uint8_t& r, uint8_t& g, uint8_t& b)
{
    float c  = v * s;
    float x  = c * (1.f - std::fabsf(std::fmod(h / 60.f, 2.f) - 1.f));
    float m  = v - c;
    float rf = 0.f, gf = 0.f, bf = 0.f;

    switch ((int)(h / 60.f) % 6) {
        case 0: rf=c; gf=x; bf=0; break;
        case 1: rf=x; gf=c; bf=0; break;
        case 2: rf=0; gf=c; bf=x; break;
        case 3: rf=0; gf=x; bf=c; break;
        case 4: rf=x; gf=0; bf=c; break;
        default:rf=c; gf=0; bf=x; break;
    }

    r = (uint8_t)(std::min(1.f, rf + m) * 255.f);
    g = (uint8_t)(std::min(1.f, gf + m) * 255.f);
    b = (uint8_t)(std::min(1.f, bf + m) * 255.f);
}

void gdihsv(HDC hdc, float t)
{
    GDI_CaptureScreen(hdc);

    float hueShift = std::fmod(t * 90.f, 360.f);
    float satScale = 1.f + 0.25f * std::sinf(t * 1.3f);

    uint8_t* px    = g_pixels.data();
    int      total = g_screenW * g_screenH;

    for (int i = 0; i < total; ++i, px += 4) {
        float h, s, v;
        RGBtoHSV(px[2], px[1], px[0], h, s, v);

        h = std::fmod(h + hueShift, 360.f);
        s = std::min(1.f, s * satScale);

        uint8_t r, g, b;
        HSVtoRGB(h, s, v, r, g, b);

        px[0] = b;  px[1] = g;  px[2] = r;
    }

    GDI_PresentBuffer(hdc);
}
