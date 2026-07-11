#include "scene.h"
#include <cmath>

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
    registry.emplace_or_replace<Enemy>(static_cast<entt::entity>(entity), hp, speed, damage);
}

bool Scene::has_enemy(uint32_t entity) {
    return registry.all_of<Enemy>(static_cast<entt::entity>(entity));
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
