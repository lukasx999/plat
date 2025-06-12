#include <print>
#include <vector>
#include <iostream>
#include <array>

#include <raylib.h>
#include <raymath.h>

constexpr auto WIDTH = 1600;
constexpr auto HEIGHT = 900;
constexpr bool DEBUG = true;

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

void render_environment(std::span<Item> items) {
    for (auto &item : items) {
        DrawRectangleRec(item.m_hitbox, item.m_color);
    }
}




enum class Direction {
    Right,
    Left,
};

class Player {
    Vector2 m_position;
    bool m_is_jumping;
    float m_speed = 0;
    static constexpr int m_size = 100;
    static constexpr int m_gravity = 1200;
    static constexpr float m_movement_speed = 600;
    static constexpr float m_jumping_speed = 900;

public:
    Player()
        : m_position(WIDTH/2.0f, HEIGHT-m_size/2.0f)
        , m_is_jumping(false)
    { }

    [[nodiscard]] bool is_grounded() const {
        return m_position.y >= HEIGHT-m_size/2.0f;
    }

    [[nodiscard]] Rectangle get_hitbox() const {
        return {
            m_position.x - m_size/2.0f,
            m_position.y - m_size/2.0f,
            m_size,
            m_size,
        };
    }

    void draw() const {
        DrawRectangleRec(get_hitbox(), DARKBLUE);
        DrawCircleV(m_position, 10, RED);

        if constexpr (DEBUG) {
            DrawText(std::format("pos: x: {}, y {}:", trunc(m_position.x), trunc(m_position.y)).c_str(), 0, 0, 50, WHITE);
            DrawText(std::format("speed: {}:", trunc(m_speed)).c_str(), 0, 50, 50, WHITE);
            DrawText(std::format("jumping: {}", m_is_jumping ? "yes" : "no").c_str(), 0, 100, 50, WHITE);
        }

    }

    void update(float dt) {
        m_position.y = Clamp(m_position.y, 0, HEIGHT-m_size/2.0f);
        m_position.x = Clamp(m_position.x, 0, WIDTH-m_size/2.0f);

        if (m_is_jumping) {
            m_position.y += m_speed * dt;
        }

        if (is_grounded()) {
            m_is_jumping = false;
            m_speed = 0;
        } else {
            m_speed += m_gravity * dt;
        }

    }

    void jump() {
        if (m_is_jumping) return;
        m_is_jumping = true;
        m_speed = -m_jumping_speed;
    }

    void resolve_collisions(std::span<Item> items) {

        for (auto &item : items) {
            const bool collision = CheckCollisionRecs(get_hitbox(), item.m_hitbox);

            if (collision && item.m_is_blocking) {

                if constexpr (DEBUG) {
                    DrawText("collision", 0, 150, 50, RED);
                }

                const auto diffyu = (m_position.y + m_size/2.0f) - item.m_hitbox.y;
                const auto diffyd = (item.m_hitbox.y + item.m_hitbox.height) - (m_position.y - m_size/2.0f);
                const auto diffxl = (m_position.x + m_size/2.0f) - item.m_hitbox.x;
                const auto diffxr = (item.m_hitbox.x + item.m_hitbox.width) - (m_position.x - m_size/2.0f);

                // handling smallest diff first

                if (diffyu < diffyd && diffyu < diffxr && diffyu < diffyd && diffyu > 0) {
                    m_position.y -= diffyu;
                }

                if (diffyd < diffyu && diffyd < diffxl && diffyd < diffxr && diffyd > 0) {
                    m_position.y += diffyd;
                }

                if (diffxl < diffxr && diffxl > 0) {
                    m_position.x -= diffxl;
                }

                if (diffxr < diffxl && diffxr > 0) {
                    m_position.x += diffxr;
                }



            }
        }

    }

    void move(Direction dir, float dt) {

        switch (dir) {
            case Direction::Right:
                m_position.x += m_movement_speed * dt;
                break;
            case Direction::Left:
                m_position.x -= m_movement_speed * dt;
                break;
        }
    }

};


int main(void) {

    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "2D Game");

    std::array items {
        Item({ 0,    0,   WIDTH, HEIGHT }, DARKGRAY, false),
        Item({ 300,  600, 500,   100 },    RED,      true),
        Item({ 1100, 800, 500,   100 },    GREEN,    true),
        Item({ 0, 800, 100,   100 },    YELLOW,    true),
    };

    Player player;

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(BLACK);

            render_environment(items);

            if (IsKeyPressed(KEY_SPACE)) {
                player.jump();
            }

            if (IsKeyDown(KEY_D)) {
                player.move(Direction::Right, GetFrameTime());
            }

            if (IsKeyDown(KEY_A)) {
                player.move(Direction::Left, GetFrameTime());
            }

            player.resolve_collisions(items);
            player.update(GetFrameTime());
            player.draw();

        }
        EndDrawing();
    }
    CloseWindow();

    return EXIT_SUCCESS;
}
