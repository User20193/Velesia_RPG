#pragma once
#include <string>
#include <unordered_map>
#include "raylib.h"
#include "../assets/asset_manager.h"

class TextureManager {
public:
    TextureManager() = default;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    ~TextureManager();
    void unload_all();

    void load_texture(const std::string& name, const std::string& filepath);
    void draw_texture(const std::string& name, float x, float y);
    void draw_texture_rec(const std::string& name, float source_x, float source_y, float source_w, float source_h, float dest_x, float dest_y);

    void load_sprite_sheet(const std::string& name, const std::string& filepath);
    void draw_sprite_frame(const std::string& name, int frame_index, float dest_x, float dest_y);

private:
    std::unordered_map<std::string, Texture2D> textures;
    std::unordered_map<std::string, SpriteSheet> sprite_sheets;
    AssetManager asset_manager;
};
