//
//  RASTLLoader.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-12.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RASTLLoader.h"
#include <fstream>
#include "RALogManager.h"
#include "RAMath.h"

namespace RAEngine
{
    STLLoader::STLLoader(const std::string& filePath)
    :
    mFilePath(filePath)
    {

    }

    STLLoader::~STLLoader()
    {
    }
    
	size_t getline(char **lineptr, size_t *n, FILE *stream) {
		char *bufptr = NULL;
		char *p = bufptr;
		size_t size;
		int c;

		if (lineptr == NULL) {
			return -1;
		}
		if (stream == NULL) {
			return -1;
		}
		if (n == NULL) {
			return -1;
		}
		bufptr = *lineptr;
		size = *n;

		c = fgetc(stream);
		if (c == EOF) {
			return -1;
		}
		if (bufptr == NULL) {
			bufptr = (char*) malloc(128);
			if (bufptr == NULL) {
				return -1;
			}
			size = 128;
		}
		p = bufptr;
		while (c != EOF) {
			if ((p - bufptr) > (size - 1)) {
				size = size + 128;
				bufptr = (char*)realloc(bufptr, size);
				if (bufptr == NULL) {
					return -1;
				}
			}
			*p++ = c;
			if (c == '\n') {
				break;
			}
			c = fgetc(stream);
		}

		*p++ = '\0';
		*lineptr = bufptr;
		*n = size;

		return p - bufptr - 1;
	}

    int STLLoader::GetNumberOfTriangles()
    {
        FILE* pFile = fopen(mFilePath.data(), "r");
        if (!pFile)
        {
            RA_LOG_ERROR("Failed to open %s", mFilePath.data());
            return 0;
        }
        
        char first5bytes[6];
        first5bytes[5] = '\0';
        if (!fread(first5bytes, sizeof(char), 5, pFile))
        {
            RA_LOG_ERROR("Failed to read %s", mFilePath.data());
            return 0;
        }
        rewind(pFile);
        
        int nTriangles = 0;
        
        if (strcmp(first5bytes, "solid") == 0)
        {
            //ASCII STL
            char* line = nullptr;
            size_t len = 0;
            size_t read;

            while ((read = getline(&line, &len, pFile)) != -1)
            {
                float x, y,z;
                if (sscanf(line, "%*[ \n\t]facet normal %f %f %f", &x, &y, &z))
                {
                    nTriangles++;
                }
            }
            
            if (line)
            {
                free(line);
            }
        }
        else
        {
            //BINARY STL
            uint8_t header[80];
            if (!fread(header, sizeof(uint8_t), 80, pFile) ||
                !fread(&nTriangles, sizeof(uint32_t), 1, pFile))
            {
                RA_LOG_ERROR_ASSERT("Failed to read number of triangles");
                if (fclose(pFile) != 0)
                {
                    RA_LOG_ERROR_ASSERT("Failed to close the file %s", mFilePath.data());
                }
                return 0;
            }
        }
        
        if (fclose(pFile) != 0)
        {
            RA_LOG_ERROR("Failed to close the file %s", mFilePath.data());
        }

        return nTriangles;
    }
    
    int STLLoader::LoadSTLFile(void* gpuBufferPtr, bool needNormals)
    {
        if (gpuBufferPtr == nullptr)
        {
            RA_LOG_ERROR("Null buffer pointer");
            return 0;
        }

        FILE* pFile = fopen(mFilePath.data(), "r");
        if (!pFile)
        {
            RA_LOG_ERROR("Failed to open %s", mFilePath.data());
            return 0;
        }

        char first5bytes[6];
        first5bytes[5] = '\0';
        if (!fread(first5bytes, sizeof(char), 5, pFile))
        {
            RA_LOG_ERROR("Failed to read %s", mFilePath.data());
            return 0;
        }
        
        rewind(pFile);
        int result;
        if (strcmp(first5bytes, "solid") == 0)
        {
            RA_LOG_VERBOSE("Reading ASCII STL file %s", mFilePath.data());
            result = LoadASCIISTL(pFile, gpuBufferPtr, needNormals);
        } 
        else
        {
            RA_LOG_VERBOSE("Reading Binary STL file %s", mFilePath.data());
            result = LoadBinarySTL(pFile, gpuBufferPtr, needNormals);
        }

        if (fclose(pFile) != 0)
        {
            RA_LOG_ERROR("Failed to close the file %s", mFilePath.data());
            return 0;
        }
        return result;
    }
    
