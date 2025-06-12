#include <print>

#include <raylib.h>
#include <raymath.h>

constexpr auto WIDTH = 1600;
constexpr auto HEIGHT = 900;

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

    void draw() const {
        DrawRectanglePro(
            { m_position.x, m_position.y, m_size, m_size },
            { m_size/2.0f, m_size/2.0f },
            0,
            BLUE
        );
        DrawCircleV(m_position, 10, RED);
    }

    void update(float dt) {
        m_position.y = Clamp(m_position.y, 0, HEIGHT-m_size/2.0f);
        m_position.x = Clamp(m_position.x, 0, WIDTH-m_size/2.0f);

        if (m_is_jumping) {
            m_position.y += m_speed * dt;
            m_speed += m_gravity * dt;
        }

        if (is_grounded()) {
            m_is_jumping = false;
        }

    }

    void jump() {
        if (m_is_jumping) return;
        m_is_jumping = true;
        m_speed = -m_jumping_speed;
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

    Player player;

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(BLACK);

            if (IsKeyPressed(KEY_SPACE)) {
                player.jump();
            }

            if (IsKeyDown(KEY_D)) {
                player.move(Direction::Right, GetFrameTime());
            }

            if (IsKeyDown(KEY_A)) {
                player.move(Direction::Left, GetFrameTime());
            }

            DrawFPS(0, 0);
            player.update(GetFrameTime());
            player.draw();

        }
        EndDrawing();
    }
    CloseWindow();

    return EXIT_SUCCESS;
}
