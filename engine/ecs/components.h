#pragma once
#include <string>

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

struct ActiveChunkTag {};

struct Enemy {
    float hp = 100.0f;
    float max_hp = 100.0f;
    float speed = 100.0f;
    float damage = 10.0f;
};
