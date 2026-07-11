#pragma once

#include <string>
#include <vector>
#include <memory>
#include "raylib.h"

// Struct to define a single frame in a sprite sheet
struct SpriteFrame {
    float x;
    float y;
    float w;
    float h;
};

// Container for a loaded texture and its calculated frames
struct SpriteSheet {
    std::string name;
    Texture2D texture;
    std::vector<SpriteFrame> frames;
    bool is_valid = false;
};

// Abstract interface for loaders
class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;

    // Returns true if the loader successfully populated the SpriteSheet
    virtual bool load(const std::string& base_filepath, SpriteSheet& out_sheet) = 0;
};

// Loader 1: Parses exact coordinates from a .json file
class JsonAtlasLoader : public IAssetLoader {
public:
    bool load(const std::string& base_filepath, SpriteSheet& out_sheet) override;
};

// Loader 2: Parses a grid config from .grid.json and slices the image uniformly
class GridSpriteLoader : public IAssetLoader {
public:
    bool load(const std::string& base_filepath, SpriteSheet& out_sheet) override;
};

// Loader 3: Fallback loader that uses Connected-Component Labeling to find frames
class AutoDetectLoader : public IAssetLoader {
public:
    bool load(const std::string& base_filepath, SpriteSheet& out_sheet) override;
private:
    void save_cache_json(const std::string& json_path, const std::vector<SpriteFrame>& frames);
};

// Factory manager to try loaders in order of priority
class AssetManager {
public:
    AssetManager();

    // Attempt to load a SpriteSheet using prioritized loaders
    SpriteSheet load_sprite_sheet(const std::string& name, const std::string& filepath);

private:
    std::vector<std::unique_ptr<IAssetLoader>> loaders;
};
