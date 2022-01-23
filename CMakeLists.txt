cmake_minimum_required (VERSION 3.10)
project (ParserGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

add_subdirectory(lib/glfw-3.3.6)

include_directories(
    lib/
    lib/glfw-3.3.6/include/
    lib/imgui
    lib/imgui/backends
)

set(ALL_LIBS
        ${OPENGL_LIBRARY}
        glfw
        ${GLFW_LIBRARIES}
)


add_executable(parsegen 
                src/main.cpp
                src/maingui.cpp
                src/rule.cpp
                src/drawing.cpp
                src/node.cpp
                
                src/parsetreerenderer.cpp
                
                src/parsergenerator/util/symbol.cpp
                
                lib/imgui/imgui.cpp
                lib/imgui/imgui_demo.cpp
                lib/imgui/imgui_draw.cpp
                lib/imgui/imgui_tables.cpp
                lib/imgui/imgui_widgets.cpp
                lib/imgui/backends/imgui_impl_opengl3.cpp
                lib/imgui/backends/imgui_impl_glfw.cpp

                lib/imgui/nodes/crude_json.cpp
                lib/imgui/nodes/imgui_canvas.cpp
                lib/imgui/nodes/imgui_node_editor_api.cpp
                lib/imgui/nodes/imgui_node_editor.cpp
)


target_link_libraries(parsegen ${ALL_LIBS})

add_custom_command(
    TARGET parsegen PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/lib/fonts/
            ${CMAKE_CURRENT_BINARY_DIR}/fonts/
)

add_custom_command(
    TARGET parsegen PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
            ${CMAKE_CURRENT_BINARY_DIR}/templates/
)

add_custom_command(
    TARGET parsegen PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/templates/
            ${CMAKE_CURRENT_BINARY_DIR}/templates/
)
