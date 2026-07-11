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
float TextureGenerator::get_seamless_noise(FastNoiseLite& noise, float x, float y, float width, float height, float frequency) {
    float s = x / width;
    float t = y / height;

    float const pi_val = 3.14159265358979323846f;

    float nx = std::cos(s * 2.0f * pi_val) * frequency / (2.0f * pi_val);
    float ny = std::cos(t * 2.0f * pi_val) * frequency / (2.0f * pi_val);
    float nz = std::sin(s * 2.0f * pi_val) * frequency / (2.0f * pi_val);
    float nw = std::sin(t * 2.0f * pi_val) * frequency / (2.0f * pi_val);

    // FastNoiseLite only supports 3D noise directly. For 4D seamless noise, we can
    // blend two 3D noise slices or just rely on a well-scaled 3D cylinder.
    // A common trick for 2D tileable is mapping a cylinder: (cos(x), sin(x), y).
    float cyl_x = std::cos(s * 2.0f * pi_val) * frequency;
    float cyl_z = std::sin(s * 2.0f * pi_val) * frequency;
    float cyl_y = y * frequency; // Not seamless on Y axis with this strict trick

    // For true 2D seamless with a 3D noise function, we blend:
    // Left-Right seamless:
    float val1 = noise.GetNoise(cyl_x, cyl_y, cyl_z);

    // But since we want FULL 2D seamless (X and Y), we blend 4 corners using standard 2D noise
    // Fallback to simple interpolation for true 2D seamless if 4D is unavailable.
    // This is a simpler seamless approach:
    float xx = x * frequency;
    float yy = y * frequency;
    float ww = width * frequency;
    float hh = height * frequency;

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

} // namespace rpg
