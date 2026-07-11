#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../core/engine.h"
#include "../graphics/camera.h"
#include "raylib.h"

namespace py = pybind11;

PYBIND11_MODULE(engine, m) {
    m.doc() = "C++ Game Engine built with Raylib and Pybind11";

    // Bind TextureManager
    py::class_<TextureManager>(m, "TextureManager")
        .def("load_texture", &TextureManager::load_texture, "Load a texture from a file")
        .def("draw_texture", &TextureManager::draw_texture, "Draw a loaded texture")
        .def("draw_texture_rec", &TextureManager::draw_texture_rec,
            py::arg("name"), py::arg("source_x"), py::arg("source_y"), py::arg("source_w"), py::arg("source_h"),
            py::arg("dest_x"), py::arg("dest_y"), "Draw a part of a texture")
        .def("load_sprite_sheet", &TextureManager::load_sprite_sheet, py::arg("name"), py::arg("filepath"), "Load a sprite sheet using the intelligent AssetManager")
        .def("draw_sprite_frame", &TextureManager::draw_sprite_frame, py::arg("name"), py::arg("frame_index"), py::arg("dest_x"), py::arg("dest_y"), "Draw a specific frame from a loaded sprite sheet");

    // Bind Scene (ECS)
    py::class_<Scene>(m, "Scene")
        .def("create_entity", &Scene::create_entity, "Create a raw new entity")
        .def("destroy_entity", &Scene::destroy_entity, py::arg("entity"), "Destroy an entity")
        .def("get_pooled_entity", &Scene::get_pooled_entity, "Get an entity from the pre-allocated pool")
        .def("return_pooled_entity", &Scene::return_pooled_entity, py::arg("entity"), "Return an entity back to the pool")
        .def("add_transform", &Scene::add_transform, py::arg("entity"), py::arg("x"), py::arg("y"), "Add or replace Transform2D component")
        .def("set_transform", &Scene::set_transform, py::arg("entity"), py::arg("x"), py::arg("y"), "Update existing Transform2D component")
        .def("get_transform", &Scene::get_transform, py::arg("entity"), "Get Transform2D component as (x, y) tuple")
        .def("add_collider", &Scene::add_collider, py::arg("entity"), py::arg("width"), py::arg("height"), "Add or replace Collider component")
        .def("has_collider", &Scene::has_collider, py::arg("entity"), "Check if entity has a Collider")
        .def("get_collider", &Scene::get_collider, py::arg("entity"), "Get Collider component as (width, height) tuple")
        .def("add_enemy", &Scene::add_enemy, py::arg("entity"), py::arg("hp"), py::arg("speed"), py::arg("damage"), "Add or replace Enemy component")
        .def("has_enemy", &Scene::has_enemy, py::arg("entity"), "Check if entity has an Enemy component")
        .def("update_active_chunks", &Scene::update_active_chunks, py::arg("center_x"), py::arg("center_y"), py::arg("radius"), "Update which entities are active based on distance from center")
        .def("get_active_entities", &Scene::get_active_entities, "Get a list of all currently active entities");

    // Bind GameCamera
    py::class_<GameCamera>(m, "GameCamera")
        .def(py::init<>(), "Create a new 2D Camera")
        .def("set_target", &GameCamera::set_target, py::arg("x"), py::arg("y"), "Set camera target position")
        .def("set_offset", &GameCamera::set_offset, py::arg("x"), py::arg("y"), "Set camera offset position")
        .def("set_zoom", &GameCamera::set_zoom, py::arg("zoom"), "Set camera zoom level")
        .def("begin_mode", &GameCamera::begin_mode, "Begin 2D camera mode")
        .def("end_mode", &GameCamera::end_mode, "End 2D camera mode")
        .def("get_screen_to_world", &GameCamera::get_screen_to_world, py::arg("screen_x"), py::arg("screen_y"), "Convert screen coordinate to world coordinate");

    // Bind Engine
    py::class_<Engine>(m, "Engine")
        .def(py::init<int, int, const std::string&>(), "Initialize the game window")
        .def("is_running", &Engine::is_running, "Check if the window should close")
        .def("begin_drawing", &Engine::begin_drawing, "Begin rendering a frame")
        .def("end_drawing", &Engine::end_drawing, "End rendering a frame")
        .def("clear_background", &Engine::clear_background, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255, "Clear background color")
        .def("is_key_down", &Engine::is_key_down, "Check if a key is being held down")
        .def("is_key_pressed", &Engine::is_key_pressed, "Check if a key was pressed this frame")
        .def("is_mouse_button_down", &Engine::is_mouse_button_down, "Check if a mouse button is being held down")
        .def("is_mouse_button_pressed", &Engine::is_mouse_button_pressed, "Check if a mouse button was pressed this frame")
        .def("get_mouse_x", &Engine::get_mouse_x, "Get mouse X position on screen")
        .def("get_mouse_y", &Engine::get_mouse_y, "Get mouse Y position on screen")
        .def("get_delta_time", &Engine::get_delta_time, "Get time between frames")
        .def("get_fps", &Engine::get_fps, "Get current frames per second")
        .def("draw_rectangle", &Engine::draw_rectangle, py::arg("x"), py::arg("y"), py::arg("width"), py::arg("height"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255, "Draw a simple rectangle")
        .def("draw_rectangle_lines", &Engine::draw_rectangle_lines, py::arg("x"), py::arg("y"), py::arg("width"), py::arg("height"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255, "Draw rectangle outlines")
        .def("draw_line", &Engine::draw_line, py::arg("startX"), py::arg("startY"), py::arg("endX"), py::arg("endY"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255, "Draw a line")
        .def("draw_text", &Engine::draw_text, py::arg("text"), py::arg("x"), py::arg("y"), py::arg("fontSize"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255, "Draw text")
        .def("check_collision_recs", &Engine::check_collision_recs, py::arg("x1"), py::arg("y1"), py::arg("w1"), py::arg("h1"), py::arg("x2"), py::arg("y2"), py::arg("w2"), py::arg("h2"), "Check if two rectangles collide")
        .def("get_texture_manager", &Engine::get_texture_manager, py::return_value_policy::reference, "Get the texture manager")
        .def("get_scene", &Engine::get_scene, py::return_value_policy::reference, "Get the ECS Scene manager");

    // Keyboard keys enum
    py::enum_<KeyboardKey>(m, "Keys")
        .value("KEY_W", KEY_W)
        .value("KEY_A", KEY_A)
        .value("KEY_S", KEY_S)
        .value("KEY_D", KEY_D)
        .value("KEY_UP", KEY_UP)
        .value("KEY_DOWN", KEY_DOWN)
        .value("KEY_LEFT", KEY_LEFT)
        .value("KEY_RIGHT", KEY_RIGHT)
        .value("KEY_SPACE", KEY_SPACE)
        .value("KEY_ESCAPE", KEY_ESCAPE)
        .value("KEY_F3", KEY_F3)
        .export_values();

    // Mouse buttons enum
    py::enum_<MouseButton>(m, "MouseButtons")
        .value("MOUSE_BUTTON_LEFT", MOUSE_BUTTON_LEFT)
        .value("MOUSE_BUTTON_RIGHT", MOUSE_BUTTON_RIGHT)
        .value("MOUSE_BUTTON_MIDDLE", MOUSE_BUTTON_MIDDLE)
        .export_values();
}
