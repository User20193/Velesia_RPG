#include "texture_generator.h"
#include "../core/external/FastNoiseLite.h"
#include <raylib.h>
#include <cmath>
#include <iostream>

namespace rpg {

TextureGenerator::TextureGenerator() : seed_(1337) {}

TextureGenerator::~TextureGenerator() {}

void TextureGenerator::set_seed(int seed) {
    seed_ = seed;
}

TextureGenerator::ColorRGB TextureGenerator::lerp_color(ColorRGB c1, ColorRGB c2, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return {
        static_cast<unsigned char>(c1.r + t * (c2.r - c1.r)),
        static_cast<unsigned char>(c1.g + t * (c2.g - c1.g)),
        static_cast<unsigned char>(c1.b + t * (c2.b - c1.b)),
        static_cast<unsigned char>(c1.a + t * (c2.a - c1.a))
    };
}

TextureGenerator::ColorRGB TextureGenerator::map_gradient(float value, const std::vector<std::pair<float, ColorRGB>>& gradient) {
    if (gradient.empty()) return {0, 0, 0, 255};
    if (value <= gradient.front().first) return gradient.front().second;
    if (value >= gradient.back().first) return gradient.back().second;

    for (size_t i = 0; i < gradient.size() - 1; ++i) {
        if (value >= gradient[i].first && value <= gradient[i+1].first) {
            float t = (value - gradient[i].first) / (gradient[i+1].first - gradient[i].first);
            return lerp_color(gradient[i].second, gradient[i+1].second, t);
        }
    }
    return gradient.back().second;
}

// Generates seamless 2D noise by mapping 2D coordinates to 4D coordinates on a torus
float TextureGenerator::get_seamless_noise(FastNoiseLite& noise, float x, float y, float width, float height, float freq_x, float freq_y) {
    if (freq_y < 0.0f) freq_y = freq_x;

    float s = x / width;
    float t = y / height;

    float const pi_val = 3.14159265358979323846f;

    float nx = std::cos(s * 2.0f * pi_val) * freq_x / (2.0f * pi_val);
    float ny = std::cos(t * 2.0f * pi_val) * freq_y / (2.0f * pi_val);
    float nz = std::sin(s * 2.0f * pi_val) * freq_x / (2.0f * pi_val);
    float nw = std::sin(t * 2.0f * pi_val) * freq_y / (2.0f * pi_val);

    // FastNoiseLite only supports 3D noise directly. For 4D seamless noise, we can
    // blend two 3D noise slices or just rely on a well-scaled 3D cylinder.
    // A common trick for 2D tileable is mapping a cylinder: (cos(x), sin(x), y).
    float cyl_x = std::cos(s * 2.0f * pi_val) * freq_x;
    float cyl_z = std::sin(s * 2.0f * pi_val) * freq_x;
    float cyl_y = y * freq_y; // Not seamless on Y axis with this strict trick

    // For true 2D seamless with a 3D noise function, we blend:
    // Left-Right seamless:
    float val1 = noise.GetNoise(cyl_x, cyl_y, cyl_z);

    // But since we want FULL 2D seamless (X and Y), we blend 4 corners using standard 2D noise
    // Fallback to simple interpolation for true 2D seamless if 4D is unavailable.
    // This is a simpler seamless approach:
    float xx = x * freq_x;
    float yy = y * freq_y;
    float ww = width * freq_x;
    float hh = height * freq_y;

    float v00 = noise.GetNoise(xx, yy);
    float v10 = noise.GetNoise(xx - ww, yy);
    float v01 = noise.GetNoise(xx, yy - hh);
    float v11 = noise.GetNoise(xx - ww, yy - hh);

    // Blend factors
    float bx = x / width;
    float by = y / height;

    // Smoothstep blending
    bx = bx * bx * (3.0f - 2.0f * bx);
    by = by * by * (3.0f - 2.0f * by);

    float blend_x0 = v00 * (1.0f - bx) + v10 * bx;
    float blend_x1 = v01 * (1.0f - bx) + v11 * bx;

    return blend_x0 * (1.0f - by) + blend_x1 * by;
}

bool TextureGenerator::generate_grass(const std::string& filepath, int width, int height) {
    FastNoiseLite baseNoise;
    baseNoise.SetSeed(seed_);
    baseNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    baseNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    baseNoise.SetFractalOctaves(3);

    FastNoiseLite bladeNoise;
    bladeNoise.SetSeed(seed_ + 1);
    bladeNoise.SetNoiseType(FastNoiseLite::NoiseType_Value);

    std::vector<std::pair<float, ColorRGB>> grassGradient = {
        {-1.0f, {34, 139, 34, 255}},   // Dark Green
        {0.0f, {50, 168, 82, 255}},    // Medium Green
        {1.0f, {85, 194, 66, 255}}     // Light Green
    };

    Image image = GenImageColor(width, height, BLANK);
    Color* pixels = (Color*)image.data;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float base = get_seamless_noise(baseNoise, x, y, width, height, 0.05f);
            float blade = get_seamless_noise(bladeNoise, x, y, width, height, 5.0f);

            // Mix base variation with high frequency "blades"
            float final_val = base * 0.7f + blade * 0.3f;

            ColorRGB rgb = map_gradient(final_val, grassGradient);
            pixels[y * width + x] = { rgb.r, rgb.g, rgb.b, rgb.a };
        }
    }

    bool success = ExportImage(image, filepath.c_str());
    UnloadImage(image);
    return success;
}

