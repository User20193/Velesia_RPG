#pragma once

#include <string>
#include <vector>

// Forward declaration is not enough when we pass by reference in cpp but instantiate objects inside the methods.
// The easiest fix is to just include it in the header.
#include "../core/external/FastNoiseLite.h"

namespace rpg {

class TextureGenerator {
public:
    TextureGenerator();
    ~TextureGenerator();

    // Set the base seed for noise generation
    void set_seed(int seed);

    // Generate Grass texture
    // Uses simplex fractal noise for base color variation and high-frequency noise for grass blades
    bool generate_grass(const std::string& filepath, int width, int height);

    // Generate Water texture
    // Uses smooth simplex for depth and cellular/voronoi for caustics (light reflections)
    bool generate_water(const std::string& filepath, int width, int height);

    // Generate Dirt/Earth texture
    // Uses low-frequency simplex for base dirt and ridge noise for bumps/cracks
    bool generate_dirt(const std::string& filepath, int width, int height);

    bool generate_stone(const std::string& filepath, int width, int height);
    bool generate_wood(const std::string& filepath, int width, int height);
    bool generate_sand(const std::string& filepath, int width, int height);

    bool generate_grass_dirt_tileset(const std::string& filepath, int tile_size);

private:
    int seed_;

    // Helper to generate seamless (tileable) 2D noise mapping a 2D coordinate to a 4D cylinder/torus
    float get_seamless_noise(class FastNoiseLite& noise, float x, float y, float width, float height, float freq_x, float freq_y = -1.0f);

    // Lerp between two colors based on a factor (0.0 to 1.0)
    struct ColorRGB { unsigned char r, g, b, a; };
    ColorRGB lerp_color(ColorRGB c1, ColorRGB c2, float t);

    // Map a noise value (-1.0 to 1.0) to a gradient of colors
    ColorRGB map_gradient(float value, const std::vector<std::pair<float, ColorRGB>>& gradient);
};

} // namespace rpg
