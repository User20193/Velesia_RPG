#pragma once
#include <string>
#include "../ecs/scene.h"

class TilemapLoader {
public:
    static bool load_tiled_json(const std::string& filepath, Scene& scene);
};
