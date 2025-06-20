#include <functional>
#include <print>
#include <array>

#include <raylib.h>
#include <raymath.h>

#include "physics.h"
#include "player.h"
#include "main.h"



class Game {
    Player m_player;
    const std::array<Item, 4> m_items = init_env();

public:
    Game()
    : m_player({ WIDTH/2.0f, HEIGHT - 500 }, GetFrameTime)
    { }

    void update() {
        handle_input(m_player);
        m_player.resolve_collisions(m_items);
        m_player.update();
    }

    void draw() const {
        draw_environment(m_items);
        m_player.draw();

        #ifdef DEBUG
        draw_debug_info();
        #endif // DEBUG
    }

    #ifdef DEBUG
    void draw_debug_info() const {
        float textsize = 50;
        auto pos = m_player.get_position();
        DrawText(std::format("pos: x: {}, y: {}", trunc(pos.x), trunc(pos.y)).c_str(), 0, 0, textsize, WHITE);
        DrawText(std::format("speed: {}:", trunc(m_player.get_speed())).c_str(), 0, 50, textsize, WHITE);
        DrawText(std::format("grounded: {}", m_player.is_grounded() ? "yes" : "no").c_str(), 0, 100, textsize, WHITE);
        DrawText(std::format("state: {}", stringify_state(m_player.get_state())).c_str(), 0, 150, textsize, WHITE);
    }
    #endif // DEBUG

private:
    [[nodiscard]] constexpr static std::array<Item, 4> init_env() {
        float floor_height = 200;
        return {
            Item({ 0, 0, WIDTH, HEIGHT }, DARKGRAY, false),
            Item({ 0.0f, HEIGHT - floor_height, WIDTH, floor_height }, GRAY, true),
            Item({ 300, 300, 300, 300 }, RED, true),
            Item({ 1100, 600, 300, 100 }, GREEN, true)
        };
    }

    void draw_environment(std::span<const Item> items) const {
        for (const auto &item : items) {
            DrawRectangleRec(item.m_hitbox, item.m_color);
        }
    }

    void handle_input(Player &player) {

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

};

int main() {

    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "platformer");

    Game game;

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(BLACK);
            game.draw();
            game.update();
        }
        EndDrawing();
    }

    CloseWindow();

    return EXIT_SUCCESS;
}
