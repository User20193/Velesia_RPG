#include "engine.h"

Engine::Engine(int width, int height, const std::string& title) {
    InitWindow(width, height, title.c_str());
    SetTargetFPS(60);
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
