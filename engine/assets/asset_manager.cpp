#include "asset_manager.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <queue>

using json = nlohmann::json;

// --- Helper Functions ---
static bool file_exists(const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

// --- JsonAtlasLoader ---
bool JsonAtlasLoader::load(const std::string& base_filepath, SpriteSheet& out_sheet) {
    std::string json_path = base_filepath + ".json";
    if (!file_exists(json_path)) {
        return false; // File doesn't exist, skip this loader
    }

    try {
        std::ifstream f(json_path);
        json data = json::parse(f);

        if (data.contains("frames") && data["frames"].is_array()) {
            for (const auto& item : data["frames"]) {
                SpriteFrame frame;
                frame.x = item["x"];
                frame.y = item["y"];
                frame.w = item["w"];
                frame.h = item["h"];
                out_sheet.frames.push_back(frame);
            }
            TraceLog(LOG_INFO, "ASSET: Loaded %d frames via JsonAtlasLoader from %s", out_sheet.frames.size(), json_path.c_str());
            return true;
        }
    } catch (json::parse_error& e) {
        TraceLog(LOG_WARNING, "ASSET: JSON Parse error in %s: %s", json_path.c_str(), e.what());
    }

    return false;
}

// --- GridSpriteLoader ---
bool GridSpriteLoader::load(const std::string& base_filepath, SpriteSheet& out_sheet) {
    std::string grid_path = base_filepath + ".grid.json";
    if (!file_exists(grid_path)) {
        return false;
    }

    try {
        std::ifstream f(grid_path);
        json data = json::parse(f);

        if (data.contains("columns") && data.contains("rows") &&
            data.contains("frame_width") && data.contains("frame_height")) {

            int cols = data["columns"];
            int rows = data["rows"];
            float fw = data["frame_width"];
            float fh = data["frame_height"];

            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c) {
                    SpriteFrame frame;
                    frame.x = c * fw;
                    frame.y = r * fh;
                    frame.w = fw;
                    frame.h = fh;
                    out_sheet.frames.push_back(frame);
                }
            }
            TraceLog(LOG_INFO, "ASSET: Loaded %d frames via GridSpriteLoader from %s", out_sheet.frames.size(), grid_path.c_str());
            return true;
        }
    } catch (json::parse_error& e) {
        TraceLog(LOG_WARNING, "ASSET: Grid JSON Parse error in %s: %s", grid_path.c_str(), e.what());
    }

    return false;
}

