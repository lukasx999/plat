#include <functional>
#include <print>
#include <vector>
#include <array>

#include <raylib.h>
#include <raymath.h>

static constexpr auto WIDTH = 1600;
static constexpr auto HEIGHT = 900;
static constexpr bool DEBUG = true;

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

void draw_environment(std::span<const Item> items) {
    for (auto &item : items) {
        DrawRectangleRec(item.m_hitbox, item.m_color);
    }
}





class Player {
    Vector2 m_position;
    bool m_is_jumping;
    float m_speed = 0;
    bool m_is_grounded = false;
    const std::function<float()> m_get_dt;
    static constexpr int m_size = 100;
    static constexpr int m_gravity = 1200;
    static constexpr float m_movement_speed = 600;
    static constexpr float m_jumping_speed = 900;

public:
    Player(std::function<float()> get_dt)
        : m_position(WIDTH/2.0f, HEIGHT - m_size/2.0f - 300)
        , m_is_jumping(true)
        , m_get_dt(get_dt)
    { }

    [[nodiscard]] Rectangle get_hitbox() const {
        return { m_position.x - m_size/2.0f, m_position.y - m_size/2.0f, m_size, m_size };
    }

    void draw() const {
        DrawRectangleRec(get_hitbox(), DARKBLUE);
        DrawCircleV(m_position, 10, RED);

        if constexpr (DEBUG) {
            DrawText(std::format("pos: x: {}, y: {}", trunc(m_position.x), trunc(m_position.y)).c_str(), 0, 0, 50, WHITE);
            DrawText(std::format("speed: {}:", trunc(m_speed)).c_str(), 0, 50, 50, WHITE);
            DrawText(std::format("jumping: {}", m_is_jumping ? "yes" : "no").c_str(), 0, 100, 50, WHITE);
            DrawText(std::format("grounded: {}", m_is_grounded ? "yes" : "no").c_str(), 0, 150, 50, WHITE);
        }

    }

    void update() {
        const float dt = m_get_dt();

        if (m_is_grounded) {
            m_is_jumping = false;
            m_speed = 0;

        } else {
            m_speed += m_gravity * dt;
            m_position.y += m_speed * dt;
        }

    }

    void resolve_collisions(std::span<const Item> items) {
        const float dt = m_get_dt();

        m_is_grounded = false;

        for (auto &item : items) {

            if (item.m_is_blocking) {

                const auto hitbox = item.m_hitbox;

                const float diffyu = (m_position.y + m_size/2.0f) - hitbox.y;
                const float diffyd = (hitbox.y + hitbox.height) - (m_position.y - m_size/2.0f);
                const float diffxl = (m_position.x + m_size/2.0f) - hitbox.x;
                const float diffxr = (hitbox.x + hitbox.width) - (m_position.x - m_size/2.0f);

                // BUG: bouncing effect when landing diagonally

                const bool grounded = diffyu + m_speed*dt > 0;

                if (grounded && diffxl > 0 && diffxr > 0 && diffyd > 0) {
                    m_is_grounded = true;
                }

                // if (diffyu < diffyd && diffyu < diffxr && diffyu < diffyd && diffyu > 0) {
                //     m_position.y -= diffyu;
                //     // m_is_grounded = true;
                //     return;
                // }
                //
                // if (diffyd < diffyu && diffyd < diffxl && diffyd < diffxr && diffyd > 0) {
                //     m_position.y += diffyd;
                //     return;
                // }
                //
                // if (diffxl < diffxr && diffxl > 0) {
                //     m_position.x -= diffxl;
                //     return;
                // }
                //
                // if (diffxr < diffxl && diffxr > 0) {
                //     m_position.x += diffxr;
                //     return;
                // }



            }
        }

    }

    void jump() {
        if (m_is_jumping) return;
        m_is_jumping = true;
        m_speed = -m_jumping_speed;
    }

    void move_right() {
        m_position.x += m_movement_speed * m_get_dt();
    }

    void move_left() {
        m_position.x -= m_movement_speed * m_get_dt();
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
    const Item floor({ 0, HEIGHT-floor_height, WIDTH, floor_height }, GRAY, true);
    const Item bg({ 0, 0, WIDTH, HEIGHT }, DARKGRAY, false);

    const std::array items {
        bg,
        floor,
        Item({ 300, 500, 300, 100 }, RED, true),
        Item({ 1100, 600, 300, 100 }, GREEN, true)
    };

    Player player(GetFrameTime);

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
