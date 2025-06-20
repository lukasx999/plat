#include <functional>
#include <print>
#include <array>

#include <raylib.h>
#include <raymath.h>

#include "physics.h"
#include "player.h"
#include "main.h"



class Environment {
    // TODO: maybe replace vector with array
    const std::vector<Item> m_items = init_env();

public:
    [[nodiscard]] std::span<const Item> get_items() const {
        return m_items;
    }

    void draw() const {
        for (const auto &item : m_items) {
            DrawRectangleRec(item.m_hitbox, item.m_color);
        }
    }

private:
    [[nodiscard]] static constexpr std::vector<Item> init_env() {
        float wall_size = 100;
        Item bg({ 0, 0, WIDTH, HEIGHT }, DARKGRAY, false);
        Item floor({ 0, HEIGHT - wall_size, WIDTH, wall_size }, GRAY, true);
        Item ceil({ 0, 0, WIDTH, wall_size }, GRAY, true);
        Item left_wall({ 0, wall_size, wall_size, HEIGHT-wall_size*2 }, GRAY, true);
        Item right_wall({ WIDTH-wall_size, wall_size, wall_size, HEIGHT-wall_size*2 }, GRAY, true);

        return {
            bg,
            ceil,
            floor,
            left_wall,
            right_wall,
            Item({ 300, 300, 300, 300 }, RED, true),
            Item({ 1100, 600, 300, 100 }, GREEN, true)
        };
    }
};


class Game {
    Player m_player;
    Environment m_env;

public:
    Game()
    : m_player({ WIDTH/2.0f, HEIGHT - 500 }, GetFrameTime)
    { }

    void update() {
        handle_input(m_player);
        m_player.resolve_collisions(m_env.get_items());
        m_player.update();
    }

    void draw() const {
        m_env.draw();
        m_player.draw();

        #ifdef DEBUG
        draw_debug_info();
        #endif // DEBUG
    }

private:

    #ifdef DEBUG
    void draw_debug_info() const {
        float textsize = 50;
        auto pos = m_player.get_position();
        DrawText(std::format("pos: x: {}, y: {}", trunc(pos.x), trunc(pos.y)).c_str(), 0, 0, textsize, WHITE);
        DrawText(std::format(
            "speed: x: {}, y: {}",
            trunc(m_player.get_speed().x),
            trunc(m_player.get_speed().y)).c_str(),
            0,
            50,
            textsize,
            WHITE
        );
        DrawText(std::format("grounded: {}", m_player.is_grounded() ? "yes" : "no").c_str(), 0, 100, textsize, WHITE);
        DrawText(std::format("state: {}", stringify_state(m_player.get_state())).c_str(), 0, 150, textsize, WHITE);
    }
    #endif // DEBUG

    void handle_input(Player &player) {

        if (IsKeyPressed(KEY_SPACE)) {
            player.jump();
        }

        if (IsKeyPressed(KEY_LEFT_SHIFT)) {
            player.dash();
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
