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

// --- GameCamera ---

GameCamera::GameCamera() {
    camera = { 0 };
    camera.offset = { 0.0f, 0.0f };
    camera.target = { 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void GameCamera::set_target(float x, float y) {
    camera.target = { x, y };
}

void GameCamera::set_offset(float x, float y) {
    camera.offset = { x, y };
}

void GameCamera::set_zoom(float zoom) {
    camera.zoom = zoom;
}

void GameCamera::begin_mode() {
    BeginMode2D(camera);
}

void GameCamera::end_mode() {
    EndMode2D();
}

std::pair<float, float> GameCamera::get_screen_to_world(float screen_x, float screen_y) const {
    Vector2 worldPos = GetScreenToWorld2D({screen_x, screen_y}, camera);
    return {worldPos.x, worldPos.y};
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

bool Engine::is_mouse_button_down(int button) {
    return IsMouseButtonDown(button);
}

bool Engine::is_mouse_button_pressed(int button) {
    return IsMouseButtonPressed(button);
}

float Engine::get_mouse_x() {
    return (float)GetMouseX();
}

float Engine::get_mouse_y() {
    return (float)GetMouseY();
}

float Engine::get_delta_time() {
    return GetFrameTime();
}

void Engine::draw_rectangle(int x, int y, int width, int height, int r, int g, int b, int a) {
    DrawRectangle(x, y, width, height, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

void Engine::draw_line(int startPosX, int startPosY, int endPosX, int endPosY, int r, int g, int b, int a) {
    DrawLine(startPosX, startPosY, endPosX, endPosY, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}
