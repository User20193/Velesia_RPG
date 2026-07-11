#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>
#include "raylib.h"
#include <entt/entt.hpp>

// --- ECS Components ---
struct Transform2D {
    float x = 0.0f;
    float y = 0.0f;
};

struct Collider {
    float width = 0.0f;
    float height = 0.0f;
};

struct Sprite {
    std::string texture_name;
    int current_frame = 0;
    int max_frames = 1;
    float frame_timer = 0.0f;
    float frame_delay = 0.1f;
    float size_x = 32.0f;
    float size_y = 32.0f;
    float r = 255.0f, g = 255.0f, b = 255.0f, a = 255.0f;
    bool is_visible = true;
};

class Scene {
public:
    Scene();
    ~Scene();

    // ECS Basics
    uint32_t create_entity();
    void destroy_entity(uint32_t entity);

    // Components
    void add_transform(uint32_t entity, float x, float y);
    void set_transform(uint32_t entity, float x, float y);
    std::pair<float, float> get_transform(uint32_t entity);

    void add_collider(uint32_t entity, float width, float height);
    bool has_collider(uint32_t entity);

    // Simple pool logic for Python
    uint32_t get_pooled_entity();
    void return_pooled_entity(uint32_t entity);

    entt::registry& get_registry() { return registry; }

private:
    entt::registry registry;
    std::vector<uint32_t> entity_pool;
};

class TextureManager {
public:
    ~TextureManager();
    void unload_all();
    void load_texture(const std::string& name, const std::string& filepath);
    void draw_texture(const std::string& name, float x, float y);
    void draw_texture_rec(const std::string& name, float source_x, float source_y, float source_w, float source_h, float dest_x, float dest_y);
private:
    std::unordered_map<std::string, Texture2D> textures;
};

class GameCamera {
public:
    GameCamera();

    void set_target(float x, float y);
    void set_offset(float x, float y);
    void set_zoom(float zoom);

    void begin_mode();
    void end_mode();

    // Returns world X and Y for a given screen coordinate (e.g. mouse)
    std::pair<float, float> get_screen_to_world(float screen_x, float screen_y) const;

private:
    Camera2D camera;
};

class Engine {
public:
    Engine(int width, int height, const std::string& title);
    ~Engine();

    bool is_running() const;
    void begin_drawing();
    void end_drawing();
    void clear_background(int r, int g, int b, int a = 255);

    // Keyboard
    bool is_key_down(int key);
    bool is_key_pressed(int key);

    // Mouse
    bool is_mouse_button_down(int button);
    bool is_mouse_button_pressed(int button);
    float get_mouse_x();
    float get_mouse_y();

    float get_delta_time();
    int get_fps();

    // Drawing
    void draw_rectangle(int x, int y, int width, int height, int r, int g, int b, int a = 255);
    void draw_rectangle_lines(int x, int y, int width, int height, int r, int g, int b, int a = 255);
    void draw_line(int startPosX, int startPosY, int endPosX, int endPosY, int r, int g, int b, int a = 255);
    void draw_text(const std::string& text, int x, int y, int fontSize, int r, int g, int b, int a = 255);

    // Physics / Collision
    bool check_collision_recs(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);

    TextureManager& get_texture_manager() { return texture_manager; }
    Scene& get_scene() { return scene; }

private:
    TextureManager texture_manager;
    Scene scene;
};
