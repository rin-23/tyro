cmake_minimum_required(VERSION 3.5.0)
project(Tyro)

# Set OS-specific things here
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Compiling on Unix")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Running a debug build") 
ENDIF()

message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW glfw3)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)

if (NOT GLFW_FOUND )
    message(STATUS "NOT Found glfw")
endif()

if (NOT OPENGL_FOUND)
    message(STATUS "NOT Found opengl")
endif()

if (NOT GLEW_FOUND)
    message(STATUS "NOT Found glew")
endif()

if (NOT OPENGL_GLU_FOUND)
    message(STATUS "NOT Found glut")
endif()

if (NOT FREETYPE_FOUND)
    message(STATUS "NOT Found freetype")
endif()



add_subdirectory(Math)
include(Core/CMakeSources.cmake)
include(Engine/CMakeSources.cmake)
include(../../libigl/include)

set(TYRO_MATH_INC_EXE 
    "./Math"
	"./Math/Algebra"
    "./Math/Approximation"
    "./Math/Assert"
    "./Math/Base"
    "./Math/ComputationalGeometry"
    "./Math/Containment"
    "./Math/CurvesSurfacesVolumes"
    "./Math/DataTypes"
    "./Math/Distance"
    "./Math/InputOutput"
    "./Math/Interpolation"
    "./Math/Intersection"
    "./Math/Memory"
    "./Math/Meshes"
    "./Math/Miscellaneous"
    "./Math/NumericalAnalysis"
    "./Math/Objects2D"
    "./Math/Objects3D"
    "./Math/ObjectSystems"
    "./Math/Query"
    "./Math/Rational"
    "./Math/Threading"
    "./Math/Time"
    "./Math/Tools"
    "./Math/Transformation"
    )

set(TYRO_CORE_INC_EXE 
    "./Core"
	"./Core/ES2"
    "./Core/Renderables"
    "./Core/Scene"
    "./Core/Tools"
    )


set(TYRO_ENGINE_INC_EXE
    "./Engine"
)

include_directories("./" ${TYRO_MATH_INC_EXE} ${TYRO_CORE_INC_EXE} ${TYRO_ENGINE_INC_EXE} "./External")
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})

add_executable(tyro main.cpp ${TYRO_CORE_SRC} ${TYRO_ENGINE_SRC})

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D_DEBUG)
    #target_compile_definitions(tyro PRIVATE _DEBUG) 
ENDIF()

target_link_libraries(tyro ${OPENGL_LIBRARIES})
target_link_libraries(tyro ${GLFW_LIBRARIES})
target_link_libraries(tyro ${GLEW_LIBRARIES})
target_link_libraries(tyro ${FREETYPE_LIBRARIES})
target_link_libraries(tyro TyroMath)
#target_link_libraries(tyro TyroCore)

