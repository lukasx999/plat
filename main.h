#pragma once

#include <functional>
#include <string>

#include <raylib.h>
#include <raymath.h>

#define DEBUG

static constexpr int WIDTH = 1600;
static constexpr int HEIGHT = 900;

#define PRINT(x) std::println("{}: {}", #x, (x))

struct Item {
    const Rectangle m_hitbox;
    const Color m_color;
    const bool m_is_blocking;

    Item(Rectangle hitbox, Color color, bool is_blocking)
        : m_hitbox(hitbox)
        , m_color(color)
        , m_is_blocking(is_blocking)
    { }

};

class SpriteAnimation {
    int m_idx = 0;
    float m_next_cycle = 0;
    const float m_delay_secs;
    const int m_max;
    const std::function<float()> m_get_time;

public:
    SpriteAnimation(float delay_secs, int max, std::function<float()> get_time)
        : m_delay_secs(delay_secs)
        , m_max(max)
        , m_get_time(get_time)
    { }

    void reset() {
        m_idx = 0;
    }

    [[nodiscard]] int get() const {
        return m_idx;
    }

    int next() {
        float time = m_get_time();

        if (time > m_next_cycle) {
            m_next_cycle = time + m_delay_secs;
            m_idx++;
            m_idx %= m_max;
        }

        return get();
    }

};
