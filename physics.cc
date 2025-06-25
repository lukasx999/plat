#include "physics.h"

void PhysicsEntity::update() {
    m_state = m_new_state;
    m_new_state = EntityState::Idle;

    if (m_is_grounded) {
        m_jump_count = m_max_jumps;
        m_dash.reset();
        m_speed.y = 0;
    } else {
        apply_gravity();
    }

    update_position();

    if (m_dash.has_ended()) {
        m_speed.x = 0;
    } else {
        // dont drag the player down while dashing
        m_speed.y = 0;
    }

    on_update();
}

void PhysicsEntity::dash() {

    // stop players from spamming the dash button
    if (!m_dash.can_dash()) return;

    m_speed.x = m_dashing_speed;
    m_dash.start();

    if (m_direction == MovementDirection::Left)
        m_speed.x *= -1;

    on_dash();
}

void PhysicsEntity::jump() {
    // TODO: scale jump intensity with duration of button press
    // BUG: all jumps available when walking off ledge
    if (!m_jump_count) return;
    m_jump_count--;
    m_speed.y = -m_jumping_speed;

    on_jump();
}

void PhysicsEntity::move(MovementDirection direction) {
    m_direction = direction;

    switch (direction) {
        case MovementDirection::Left:
            m_new_state = EntityState::MovingLeft;
            m_position.x -= m_movement_speed * GetFrameTime();
            break;

        case MovementDirection::Right:
            m_new_state = EntityState::MovingRight;
            m_position.x += m_movement_speed * GetFrameTime();
            break;
    }

    on_move(direction);
}


void PhysicsEntity::resolve_collisions(std::span<const Item> items) {
    m_is_grounded = false;

    for (const auto &item : items) {
        if (item.m_is_blocking) {

            auto hitbox = item.m_hitbox;
            float delta_ver = m_speed.y * GetFrameTime();
            float delta_hor = std::abs(m_speed.x) * GetFrameTime();

            // let the player clip a bit into the floor when grounded, to prevent
            // oscillation of grounding state
            // also prevent the player from teleporting down after walking off a ledge
            float clip = 1;

            handle_collision_left(hitbox, clip, delta_hor);
            handle_collision_right(hitbox, clip, delta_hor);
            handle_collision_top(hitbox, clip, delta_ver);
            handle_collision_bottom(hitbox, delta_ver);
        }

    }

}
