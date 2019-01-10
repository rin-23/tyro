set(TYRO_INC_LOCAL 
"Source/Engine/RAAxisAlignedBBox.h" 
"Source/Engine/RADepthProxy.h" 
"Source/Engine/RAES2TextOverlay.h" 
"Source/Engine/RAHitProxy.h" 
"Source/Engine/RANormalMapProxy.h" 
"Source/Engine/RAOverlayManager.h" 
"Source/Engine/load_mesh_sequence.h"
#"Engine/RAPointOctree.h" 
"Source/Engine/TyroWindow.h"
#"Engine/TyroApp.h"
"Source/Engine/Console.h"
"Source/Engine/stb_image_write.h"
"Source/Engine/stb_image.h"
"Source/Engine/TyroIGLMesh.h"
#"Engine/stop_motion_data.h"
#"Engine/TyroTimeLine.h"


#"Engine/stbi_DDS_aug.h"
#"Engine/stbi_DDS_aug_c.h"
)

set(TYRO_SRC_LOCAL 
"Source/Engine/load_mesh_sequence.cpp"
"Source/Engine/RAAxisAlignedBBox.cpp" 
"Source/Engine/RADepthProxy.cpp" 
"Source/Engine/RAES2TextOverlay.cpp" 
"Source/Engine/RAHitProxy.cpp" 
"Source/Engine/RANormalMapProxy.cpp" 
"Source/Engine/RAOverlayManager.cpp" 
#"Engine/RAPointOctree.cpp"
"Source/Engine/TyroWindow.cpp" 
#"Engine/TyroApp.cpp"
"Source/Engine/Console.cpp"
"Source/Engine/stb_image_write.c"
"Source/Engine/stb_image.c"
"Source/Engine/TyroIGLMesh.cpp"
#"Engine/stop_motion_data.cpp"
"Source/Engine/TyroTimeLine.cpp"
)

set(TYRO_ENGINE_SRC
${TYRO_SRC_LOCAL}
)