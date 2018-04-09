//
//  RASTLLoader.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-12.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <string>
#include <fstream>

namespace tyro
{
    class STLLoader
    {
    public:
        STLLoader(const std::string& filePath);

        ~STLLoader();
        
        //call this to allocate buffer storage
        int GetNumberOfTriangles();

        /*
         gpuBufferPtr must not be null.
         will be populated with struct Vertex or VertexN if needNormals is false
         
         typedef struct 
         {
            float position[3];
            float normal[3];
         } Vertex;
         
         typedef struct 
         {
            float position[3];
         } Vertex;
         
         Every face has 3 Vertex
        */
        
        int LoadSTLFile(void* gpuBufferPtr, bool needNormals = true);
    private:
        
        struct Vertex
        {
            float pos[3];
            float normal[3];
        };
        
        int LoadASCIISTL(FILE* pFile, void* gpuBufferPtr, bool needNormals);
        int LoadBinarySTL(FILE* pFile, void* gpuBufferPtr, bool needNormals);
        size_t getline(char **lineptr, size_t *n, FILE *stream);

        std::string mFilePath;
    };
}
