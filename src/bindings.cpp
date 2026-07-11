#include <pybind11/pybind11.h>
#include "engine.h"
#include "raylib.h"

namespace py = pybind11;

PYBIND11_MODULE(engine, m) {
    m.doc() = "C++ Game Engine built with Raylib and Pybind11";

    // Bind TextureManager
    py::class_<TextureManager>(m, "TextureManager")
        .def("load_texture", &TextureManager::load_texture, "Load a texture from a file")
        .def("draw_texture", &TextureManager::draw_texture, "Draw a loaded texture");

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
        .def("draw_rectangle", &Engine::draw_rectangle, py::arg("x"), py::arg("y"), py::arg("width"), py::arg("height"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255, "Draw a simple rectangle")
        .def("draw_line", &Engine::draw_line, py::arg("startX"), py::arg("startY"), py::arg("endX"), py::arg("endY"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255, "Draw a line")
        .def("get_texture_manager", &Engine::get_texture_manager, py::return_value_policy::reference, "Get the texture manager");

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
        .export_values();

    // Mouse buttons enum
    py::enum_<MouseButton>(m, "MouseButtons")
        .value("MOUSE_BUTTON_LEFT", MOUSE_BUTTON_LEFT)
        .value("MOUSE_BUTTON_RIGHT", MOUSE_BUTTON_RIGHT)
        .value("MOUSE_BUTTON_MIDDLE", MOUSE_BUTTON_MIDDLE)
        .export_values();
}
