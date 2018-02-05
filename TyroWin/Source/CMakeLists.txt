cmake_minimum_required(VERSION 3.0.0)

project(Tyro)

# Set OS-specific things here
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
message(STATUS "Compiling on Unix")


find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW glfw3)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

if (NOT GLFW_FOUND )
    message(STATUS "NOT Found glfw")
endif()

if (NOT OPENGL_FOUND)
    message(STATUS "NOT Found opengl")
endif()

if (NOT GLEW_FOUND)
    message(STATUS "NOT Found glew")
endif()


include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})

add_executable(tyro mainTest.cpp)
target_link_libraries(tyro ${OPENGL_gl_LIBRARY})
target_link_libraries(tyro ${GLFW_LIBRARIES})
target_link_libraries(tyro ${GLEW_LIBRARIES})