    int STLLoader::LoadASCIISTL(FILE* pFile, void* gpuBufferPtr, bool needNormals)
    {
        if (!pFile)
        {
            RA_LOG_ERROR("Bad file pointer");
            return 0;
        }
        
        if (gpuBufferPtr == nullptr)
        {
            RA_LOG_ERROR("Null buffer pointer");
            return 0;
        }
        
        char* line = nullptr;
        size_t len = 0;
        size_t read;

        float normal[3];
//        bool error = false;
        int nTrianglesCounter = 0;
        
        while ((read = getline(&line, &len, pFile)) != -1)
        {
//            if (error)
//            {
//                RA_LOG_ERROR_ASSERT("Error when reading file");
//                break;
//            }
            
            char* pch = strtok(line, " \t");
            if (strcmp(pch, "facet") == 0)
            {
                //read normal vector
                pch = strtok(NULL, " \t"); //skip normal string
                assert(strcmp(pch, "normal") == 0);
                for (int i = 0; i < 3; ++i)
                {
                    pch = strtok(NULL, " \t");
                    assert(sscanf(pch,"%f", &normal[i]) == 1);
                }
                
                //read outer loop line
                getline(&line, &len, pFile);

                //read vertex lines
                for (int i = 0; i < 3; i++)
                {
                    getline(&line, &len, pFile);
                    Vertex v;
                    memcpy(v.normal, normal, 3*sizeof(float));
                    
                    pch = strtok(line, " \t");
                    assert(strcmp(pch, "vertex") == 0);
                    for (int j = 0; j < 3; ++j)
                    {
                        pch = strtok(NULL, " \t");
                        assert(sscanf(pch,"%f", &v.pos[j]) == 1);
                    }
                    
                    if (needNormals)
                    {
                        memcpy(((char*)gpuBufferPtr) + sizeof(Vertex) * (3*nTrianglesCounter + i), &v, sizeof(Vertex));
                    }
                    else
                    {
                        memcpy(((char*)gpuBufferPtr) + 3*sizeof(float) * (3*nTrianglesCounter + i), &v.pos, 3*sizeof(float));
                    }
                    
                }
                
                nTrianglesCounter++;
            }
        }
        
        if (line)
        {
            free(line);
        }
        
        return 1;
//        if (error)
//        {
//            return 0;
//        }
//        else
//        {
//            return 1;
//        }
    }
    
    /*
     UINT8[80] – Header
     UINT32 – Number of triangles
     
     foreach triangle
     REAL32[3] – Normal vector
     REAL32[3] – Vertex 1
     REAL32[3] – Vertex 2
     REAL32[3] – Vertex 3
     UINT16 – Attribute byte count
     end
     */
    
    int STLLoader::LoadBinarySTL(FILE* pFile, void* gpuBufferPtr, bool needNormals)
    {
        
        if (!pFile)
        {
            RA_LOG_ERROR("Bad file pointer");
            return 0;
        }
        
        if (gpuBufferPtr == nullptr)
        {
            RA_LOG_ERROR("Null buffer pointer");
            return 0;
        }
        
        //read 80 byte header
        uint8_t header[80];
        if (!fread(header, sizeof(uint8_t), 80, pFile))
        {
            RA_LOG_ERROR("Failed to read");
            return 0;
        }
        
        //read number of triangles
        uint32_t nTriangles;
        if (!fread(&nTriangles, sizeof(uint32_t), 1, pFile))
        {
            RA_LOG_ERROR("Failed to read number of triangles");
            return 0;
        }
        RA_LOG_VERBOSE("Number of Triangles %i", nTriangles);
        
        int nTrianglesCounter = 0;

        while (nTrianglesCounter != nTriangles)
        {
//            RA_LOG_INFO("Reading Vertex %i", nTrianglesCounter);
            
            float face[12];
            if (!fread(face, sizeof(float), 12, pFile))
            {
                RA_LOG_ERROR("Failed to read normal and verticies");
                return 0;
            }
            //face[0], face[1], face[2] are normal coordinates

            if (needNormals)
            {
                Vertex verticies[3] = {
                    {{face[3],  face[4],  face[5]}, {face[0],face[1],face[2]}},
                    {{face[6],  face[7],  face[8]}, {face[0],face[1],face[2]}},
                    {{face[9], face[10], face[11]}, {face[0],face[1],face[2]}}
                };
                int stride = 3 * sizeof(Vertex);
                memcpy(((char*)gpuBufferPtr) + stride * nTrianglesCounter, verticies, stride);
            }
            else
            {
                int stride = 9 * sizeof(float);
                memcpy(((char*)gpuBufferPtr) + stride * nTrianglesCounter, &face[3], stride);
            }
            
            uint16_t attribute;
            if(!fread(&attribute, sizeof(uint16_t), 1, pFile))
            {
                RA_LOG_ERROR("Failed to read attribute");
                return 0;
            }

            nTrianglesCounter++;
        }
        
        assert(nTriangles == nTrianglesCounter);
        
        return 1;
    }
}