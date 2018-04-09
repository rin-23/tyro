//
//  RAES2ShaderUniforms.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-14.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2ShaderUniforms.h"
#include "RALogManager.h"
#include <cstring>

namespace tyro
{
    ES2ShaderUniforms::ES2ShaderUniforms(int numOfUniforms)
    {
        mNumOfUniforms = numOfUniforms;
        mUniforms = new Uniform[numOfUniforms];
    }
    
    ES2ShaderUniforms::~ES2ShaderUniforms()
    {
        for (int i = 0; i < mNumOfUniforms; i++)
        {
            Uniform un = mUniforms[i];
            
            if (UniformTypeIsFloat(un.type))
            {
                delete [] un.floatValue;
            }
            else
            {
                delete [] un.intValue;
            }
        }
        
        delete [] mUniforms;
    }
    
    int ES2ShaderUniforms::GetNumOfUniforms() const
    {
        return mNumOfUniforms;
    }
    
    void ES2ShaderUniforms::SetUniform(int index, GLint shaderLocation, GLsizei count, const std::string& name, UniformType type)
    {
        assert(shaderLocation != -1);
        
        if (index >= mNumOfUniforms)
        {
            RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumOfUniforms);
            assert(false);
            return;
        }

        Uniform uniform;
        uniform.index = index;
        uniform.shaderLocation = shaderLocation;
        uniform.count = count;
        uniform.name = name;
        uniform.type = type;
        int numOfElements = NumOfElementsForUniformType(uniform.type);
        if (UniformTypeIsFloat(uniform.type))
        {
            uniform.floatValue = new GLfloat[numOfElements*count];
        }
        else
        {
            uniform.intValue = new GLint[numOfElements*count];
        }
        
        mUniforms[index] = uniform;
    }
    
    const ES2ShaderUniforms::Uniform* ES2ShaderUniforms::GetUniform(int index) const
    {
        if (index >= mNumOfUniforms)
        {
            RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumOfUniforms);
            assert(false);
        }
        return &mUniforms[index];
    }
//
//    ES2ShaderUniforms::UniformType ES2ShaderUniforms::getType(int index) const
//    {
//        if (index >= mNumOfUniforms)
//        {
//            RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumOfUniforms);
//            assert(false);
//            return UniformNone;
//        }
//        return mUniforms[index].type;
//    }
//    
//    const GLfloat* ES2ShaderUniforms::getFloatValue(int index) const
//    {
//        if (index >= mNumOfUniforms)
//        {
//            RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumOfUniforms);
//            assert(false);
//            return NULL;
//        }
//
//        Uniform un = mUniforms[index];
//        return un.floatValue;
//    }

    
    void ES2ShaderUniforms::UpdateFloatUniform(int index, const GLfloat* value)
    {
        if (index >= mNumOfUniforms)
        {
            RA_LOG_ERROR_ASSERT("Attribute index %i is out of  max bound %i", index, mNumOfUniforms);
            return;
        }
        
        Uniform* un = &mUniforms[index];
        int numOfElements = NumOfElementsForUniformType(un->type);
        memcpy(un->floatValue, value, un->count * numOfElements * sizeof(GLfloat));
    }
    
    void ES2ShaderUniforms::UpdateFloatUniform(int index, int first, int count, const GLfloat* value)
    {
        if (index >= mNumOfUniforms)
        {
            RA_LOG_ERROR_ASSERT("Attribute index %i is out of  max bound %i", index, mNumOfUniforms);
            return;
        }
        
        Uniform* un = &mUniforms[index];
        int numOfElements = NumOfElementsForUniformType(un->type);
        int offset = first * numOfElements * sizeof(GLfloat);
        memcpy(un->floatValue + offset, value, count*numOfElements*sizeof(GLfloat));
    }
    
    void ES2ShaderUniforms::UpdateIntUniform(int index, const GLint* value)
    {
        if (index >= mNumOfUniforms)
        {
            RA_LOG_ERROR_ASSERT("Attribute index %i is out of  max bound %i", index, mNumOfUniforms);
            return;
        }
        
        Uniform* un = &mUniforms[index];
        int numOfElements = NumOfElementsForUniformType(un->type);
        memcpy(un->intValue, value, un->count * numOfElements * sizeof(GLint));
    }
    
    void ES2ShaderUniforms::UpdateIntUniform(int index, int first, int count, const GLint* value)
    {
        if (index >= mNumOfUniforms)
        {
            RA_LOG_ERROR_ASSERT("Attribute index %i is out of  max bound %i", index, mNumOfUniforms);
            return;
        }
        
        Uniform* un = &mUniforms[index];
        int numOfElements = NumOfElementsForUniformType(un->type);
        int offset = first * numOfElements * sizeof(GLint);
        memcpy(un->intValue + offset, value, count*numOfElements*sizeof(GLint));
    }
    
    int ES2ShaderUniforms::NumOfElementsForUniformType(UniformType type) const
    {
        int numOfElements = 0;
        switch (type)
        {
            case UniformMatrix4fv:
                numOfElements = 16;
                break;
            case UniformMatrix3fv:
                numOfElements = 9;
                break;
            case UniformMatrix2fv:
                numOfElements = 4;
                break;
            case Uniform4fv:
                numOfElements = 4;
                break;
            case Uniform3fv:
                numOfElements = 3;
                break;
            case Uniform2fv:
                numOfElements = 2;
                break;
            case Uniform1fv:
                numOfElements = 1;
                break;
            case Uniform1iv:
                numOfElements = 1;
            default:
                break;
        }
        return numOfElements;
    }
    
    bool ES2ShaderUniforms::UniformTypeIsFloat(UniformType type) const
    {
        switch (type)
        {
            case UniformMatrix4fv:
                return true;
            case UniformMatrix3fv:
                return true;
            case UniformMatrix2fv:
                return true;
            case Uniform4fv:
                return true;
            case Uniform3fv:
                return true;
            case Uniform2fv:
                return true;
            case Uniform1fv:
                return true;
            default:
                break;
        }
        return false;
    }

}