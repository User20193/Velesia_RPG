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

void TextureManager::draw_texture_rec(const std::string& name, float source_x, float source_y, float source_w, float source_h, float dest_x, float dest_y) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        Rectangle source = { source_x, source_y, source_w, source_h };
        Vector2 position = { dest_x, dest_y };
        DrawTextureRec(it->second, source, position, WHITE);
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

int Engine::get_fps() {
    return GetFPS();
}

void Engine::draw_rectangle(int x, int y, int width, int height, int r, int g, int b, int a) {
    DrawRectangle(x, y, width, height, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

void Engine::draw_rectangle_lines(int x, int y, int width, int height, int r, int g, int b, int a) {
    DrawRectangleLines(x, y, width, height, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

void Engine::draw_line(int startPosX, int startPosY, int endPosX, int endPosY, int r, int g, int b, int a) {
    DrawLine(startPosX, startPosY, endPosX, endPosY, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

void Engine::draw_text(const std::string& text, int x, int y, int fontSize, int r, int g, int b, int a) {
    DrawText(text.c_str(), x, y, fontSize, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

bool Engine::check_collision_recs(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    Rectangle rec1 = { x1, y1, w1, h1 };
    Rectangle rec2 = { x2, y2, w2, h2 };
    return CheckCollisionRecs(rec1, rec2);
}