bool TextureGenerator::generate_stone(const std::string& filepath, int width, int height) {
    FastNoiseLite stoneNoise;
    stoneNoise.SetSeed(seed_ + 5);
    stoneNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    stoneNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    stoneNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
    stoneNoise.SetCellularJitter(0.9f);

    FastNoiseLite crackNoise;
    crackNoise.SetSeed(seed_ + 6);
    crackNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    crackNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    crackNoise.SetFractalOctaves(3);

    std::vector<std::pair<float, ColorRGB>> stoneGradient = {
        {-1.0f, {90, 90, 95, 255}},
        {0.0f, {130, 130, 135, 255}},
        {1.0f, {180, 180, 185, 255}}
    };

    Image image = GenImageColor(width, height, BLANK);
    Color* pixels = (Color*)image.data;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float val = get_seamless_noise(stoneNoise, x, y, width, height, 0.15f);
            float crack = get_seamless_noise(crackNoise, x, y, width, height, 0.1f);

            ColorRGB rgb = map_gradient(val, stoneGradient);

            // Add cracks
            if (crack > 0.6f) {
                rgb.r /= 2; rgb.g /= 2; rgb.b /= 2;
            }

            pixels[y * width + x] = { rgb.r, rgb.g, rgb.b, rgb.a };
        }
    }

    bool success = ExportImage(image, filepath.c_str());
    UnloadImage(image);
    return success;
}

bool TextureGenerator::generate_wood(const std::string& filepath, int width, int height) {
    FastNoiseLite woodNoise;
    woodNoise.SetSeed(seed_ + 7);
    woodNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    woodNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    woodNoise.SetFractalOctaves(3);

    std::vector<std::pair<float, ColorRGB>> woodGradient = {
        {-1.0f, {74, 47, 29, 255}},    // Dark Wood
        {0.0f, {125, 82, 51, 255}},    // Medium Wood
        {1.0f, {158, 107, 70, 255}}    // Light Wood
    };

    Image image = GenImageColor(width, height, BLANK);
    Color* pixels = (Color*)image.data;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Stretch the noise vertically to create wood grain
            float val = get_seamless_noise(woodNoise, x, y, width, height, 0.3f, 0.02f);

            // Add some "rings" by taking the sine of the noise
            val = std::sin(val * 20.0f);

            ColorRGB rgb = map_gradient(val, woodGradient);
            pixels[y * width + x] = { rgb.r, rgb.g, rgb.b, rgb.a };
        }
    }

    bool success = ExportImage(image, filepath.c_str());
    UnloadImage(image);
    return success;
}

bool TextureGenerator::generate_sand(const std::string& filepath, int width, int height) {
    FastNoiseLite sandNoise;
    sandNoise.SetSeed(seed_ + 8);
    sandNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    sandNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    sandNoise.SetFractalOctaves(5);

    std::vector<std::pair<float, ColorRGB>> sandGradient = {
        {-1.0f, {194, 178, 128, 255}},
        {0.0f, {219, 203, 153, 255}},
        {1.0f, {232, 219, 179, 255}}
    };

    Image image = GenImageColor(width, height, BLANK);
    Color* pixels = (Color*)image.data;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float val = get_seamless_noise(sandNoise, x, y, width, height, 0.2f);
            ColorRGB rgb = map_gradient(val, sandGradient);
            pixels[y * width + x] = { rgb.r, rgb.g, rgb.b, rgb.a };
        }
    }

    bool success = ExportImage(image, filepath.c_str());
    UnloadImage(image);
    return success;
}

bool TextureGenerator::generate_water(const std::string& filepath, int width, int height) {
    FastNoiseLite depthNoise;
    depthNoise.SetSeed(seed_ + 2);
    depthNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    depthNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    FastNoiseLite causticNoise;
    causticNoise.SetSeed(seed_ + 3);
    causticNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    causticNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    causticNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add);

    std::vector<std::pair<float, ColorRGB>> waterGradient = {
        {-1.0f, {15, 60, 150, 255}},   // Deep Blue
        {0.0f, {25, 100, 200, 255}},   // Mid Blue
        {1.0f, {60, 170, 220, 255}}    // Shallow Cyan
    };

    Image image = GenImageColor(width, height, BLANK);
    Color* pixels = (Color*)image.data;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float depth = get_seamless_noise(depthNoise, x, y, width, height, 0.03f);
            float caustic = get_seamless_noise(causticNoise, x, y, width, height, 0.15f);

            ColorRGB base_color = map_gradient(depth, waterGradient);

            // Add caustic highlights (Voronoi edges)
            // Voronoi distance returns values usually from -1 to 1. We want to highlight the edges.
            float highlight = std::abs(caustic);
            if (highlight < 0.2f) { // Very thin bright lines
                float intensity = (0.2f - highlight) * 5.0f; // 0 to 1
                base_color.r = static_cast<unsigned char>(std::min(255.0f, base_color.r + 100 * intensity));
                base_color.g = static_cast<unsigned char>(std::min(255.0f, base_color.g + 100 * intensity));
                base_color.b = static_cast<unsigned char>(std::min(255.0f, base_color.b + 100 * intensity));
            }

            pixels[y * width + x] = { base_color.r, base_color.g, base_color.b, base_color.a };
        }
    }

    bool success = ExportImage(image, filepath.c_str());
    UnloadImage(image);
    return success;
}

