include(CMakeSources.cmake)

set(TYRO_MATH_INC 
    "./"
    "../External"
	"./Algebra" 
    #"./Approximation" 
    "./Assert"
    "./Base" 
    #"./ComputationalGeometry" 
    #"./Containment"
    #"./CurvesSurfacesVolumes" 
    "./DataTypes"
    #"./Distance" 
    "./InputOutput" 
    #"./Interpolation" 
    #"./Intersection" 
    "./Memory"
    #"./Meshes" 
    #"./Miscellaneous" 
    "./NumericalAnalysis" 
    #"./Objects2D"
    #"./Objects3D" 
    "./ObjectSystems" 
    "./Query"
    "./Rational" 
    #"./Threading" 
    "./Time" 
    #"./Tools" 
    #"./Transformation"
    )

include_directories(${TYRO_MATH_INC})	

add_library(TyroMath SHARED ${TYRO_MATH_SRC})
