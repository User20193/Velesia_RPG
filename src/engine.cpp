#include "engine.h"
#include <iostream>

// --- TextureManager ---

TextureManager::~TextureManager() {
    unload_all();
}

void TextureManager::unload_all() {
    for (auto& pair : textures) {
        UnloadTexture(pair.second);
    }
    textures.clear();
}

void TextureManager::load_texture(const std::string& name, const std::string& filepath) {
    if (textures.find(name) == textures.end()) {
        Texture2D tex = LoadTexture(filepath.c_str());
        textures[name] = tex;
    }
}

void TextureManager::draw_texture(const std::string& name, float x, float y) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        DrawTextureV(it->second, {x, y}, WHITE);
    } else {
        std::cerr << "Warning: Texture '" << name << "' not found!" << std::endl;
    }
}

// --- Engine ---

Engine::Engine(int width, int height, const std::string& title) {
    InitWindow(width, height, title.c_str());
    SetTargetFPS(60); // Default to 60 FPS
}

Engine::~Engine() {
    texture_manager.unload_all();
    CloseWindow();
}

bool Engine::is_running() const {
    return !WindowShouldClose();
}

void Engine::begin_drawing() {
    BeginDrawing();
}

void Engine::end_drawing() {
    EndDrawing();
}

void Engine::clear_background(int r, int g, int b, int a) {
    ClearBackground({(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

bool Engine::is_key_down(int key) {
    return IsKeyDown(key);
}

bool Engine::is_key_pressed(int key) {
    return IsKeyPressed(key);
}

float Engine::get_delta_time() {
    return GetFrameTime();
}

void Engine::draw_rectangle(int x, int y, int width, int height, int r, int g, int b, int a) {
    DrawRectangle(x, y, width, height, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}
