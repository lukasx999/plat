#include <functional>
#include <print>
#include <array>

#include <raylib.h>
#include <raymath.h>

static constexpr auto WIDTH = 1600;
static constexpr auto HEIGHT = 900;
#define DEBUG
#define DEBUG_COLLISIONS

struct Item {
    const Rectangle m_hitbox;
    const Color m_color;
    const bool m_is_blocking;
    const std::string m_name;

    Item(Rectangle hitbox, Color color, bool is_blocking, std::string_view name)
        : m_hitbox(hitbox)
        , m_color(color)
        , m_is_blocking(is_blocking)
        , m_name(name)
    { }

};

void draw_environment(const std::span<const Item> items) {
    for (auto &item : items) {
        DrawRectangleRec(item.m_hitbox, item.m_color);
    }
}

enum class MovementDirection { Left, Right };

class PhysicsEntity {
    float m_speed = 0;
    bool m_is_grounded = false;
    Vector2 m_position;
    float m_width;
    float m_height;
    static constexpr int m_gravity = 1000;
    static constexpr float m_movement_speed = 500;
    static constexpr float m_jumping_speed = 900;

public:
    MovementDirection m_direction = MovementDirection::Left;

    PhysicsEntity(Vector2 position)
    : m_position(position)
    { }

    virtual void update() {
        if (m_is_grounded) {
            m_speed = 0;
        } else {
            m_speed += m_gravity * GetFrameTime();
            m_position.y += m_speed * GetFrameTime();
        }
    }

    virtual void jump() {
        if (!m_is_grounded) return;
        m_speed = -m_jumping_speed;
    }

    void set_width(float width) {
        m_width = width;
    }

    void set_height(float height) {
        m_height = height;
    }

    virtual void move(MovementDirection direction) {
        m_direction = direction;
        switch (direction) {
            case MovementDirection::Left:
                m_position.x -= m_movement_speed * GetFrameTime();
                break;
            case MovementDirection::Right:
                m_position.x += m_movement_speed * GetFrameTime();
                break;
        }
    }

    [[nodiscard]] Rectangle hitbox() const {
        return {
            m_position.x - m_width/2.0f,
            m_position.y - m_height/2.0f,
            m_width,
            m_height,
        };
    }

    virtual void resolve_collisions(const std::span<const Item> items) {
        m_is_grounded = false;

        for (const auto &item : items) {
            if (item.m_is_blocking) {

                const auto item_hitbox = item.m_hitbox;
                const float delta_ver = m_speed * GetFrameTime();
                const float delta_hor = m_movement_speed * GetFrameTime();

                // let the player clip a bit into the floor when grounded, to prevent
                // oscillation of grounding state
                // also prevent the player from teleporting down after walking off a ledge
                const float clip = 1;

                const Rectangle left = {
                    item_hitbox.x - delta_hor,
                    item_hitbox.y + clip,
                    delta_hor,
                    item_hitbox.height - clip*2,
                };

                const Rectangle right = {
                    item_hitbox.x + item_hitbox.width,
                    item_hitbox.y + clip,
                    delta_hor,
                    item_hitbox.height - clip*2,
                };

                const float top_delta_mult_factor = 1.3;
                const Rectangle top = {
                    // remove some padding from each edge to prevent
                    // instant teleportation when jumping up and edge
                    // and moving to the right
                    item_hitbox.x + delta_hor*top_delta_mult_factor,
                    item_hitbox.y - delta_ver,
                    item_hitbox.width - delta_hor*top_delta_mult_factor*2,
                    delta_ver,
                };

                const Rectangle bottom = {
                    item_hitbox.x,
                    item_hitbox.y + item_hitbox.height,
                    item_hitbox.width,
                    // when the player bumps their head by jumping, the
                    // speed is negative, therefore it must be inverted
                    -delta_ver,
                };

#ifdef DEBUG_COLLISIONS
                DrawRectangleRec(left,   PURPLE);
                DrawRectangleRec(right,  PURPLE);
                DrawRectangleRec(top,    PURPLE);
                DrawRectangleRec(bottom, PURPLE);
#endif // DEBUG_COLLISIONS

                if (CheckCollisionRecs(hitbox(), top)) {
                    m_is_grounded = true;
                    m_position.y = item_hitbox.y - hitbox().height/2.0f + clip;
                }

                if (CheckCollisionRecs(hitbox(), left)) {
                    m_position.x = item_hitbox.x - hitbox().width/2.0f;
                }

                if (CheckCollisionRecs(hitbox(), right)) {
                    m_position.x = item_hitbox.x + item_hitbox.width + hitbox().width/2.0f;
                }

                if (CheckCollisionRecs(hitbox(), bottom)) {
                    m_speed = 0;
                }

            }

        }

    }

};


