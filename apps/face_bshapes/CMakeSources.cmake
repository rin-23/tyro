set(TYRO_INC_LOCAL 
"apps/face_bshapes/app.h"
"apps/face_bshapes/bshapes.h"
"apps/face_bshapes/OpenFaceTexture.h"
)

set(TYRO_SRC_LOCAL 
"apps/face_bshapes/app.cpp"
"apps/face_bshapes/bshapes.cpp"
"apps/face_bshapes/animation.cpp"
"apps/face_bshapes/OpenFaceTexture.cpp"
"apps/face_bshapes/torch_model.cpp"

)

set(TYROAPP_SRC
${TYRO_SRC_LOCAL}
)