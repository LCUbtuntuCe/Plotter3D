cmake_minimum_required(VERSION 3.10)

project(3DPlotter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find wxwidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)

set(wxWidgets_USE_STATIC ON)
add_definitions(-DwxUSE_GUI=1)

include(${wxWidgets_USE_FILE})

# add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(3DPlotter ${SOURCES})

# link wx libraries
target_link_libraries(3DPlotter ${wxWidgets_LIBRARIES})
