//
//  RAEnginePrerequisites.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-02-08.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RALogManager.h"
#include <memory>

namespace tyro
{
    /// Gesture State. Similar to iOS
    enum class GestureState {Possible, Began, Changed, Ended, Canceled, Failed};
    
    class NormalProxy;
    class Touchable;
    class DepthProxy;
    class TriHitProxy;
    struct Joint;
    struct Skeleton;
    struct SkeletonPose;
    class ES2Font;
    class FontManager;
    class ES2VertexFormat;
    class VertexHardwareBuffer;
    class TextureBuffer;
    class ES2TextOverlay;
    class HitProxy;
    class Camera;
    class Spatial;
    class VisibleSet;
    class ES2VertexFormat;
    class ES2ShaderProgram;
    class ES2ShaderUniforms;
    class ES2VisualStates;
    class ES2Octree;
	class ES2Context;
    class TriOctree;
    struct TouchInfo;
    class AxisAlignedBBox;
    class Node;
    class Spatial;
    class ES2Renderable;
    class iOSCamera;
    class ES2Cylinder;
    class ES2Sphere;
    class ES2Box;
    class ES2Line;
    class ES2Rectangle;
    class ES2Vector;
    class MarchingCubes;
    class ES2TriMesh;
    class ES2BBox;
    class ES2VisualEffect;
    class ES2Texture2D;
    class ES2VertexHardwareBuffer;
    class ES2VertexArray;
    class ES2IndexHardwareBuffer;
    class ES2Polyline;
    class ES2Polypoint;
    class ES2DefaultPolyline;
    class ES2DefaultTriMesh;
    class ES2BufferTexture;
    class ES2BBox;
    class RectHitProxy;
    class TriHitProxy;
    class DepthProxy;
    class IGLMesh;
    class IGLMeshWireframe;
    class ES2PolygonOffset;
    class ES2VideoTexture;
    class ES2CameraTexture;
    class ShaderBox;
    class OpenFaceTexture;
    class IGLMeshTwo;
    class MuscleMesh;
    
    
    using DepthProxySPtr = std::shared_ptr<DepthProxy>;
    using DepthProxyWPtr = std::weak_ptr<DepthProxy>;
    using DepthProxyUPtr = std::weak_ptr<DepthProxy>;
    
    using TriHitProxySPtr = std::shared_ptr<TriHitProxy>;
    using TriHitProxyWPtr = std::weak_ptr<TriHitProxy>;
    using TriHitProxyUPtr = std::weak_ptr<TriHitProxy>;
    
    using RectHitProxySPtr = std::shared_ptr<RectHitProxy>;
    using RectHitProxyWPtr = std::weak_ptr<RectHitProxy>;
    using RectHitProxyUPtr = std::weak_ptr<RectHitProxy>;

    using ES2VectorSPtr = std::shared_ptr<ES2Vector>;
    using ES2VectorWPtr = std::weak_ptr<ES2Vector>;
    using ES2VectorUPtr = std::weak_ptr<ES2Vector>;

    using ES2RectangleSPtr = std::shared_ptr<ES2Rectangle>;
    using ES2RectangleWPtr = std::weak_ptr<ES2Rectangle>;
    using ES2RectangleUPtr = std::weak_ptr<ES2Rectangle>;
    
    using HitProxySPtr = std::shared_ptr<HitProxy>;
    using HitProxyWPtr = std::weak_ptr<HitProxy>;
    using HitProxyUPtr = std::weak_ptr<HitProxy>;
    
    using ES2TextOverlaySPtr = std::shared_ptr<ES2TextOverlay>;
    using ES2TextOverlayWPtr = std::weak_ptr<ES2TextOverlay>;
    using ES2TextOverlayUPtr = std::weak_ptr<ES2TextOverlay>;
    
    using ES2TextOverlaySPtr = std::shared_ptr<ES2TextOverlay>;
    using ES2TextOverlayWPtr = std::weak_ptr<ES2TextOverlay>;
    using ES2TextOverlayUPtr = std::weak_ptr<ES2TextOverlay>;

