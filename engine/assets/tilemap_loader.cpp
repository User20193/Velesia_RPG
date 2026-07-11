#include "tilemap_loader.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include "raylib.h"

using json = nlohmann::json;

bool TilemapLoader::load_tiled_json(const std::string& filepath, Scene& scene) {
    try {
        std::ifstream f(filepath);
        if (!f.is_open()) {
            TraceLog(LOG_ERROR, "TILEMAP: Could not open file %s", filepath.c_str());
            return false;
        }

        json data = json::parse(f);

        int tilewidth = data.value("tilewidth", 32);
        int tileheight = data.value("tileheight", 32);

        if (data.contains("layers") && data["layers"].is_array()) {
            for (const auto& layer : data["layers"]) {
                if (layer.value("type", "") == "tilelayer" && layer.contains("data")) {
                    int width = layer.value("width", 0);
                    int height = layer.value("height", 0);
                    const auto& layer_data = layer["data"];

                    // Simple property check to see if layer is solid
                    bool is_solid = false;
                    if (layer.contains("properties")) {
                        for (const auto& prop : layer["properties"]) {
                            if (prop.value("name", "") == "solid" && prop.value("value", false) == true) {
                                is_solid = true;
                                break;
                            }
                        }
                    }

                    int i = 0;
                    for (int y = 0; y < height; ++y) {
                        for (int x = 0; x < width; ++x) {
                            if (i < layer_data.size()) {
                                int tile_id = layer_data[i];
                                if (tile_id > 0) {
                                    // Generate entity
                                    uint32_t ent = scene.get_pooled_entity();
                                    scene.add_transform(ent, x * tilewidth, y * tileheight);

                                    if (is_solid) {
                                        scene.add_collider(ent, tilewidth, tileheight);
                                    }
                                }
                            }
                            i++;
                        }
                    }
                }
            }
        }
        TraceLog(LOG_INFO, "TILEMAP: Loaded successfully from %s", filepath.c_str());
        return true;
    } catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "TILEMAP: Parse error in %s: %s", filepath.c_str(), e.what());
    }
    return false;
}
