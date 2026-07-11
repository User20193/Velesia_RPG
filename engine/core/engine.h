#pragma once
#include <string>
#include "raylib.h"
#include "../graphics/texture_manager.h"
#include "../ecs/scene.h"

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
