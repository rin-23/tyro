set(TYRO_INC_LOCAL 
"StopMotion/TyroWindow.h"
"StopMotion/TyroApp.h"
"StopMotion/TyroIGLMesh.h"
"StopMotion/compute_deformation.h"
"StopMotion/TyroTimeLine.h"
"StopMotion/load_mesh_sequence.h"
"StopMotion/mesh_split.h"

)

set(TYRO_SRC_LOCAL 
"StopMotion/TyroWindow.cpp" 
"StopMotion/TyroApp.cpp"
"StopMotion/TyroIGLMesh.cpp"
"StopMotion/compute_deformation.cpp"
"StopMotion/TyroTimeLine.cpp"
"StopMotion/load_mesh_sequence.cpp"
"StopMotion/mesh_split.cpp"
"StopMotion/kmeans.cpp"
"StopMotion/ES2VideoTexture.cpp"

"StopMotion/GraphCutMethods/GCOLib/GCoptimization.cpp" 
"StopMotion/GraphCutMethods/GCOLib/graph.cpp" 
"StopMotion/GraphCutMethods/GCOLib/LinkedBlockList.cpp" 
"StopMotion/GraphCutMethods/GCOLib/maxflow.cpp" 

#"StopMotion/GraphCutMethods/kmeans/kmeans.cpp" 
#"StopMotion/GraphCutMethods/kmeans/point.cpp" 

"StopMotion/GraphCutMethods/labeling/labelingStep.cpp" 
"StopMotion/GraphCutMethods/labeling/updateStep.cpp" 
"StopMotion/GraphCutMethods/labeling/utils.cpp" 
"StopMotion/GraphCutMethods/labeling/stop_motion.cpp" 

"StopMotion/GraphCutMethods/segmentation/segmentation.cpp" 
)

set(TYRO_STOPMOTION_SRC
${TYRO_SRC_LOCAL}
)