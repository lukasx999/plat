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
    }

private:
    [[nodiscard]] constexpr static std::array<Item, 4> init_env() {
        int floor_height = 200;
        Item floor{ { 0.0f, static_cast<float>(HEIGHT-floor_height), WIDTH, static_cast<float>(floor_height) }, GRAY, true};
        Item bg{ { 0, 0, WIDTH, HEIGHT }, DARKGRAY, false };

        return {
            bg,
            floor,
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
            game.update();
            game.draw();
        }
        EndDrawing();
    }
    CloseWindow();

    return EXIT_SUCCESS;
}
