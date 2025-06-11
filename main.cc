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
    Vector2 m_counter;
    static constexpr int m_size = 100;

public:
    Player()
        : m_position(WIDTH/2.0f, HEIGHT-m_size/2.0f)
        , m_velocity(0, 0)
        , m_acceleration(0, 0)
    { }

    void update() {
        // m_acceleration = Vector2Add(m_acceleration, m_counter);
        m_velocity     = Vector2Add(m_velocity, m_acceleration);
        m_position     = Vector2Add(m_position, m_velocity);

        m_position.y = Clamp(m_position.y, 0, HEIGHT-m_size/2.0f);
        m_position.x = Clamp(m_position.x, 0, WIDTH-m_size/2.0f);

        std::println("-------------------");
        std::println("m_position: {}, {}", m_position.x, m_position.y);
        std::println("m_velocity: {}, {}", m_velocity.x, m_velocity.y);
        std::println("m_acceleration: {}, {}", m_acceleration.x, m_acceleration.y);
    }

    void draw() const {
        DrawRectanglePro({ m_position.x, m_position.y, m_size, m_size }, { m_size/2.0f, m_size/2.0f }, 0, BLUE);
        DrawCircleV(m_position, 10, RED);
        DrawLineEx(m_position, Vector2Add(m_position, Vector2Scale(m_velocity, 100.0f)), 5, RED);
        DrawLineEx(m_position, Vector2Add(m_position, Vector2Scale(m_acceleration, 100.0f)), 5, GREEN);
    }

    void jump() {
        m_acceleration.y -= 1;
    }

    void apply_force() {
        m_acceleration.y -= 5;
    }

};

int main(void) {

    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(6);
    InitWindow(WIDTH, HEIGHT, "2D Game");

    Player player;

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(BLACK);
            player.draw();
            player.update();

            if (IsKeyDown(KEY_SPACE))
                player.jump();


            if (IsKeyPressed(KEY_J)) {
                player.apply_force();
            }

        }
        EndDrawing();
    }
    CloseWindow();

    return EXIT_SUCCESS;
}
