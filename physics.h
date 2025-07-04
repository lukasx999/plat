#pragma once

#include <cassert>
#include <functional>
#include <print>
#include <span>

#include <raylib.h>
#include <raymath.h>

#include "main.h"



enum class MovementDirection { Left, Right };

enum class EntityState {
    MovingLeft,
    MovingRight,
    Idle,
};

[[nodiscard]] constexpr static auto stringify_state(EntityState state) {
    switch (state) {
        case EntityState::MovingLeft:
            return "MovingLeft";
        case EntityState::MovingRight:
            return "MovingRight";
        case EntityState::Idle:
            return "Idle";
    }
}

class Dash {
    int m_dash_count = 0;
    float m_dash_time = 0;
    static constexpr float m_dash_cooldown_secs = 0.2;
    static constexpr float m_dash_duration_secs = 0.1;
    static constexpr int m_max_dashes = 2;

public:
    Dash() = default;

    [[nodiscard]] int dash_count() const {
        return m_dash_count;
    }

    [[nodiscard]] bool has_ended() const {
        return GetTime() > m_dash_time;
    }

    [[nodiscard]] bool can_dash() const {
        bool cooldown_over = GetTime() > m_dash_time + m_dash_cooldown_secs;
        bool any_dashes_left = m_dash_count;
        return cooldown_over && any_dashes_left;
    }

    void start() {
        m_dash_count--;
        m_dash_time = GetTime() + m_dash_duration_secs;
    }

    void reset() {
        m_dash_count = m_max_dashes;
    }

};

class PhysicsEntity {
    Dash m_dash;
    Vector2 m_position;
    Vector2 m_speed { };
    bool m_is_grounded = false;
    MovementDirection m_direction = MovementDirection::Right;
    EntityState m_new_state = EntityState::Idle;
    EntityState m_state = EntityState::Idle;
    // cannot simplify this to a jumpcount, because the first jump
    // may only be used from the ground and not in the air
    bool m_is_jumping = false;
    int m_jump_count = 0;
    const float m_width;
    const float m_height;
    static constexpr int m_extra_jumps = 1;
    static constexpr int m_gravity = 1000;
    static constexpr float m_movement_speed = 500;
    static constexpr float m_jumping_speed = 700;
    static constexpr float m_dashing_speed = 2000;

public:
    PhysicsEntity(Vector2 position, float width, float height)
        : m_position(position)
        , m_width(width)
        , m_height(height)
    { }

    [[nodiscard]] Vector2 get_position() const {
        return m_position;
    }

    [[nodiscard]] bool is_grounded() const {
        return m_is_grounded;
    }

    [[nodiscard]] Vector2 get_speed() const {
        return m_speed;
    }

    [[nodiscard]] MovementDirection get_direction() const {
        return m_direction;
    }

    [[nodiscard]] EntityState get_state() const {
        return m_state;
    }

    [[nodiscard]] int get_jumpcount() const {
        return m_jump_count;
    }

    [[nodiscard]] int get_dashcount() const {
        return m_dash.dash_count();
    }

    [[nodiscard]] Rectangle get_hitbox() const {
        return {
            m_position.x - m_width  / 2.0f,
            m_position.y - m_height / 2.0f,
            m_width,
            m_height,
        };
    }

    void update();
    void dash();
    void jump();
    void move(MovementDirection direction);
    void resolve_collisions(std::span<const Item> items);

private:
    virtual void on_jump() { }
    virtual void on_dash() { }
    virtual void on_update() { }
    virtual void on_move([[maybe_unused]] MovementDirection direction) { }

    void update_position() {
        m_position += m_speed * GetFrameTime();
    }

    void apply_gravity() {
        m_speed.y += m_gravity * GetFrameTime();
    }

    void handle_collision(Rectangle hitbox, std::function<void()> handler) {

        if (CheckCollisionRecs(get_hitbox(), hitbox))
            handler();

        #ifdef DEBUG
        // TODO:
        // DrawRectangleRec(hitbox, PURPLE);
        #endif // DEBUG

    }

    void handle_collision_top(Rectangle hitbox, float clip, float delta_ver) {

        Rectangle rect { hitbox.x, hitbox.y - delta_ver, hitbox.width, delta_ver };

        handle_collision(rect, [&] {
            m_is_grounded = true;
            m_position.y = hitbox.y - get_hitbox().height/2.0f + clip;
        });
    }

    void handle_collision_bottom(Rectangle hitbox, float delta_ver) {
        // when the player bumps their head by jumping, the
        // speed is negative, therefore delta_ver must be inverted
        Rectangle rect { hitbox.x, hitbox.y + hitbox.height, hitbox.width, -delta_ver };
        handle_collision(rect, [&] { m_speed.y = 0; });
    }

    void handle_collision_left(Rectangle hitbox, float clip, float delta_hor) {

        Rectangle rect = {
            hitbox.x - delta_hor,
            hitbox.y + clip,
            delta_hor,
            hitbox.height - clip,
        };

        handle_collision(rect, [&] {
            m_speed.x = 0;
            m_position.x = hitbox.x - get_hitbox().width/2.0f;
        });
    }

    void handle_collision_right(Rectangle hitbox, float clip, float delta_hor) {

        Rectangle rect = {
            hitbox.x + hitbox.width,
            hitbox.y + clip,
            delta_hor,
            hitbox.height - clip,
        };

        handle_collision(rect, [&] {
            m_speed.x = 0;
            m_position.x = hitbox.x + hitbox.width + get_hitbox().width/2.0f;
        });
    }

};
