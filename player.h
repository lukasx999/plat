#pragma once

#include <cassert>
#include <functional>
#include <span>
#include <format>

#include <raylib.h>
#include <raymath.h>

#include "physics.h"



class Player : public PhysicsEntity {
    Rectangle m_tex_origin;
    const Texture2D m_tex;
    static constexpr float m_texture_scale = 5;
    static constexpr auto m_tex_path = "./assets/sprites/knight.png";
    static constexpr std::array m_sprites_idle {
        Rectangle { 9, 9, 13, 19 },
        Rectangle { 41, 10, 13, 18 },
        Rectangle { 73, 10, 13, 18 },
        Rectangle { 73, 10, 13, 18 },
        Rectangle { 105, 10, 13, 18 },
    };
    SpriteAnimation m_spritesheet_idle;
    SpriteAnimation m_spritesheet_running;
    static constexpr std::array m_sprites_running {
        Rectangle { 8,   74,  14, 18 },
        Rectangle { 41,  74,  13, 18 },
        Rectangle { 73,  74,  13, 18 },
        Rectangle { 105, 74,  13, 18 },
        Rectangle { 136, 74,  14, 18 },
        Rectangle { 169, 74,  13, 18 },
        Rectangle { 201, 74,  13, 18 },
        Rectangle { 233, 74,  13, 18 },
        Rectangle { 8,   106, 14, 17 },
        Rectangle { 41,  106, 13, 18 },
        Rectangle { 73,  106, 13, 18 },
        Rectangle { 105, 106, 13, 18 },
        Rectangle { 136, 106, 14, 18 },
        Rectangle { 169, 106, 13, 18 },
        Rectangle { 201, 106, 13, 18 },
        Rectangle { 233, 106, 13, 18 },
    };

public:
    Player(Vector2 position)
        : PhysicsEntity(position, 14*m_texture_scale, 19*m_texture_scale)
        , m_tex_origin(m_sprites_idle[0])
        , m_tex(LoadTexture(m_tex_path))
        , m_spritesheet_idle(0.2, m_sprites_idle.size(), GetTime)
        , m_spritesheet_running(0.1, m_sprites_running.size(), GetTime)
    { }

    void update() override {
        PhysicsEntity::update();

        switch (get_state()) {
            case EntityState::MovingLeft:
                m_tex_origin = m_sprites_running[m_spritesheet_running.next()];
                m_spritesheet_idle.reset();
                break;

            case EntityState::MovingRight:
                m_tex_origin = m_sprites_running[m_spritesheet_running.next()];
                m_spritesheet_idle.reset();
                break;

            case EntityState::Idle:
                m_tex_origin = m_sprites_idle[m_spritesheet_idle.next()];
                m_spritesheet_running.reset();
                break;
        }

    }

    void draw() const {
        auto origin = m_tex_origin;

        if (get_direction() == MovementDirection::Left)
            origin.width *= -1;

        DrawTexturePro(m_tex, origin, get_hitbox(), { 0, 0 }, 0, WHITE);

    }

    void draw_debug_ui() const {
        DrawRectangleLinesEx(get_hitbox(), 1, BLACK);
        draw_debug_spritesheet();
    }

private:
    void draw_debug_spritesheet() const {
        DrawTexture(m_tex, WIDTH-m_tex.width, 0, WHITE);

        Rectangle rect = {
            m_tex_origin.x+(WIDTH-m_tex.width),
            m_tex_origin.y,
            m_tex_origin.width,
            m_tex_origin.height,
        };
        DrawRectangleLinesEx(rect, 1, RED);
    }

};
