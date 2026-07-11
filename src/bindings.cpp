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

    // Bind Engine
    py::class_<Engine>(m, "Engine")
        .def(py::init<int, int, const std::string&>(), "Initialize the game window")
        .def("is_running", &Engine::is_running, "Check if the window should close")
        .def("begin_drawing", &Engine::begin_drawing, "Begin rendering a frame")
        .def("end_drawing", &Engine::end_drawing, "End rendering a frame")
        .def("clear_background", &Engine::clear_background, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255, "Clear background color")
        .def("is_key_down", &Engine::is_key_down, "Check if a key is being held down")
        .def("is_key_pressed", &Engine::is_key_pressed, "Check if a key was pressed this frame")
        .def("get_delta_time", &Engine::get_delta_time, "Get time between frames")
        .def("draw_rectangle", &Engine::draw_rectangle, py::arg("x"), py::arg("y"), py::arg("width"), py::arg("height"), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a") = 255, "Draw a simple rectangle")
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
}
