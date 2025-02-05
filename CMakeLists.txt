cmake_minimum_required(VERSION 3.10)

project(Plotter3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(wxWidgets REQUIRED COMPONENTS core base gl)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

set(wxWidgets_USE_STATIC ON)
add_definitions(-DwxUSE_GUI=1)

include(${wxWidgets_USE_FILE})

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(Plotter3D ${SOURCES})

target_include_directories(Plotter3D PRIVATE ${GLEW_INCLUDE_DIRS})
target_link_libraries(Plotter3D ${wxWidgets_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
