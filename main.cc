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
    const Texture2D m_tex_grass;
    const Texture2D m_tex_background;
    static constexpr Rectangle m_tex_grass_origin { 0, 0, 16, 16 };
    static constexpr Rectangle m_tex_dirt_origin { 0, 16, 16, 16 };

public:
    Environment()
        : m_tex_grass(LoadTexture("./assets/sprites/world_tileset.png"))
        , m_tex_background(LoadTexture("./assets/background.png"))
    { }

    [[nodiscard]] std::span<const Item> get_items() const {
        return m_items;
    }

    void draw() const {
        DrawTexture(m_tex_background, 0, 0, WHITE);
        for (const auto &item : m_items) {
            DrawTexturePro(m_tex_grass, item.m_tex_origin, item.m_hitbox, { 0, 0 }, 0, WHITE);
            #ifdef DEBUG
            DrawRectangleLinesEx(item.m_hitbox, 1, RED);
            #endif // DEBUG
        }
    }

private:
    static void gen_floor(std::vector<Item> &items, float block_size) {
        for (int i=0; i < WIDTH/block_size; ++i) {
            items.push_back(Item(
                { i*block_size, HEIGHT - block_size, block_size, block_size },
                m_tex_grass_origin
            ));
        }
    }

    [[nodiscard]] static std::vector<Item> init_env() {

        float block_size = 75;
        std::vector<Item> items;

        gen_floor(items, block_size);

        Vector2 start = { 500, 500 };
        items.push_back(Item(
            { start.x, start.y, block_size, block_size },
            m_tex_grass_origin
        ));

        items.push_back(Item(
            { start.x+block_size, start.y, block_size, block_size },
            m_tex_grass_origin
        ));

        items.push_back(Item(
            { start.x, start.y+block_size, block_size, block_size },
            m_tex_dirt_origin
        ));

        items.push_back(Item(
            { start.x+block_size, start.y+block_size, block_size, block_size },
            m_tex_dirt_origin
        ));

        return items;
    }
};


class Game {
    Player m_player;
    Environment m_env;

public:
    Game()
    : m_player({ WIDTH/2.0f, HEIGHT - 500 })
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
        auto pos      = m_player.get_position();
        auto speed    = m_player.get_speed();
        auto grounded = m_player.is_grounded() ? "yes" : "no";
        auto state    = stringify_state(m_player.get_state());
        int jumps     = m_player.get_jumpcount();
        int dashes    = m_player.get_dashcount();

        add_debug_text(std::format("pos: x: {}, y: {}", trunc(pos.x), trunc(pos.y)), 0);
        add_debug_text(std::format("speed: x: {}, y: {}", trunc(speed.x), trunc(speed.y)), 1);
        add_debug_text(std::format("grounded: {}", grounded), 2);
        add_debug_text(std::format("state: {}", state), 3);
        add_debug_text(std::format("jumps: {}", jumps), 4);
        add_debug_text(std::format("dashes: {}", dashes), 5);
    }

    void add_debug_text(std::string text, int idx) const {
        float textsize = 50;
        DrawText(text.c_str(), 0, idx*textsize, textsize, WHITE);
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

    #ifndef DEBUG
    SetTraceLogLevel(LOG_ERROR);
    #endif // DEBUG
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
