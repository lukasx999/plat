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
    Vector2 m_velocity;
    Vector2 m_acceleration;
    static constexpr int m_size = 100;

public:
    Player()
        : m_position(WIDTH/2.0f, HEIGHT-m_size/2.0f)
        , m_velocity(0, 0)
        , m_acceleration(0, 0)
    { }

    [[nodiscard]] bool is_on_floor() const {
        return m_position.y == HEIGHT-m_size/2.0f;
    }

    void draw() const {
        DrawRectanglePro({ m_position.x, m_position.y, m_size, m_size }, { m_size/2.0f, m_size/2.0f }, 0, BLUE);
        DrawCircleV(m_position, 10, RED);
        DrawLineEx(m_position, Vector2Add(m_position, Vector2Scale(m_velocity, 100.0f)), 5, RED);
        DrawLineEx(m_position, Vector2Add(m_position, Vector2Scale(m_acceleration, 100.0f)), 5, GREEN);
    }

    void update() {
        m_velocity = Vector2Add(m_velocity, m_acceleration);
        m_position = Vector2Add(m_position, m_velocity);

        if (is_on_floor()) {
            m_velocity.y = 0;
            m_acceleration.y = 0;
        } else {
            // gravity
            m_acceleration.y += 1;
        }

        m_position.y = Clamp(m_position.y, 0, HEIGHT-m_size/2.0f);
        m_position.x = Clamp(m_position.x, 0, WIDTH-m_size/2.0f);

        std::println("-------------------");
        std::println("m_position: {}, {}", m_position.x, m_position.y);
        std::println("m_velocity: {}, {}", m_velocity.x, m_velocity.y);
        std::println("m_acceleration: {}, {}", m_acceleration.x, m_acceleration.y);
    }

    void jump() {
        m_acceleration.y = -10;
    }

    void move(Direction dir) {
        switch (dir) {
            case Direction::Right:
                m_acceleration.x += 0.01;
            break;
            case Direction::Left:
                m_acceleration.x -= 0.01;
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
            player.draw();
            player.update();

            if (IsKeyPressed(KEY_SPACE)) {
                std::println("jump");
                player.jump();
            }

            if (IsKeyDown(KEY_D)) {
                player.move(Direction::Right);
            }

            if (IsKeyDown(KEY_A)) {
                player.move(Direction::Left);
            }

        }
        EndDrawing();
    }
    CloseWindow();

    return EXIT_SUCCESS;
}
