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
pkg_search_module(GLFW REQUIRED glfw3)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

#SET(GCC_COVERAGE_LINK_FLAGS  "-ldl")
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")


#find_library(PocoFoundation_LIB foo)

if (NOT GLFW_FOUND)
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

#SET (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ldl -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm")


add_subdirectory(Source/Math)
include(Source/Core/CMakeSources.cmake)
include(Source/Engine/CMakeSources.cmake)
#include(apps/StopMotion/CMakeSources.cmake)
#include(apps/TyroVJ/CMakeSources.cmake)
#include(apps/TyroVJ/CMakeSources.cmake)
include(apps/face_warp/CMakeSources.cmake)

set(TYRO_MATH_INC_EXE 
    "./Source/Math"
	"./Source/Math/Algebra"
    "./Source/Math/Approximation"
    "./Source/Math/Assert"
    "./Source/Math/Base"
    "./Source/Math/ComputationalGeometry"
    "./Source/Math/Containment"
    "./Source/Math/CurvesSurfacesVolumes"
    "./Source/Math/DataTypes"
    "./Source/Math/Distance"
    "./Source/Math/InputOutput"
    "./Source/Math/Interpolation"
    "./Source/Math/Intersection"
    "./Source/Math/Memory"
    "./Source/Math/Meshes"
    "./Source/Math/Miscellaneous"
    "./Source/Math/NumericalAnalysis"
    "./Source/Math/Objects2D"
    "./Source/Math/Objects3D"
    "./Source/Math/ObjectSystems"
    "./Source/Math/Query"
    "./Source/Math/Rational"
    #"./Source/Math/Threading"
    "./Source/Math/Time"
    "./Source/Math/Tools"
    "./Source/Math/Transformation"
    )

set(TYRO_CORE_INC_EXE 
    "./Source/Core"
	"./Source/Core/ES2"
    "./Source/Core/Renderables"
    "./Source/Core/Scene"
    "./Source/Core/Tools"
    )

set(TYRO_ENGINE_INC_EXE
    "./Source/Engine"
   )

set(TYRO_EXTERNAL_INC_EXE
    "./Source/External"
    )

#set(TYROVJ_INC_EXE
#    "./apps/TyroVJ"
#    )    

set(TYROAPP_INC_EXE
    "./apps/face_warp"
    )    


#set(TYRO_STOPMOTION_INC_EXE 
#    "./StopMotion"
#    "./StopMotion/GraphCutMethods/GCOLib"
#    "./StopMotion/GraphCutMethods/kmeans"
#    "./StopMotion/GraphCutMethods/labeling"   
#    "./StopMotion/GraphCutMethods/segmentation" 
#    )

include_directories("./Source" 
                    ${TYRO_MATH_INC_EXE} 
                    ${TYRO_CORE_INC_EXE} 
                    ${TYRO_ENGINE_INC_EXE} 
                    ${TYRO_EXTERNAL_INC_EXE} 
                    ${TYROAPP_INC_EXE})
                    
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(SYSTEM "Source/External/libigl/include")
include_directories(SYSTEM "Source/External/libigl/external/eigen")
include_directories(SYSTEM "/home/rinat/Workspace/poco-1.9.0/Foundation/include")
include_directories(SYSTEM "/home/rinat/Workspace/Tyro/Source/External/tinyobjloader/")

add_executable(tyro main.cpp ${TYRO_CORE_SRC} ${TYRO_ENGINE_SRC} ${TYROFACEWARP_SRC})

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D_DEBUG)
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-D_RELEASE)
ENDIF()

target_link_libraries(tyro ${OPENGL_LIBRARIES})
target_link_libraries(tyro ${GLFW_LIBRARIES})
target_link_libraries(tyro  "/usr/lib64/libGLEW.so") #${GLEW_LIBRARIES})
target_link_libraries(tyro ${FREETYPE_LIBRARIES})
target_link_libraries(tyro ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(tyro ${OpenCV_LIBS})
target_link_libraries(tyro TyroMath)
target_link_libraries(tyro "/home/rinat/Workspace/poco-1.9.0/cmake-build/lib/libPocoFoundation.so")
target_link_libraries(tyro -ldl -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor)

#target_link_libraries(tyro "/usr/lib/x86_64-linux-gnu/libdl.so")
#target_link_libraries(tyro "/usr/lib/x86_64-linux-gnu/libX11.so")
#target_link_libraries(tyro "/usr/lib/x86_64-linux-gnu/libX11.so")

#target_compile_options(tyro PUBLIC -ldl)
#target_link_libraries(tyro Qt5::Core)
#target_link_libraries(tyro Qt5::Widgets)
#target_link_libraries(tyro TyroCore)
#target_link_libraries(tyro "/home/rinat/Workspace/Tyro/Source/External/libigl/lib/libigl_core.a")

