#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <functional>
#include <vector>
#include <thread>
#include <atomic> // steel atomic...

#pragma comment(lib, "winmm.lib")

using bytebeat_function = std::function<unsigned char(int t)>;

struct Bytebeat
{
    Bytebeat(int sample_rate = 8000);
    ~Bytebeat();

    void play(bytebeat_function fn); // async
    void stop();
    bool isPlaying() const;

private:
    static constexpr int CHUNK = 4096;
    static constexpr int BUFS = 4;

    int m_sr;
    HWAVEOUT m_hwo = nullptr;
    std::atomic<bool> m_running { false };
    std::thread m_thread;

    std::vector<std::vector<unsigned char>> m_bufs;
    std::vector<WAVEHDR> m_hdrs;

    void stream_loop(bytebeat_function fn);    
};
