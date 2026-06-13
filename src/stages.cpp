#include <stages.hpp>
#include <GDI.h>
#include <string>
#include <random>

StageManager::StageManager(float stageDuration) : m_duration(stageDuration) {
    m_stages = {
        {
            { gdihsv },
            [](int t) -> unsigned char {
                return t >> 5 | (t << 3) + 12 * t * (t >> 13 | (t >> 1 | t >> 10 | t >> 2) & t >> 8);
            }
        },

        {
            { gdi_abstract_rectangles, gdi_invert },
            [](int t) -> unsigned char {
                return 43 * (t >> 41 | t >> 2);
            }
        },

        {
            { gdihsv, gdi_pixelate, gdi_scanlines, gdi_invert, gdi_cursorsymbol },
            [](int t) -> unsigned char {
                return t ^ t >> 4 ^ (t >> 11 + (t >> 16) % 3) % 16 * t ^ 3 * t;
            }
        },

        {
            { gdihsv, gdi_radial_zoom, gdi_scanlines, gdi_cursorsymbol  },
            [](int t) -> unsigned char {
                return t >> 5 | t << 4 | t & 1023 ^ 1981 | t - 67 >> 4;
            }
        },

        {
            { gdihsv, gdi_invert_pulse, gdi_radial_zoom, gdi_scanlines, gdi_cursorsymbol },
            [](int t) -> unsigned char {
                return 10 * (t & 5 * t | t >> 6 | (t & 32768 ? -6 * t / 7 : (t & 65536 ? -9 * t & 100 : -9 * (t & 100)) / 11));
            }
        },

        {
            { gdihsv, gdi_glitch, gdi_invert_pulse, gdi_radial_zoom, gdi_cursorsymbol },
            [](int t) -> unsigned char {
                return t * ((t >> 13) % 8 + 1) & t * ((t >> 10 & 4 | t >> 12 & 6 | t >> 16 & 12) & 6 | (t >> 18) % 8) - (-t >> 16) % 4 * 9;
            }
        },

        {
            { gdihsv, gdi_glitch, gdi_pixelate, gdi_invert_pulse, gdi_radial_zoom, gdi_scanlines, gdi_invert, gdi_cursorsymbol},
            [](int t) -> unsigned char {
                return t >> 2 | (0xC098C098 >> (t >> 10)) * t | t >> 10;
            }
        },

        // final stage -> death

        {
            { gdi_screenmelt },
            [](int t) -> unsigned char {
                return (
                (((t*(t>>9|t>>13))&255)^((t>>5)*(t>>7)))
                +
                ((t>>((t>>14)&3))|(t*11))
                ^
                ((t*3)&(t>>10))
                );
            }
        }
    };
}

static const wchar_t* s_msgs[] = {
    L"hgvcuj",
    L"success",
    L"HTTP 200 -> https://facebook.com/hb", // INSANE reference...
    L"Attempted to index nil with '__index' : hgvcuj.exe, line 30",
    L"Attempted to use luaL_checkstring on invalid lua_State*",
    L"hgvcuj.exe",
    L"HTTP 200 -> http://127.0.0.1:200200/hgvcuj",
};

thread_local int t_posX = 0;
thread_local int t_posY = 0;

LRESULT CALLBACK MessageBoxHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HCBT_ACTIVATE) {
        HWND hwndMsgBox = (HWND)wParam;
        MoveWindow(hwndMsgBox, t_posX, t_posY, 300, 150, TRUE);
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void StageManager::maybe_spawn_error(float total_time) {
    if (total_time - m_lastSpawn < 1.f) return;
    m_lastSpawn = total_time;

    std::wstring title = L"hgvcuj.exe";
    std::wstring msg   = s_msgs[rand() % 6]; 

    std::thread([title, msg]() {
        srand(GetTickCount() ^ GetCurrentThreadId());

        int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        t_posX = rand() % (screenWidth - 300);
        t_posY = rand() % (screenHeight - 150);

        HHOOK hHook = SetWindowsHookExW(WH_CBT, MessageBoxHookProc, nullptr, GetCurrentThreadId());
        MessageBoxW(nullptr, msg.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
        UnhookWindowsHookEx(hHook);
    }).detach();
}

void StageManager::update(HDC hdc, float t, Bytebeat& bb) {
    if (t - m_lastSwitch >= m_duration) {
        m_current    = (m_current + 1) % (int)m_stages.size();
        m_lastSwitch = t;
        m_lastSpawn  = -999.f; 
    }

    if (m_current != m_last) {
        bb.stop();
        bb.play(m_stages[m_current].beat);
        m_last = m_current;
    }

    for (auto& fn : m_stages[m_current].effects)
        fn(hdc, t);

    if (m_current >= 4 && m_current < (int)m_stages.size() - 1)
        maybe_spawn_error(t);
}

bool StageManager::is_finished() const {
    return (m_current == (int)m_stages.size() - 1) && (GetTickCount() / 1000.f - m_lastSwitch >= m_duration);
}