// --- AutoDetectLoader (Connected-Component Labeling) ---
bool AutoDetectLoader::load(const std::string& base_filepath, SpriteSheet& out_sheet) {
    Image img = LoadImage(base_filepath.c_str());
    if (img.width == 0 || img.height == 0) {
        TraceLog(LOG_ERROR, "ASSET: Failed to load image for AutoDetectLoader: %s", base_filepath.c_str());
        return false;
    }

    int width = img.width;
    int height = img.height;
    Color* pixels = LoadImageColors(img);

    std::vector<bool> visited(width * height, false);
    std::vector<SpriteFrame> detected_frames;

    // Helper lambda to get 1D index
    auto get_idx = [&](int x, int y) { return y * width + x; };

    // CCL Algorithm using BFS
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = get_idx(x, y);
            if (!visited[idx]) {
                visited[idx] = true;

                // If pixel is not fully transparent, it's part of a new component
                if (pixels[idx].a > 0) {
                    int minX = x, maxX = x;
                    int minY = y, maxY = y;

                    std::queue<std::pair<int, int>> q;
                    q.push({x, y});

                    while (!q.empty()) {
                        auto [cx, cy] = q.front();
                        q.pop();

                        // 8-way connectivity
                        for (int dy = -1; dy <= 1; ++dy) {
                            // We use 4-way connectivity to avoid merging frames that merely touch at their corners
                            // (which is common in tightly packed sprite sheets).
                            for (int dx = -1; dx <= 1; ++dx) {
                                if (dx == 0 && dy == 0) continue;

                                int nx = cx + dx;
                                int ny = cy + dy;

                                // To prevent perfectly adjacent sprites from merging (e.g. at x=31 and x=32),
                                // CCL alone isn't enough because they share an edge. However, if they share an edge,
                                // there's no transparent pixel between them.
                                // Since this is a fallback loader, if frames physically touch with no transparency,
                                // CCL will natively merge them into one bounding box.

                                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                    int nidx = get_idx(nx, ny);
                                    if (!visited[nidx] && pixels[nidx].a > 0) {
                                        visited[nidx] = true;
                                        q.push({nx, ny});
                                        if (nx < minX) minX = nx;
                                        if (nx > maxX) maxX = nx;
                                        if (ny < minY) minY = ny;
                                        if (ny > maxY) maxY = ny;
                                    }
                                }
                            }
                        }
                    }

                    SpriteFrame frame;
                    frame.x = minX;
                    frame.y = minY;
                    frame.w = (maxX - minX) + 1;
                    frame.h = (maxY - minY) + 1;
                    detected_frames.push_back(frame);
                }
            }
        }
    }

    UnloadImageColors(pixels);
    UnloadImage(img);

    if (detected_frames.empty()) {
        TraceLog(LOG_WARNING, "ASSET: AutoDetectLoader found 0 frames in %s", base_filepath.c_str());
        return false;
    }

    // Sort top-to-bottom, then left-to-right
    std::sort(detected_frames.begin(), detected_frames.end(), [](const SpriteFrame& a, const SpriteFrame& b) {
        if (std::abs(a.y - b.y) > 5.0f) { // Give a little leeway for row alignment
            return a.y < b.y;
        }
        return a.x < b.x;
    });

    // Validation against standard 32x32 size
    int irregular_frames = 0;
    for(const auto& f : detected_frames) {
        if (f.w > 48 || f.w < 16 || f.h > 48 || f.h < 16) {
            irregular_frames++;
        }
    }

    if (irregular_frames > 0) {
        TraceLog(LOG_WARNING, "ASSET: AutoDetectLoader found %d frames with sizes highly deviating from 32x32 in %s. Total frames found: %d",
                irregular_frames, base_filepath.c_str(), detected_frames.size());
    } else {
        TraceLog(LOG_INFO, "ASSET: Loaded %d frames via AutoDetectLoader from %s", detected_frames.size(), base_filepath.c_str());
    }

    out_sheet.frames = detected_frames;

    // Cache the result to JSON
    save_cache_json(base_filepath + ".json", detected_frames);

    return true;
}

void AutoDetectLoader::save_cache_json(const std::string& json_path, const std::vector<SpriteFrame>& frames) {
    json data;
    data["frames"] = json::array();
    for (const auto& f : frames) {
        data["frames"].push_back({
            {"x", f.x},
            {"y", f.y},
            {"w", f.w},
            {"h", f.h}
        });
    }

    std::ofstream o(json_path);
    o << std::setw(4) << data << std::endl;
    TraceLog(LOG_INFO, "ASSET: AutoDetectLoader cached frames to %s", json_path.c_str());
}

// --- AssetManager ---
AssetManager::AssetManager() {
    // Add loaders in priority order
    loaders.push_back(std::make_unique<JsonAtlasLoader>());
    loaders.push_back(std::make_unique<GridSpriteLoader>());
    loaders.push_back(std::make_unique<AutoDetectLoader>());
}

SpriteSheet AssetManager::load_sprite_sheet(const std::string& name, const std::string& filepath) {
    SpriteSheet sheet;
    sheet.name = name;
    sheet.is_valid = false;

    // Load the texture into VRAM
    sheet.texture = LoadTexture(filepath.c_str());
    if (sheet.texture.id == 0) {
        TraceLog(LOG_ERROR, "ASSET: Failed to load texture into VRAM: %s", filepath.c_str());
        return sheet;
    }

    // Attempt to load frame data
    for (auto& loader : loaders) {
        if (loader->load(filepath, sheet)) {
            sheet.is_valid = true;
            break; // Success, stop trying other loaders
        }
    }

    // If no loader succeeded, at least add a single frame covering the whole image
    if (!sheet.is_valid) {
        TraceLog(LOG_WARNING, "ASSET: All loaders failed for %s. Defaulting to full image frame.", filepath.c_str());
        SpriteFrame full_frame = {0, 0, (float)sheet.texture.width, (float)sheet.texture.height};
        sheet.frames.push_back(full_frame);
        sheet.is_valid = true;
    }

    return sheet;
}
