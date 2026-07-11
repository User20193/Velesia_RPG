#include "texture_manager.h"
#include <iostream>

TextureManager::~TextureManager() {
    unload_all();
}

void TextureManager::unload_all() {
    for (auto& pair : textures) {
        UnloadTexture(pair.second);
    }
    textures.clear();

    for (auto& pair : sprite_sheets) {
        UnloadTexture(pair.second.texture);
    }
    sprite_sheets.clear();
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

void TextureManager::load_sprite_sheet(const std::string& name, const std::string& filepath) {
    if (sprite_sheets.find(name) == sprite_sheets.end()) {
        SpriteSheet sheet = asset_manager.load_sprite_sheet(name, filepath);
        if (sheet.is_valid) {
            sprite_sheets[name] = sheet;
        }
    }
}

void TextureManager::draw_sprite_frame(const std::string& name, int frame_index, float dest_x, float dest_y) {
    auto it = sprite_sheets.find(name);
    if (it != sprite_sheets.end()) {
        const SpriteSheet& sheet = it->second;
        if (!sheet.frames.empty()) {
            int safe_index = frame_index % sheet.frames.size();
            const SpriteFrame& frame = sheet.frames[safe_index];

            Rectangle source = { frame.x, frame.y, frame.w, frame.h };
            Vector2 position = { dest_x, dest_y };
            DrawTextureRec(sheet.texture, source, position, WHITE);
        }
    } else {
        std::cerr << "Warning: SpriteSheet '" << name << "' not found!" << std::endl;
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
