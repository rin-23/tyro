//
//  RAES2ShaderProgram.cpp
//  PAM2
//
//  Created by Rinat Abdrashitov on 2014-06-11.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2ShaderProgram.h"
#include <fstream>
#include <iostream>
//#include <OpenGLES/ES2/glext.h>
#include <GL/glew.h>
#include "RALogManager.h"

namespace RAEngine
{
    std::map<std::string, GLuint> ES2ShaderProgram::mShaderNameToProgramMap;
    std::map<std::string, int> ES2ShaderProgram::mShaderNameToCountMap;

#pragma mark - CONSTRUCTOR/DESTRUCTOR
    
    ES2ShaderProgram::~ES2ShaderProgram()
    {
        auto it = mShaderNameToCountMap.find(mShaderNameKey);
        if (it != mShaderNameToCountMap.end()) {
            int newCount = it->second - 1;
            if (newCount == 0) {
                glDeleteProgram(mProgram);
                GL_CHECK_ERROR;
                mShaderNameToProgramMap.erase(mShaderNameKey);
                mShaderNameToCountMap.erase(mShaderNameKey);
            } else {
                mShaderNameToCountMap[mShaderNameKey] = newCount;
            }
        }
    }

#pragma mark - PUBLIC METHODS
    
    int ES2ShaderProgram::LoadProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
    {
        mShaderNameKey = vertexShaderPath + fragmentShaderPath;
        auto it = mShaderNameToProgramMap.find(mShaderNameKey);
        
        //check if program was previously compiled and return it, create new otherwise
        if (it != mShaderNameToProgramMap.end()) {
            mProgram = it->second;
            mShaderNameToCountMap[mShaderNameKey] += 1;
        } else {
            mProgram = CreateProgram(vertexShaderPath, fragmentShaderPath);
            if (mProgram == 0)
            {
                RA_LOG_ERROR_ASSERT("Couldn't compile shader %s %s", vertexShaderPath.data(), fragmentShaderPath.data());
                return 0;
            }
            
            mShaderNameToCountMap[mShaderNameKey] = 1;
            mShaderNameToProgramMap[mShaderNameKey] = mProgram;
        }

        return 1;
    }
    
    void ES2ShaderProgram::UseProgram() const
    {
        glUseProgram(mProgram);
        GL_CHECK_ERROR;
    }
    
    GLuint ES2ShaderProgram::GetProgram() const
    {
        return mProgram;
    }
    
    GLint ES2ShaderProgram::GetAttributeLocation(const GLchar* name) const
    {
        GLint loc = glGetAttribLocation(mProgram, name);
        GL_CHECK_ERROR;
        assert(loc != -1);
        return loc;
    }
    
    GLint ES2ShaderProgram::GetUniformLocation(const GLchar* name) const
    {
        GLint loc = glGetUniformLocation(mProgram, name);
        GL_CHECK_ERROR;
        assert(loc != -1);
        return loc;
    }

#pragma mark - PRIVATE METHODS
    
    GLuint ES2ShaderProgram::CreateProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
    {
        GLuint vertShader, fragShader;
        
        // Create shader program.
        GLuint program = glCreateProgram();
        if (program == 0) {
            return 0;
        }
        
        // Create and compile vertex/fragment shaders
        if (!CompileShader(&vertShader, GL_VERTEX_SHADER, vertexShaderPath)) {
            RA_LOG_ERROR("Failed to compile vertex shader");
            return 0;
        }
        
        if (!CompileShader(&fragShader, GL_FRAGMENT_SHADER, fragmentShaderPath)) {
            RA_LOG_ERROR("Failed to compile fragmet shader");
            return 0;
        }
        
        // Attach vertex/fragment shaders to program
        glAttachShader(program, vertShader);
        GL_CHECK_ERROR;
        glAttachShader(program, fragShader);
        GL_CHECK_ERROR;
        
        // Link program
        if (!LinkProgram(program)) {
            RA_LOG_ERROR("Failed to link program %i", program);
            if (vertShader) {
                glDeleteShader(vertShader);
            }
            if (fragShader) {
                glDeleteShader(fragShader);
            }
            if (program) {
                glDeleteProgram(program);
            }
            return 0;
        }
        GL_CHECK_ERROR;
        
        // Release vertex and fragment shaders.
        if (vertShader) {
            glDetachShader(program, vertShader);
            GL_CHECK_ERROR;
            glDeleteShader(vertShader);
            GL_CHECK_ERROR;
        }
        if (fragShader) {
            glDetachShader(program, fragShader);
            GL_CHECK_ERROR;
            glDeleteShader(fragShader);
            GL_CHECK_ERROR;
        }
        
        return program;
    }

    int ES2ShaderProgram::CompileShader(GLuint* shader, GLenum type, const std::string& filename)
    {
        std::ifstream ifs(filename.c_str());
        if (!ifs.is_open()) {
            RA_LOG_ERROR("Couldnt open shader file %s", filename.c_str());
            return 0;
        }
        std::string content((std::istreambuf_iterator<GLchar>(ifs)),
                            (std::istreambuf_iterator<GLchar>()));

        const GLchar* source = content.c_str();
        if (source == nullptr)
        {
            RA_LOG_ERROR("Couldnt load shader text %s", filename.c_str());
            return 0;
        }
        
        *shader = glCreateShader(type);
        GL_CHECK_ERROR;
        glShaderSource(*shader, 1, &source, NULL);
        GL_CHECK_ERROR;
        glCompileShader(*shader);
        GL_CHECK_ERROR;
        
#ifdef _DEBUG
#if _DEBUG
        GLint logLength;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
        GL_CHECK_ERROR;
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(*shader, logLength, &logLength, log);
            RA_LOG_ERROR("Shader compile log: %s", log);
            free(log);
        }
#endif
#endif

        GLint status;
        glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
        GL_CHECK_ERROR;
        if (status == 0) {
            glDeleteShader(*shader);
            RA_LOG_ERROR("Couldnt compile shader. Deleting it...");
            return 0;
        }
        
        return 1;
    }


    int ES2ShaderProgram::LinkProgram(GLuint prog)
    {
        GLint status;
        glLinkProgram(prog);
        GL_CHECK_ERROR;

#ifdef _DEBUG
#if _DEBUG
        GLint logLength;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
        GL_CHECK_ERROR;
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetProgramInfoLog(prog, logLength, &logLength, log);
            RA_LOG_WARN("Program link log: %s", log);
            free(log);
        }
#endif
#endif
        glGetProgramiv(prog, GL_LINK_STATUS, &status);
        if (status == 0) {
            RA_LOG_ERROR("Couldnt link");
            return 0;
        }
        
        return 1;
    }

    int ES2ShaderProgram::ValidateProgram(GLuint prog) 
    {
        GLint logLength, status;
        
        glValidateProgram(prog);
        GL_CHECK_ERROR;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetProgramInfoLog(prog, logLength, &logLength, log);
            RA_LOG_ERROR("Program validate log: %s", log);
            free(log);
        }
        
        glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
        if (status == 0) {
            RA_LOG_ERROR("Couldn't validate");
            return 0;
        }
        
        return 1;
    }
}