bool TextureGenerator::generate_dirt(const std::string& filepath, int width, int height) {
    FastNoiseLite dirtNoise;
    dirtNoise.SetSeed(seed_ + 4);
    dirtNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    dirtNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    dirtNoise.SetFractalOctaves(4);

    std::vector<std::pair<float, ColorRGB>> dirtGradient = {
        {-1.0f, {61, 40, 23, 255}},    // Dark Brown (Cracks)
        {0.0f, {102, 70, 44, 255}},    // Medium Brown
        {1.0f, {133, 94, 60, 255}}     // Light Brown (Highlights)
    };

    Image image = GenImageColor(width, height, BLANK);
    Color* pixels = (Color*)image.data;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float val = get_seamless_noise(dirtNoise, x, y, width, height, 0.08f);
            ColorRGB rgb = map_gradient(val, dirtGradient);
            pixels[y * width + x] = { rgb.r, rgb.g, rgb.b, rgb.a };
        }
    }

    bool success = ExportImage(image, filepath.c_str());
    UnloadImage(image);
    return success;
}

bool TextureGenerator::generate_grass_dirt_tileset(const std::string& filepath, int tile_size) {
    int width = tile_size * 3;
    int height = tile_size * 3;

    // We need 3 images to blend: grass, dirt, and a noisy transition mask
    std::string temp_grass = "temp_grass.png";
    std::string temp_dirt = "temp_dirt.png";

    // Generate base seamless textures for the whole 3x3 area
    if (!generate_grass(temp_grass, width, height) || !generate_dirt(temp_dirt, width, height)) {
        return false;
    }

    Image grass_img = LoadImage(temp_grass.c_str());
    Image dirt_img = LoadImage(temp_dirt.c_str());

    Image final_img = GenImageColor(width, height, BLANK);
    Color* final_pixels = (Color*)final_img.data;
    Color* grass_pixels = (Color*)grass_img.data;
    Color* dirt_pixels = (Color*)dirt_img.data;

    FastNoiseLite maskNoise;
    maskNoise.SetSeed(seed_ + 9);
    maskNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    maskNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Determine which tile we are in (0-2 on X and Y)
            int tx = x / tile_size;
            int ty = y / tile_size;

            float u = (float)(x % tile_size) / tile_size;
            float v = (float)(y % tile_size) / tile_size;

            // Base mask value based on tile position
            // Center (1,1) is full grass (1.0). Edges transition to dirt (0.0).
            float mask = 0.0f;
            if (tx == 1 && ty == 1) mask = 1.0f;
            else if (tx == 1 && ty == 0) mask = v; // Top edge
            else if (tx == 1 && ty == 2) mask = 1.0f - v; // Bottom edge
            else if (tx == 0 && ty == 1) mask = u; // Left edge
            else if (tx == 2 && ty == 1) mask = 1.0f - u; // Right edge
            else if (tx == 0 && ty == 0) mask = u * v; // Top-Left corner
            else if (tx == 2 && ty == 0) mask = (1.0f - u) * v; // Top-Right corner
            else if (tx == 0 && ty == 2) mask = u * (1.0f - v); // Bottom-Left corner
            else if (tx == 2 && ty == 2) mask = (1.0f - u) * (1.0f - v); // Bottom-Right corner

            // Add organic noise to the mask so it doesn't look perfectly straight
            float n = maskNoise.GetNoise((float)x * 5.0f, (float)y * 5.0f); // -1 to 1
            mask += n * 0.4f;

            // Clamp mask
            if (mask < 0.0f) mask = 0.0f;
            if (mask > 1.0f) mask = 1.0f;

            // Sharp threshold for pixel-art style, or smooth lerp for smooth texture.
            // Let's use a sharp threshold with slight anti-aliasing for RPG look
            if (mask > 0.5f) {
                final_pixels[y * width + x] = grass_pixels[y * width + x];
            } else {
                final_pixels[y * width + x] = dirt_pixels[y * width + x];
            }
        }
    }

    bool success = ExportImage(final_img, filepath.c_str());

    UnloadImage(final_img);
    UnloadImage(grass_img);
    UnloadImage(dirt_img);

    // Cleanup temp files
    remove(temp_grass.c_str());
    remove(temp_dirt.c_str());

    return success;
}

} // namespace rpg
