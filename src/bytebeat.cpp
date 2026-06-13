#include <bytebeat.hpp>
#include <stdexcept>

Bytebeat::Bytebeat(int sample_rate) : m_sr(sample_rate) {
    m_bufs.resize(BUFS, std::vector<unsigned char>(CHUNK));
    m_hdrs.resize(BUFS, WAVEHDR{});
}

Bytebeat::~Bytebeat() {
    stop();
}

void Bytebeat::play(bytebeat_function fn) {
    stop();

    WAVEFORMATEX wfx{};
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = 1;
    wfx.nSamplesPerSec  = m_sr;
    wfx.wBitsPerSample  = 8;
    wfx.nBlockAlign     = 1;
    wfx.nAvgBytesPerSec = m_sr;

    if (waveOutOpen(&m_hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
        throw std::runtime_error("waveOutOpen failed");

    m_running = true;
    m_thread  = std::thread(&Bytebeat::stream_loop, this, std::move(fn));
}

void Bytebeat::stop() {
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();

    if (m_hwo) {
        waveOutReset(m_hwo);
        for (auto& hdr : m_hdrs)
            if (hdr.dwFlags & WHDR_PREPARED)
                waveOutUnprepareHeader(m_hwo, &hdr, sizeof(WAVEHDR));
        waveOutClose(m_hwo);
        m_hwo = nullptr;
    }

    for (auto& hdr : m_hdrs)
        hdr = WAVEHDR{};
}

bool Bytebeat::isPlaying() const {
    return m_running;
}

void Bytebeat::stream_loop(bytebeat_function fn) {
    int t = 0;

    for (int i = 0; i < BUFS; i++) {
        for (int s = 0; s < CHUNK; s++)
            m_bufs[i][s] = fn(t++);

        WAVEHDR& hdr  = m_hdrs[i];
        hdr.lpData    = reinterpret_cast<LPSTR>(m_bufs[i].data());
        hdr.dwBufferLength = CHUNK;
        hdr.dwFlags   = 0;

        waveOutPrepareHeader(m_hwo, &hdr, sizeof(WAVEHDR));
        waveOutWrite(m_hwo, &hdr, sizeof(WAVEHDR));
    }

    int i = 0;
    while (m_running) {
        WAVEHDR& hdr = m_hdrs[i];

        while ((hdr.dwFlags & WHDR_DONE) == 0 && m_running)
            Sleep(1);

        if (!m_running) break;

        for (int s = 0; s < CHUNK; s++)
            m_bufs[i][s] = fn(t++);

        hdr.dwFlags = 0;
        waveOutPrepareHeader(m_hwo, &hdr, sizeof(WAVEHDR));
        waveOutWrite(m_hwo, &hdr, sizeof(WAVEHDR));

        i = (i + 1) % BUFS;
    }
}