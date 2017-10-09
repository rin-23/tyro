//
//  RAES2ShaderProgram.h
//  PAM2
//
//  Created by Rinat Abdrashitov on 2014-06-11.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <map>
#include <string>
//#include <OpenGLES/ES2/gl.h>
#include <GL/glew.h>

namespace RAEngine
{
    class ES2ShaderProgram
    {
    public:        
        
        ~ES2ShaderProgram();
        
        /**
         Creates shader program. Compiles vertex/fragment shaders and attaches them
         to the shader program.
         @param vertexShaderPath Absolute path to a vertex shader source
         @param fragmentShaderPath Absolute path to a fragment shader source
         @return 0 is returned upon failure
         */
        int LoadProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
        
        void UseProgram() const;
        
        /**
         Get compiled shader program
         @return shader program
         */
        GLuint GetProgram() const;
        
        /**
         Get the location of an attribute variable
         @param name name of the attribute variable whose location is to be queried.
         @return location of attribute variable
         */
        GLint GetAttributeLocation(const GLchar* name) const;
        
        /**
         Get the location of an uniform variable
         @param name name of the uniform variable whose location is to be queried.
         @return location of uniform variable
         */
        GLint GetUniformLocation(const GLchar* name) const;
        
    private:
        GLuint mProgram; // shader program
        std::string mShaderNameKey; // vertex+fragment shader name
        
        // Store compiled shader programs. vertex+fragment shader name is key, program number is value.
        static std::map<std::string, GLuint> mShaderNameToProgramMap;
        // Count how many times program is requested. Delete the program only when count goes to zero.
        static std::map<std::string, int> mShaderNameToCountMap;
        
        static GLuint CreateProgram(const std::string& vertexShaderPath,
                                    const std::string& fragmentShaderPath);
        
        static int CompileShader(GLuint* shader, GLenum type, const std::string& filename);
        static int LinkProgram(GLuint prog);
        static int ValidateProgram(GLuint prog);
    };
}