class Player : public PhysicsEntity {
    Rectangle m_origin;
    const std::function<float()> m_get_dt;
    const Texture2D m_tex;
    static constexpr std::array m_sprites_running {
        Rectangle { 8,   74,  14, 18 },
        Rectangle { 41,  74,  13, 18 },
        Rectangle { 73,  74,  13, 18 },
        Rectangle { 105, 74,  13, 18 },
        Rectangle { 136, 74,  14, 18 },
        Rectangle { 169, 74,  13, 18 },
        Rectangle { 201, 74,  13, 18 },
        Rectangle { 233, 74,  13, 18 },
        Rectangle { 8,   106, 14, 17 },
        Rectangle { 41,  106, 13, 18 },
        Rectangle { 73,  106, 13, 18 },
        Rectangle { 105, 106, 13, 18 },
        Rectangle { 136, 106, 14, 18 },
        Rectangle { 169, 106, 13, 18 },
        Rectangle { 201, 106, 13, 18 },
        Rectangle { 233, 106, 13, 18 },
    };
    static constexpr float m_texture_scale = 5;
    static constexpr float m_walk_delay_secs = 0.1;
    static constexpr const char *m_tex_path = "./assets/sprites/knight.png";

public:
    Player(Vector2 position, std::function<float()> get_dt)
        : PhysicsEntity(position)
        , m_origin(m_sprites_running[0])
        , m_get_dt(get_dt)
        , m_tex(LoadTexture(m_tex_path))
    { }

    void update() override {
        PhysicsEntity::update();
        set_width(m_origin.width*m_texture_scale);
        set_height(m_origin.height*m_texture_scale);
    }

    void move(MovementDirection direction) override {
        PhysicsEntity::move(direction);
        walk();
    }

    void draw() const {

        auto origin = m_origin;

        switch (m_direction) {
            case MovementDirection::Left:
                // flip texture
                origin.width *= -1;
                break;
            case MovementDirection::Right:
                ;
                break;
        }

        DrawTexturePro(m_tex, origin, hitbox(), { 0, 0 }, 0, WHITE);

#ifdef DEBUG
        DrawTexture(m_tex, WIDTH-m_tex.width, 0, WHITE);
        for (const auto &sprite : m_sprites_running) {
            const Rectangle rect = {
                sprite.x+(WIDTH-m_tex.width),
                sprite.y,
                sprite.width,
                sprite.height,
            };
            DrawRectangleLinesEx(rect, 1, BLUE);
        }

        const Rectangle rect = {
            m_origin.x+(WIDTH-m_tex.width),
            m_origin.y,
            m_origin.width,
            m_origin.height,
        };
        DrawRectangleLinesEx(rect, 1, RED);

        DrawRectangleLinesEx(hitbox(), 1, BLACK);

        // DrawText(std::format("pos: x: {}, y: {}", trunc(m_position.x), trunc(m_position.y)).c_str(), 0, 0, 50, WHITE);
        // DrawText(std::format("speed: {}:", trunc(m_speed)).c_str(), 0, 50, 50, WHITE);
        // DrawText(std::format("grounded: {}", m_is_grounded ? "yes" : "no").c_str(), 0, 100, 50, WHITE);
#endif // DEBUG

    }

private:
    void walk() {
        static int sprite_idx = 0;
        static float fut = 0;
        if (GetTime() > fut) {
            sprite_idx++;
            sprite_idx %= m_sprites_running.size();
            m_origin = m_sprites_running[sprite_idx];
            fut = GetTime() + m_walk_delay_secs;
        }
    }

};

static void handle_input(Player &player) {

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

int main() {

    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "2D Game");

    const int floor_height = 200;
    const Item floor({ 0, HEIGHT-floor_height, WIDTH, floor_height }, GRAY, true, "floor");
    const Item bg({ 0, 0, WIDTH, HEIGHT }, DARKGRAY, false, "bg");

    const std::array items {
        bg,
        floor,
        Item({ 300, 300, 300, 300 }, RED, true, "red"),
        Item({ 1100, 600, 300, 100 }, GREEN, true, "green")
    };

    Player player({ WIDTH/2.0f, HEIGHT - 500 }, GetFrameTime);

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
