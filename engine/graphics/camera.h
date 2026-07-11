#pragma once
#include <utility>
#include "raylib.h"

class GameCamera {
public:
    GameCamera();

    void set_target(float x, float y);
    void set_offset(float x, float y);
    void set_zoom(float zoom);

    void begin_mode();
    void end_mode();

    std::pair<float, float> get_screen_to_world(float screen_x, float screen_y) const;

private:
    Camera2D camera;
};
