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
    static constexpr Rectangle m_tex_grass_origin { 0, 0, 16, 16 };
    static constexpr Rectangle m_tex_dirt_origin { 0, 16, 16, 16 };

public:
    Environment()
        : m_tex_grass(LoadTexture("./assets/sprites/world_tileset.png"))
    { }

    [[nodiscard]] std::span<const Item> get_items() const {
        return m_items;
    }

    void draw() const {

        DrawRectangle(0, 0, WIDTH, HEIGHT, DARKGRAY);


        for (const auto &item : m_items) {
            auto hitbox = item.m_hitbox;
            // TODO:
            // hitbox.x += std::fmod(std::lerp(0, 100, GetTime()), 100);
            DrawTexturePro(m_tex_grass, item.m_tex_origin, hitbox, { 0, 0 }, 0, WHITE);
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
    static constexpr float m_scroll_factor = 0.1;
    static constexpr float m_camera_follow_factor = 1;

public:
    // TODO: make private
    Camera2D m_cam;

    Game()
        : m_player({ WIDTH/2.0f, HEIGHT - 500 })
        , m_cam({ WIDTH/2.0f, HEIGHT/2.0f }, m_player.get_position(), 0, 1)
    { }

    void update() {
        update_camera();
        handle_input();
        m_player.resolve_collisions(m_env.get_items());
        m_player.update();
    }

    void draw_hud() const {
        #ifdef DEBUG
        draw_debug_info();
        m_player.draw_debug_ui();
        #endif // DEBUG
    }

    void draw_world() const {
        m_env.draw();
        m_player.draw();
        m_player.draw_hitbox();
    }

private:
    void update_camera() {
        auto diff = m_player.get_position() - m_cam.target;
        float len = Vector2Length(diff);
        float speed = len * m_camera_follow_factor * GetFrameTime();
        m_cam.target += Vector2Normalize(diff) * speed;
    }

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

    void handle_input() {

        if (IsKeyPressed(KEY_SPACE)) {
            m_player.jump();
        }

        if (IsKeyPressed(KEY_LEFT_SHIFT)) {
            m_player.dash();
        }

        if (IsKeyDown(KEY_D)) {
            m_player.move(MovementDirection::Right);
        }

        if (IsKeyDown(KEY_A)) {
            m_player.move(MovementDirection::Left);
        }

        handle_zoom();

    }

    void handle_zoom() {
        m_cam.zoom += GetMouseWheelMove() * m_scroll_factor;
        m_cam.zoom = std::max(m_cam.zoom, 0.0f);
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
            BeginMode2D(game.m_cam);
            {
                game.draw_world();
            }
            EndMode2D();
            game.draw_hud();
            game.update();
        }
        EndDrawing();
    }

    CloseWindow();

    return EXIT_SUCCESS;
}
