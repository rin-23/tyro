cmake_minimum_required(VERSION 3.5.0)
project(Tyro)

# Set OS-specific things here
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

message(STATUS "Compiling on Unix")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Running a debug build")
    message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES Release)
    message("Running a release build")
    message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
ENDIF()


# Find includes in corresponding build directories
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
#set(Qt5_DIR "/opt/Qt/5.9/gcc_64/lib/cmake/Qt5")
set(OpenCV_DIR /usr/local/share/OpenCV)


# Find the QtWidgets library
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW glfw3)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)


#find_library(PocoFoundation_LIB foo)

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

if (NOT OpenCV_FOUND)
    message(STATUS "NOT Found opencv")
endif()
#set(CMAKE_CXX_FLAGS "-Wall")


add_subdirectory(Math)
include(Core/CMakeSources.cmake)
include(Engine/CMakeSources.cmake)
include(StopMotion/CMakeSources.cmake)

#include(External/CMakeSources.cmake)

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
    #"./Math/Threading"
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

set(TYRO_EXTERNAL_INC_EXE
    "./External"
    )

set(TYRO_STOPMOTION_INC_EXE 
    "./StopMotion"
    "./StopMotion/GraphCutMethods/GCOLib"
    #"./StopMotion/GraphCutMethods/kmeans"
    "./StopMotion/GraphCutMethods/labeling"   
    "./StopMotion/GraphCutMethods/segmentation" 
    )

include_directories("./" ${TYRO_MATH_INC_EXE} ${TYRO_CORE_INC_EXE} ${TYRO_ENGINE_INC_EXE} ${TYRO_EXTERNAL_INC_EXE} ${TYRO_STOPMOTION_INC_EXE})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(SYSTEM "External/libigl/include")
include_directories(SYSTEM "External/libigl/external/eigen")
include_directories(SYSTEM "/home/rinat/Workspace/poco-1.9.0/Foundation/include")
include_directories(SYSTEM "/home/rinat/Workspace/Tyro/Source/External/tinyobjloader/")


add_executable(tyro main.cpp ${TYRO_CORE_SRC} ${TYRO_ENGINE_SRC} ${TYRO_GCOLIB_SRC} ${TYRO_STOPMOTION_SRC})

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D_DEBUG )
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-D_RELEASE )
ENDIF()

target_link_libraries(tyro ${OPENGL_LIBRARIES})
target_link_libraries(tyro ${GLFW_LIBRARIES})
target_link_libraries(tyro ${GLEW_LIBRARIES})
target_link_libraries(tyro ${FREETYPE_LIBRARIES})
target_link_libraries(tyro ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(tyro ${OpenCV_LIBS})
target_link_libraries(tyro TyroMath)
target_link_libraries(tyro "/home/rinat/Workspace/poco-1.9.0/cmake-build/lib/libPocoFoundation.so")
#target_link_libraries(tyro Qt5::Core)
#target_link_libraries(tyro Qt5::Widgets)
#target_link_libraries(tyro TyroCore)
#target_link_libraries(tyro "/home/rinat/Workspace/Tyro/Source/External/libigl/lib/libigl_core.a")

