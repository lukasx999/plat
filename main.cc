#include <functional>
#include <print>
#include <array>

#include <raylib.h>
#include <raymath.h>

static constexpr auto WIDTH = 1600;
static constexpr auto HEIGHT = 900;
#define DEBUG
#define DEBUG_COLLISIONS

struct Item {
    const Rectangle m_hitbox;
    const Color m_color;
    const bool m_is_blocking;
    const std::string m_name;

    Item(Rectangle hitbox, Color color, bool is_blocking, std::string_view name)
        : m_hitbox(hitbox)
        , m_color(color)
        , m_is_blocking(is_blocking)
        , m_name(name)
    { }

};

void draw_environment(const std::span<const Item> items) {
    for (auto &item : items) {
        DrawRectangleRec(item.m_hitbox, item.m_color);
    }
}

enum class MovementDirection { Left, Right };

enum class EntityState {
    MovingLeft,
    MovingRight,
    Idle,
};

[[nodiscard]] constexpr static
auto stringify_state(EntityState state) {
    switch (state) {
        case EntityState::MovingLeft:
            return "MovingLeft";
        case EntityState::MovingRight:
            return "MovingRight";
        case EntityState::Idle:
            return "Idle";
    }
}

class PhysicsEntity {
    float m_width = 0;
    float m_height = 0;
    const std::function<float()> m_get_dt;
    static constexpr int m_gravity = 1000;
    static constexpr float m_movement_speed = 500;
    static constexpr float m_jumping_speed = 700;
    EntityState m_new_state = EntityState::Idle;

public:
    Vector2 m_position;
    float m_speed = 0;
    bool m_is_grounded = false;
    MovementDirection m_direction = MovementDirection::Right;
    EntityState m_state = EntityState::Idle;

    PhysicsEntity(Vector2 position, std::function<float()> get_dt)
        : m_get_dt(get_dt)
        , m_position(position)
    { }

    void set_width(float width) {
        m_width = width;
    }

    void set_height(float height) {
        m_height = height;
    }

    virtual void update() {
        m_state = m_new_state;
        m_new_state = EntityState::Idle;

        if (m_is_grounded) {
            m_speed = 0;
        } else {
            m_speed += m_gravity * m_get_dt();
            m_position.y += m_speed * m_get_dt();
        }

    }

    virtual void jump() {
        if (!m_is_grounded) return;
        m_speed = -m_jumping_speed;
    }

    virtual void move(MovementDirection direction) {
        m_direction = direction;

        switch (direction) {
            case MovementDirection::Left:
                m_new_state = EntityState::MovingLeft;
                m_position.x -= m_movement_speed * m_get_dt();
                break;

            case MovementDirection::Right:
                m_new_state = EntityState::MovingRight;
                m_position.x += m_movement_speed * m_get_dt();
                break;
        }

    }

    [[nodiscard]] Rectangle get_hitbox() const {
        return {
            m_position.x - m_width/2.0f,
            m_position.y - m_height/2.0f,
            m_width,
            m_height,
        };
    }

    virtual void resolve_collisions(std::span<const Item> items) {
        m_is_grounded = false;

        for (const auto &item : items) {
            if (item.m_is_blocking) {

                auto hitbox = item.m_hitbox;
                float delta_ver = m_speed * m_get_dt();
                float delta_hor = m_movement_speed * m_get_dt();
                // let the player clip a bit into the floor when grounded, to prevent
                // oscillation of grounding state
                // also prevent the player from teleporting down after walking off a ledge
                float clip = 1;

                handle_collision_left(hitbox, clip, delta_hor);
                handle_collision_right(hitbox, clip, delta_hor);
                handle_collision_top(hitbox, clip, delta_hor, delta_ver);
                handle_collision_bottom(hitbox, delta_ver);

            }

        }

    }

private:
    void handle_collision(Rectangle hitbox, std::function<void()> handler) {

        if (CheckCollisionRecs(get_hitbox(), hitbox)) {
            handler();
        }

        #ifdef DEBUG_COLLISIONS
        DrawRectangleRec(hitbox, PURPLE);
        #endif // DEBUG_COLLISIONS
    }

    void handle_collision_top(Rectangle hitbox, float clip, float delta_hor, float delta_ver) {

        float top_delta_mult_factor = 1.3;
        Rectangle rect = {
            // remove some padding from each edge to prevent
            // instant teleportation when jumping up and edge
            // and moving to the right
            hitbox.x + delta_hor*top_delta_mult_factor,
            hitbox.y - delta_ver,
            hitbox.width - delta_hor*top_delta_mult_factor*2,
            delta_ver,
        };

        handle_collision(rect, [&] {
            m_is_grounded = true;
            m_position.y = hitbox.y - get_hitbox().height/2.0f + clip;
        });

    }

