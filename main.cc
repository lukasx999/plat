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





class Player {
    enum class MovementDirection { Left, Right };

    float m_speed = 0;
    bool m_is_grounded = false;
    Rectangle m_origin;
    Vector2 m_position;
    const std::function<float()> m_get_dt;
    const Texture2D m_tex;
    MovementDirection m_direction = MovementDirection::Left;
    static constexpr int m_gravity = 1000;
    static constexpr float m_movement_speed = 600;
    static constexpr float m_jumping_speed = 900;
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
    static constexpr float m_texture_scale = 5;
    static constexpr float m_walk_delay_secs = 0.1;

public:
    Player(Vector2 position, std::function<float()> get_dt)
        : m_origin(m_sprites_running[0])
        , m_position(position)
        , m_get_dt(get_dt)
        , m_tex(LoadTexture("./assets/sprites/knight.png"))
    { }

    [[nodiscard]] Rectangle get_hitbox() const {
        return {
            m_position.x - m_origin.width*m_texture_scale/2.0f,
            m_position.y - m_origin.height*m_texture_scale/2.0f,
            m_origin.width*m_texture_scale,
            m_origin.height*m_texture_scale,
        };
    }

    void draw() const {

        auto origin = m_origin;

        switch (m_direction) {
            case MovementDirection::Left:
                // flip texture
                origin.width *= -1;
                break;
            case MovementDirection::Right:
                ;
                break;
        }

        DrawTexturePro(m_tex, origin, get_hitbox(), { 0, 0 }, 0, WHITE);

#ifdef DEBUG
        DrawTexture(m_tex, WIDTH-m_tex.width, 0, WHITE);
        for (const auto &sprite : m_sprites_running) {
            DrawRectangleLinesEx({ sprite.x+(WIDTH-m_tex.width), sprite.y, sprite.width, sprite.height }, 1, BLUE);
        }
        DrawRectangleLinesEx({ m_origin.x+(WIDTH-m_tex.width), m_origin.y, m_origin.width, m_origin.height }, 1, RED);

        DrawRectangleLinesEx(get_hitbox(), 1, BLACK);

        DrawText(std::format("pos: x: {}, y: {}", trunc(m_position.x), trunc(m_position.y)).c_str(), 0, 0, 50, WHITE);
        DrawText(std::format("speed: {}:", trunc(m_speed)).c_str(), 0, 50, 50, WHITE);
        DrawText(std::format("grounded: {}", m_is_grounded ? "yes" : "no").c_str(), 0, 100, 50, WHITE);
#endif // DEBUG

    }

    void update() {
        if (m_is_grounded) {
            m_speed = 0;
        } else {
            m_speed += m_gravity * m_get_dt();
            m_position.y += m_speed * m_get_dt();
        }
    }

    void resolve_collisions(const std::span<const Item> items) {
        m_is_grounded = false;

        for (const auto &item : items) {
            if (item.m_is_blocking) {

                const auto hitbox = item.m_hitbox;
                const float delta_ver = m_speed * m_get_dt();
                const float delta_hor = m_movement_speed * m_get_dt();

                // let the player clip a bit into the floor when grounded, to prevent
                // oscillation of grounding state
                // also prevent the player from teleporting down after walking off a ledge
                const float clip = 1;

                const Rectangle left = {
                    hitbox.x - delta_hor,
                    hitbox.y + clip,
                    delta_hor,
                    hitbox.height - clip*2,
                };

                const Rectangle right = {
                    hitbox.x + hitbox.width,
                    hitbox.y + clip,
                    delta_hor,
                    hitbox.height - clip*2,
                };

                const Rectangle top = {
                    // remove some padding from each edge to prevent
                    // instant teleportation when jumping up and edge
                    // and moving to the right
                    hitbox.x + delta_hor,
                    hitbox.y - delta_ver,
                    hitbox.width - delta_hor*2,
                    delta_ver,
                };

                const Rectangle bottom = {
                    hitbox.x,
                    hitbox.y + hitbox.height,
                    hitbox.width,
                    delta_ver,
                };

#ifdef DEBUG_COLLISIONS
                DrawRectangleRec(left,   PURPLE);
                DrawRectangleRec(right,  PURPLE);
                DrawRectangleRec(top,    PURPLE);
                DrawRectangleRec(bottom, PURPLE);
#endif // DEBUG_COLLISIONS

                if (CheckCollisionRecs(get_hitbox(), top)) {
                    m_is_grounded = true;
                    m_position.y = hitbox.y - get_hitbox().height/2.0f + clip;
                }

                if (CheckCollisionRecs(get_hitbox(), left)) {
                    m_position.x = hitbox.x - get_hitbox().width/2.0f;
                }

                if (CheckCollisionRecs(get_hitbox(), right)) {
                    m_position.x = hitbox.x + hitbox.width + get_hitbox().width/2.0f;
                }

                if (CheckCollisionRecs(get_hitbox(), bottom)) {
                    // BUG: overshoot on high jump speeds
                    // m_position.y = hitbox.y + hitbox.height + get_hitbox().height/2.0f;
                    m_speed = 0;
                }

            }

        }

    }

    void jump() {
        if (!m_is_grounded) return;
        m_speed = -m_jumping_speed;
    }

    void move_right() {
        m_direction = MovementDirection::Right;
        m_position.x += m_movement_speed * m_get_dt();
        walk();
    }

    void move_left() {
        m_direction = MovementDirection::Left;
        m_position.x -= m_movement_speed * m_get_dt();
        walk();
    }

private:
    void walk() {
        static int sprite_idx = 0;
        static float fut = 0;
        if (GetTime() > fut) {
            sprite_idx++;
            sprite_idx %= m_sprites_running.size();
            m_origin = m_sprites_running[sprite_idx];
            fut = GetTime() + m_walk_delay_secs;
        }
    }

};

static void handle_input(Player &player) {

    if (IsKeyPressed(KEY_SPACE)) {
        player.jump();
    }

    if (IsKeyDown(KEY_D)) {
        player.move_right();
    }

    if (IsKeyDown(KEY_A)) {
        player.move_left();
    }

}

int main() {

    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "2D Game");

    const int floor_height = 200;
    const Item floor({ 0, HEIGHT-floor_height, WIDTH, floor_height }, GRAY, true, "floor");
    const Item bg({ 0, 0, WIDTH, HEIGHT }, DARKGRAY, false, "bg");

    const std::array items {
        bg,
        floor,
        Item({ 300, 300, 300, 200 }, RED, true, "red"),
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
