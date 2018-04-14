set(TYRO_INC_LOCAL 
"Engine/RAAxisAlignedBBox.h" 
"Engine/RADepthProxy.h" 
"Engine/RAES2TextOverlay.h" 
"Engine/RAHitProxy.h" 
"Engine/RANormalMapProxy.h" 
"Engine/RAOverlayManager.h" 
#"Engine/RAPointOctree.h" 
"Engine/TyroWindow.h"
"Engine/TyroApp.h"
"Engine/Console.h"
"Engine/stb_image_write.h"
"Engine/stb_image.h"
"Engine/TyroIGLMesh.h"
"Engine/stop_motion_data.h"
"Engine/TyroTimeLine.h"


#"Engine/stbi_DDS_aug.h"
#"Engine/stbi_DDS_aug_c.h"
)

set(TYRO_SRC_LOCAL 
"Engine/RAAxisAlignedBBox.cpp" 
"Engine/RADepthProxy.cpp" 
"Engine/RAES2TextOverlay.cpp" 
"Engine/RAHitProxy.cpp" 
"Engine/RANormalMapProxy.cpp" 
"Engine/RAOverlayManager.cpp" 
#"Engine/RAPointOctree.cpp"
"Engine/TyroWindow.cpp" 
"Engine/TyroApp.cpp"
"Engine/Console.cpp"
"Engine/stb_image_write.c"
"Engine/stb_image.c"
"Engine/TyroIGLMesh.cpp"
"Engine/stop_motion_data.cpp"
"Engine/TyroTimeLine.cpp"
)

set(TYRO_ENGINE_SRC
${TYRO_SRC_LOCAL}
)