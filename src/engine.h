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

class Engine {
public:
    Engine(int width, int height, const std::string& title);
    ~Engine();

    bool is_running() const;
    void begin_drawing();
    void end_drawing();
    void clear_background(int r, int g, int b, int a = 255);

    bool is_key_down(int key);
    bool is_key_pressed(int key);
    float get_delta_time();

    void draw_rectangle(int x, int y, int width, int height, int r, int g, int b, int a = 255);

    TextureManager& get_texture_manager() { return texture_manager; }

private:
    TextureManager texture_manager;
};
