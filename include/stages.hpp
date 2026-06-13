#pragma once
#include "GDI.h"
#include <windows.h>
#include <vector>
#include <functional>

#include <bytebeat.hpp>

using EffectFn = std::function<void(HDC, float)>;

using BytebeatFn = std::function<unsigned char(int t)>;

struct Stage {
    std::vector<EffectFn> effects;
    BytebeatFn            beat;

    Stage(std::vector<EffectFn> fx, BytebeatFn b)
        : effects(std::move(fx)), beat(std::move(b)) {}
};

struct StageManager {
    StageManager(float stageDuration = 30.f);

    void update(HDC hdc, float t, Bytebeat& bb);

    int  current() const { return m_current; }
    const BytebeatFn& currentBeat() const { return m_stages[m_current].beat; }

    bool is_finished() const;

private:
    std::vector<Stage> m_stages;
    float m_duration;
    float m_lastSwitch = 0.f;
    int   m_current    = 0;
    int   m_last       = -1;
    float m_lastSpawn  = -999.f;
    void maybe_spawn_error(float t);
};