    using ES2BBoxSPtr = std::shared_ptr<ES2BBox>;
    using ES2BBoxWPtr = std::weak_ptr<ES2BBox>;
    
    using ES2DefaultTriMeshSPtr = std::shared_ptr<ES2DefaultTriMesh>;
    using ES2DefaultTriMeshWPtr = std::weak_ptr<ES2DefaultTriMesh>;

    using ES2DefaultPolylineSPtr = std::shared_ptr<ES2DefaultPolyline>;
    using ES2DefaultPolylineWPtr = std::weak_ptr<ES2DefaultPolyline>;

    using ES2PolypointSPtr = std::shared_ptr<ES2Polypoint>;
    using ES2PolypointWPtr = std::weak_ptr<ES2Polypoint>;
    
    using ES2PolylineSPtr = std::shared_ptr<ES2Polyline>;
    using ES2PolylineWPtr = std::weak_ptr<ES2Polyline>;
    
    using SpatialUPtr = std::unique_ptr<Spatial>;
    using SpatialSPtr = std::shared_ptr<Spatial>;
    using SpatialWPtr = std::weak_ptr<Spatial>;
    
    using ES2OctreeSPtr = std::shared_ptr<ES2Octree>;
    using ES2OctreeWPtr = std::weak_ptr<ES2Octree>;
    
    using ES2TriMeshSPtr = std::shared_ptr<ES2TriMesh>;
    using ES2TriMeshWPtr = std::weak_ptr<ES2TriMesh>;
    
    using ES2CylinderSPtr = std::shared_ptr<ES2Cylinder>;
    using ES2CylinderWPtr = std::weak_ptr<ES2Cylinder>;
    
    using ES2SphereSPtr = std::shared_ptr<ES2Sphere>;
    using ES2SphereWPtr = std::weak_ptr<ES2Sphere>;
    
    using ES2BoxSPtr = std::shared_ptr<ES2Box>;
    using ES2BoxWPtr = std::weak_ptr<ES2Box>;
    
    using ES2LineSPtr = std::shared_ptr<ES2Line>;
    using ES2LineWPtr = std::weak_ptr<ES2Line>;

    using ES2RectangleSPtr = std::shared_ptr<ES2Rectangle>;
    using ES2RectangleWPtr = std::weak_ptr<ES2Rectangle>;
    
    using ES2VectorSPtr = std::shared_ptr<ES2Vector>;
    using ES2VectorWPtr = std::weak_ptr<ES2Vector>;
    
    using ES2RenderableSPtr = std::shared_ptr<ES2Renderable>;
    using ES2RenderableWPtr = std::weak_ptr<ES2Renderable>;
    
    using TouchableSPtr = std::shared_ptr<Touchable>;
    using TouchableWPtr = std::weak_ptr<Touchable>;
    
    using ES2FontSPtr = std::shared_ptr<ES2Font>;
    using ES2FontWPtr = std::weak_ptr<ES2Font>;
    
    using NodeSPtr = std::shared_ptr<Node>;
    using NodeWPtr = std::weak_ptr<Node>;
    
    using ES2VisualEffectSPtr = std::shared_ptr<ES2VisualEffect>;
    using ES2VisualEffectWPtr = std::weak_ptr<ES2VisualEffect>;
    
    using ES2Texture2DSPtr =std::shared_ptr<ES2Texture2D>;
    using ES2Texture2DWPtr =std::weak_ptr<ES2Texture2D>;

    using ES2VertexHardwareBufferSPtr = std::shared_ptr<ES2VertexHardwareBuffer>;
    using ES2VertexHardwareBufferWPtr = std::weak_ptr<ES2VertexHardwareBuffer>;

    using ES2IndexHardwareBufferSPtr = std::shared_ptr<ES2IndexHardwareBuffer>;
    using ES2IndexHardwareBufferWPtr = std::weak_ptr<ES2IndexHardwareBuffer>;
    
