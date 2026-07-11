#include "engine.h"
#include <iostream>

// --- Scene (ECS) ---
Scene::Scene() {
    // Pre-allocate a pool of 1000 entities
    for (int i = 0; i < 1000; ++i) {
        auto entity = registry.create();
        entity_pool.push_back(static_cast<uint32_t>(entity));
    }
}

Scene::~Scene() {
    registry.clear();
}

uint32_t Scene::create_entity() {
    return static_cast<uint32_t>(registry.create());
}

void Scene::destroy_entity(uint32_t entity) {
    registry.destroy(static_cast<entt::entity>(entity));
}

void Scene::add_transform(uint32_t entity, float x, float y) {
    registry.emplace_or_replace<Transform2D>(static_cast<entt::entity>(entity), x, y);
}

void Scene::set_transform(uint32_t entity, float x, float y) {
    auto& t = registry.get<Transform2D>(static_cast<entt::entity>(entity));
    t.x = x;
    t.y = y;
}

std::pair<float, float> Scene::get_transform(uint32_t entity) {
    if(registry.all_of<Transform2D>(static_cast<entt::entity>(entity))) {
        auto& t = registry.get<Transform2D>(static_cast<entt::entity>(entity));
        return {t.x, t.y};
    }
    return {0.0f, 0.0f};
}

void Scene::add_collider(uint32_t entity, float width, float height) {
    registry.emplace_or_replace<Collider>(static_cast<entt::entity>(entity), width, height);
}

bool Scene::has_collider(uint32_t entity) {
    return registry.all_of<Collider>(static_cast<entt::entity>(entity));
}

uint32_t Scene::get_pooled_entity() {
    if (!entity_pool.empty()) {
        uint32_t ent = entity_pool.back();
        entity_pool.pop_back();
        return ent;
    }
    // If pool is empty, create a new one
    return create_entity();
}

void Scene::return_pooled_entity(uint32_t entity) {
    // Destroy entity entirely (EnTT manages recycling entity IDs internally)
    // We recreate it when needed from the pool instead of clearing components
    auto entt_id = static_cast<entt::entity>(entity);
    registry.destroy(entt_id);
    // Actually, to simulate pooling properly with EnTT without remove_all (which was removed in v3),
    // we just let EnTT handle the ID recycle implicitly via destroy, and we push a new ID to the pool wrapper
    entity_pool.push_back(static_cast<uint32_t>(registry.create()));
}

// --- TextureManager ---

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

// --- GameCamera ---

GameCamera::GameCamera() {
    camera = { 0 };
    camera.offset = { 0.0f, 0.0f };
    camera.target = { 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void GameCamera::set_target(float x, float y) {
    camera.target = { x, y };
}

void GameCamera::set_offset(float x, float y) {
    camera.offset = { x, y };
}

void GameCamera::set_zoom(float zoom) {
    camera.zoom = zoom;
}

void GameCamera::begin_mode() {
    BeginMode2D(camera);
}

void GameCamera::end_mode() {
    EndMode2D();
}

std::pair<float, float> GameCamera::get_screen_to_world(float screen_x, float screen_y) const {
    Vector2 worldPos = GetScreenToWorld2D({screen_x, screen_y}, camera);
    return {worldPos.x, worldPos.y};
}

// --- Engine ---

Engine::Engine(int width, int height, const std::string& title) {
    InitWindow(width, height, title.c_str());
    SetTargetFPS(60); // Default to 60 FPS
}

Engine::~Engine() {
    texture_manager.unload_all();
    CloseWindow();
}

bool Engine::is_running() const {
    return !WindowShouldClose();
}

void Engine::begin_drawing() {
    BeginDrawing();
}

void Engine::end_drawing() {
    EndDrawing();
}

void Engine::clear_background(int r, int g, int b, int a) {
    ClearBackground({(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

bool Engine::is_key_down(int key) {
    return IsKeyDown(key);
}

bool Engine::is_key_pressed(int key) {
    return IsKeyPressed(key);
}

bool Engine::is_mouse_button_down(int button) {
    return IsMouseButtonDown(button);
}

bool Engine::is_mouse_button_pressed(int button) {
    return IsMouseButtonPressed(button);
}

float Engine::get_mouse_x() {
    return (float)GetMouseX();
}

float Engine::get_mouse_y() {
    return (float)GetMouseY();
}

float Engine::get_delta_time() {
    return GetFrameTime();
}

int Engine::get_fps() {
    return GetFPS();
}

void Engine::draw_rectangle(int x, int y, int width, int height, int r, int g, int b, int a) {
    DrawRectangle(x, y, width, height, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

void Engine::draw_rectangle_lines(int x, int y, int width, int height, int r, int g, int b, int a) {
    DrawRectangleLines(x, y, width, height, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

void Engine::draw_line(int startPosX, int startPosY, int endPosX, int endPosY, int r, int g, int b, int a) {
    DrawLine(startPosX, startPosY, endPosX, endPosY, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

void Engine::draw_text(const std::string& text, int x, int y, int fontSize, int r, int g, int b, int a) {
    DrawText(text.c_str(), x, y, fontSize, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
}

bool Engine::check_collision_recs(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    Rectangle rec1 = { x1, y1, w1, h1 };
    Rectangle rec2 = { x2, y2, w2, h2 };
    return CheckCollisionRecs(rec1, rec2);
}
