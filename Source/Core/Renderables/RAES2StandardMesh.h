//
//  RAES2StandardMesh.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-09.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2DefaultTriMesh.h"
#include "RAES2DefaultPolyline.h"
#include "RAEnginePrerequisites.h"

namespace tyro
{
    class ES2Cylinder : public ES2DefaultTriMesh
    {
    public:
        //no axis samples 
        //screw points down the Y axis;
        //center of the top ring is at (0,0,0);
        //rings lie in XZ plane
        ES2Cylinder() {}

        virtual ~ES2Cylinder() {}

        static ES2CylinderSPtr Create(int axisSamples, int radialSamples, float topRadius, float btmRadius, float height, bool open);
        
        float GetHeight() const;
        void SetHeight(float height);
        
    public: //Serialization
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
            archive(mAxisSamples, mRadialSamples, mTopRadius, mBtmRadius, mHeight, mOpen);
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
            archive(mAxisSamples, mRadialSamples, mTopRadius, mBtmRadius, mHeight, mOpen);
        }
    protected:

        void Init(int axisSamples, int radialSamples, float topRadius, float btmRadius, float height, bool open);

    private:
        int mAxisSamples;
        int mRadialSamples;
        float mTopRadius;
        float mBtmRadius;
        float mHeight;
        bool mOpen;
    };
    
    class ES2Sphere : public ES2DefaultTriMesh
    {
    public:
        ES2Sphere() {}

        virtual ~ES2Sphere() {}
        
        static ES2SphereSPtr Create(ES2SphereSPtr sphere);
        
        static ES2SphereSPtr Create(int zSamples, int radialSamples, float radius);
        
        float GetRadius() const;

    public: //Serialization
    
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
            archive(mRadius);
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
            archive(mRadius);
        }
    protected:
        void Init(int zSamples, int radialSamples, float radius);
        void Init(ES2SphereSPtr sphere);
    private:
        
        float mRadius;        
    };
    
    class ES2Box : public ES2DefaultTriMesh
    {
    public:
        // The box has center (0,0,0); unit-length axes (1,0,0), (0,1,0), and
        // (0,0,1); and extents (half-lengths) 'xExtent', 'yExtent', and
        // 'zExtent'.  The mesh has 8 vertices and 12 triangles.  For example,
        // the box corner in the first octant is (xExtent, yExtent, zExtent).
        ES2Box() {}
        virtual ~ES2Box() {}
        
        static ES2BoxSPtr Create(float xyzExtent);
        static ES2BoxSPtr Create(float xExtent, float yExtent, float zExtent);

    public: //Serialization
    
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
        }

    protected:
    
        void Init(float xyzExtent);
        void Init(float xExtent, float yExtent, float zExtent);

    };
    
    class ES2Line : public ES2DefaultPolyline
    {
    public:
        ES2Line() {}
        virtual ~ES2Line() {}
        
        static ES2LineSPtr Create(bool isContegious = false);
        static ES2LineSPtr Create(const Wm5::Vector3f* points, int numOfPoints, bool isContegious = false);

    public: //Serialization
        
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<ES2DefaultPolyline>(this));
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<ES2DefaultPolyline>(this));
        }
        
    protected:
        void Init(bool isContegious);
        void Init(const Wm5::Vector3f* points, int numOfPoints, bool isContegious);
        void SetPoints(const Wm5::Vector3f* points, int numOfPoints);
    };
    
    class ES2Rectangle : public ES2DefaultTriMesh
    {
    public:
        ES2Rectangle() {}
        virtual ~ES2Rectangle() {}
        static ES2RectangleSPtr Create(int xSamples, int ySamples, float xExtent, float yExtent);

    public: //Serialization
        
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
        }
    protected:
        void Init(int xSamples, int ySamples, float xExtent, float yExtent);
    };
    
    class ES2Vector : public ES2Line
    {
    public:
        //length is in mm
        ES2Vector() {}
        virtual ~ES2Vector() {}
        static ES2VectorSPtr Create(const Wm5::Vector3f& point, const Wm5::Vector3f& vector, float length = 5.0f);

    public: //Serialization
        
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<ES2Line>(this));
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<ES2Line>(this));
        }
        
    protected:
        void Init(const Wm5::Vector3f& point, const Wm5::Vector3f& vector, float length = 5.0f);
    };
}


CEREAL_REGISTER_TYPE(tyro::ES2Cylinder);
CEREAL_REGISTER_TYPE(tyro::ES2Sphere);
CEREAL_REGISTER_TYPE(tyro::ES2Box);
CEREAL_REGISTER_TYPE(tyro::ES2Line);
CEREAL_REGISTER_TYPE(tyro::ES2Rectangle);
CEREAL_REGISTER_TYPE(tyro::ES2Vector);