	using ES2VertexArraySPtr = std::shared_ptr<ES2VertexArray>;

    using NormalProxySPtr = std::shared_ptr<NormalProxy>;
    using NormalProxyUPtr = std::unique_ptr<NormalProxy>;
    using NormalProxyWPtr = std::weak_ptr<NormalProxy>;
    
    using NormalProxySPtr = std::shared_ptr<NormalProxy>;
    using NormalProxyUPtr = std::unique_ptr<NormalProxy>;
    using NormalProxyWPtr = std::weak_ptr<NormalProxy>;

    using IGLMeshSPtr = std::shared_ptr<IGLMesh>;
    using IGLMeshUPtr = std::unique_ptr<IGLMesh>;
    using IGLMeshWPtr = std::weak_ptr<IGLMesh>;


    using MuscleMeshSPtr = std::shared_ptr<MuscleMesh>;
    using MuscleMeshUPtr = std::unique_ptr<MuscleMesh>;
    using MuscleMeshWPtr = std::weak_ptr<MuscleMesh>;


    using IGLMeshWireframeSPtr = std::shared_ptr<IGLMeshWireframe>;
    using IGLMeshWireframeUPtr = std::unique_ptr<IGLMeshWireframe>;
    using IGLMeshWireframeWPtr = std::weak_ptr<IGLMeshWireframe>;

    using ES2PolygonOffsetSPtr = std::shared_ptr<ES2PolygonOffset>;
    using ES2PolygonOffsetUPtr = std::unique_ptr<ES2PolygonOffset>;
    using ES2PolygonOffsetWPtr = std::weak_ptr<ES2PolygonOffset>;

    using ES2VideoTextureSPtr = std::shared_ptr<ES2VideoTexture>;
    using ES2VideoTextureUPtr = std::unique_ptr<ES2VideoTexture>;
    using ES2VideoTextureWPtr = std::weak_ptr<ES2VideoTexture>;

    using ES2BufferTextureSPtr = std::shared_ptr<ES2BufferTexture>;
    using ES2BufferTextureUPtr = std::unique_ptr<ES2BufferTexture>;
    using ES2BufferTextureWPtr = std::weak_ptr<ES2BufferTexture>;

    using ES2CameraTextureSPtr = std::shared_ptr<ES2CameraTexture>;
    using ES2CameraTextureUPtr = std::unique_ptr<ES2CameraTexture>;
    using ES2CameraTextureWPtr = std::weak_ptr<ES2CameraTexture>;

    using ShaderBoxSPtr = std::shared_ptr<ShaderBox>;
    using ShaderBoxUPtr = std::unique_ptr<ShaderBox>;
    using ShaderBoxWPtr = std::weak_ptr<ShaderBox>;

    using OpenFaceTextureSPtr = std::shared_ptr<OpenFaceTexture>;
    using OpenFaceTextureUPtr = std::unique_ptr<OpenFaceTexture>;
    using OpenFaceTextureWPtr = std::weak_ptr<OpenFaceTexture>;

    using IGLMeshTwoSPtr = std::shared_ptr<IGLMeshTwo>;
    using IGLMeshTwoUPtr = std::unique_ptr<IGLMeshTwo>;
    using IGLMeshTwoWPtr = std::weak_ptr<IGLMeshTwo>;
}

namespace Wm5
{
    class Transform;

    template <typename T> class Vector2;
    template <typename T> class Vector3;
    template <typename T> class Vector4;

    typedef Vector2<int> Vector2i;
    typedef Vector2<float> Vector2f;
    typedef Vector3<int> Vector3i;
    typedef Vector3<float> Vector3f;
    typedef Vector4<int> Vector4i;
    typedef Vector4<float> Vector4f;
    typedef Vector4<unsigned char> Vector4uc;
}


namespace cereal
{
    class BinaryOutputArchive;
    class BinaryInputArchive;
}

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