    void handle_collision_bottom(const Rectangle hitbox, const float delta_ver) {
        Rectangle rect = {
            hitbox.x,
            hitbox.y + hitbox.height,
            hitbox.width,
            // when the player bumps their head by jumping, the
            // speed is negative, therefore it must be inverted
            -delta_ver,
        };

        handle_collision(rect, [&] {
            m_speed = 0;
        });
    }

    void handle_collision_left(Rectangle hitbox, float clip, float delta_hor) {

        Rectangle rect = {
            hitbox.x - delta_hor,
            hitbox.y + clip,
            delta_hor,
            hitbox.height - clip*2,
        };

        if (CheckCollisionRecs(get_hitbox(), rect)) {
            m_position.x = hitbox.x - get_hitbox().width/2.0f;
        }

        #ifdef DEBUG_COLLISIONS
        DrawRectangleRec(rect, PURPLE);
        #endif // DEBUG_COLLISIONS
    }

    void handle_collision_right(Rectangle hitbox, float clip, float delta_hor) {

        Rectangle rect = {
            hitbox.x + hitbox.width,
            hitbox.y + clip,
            delta_hor,
            hitbox.height - clip*2,
        };

        handle_collision(rect, [&] {
            m_position.x = hitbox.x + hitbox.width + get_hitbox().width/2.0f;
        });

    }

};

class SpriteAnimation {
    int m_idx = 0;
    const float m_delay_secs;
    const int m_max;

public:
    SpriteAnimation(float delay_secs, int max)
        : m_delay_secs(delay_secs)
        , m_max(max)
    { }

    void reset() {
        m_idx = 0;
    }

    [[nodiscard]] int get() const {
        return m_idx;
    }

    int next() {
        static float fut = 0;

        if (GetTime() > fut) {
            fut = GetTime() + m_delay_secs;
            m_idx++;
            m_idx %= m_max;
        }

        return get();
    }

};


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
    Player(Vector2 position, std::function<float()> get_dt)
        : PhysicsEntity(position, get_dt)
        , m_tex_origin(m_sprites_idle[0])
        , m_tex(LoadTexture(m_tex_path))
        , m_spritesheet_idle(0.2, m_sprites_idle.size())
        , m_spritesheet_running(0.1, m_sprites_running.size())
    { }

    void update() override {
        PhysicsEntity::update();
        // TODO: make hitbox constant
        set_width(m_tex_origin.width*m_texture_scale);
        set_height(m_tex_origin.height*m_texture_scale);

        switch (m_state) {
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
        if (m_direction == MovementDirection::Left)
            origin.width *= -1;
        DrawTexturePro(m_tex, origin, get_hitbox(), { 0, 0 }, 0, WHITE);

        DrawRectangleLinesEx(get_hitbox(), 1, BLACK);

#ifdef DEBUG
        DrawTexture(m_tex, WIDTH-m_tex.width, 0, WHITE);

        Rectangle rect = {
            m_tex_origin.x+(WIDTH-m_tex.width),
            m_tex_origin.y,
            m_tex_origin.width,
            m_tex_origin.height,
        };
        DrawRectangleLinesEx(rect, 1, RED);

        draw_debug_text();

#endif // DEBUG

    }

    void draw_debug_text() const {
        float textsize = 50;
        DrawText(std::format("pos: x: {}, y: {}", trunc(m_position.x), trunc(m_position.y)).c_str(), 0, 0, textsize, WHITE);
        DrawText(std::format("speed: {}:", trunc(m_speed)).c_str(), 0, 50, textsize, WHITE);
        DrawText(std::format("grounded: {}", m_is_grounded ? "yes" : "no").c_str(), 0, 100, textsize, WHITE);
        DrawText(std::format("state: {}", stringify_state(m_state)).c_str(), 0, 150, textsize, WHITE);
    }

};

static void handle_input(Player &player) {

    if (IsKeyPressed(KEY_SPACE)) {
        player.jump();
    }

    if (IsKeyDown(KEY_D)) {
        player.move(MovementDirection::Right);
    }

    if (IsKeyDown(KEY_A)) {
        player.move(MovementDirection::Left);
    }

}

int main() {

    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "2D Game");

    int floor_height = 200;
    Item floor({ 0.0f, static_cast<float>(HEIGHT-floor_height), WIDTH, static_cast<float>(floor_height) }, GRAY, true, "floor");
    Item bg({ 0, 0, WIDTH, HEIGHT }, DARKGRAY, false, "bg");

    std::array items {
        bg,
        floor,
        Item({ 300, 300, 300, 300 }, RED, true, "red"),
        Item({ 1100, 600, 300, 100 }, GREEN, true, "green")
    };

    Player player({ WIDTH/2.0f, HEIGHT - 500 }, GetFrameTime);

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(BLACK);

            handle_input(player);
            draw_environment(items);
            player.resolve_collisions(items);
            player.update();
            player.draw();

        }
        EndDrawing();
    }
    CloseWindow();

    return EXIT_SUCCESS;
}
