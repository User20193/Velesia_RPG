#pragma once
#include <string>
#include <unordered_map>
#include "raylib.h"

class TextureManager {
public:
    ~TextureManager();
    void unload_all();
    void load_texture(const std::string& name, const std::string& filepath);
    void draw_texture(const std::string& name, float x, float y);
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

    void draw_rectangle(int x, int y, int width, int height, int r, int g, int b, int a = 255);
    void draw_line(int startPosX, int startPosY, int endPosX, int endPosY, int r, int g, int b, int a = 255);

    TextureManager& get_texture_manager() { return texture_manager; }

private:
    TextureManager texture_manager;
};
