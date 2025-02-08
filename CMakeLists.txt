cmake_minimum_required(VERSION 3.12)

project(Plotter3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)
set(wxWidgets_USE_STATIC ON)

add_definitions(-DwxUSE_GUI=1)
add_compile_definitions(_USE_MATH_DEFINES)

find_package(wxWidgets REQUIRED COMPONENTS core base gl)
find_package(OpenGL REQUIRED)

include_directories(include)
include_directories(libs/glad/include)
include_directories(libs)
include(${wxWidgets_USE_FILE})

file(GLOB_RECURSE SOURCES "src/*.cpp" "libs/glad/src/glad.c")

if (WIN32)
    add_executable(Plotter3D WIN32 ${SOURCES})
elseif (UNIX)
    add_executable(Plotter3D ${SOURCES})
endif()

target_include_directories(Plotter3D PRIVATE)
target_link_libraries(Plotter3D PRIVATE ${wxWidgets_LIBRARIES} ${OPENGL_LIBRARIES})