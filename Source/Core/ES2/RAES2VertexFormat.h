//
//  RAES2VertexFormat.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-14.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <GL/glew.h>
//#include <OpenGLES/gltypes.h>
#include <string>

namespace tyro
{
    class ES2VertexFormat
    {
    public:
        
        enum
        {
            // The maximum number of attributes for a vertex format.
            AM_MAX_ATTRIBUTES = 16,
            
            // The maximum number of texture coordinate units.
            AM_MAX_TCOORD_UNITS = 8,
            
            // The maximum number of color units.
            AM_MAX_COLOR_UNITS = 2
        };
        
        enum AttributeType
        {
            AT_FLOAT1,
            AT_FLOAT2,
            AT_FLOAT3,
            AT_FLOAT4,
            AT_HALF1,
            AT_HALF2,
            AT_HALF3,
            AT_HALF4,
            AT_UBYTE4,
            AT_SHORT1,
            AT_SHORT2,
            AT_SHORT4,
            AT_QUANTITY
        };
        
        enum AttributeUsage
        {
            AU_CUSTOM,
            AU_POSITION,
            AU_PACKED_NORMAL,
            AU_NORMAL,
            AU_TANGENT,
            AU_BINORMAL,
            AU_TEXCOORD,
            AU_COLOR,
            AU_JOINT_INDEX_1,
            AU_JOINT_INDEX_2,
            AU_BLENDWEIGHT_1,
            AU_BLENDWEIGHT_2,
            AU_FOGCOORD,
            AU_PSIZE,
            AU_QUANTITY
        };
        
        ES2VertexFormat(int numAttributes);

        ~ES2VertexFormat();

        void SetAttribute(int index,
                          int shaderLocation,
                          GLsizeiptr offset,
                          AttributeType type,
                          AttributeUsage usage);

        int GetNumOfAttributes() const;
        
        int GetIndex(AttributeUsage usage) const;
        
        int GetShaderLocation(int index) const;
        
        GLsizeiptr GetOffset(int index) const;

        GLint GetSize(int index) const;

        GLsizeiptr GetOffsetForNextAttrib(int index) const;
        
        void GetGLAttribute(int index,
                            int& shaderLocation,
                            GLint& size,
                            GLsizeiptr& offset,
                            GLenum& dataType,
                            GLboolean& normalized,
                            GLboolean& enabled) const;
        
        // The number of bytes required by a component of the attribute.
        inline static int GetComponentSize (AttributeType type);
        
        // The number of components of the attribute.
        inline static int GetNumComponents (AttributeType type);
        
        // The number of bytes for all the components of the attribute.
        inline static int GetTypeSize (AttributeType type);
        
        inline static GLenum GetGLType (AttributeType type);

    private:
        class Attribute
        {
        public:
            int shaderLocation;
            std::string nameInShader;
            GLint size;
            GLsizeiptr offset;
            GLenum dataType;
            GLboolean normalized;
            GLboolean enabled;
            AttributeUsage usage;
            AttributeType atType;
        };
        
        int mNumAttributes;
        Attribute* mAttributes;
        
        static int msComponentSize[AT_QUANTITY];
        static int msNumComponents[AT_QUANTITY];
        static int msTypeSize[AT_QUANTITY];
        static GLenum msGLType[AT_QUANTITY];

    };
    
    inline int ES2VertexFormat::GetComponentSize (AttributeType type)
    {
        return msComponentSize[type];
    }

    inline int ES2VertexFormat::GetNumComponents (AttributeType type)
    {
        return msNumComponents[type];
    }
    
    inline int ES2VertexFormat::GetTypeSize (AttributeType type)
    {
        return msTypeSize[type];
    }
    
    inline GLenum ES2VertexFormat::GetGLType (AttributeType type)
    {
        return msGLType[type];
    }
}
