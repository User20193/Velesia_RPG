#include "camera.h"

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
