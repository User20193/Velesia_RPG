#include "scene.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "raylib.h"

using json = nlohmann::json;

Scene::Scene() {
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

std::pair<float, float> Scene::get_collider(uint32_t entity) {
    if(registry.all_of<Collider>(static_cast<entt::entity>(entity))) {
        auto& c = registry.get<Collider>(static_cast<entt::entity>(entity));
        return {c.width, c.height};
    }
    return {0.0f, 0.0f};
}

void Scene::add_enemy(uint32_t entity, float hp, float speed, float damage) {
    registry.emplace_or_replace<Enemy>(static_cast<entt::entity>(entity), hp, hp, speed, damage);
}

bool Scene::has_enemy(uint32_t entity) {
    return registry.all_of<Enemy>(static_cast<entt::entity>(entity));
}

float Scene::get_enemy_hp(uint32_t entity) {
    if (has_enemy(entity)) {
        return registry.get<Enemy>(static_cast<entt::entity>(entity)).hp;
    }
    return 0.0f;
}

void Scene::set_enemy_hp(uint32_t entity, float hp) {
    if (has_enemy(entity)) {
        registry.get<Enemy>(static_cast<entt::entity>(entity)).hp = hp;
    }
}

float Scene::get_enemy_max_hp(uint32_t entity) {
    if (has_enemy(entity)) {
        return registry.get<Enemy>(static_cast<entt::entity>(entity)).max_hp;
    }
    return 1.0f;
}

uint32_t Scene::get_pooled_entity() {
    if (!entity_pool.empty()) {
        uint32_t ent = entity_pool.back();
        entity_pool.pop_back();
        return ent;
    }
    return create_entity();
}

void Scene::return_pooled_entity(uint32_t entity) {
    auto entt_id = static_cast<entt::entity>(entity);
    registry.destroy(entt_id);
    entity_pool.push_back(static_cast<uint32_t>(registry.create()));
}

void Scene::update_active_chunks(float center_x, float center_y, float radius) {
    registry.clear<ActiveChunkTag>();

    auto view = registry.view<Transform2D>();
    for (auto entity : view) {
        auto& t = view.get<Transform2D>(entity);
        if (std::abs(t.x - center_x) <= radius && std::abs(t.y - center_y) <= radius) {
            registry.emplace<ActiveChunkTag>(entity);
        }
    }
}

std::vector<uint32_t> Scene::get_active_entities() {
    std::vector<uint32_t> active_ents;
    auto view = registry.view<ActiveChunkTag>();
    for (auto entity : view) {
        active_ents.push_back(static_cast<uint32_t>(entity));
    }
    return active_ents;
}

bool Scene::save_to_json(const std::string& filepath) {
    try {
        json out;
        out["entities"] = json::array();

        // Iterate over all entities that have a Transform2D (which is our base requirement for world objects)
        auto view = registry.view<Transform2D>();
        for (auto entity : view) {
            json j_ent;
            j_ent["id"] = static_cast<uint32_t>(entity);

            auto& t = view.get<Transform2D>(entity);
            j_ent["Transform2D"] = {{"x", t.x}, {"y", t.y}};

            if (registry.all_of<Collider>(entity)) {
                auto& c = registry.get<Collider>(entity);
                j_ent["Collider"] = {{"width", c.width}, {"height", c.height}};
            }

            if (registry.all_of<Enemy>(entity)) {
                auto& e = registry.get<Enemy>(entity);
                j_ent["Enemy"] = {{"hp", e.hp}, {"speed", e.speed}, {"damage", e.damage}};
            }

            out["entities"].push_back(j_ent);
        }

        std::ofstream f(filepath);
        f << std::setw(4) << out << std::endl;
        TraceLog(LOG_INFO, "SCENE: Saved to %s", filepath.c_str());
        return true;
    } catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "SCENE: Failed to save %s: %s", filepath.c_str(), e.what());
    }
    return false;
}

bool Scene::load_from_json(const std::string& filepath) {
    try {
        std::ifstream f(filepath);
        if (!f.is_open()) return false;

        json data = json::parse(f);

        // Clear current active non-pooled entities (reset scene)
        registry.clear();
        entity_pool.clear();

        // Re-init pool
        for (int i = 0; i < 1000; ++i) {
            auto entity = registry.create();
            entity_pool.push_back(static_cast<uint32_t>(entity));
        }

        if (data.contains("entities") && data["entities"].is_array()) {
            for (const auto& j_ent : data["entities"]) {
                uint32_t ent = get_pooled_entity();

                if (j_ent.contains("Transform2D")) {
                    add_transform(ent, j_ent["Transform2D"]["x"], j_ent["Transform2D"]["y"]);
                }

                if (j_ent.contains("Collider")) {
                    add_collider(ent, j_ent["Collider"]["width"], j_ent["Collider"]["height"]);
                }

                if (j_ent.contains("Enemy")) {
                    add_enemy(ent, j_ent["Enemy"]["hp"], j_ent["Enemy"]["speed"], j_ent["Enemy"]["damage"]);
                }
            }
        }

        TraceLog(LOG_INFO, "SCENE: Loaded from %s", filepath.c_str());
        return true;
    } catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "SCENE: Failed to load %s: %s", filepath.c_str(), e.what());
    }
    return false;
}
