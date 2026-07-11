#pragma once
#include <vector>
#include <cstdint>
#include <utility>
#include <entt/entt.hpp>
#include "components.h"

class Scene {
public:
    Scene();
    ~Scene();

    uint32_t create_entity();
    void destroy_entity(uint32_t entity);

    void add_transform(uint32_t entity, float x, float y);
    void set_transform(uint32_t entity, float x, float y);
    std::pair<float, float> get_transform(uint32_t entity);

    void add_collider(uint32_t entity, float width, float height);
    bool has_collider(uint32_t entity);
    std::pair<float, float> get_collider(uint32_t entity);

    void add_enemy(uint32_t entity, float hp, float speed, float damage);
    bool has_enemy(uint32_t entity);

    uint32_t get_pooled_entity();
    void return_pooled_entity(uint32_t entity);

    void update_active_chunks(float center_x, float center_y, float radius);
    std::vector<uint32_t> get_active_entities();

    entt::registry& get_registry() { return registry; }

private:
    entt::registry registry;
    std::vector<uint32_t> entity_pool;
};
