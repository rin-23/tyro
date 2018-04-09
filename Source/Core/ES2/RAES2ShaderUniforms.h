//
//  RAES2ShaderUniforms.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-14.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <string>
//#include <OpenGLES/gltypes.h>
#include <GL/glew.h>

namespace tyro
{
    class ES2ShaderUniforms
    {
    public:
        
        ES2ShaderUniforms(int numOfUniforms);
        ~ES2ShaderUniforms();
        
        using UniformType = enum
        {
           /* Uniform1i,
            Uniform2i,
            Uniform3i,
            Uniform4i, */
            Uniform1iv,
/*            Uniform2iv,
            Uniform3iv,
            Uniform4iv,
            Uniform1f,
            Uniform2f,
            Uniform3f,
            Uniform4f,*/
            Uniform1fv,
            Uniform2fv,
            Uniform3fv,
            Uniform4fv,
            UniformMatrix2fv,
            UniformMatrix3fv,
            UniformMatrix4fv,
            UniformNone
        };
        
        class Uniform
        {
        public:
            int index;
            GLint shaderLocation;
            GLsizei count;
            std::string name;
            UniformType type;
            GLfloat* floatValue;
            GLint* intValue;
        };

        int GetNumOfUniforms() const;
        void SetUniform(int index, GLint shaderLocation, GLsizei count, const std::string& name, UniformType type);
        const ES2ShaderUniforms::Uniform* GetUniform(int index) const;

        //update all data
        void UpdateFloatUniform(int index, const GLfloat* value);
        
        //update only part of the uniform array
        //update count number of elements starting at first
        void UpdateFloatUniform(int index, int first, int count, const GLfloat* value);

        void UpdateIntUniform(int index, const GLint* value);
        
        void UpdateIntUniform(int index, int first, int count, const GLint* value);
        
    private:

        int NumOfElementsForUniformType(UniformType type) const;
        bool UniformTypeIsFloat(UniformType type) const;
        
        int mNumOfUniforms;
        Uniform* mUniforms;
    };
}
