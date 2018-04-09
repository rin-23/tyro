//
//  RAMarchingCubes.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-09-06.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAMarchingCubesTable.h"
#include "Wm5Vector3.h"

namespace tyro
{
    class MarchingCubes
    {
    public:
        MarchingCubes();
        
        struct TRIANGLE
        {
            Wm5::Vector3f p[3];
        };
        
        struct GRIDCELL
        {
            Wm5::Vector3f p[8];
            float val[8];
        };
        
        static int Polygonise(GRIDCELL& grid, float isolevel, Wm5::Vector3f* vertlist, int* indexList, int& edgeInfo)
        {
            /*
             Determine the index into the edge table which
             tells us which vertices are inside of the surface
             */
            int cubeindex = 0;
            if (grid.val[0] < isolevel) cubeindex |= 1;
            if (grid.val[1] < isolevel) cubeindex |= 2;
            if (grid.val[2] < isolevel) cubeindex |= 4;
            if (grid.val[3] < isolevel) cubeindex |= 8;
            if (grid.val[4] < isolevel) cubeindex |= 16;
            if (grid.val[5] < isolevel) cubeindex |= 32;
            if (grid.val[6] < isolevel) cubeindex |= 64;
            if (grid.val[7] < isolevel) cubeindex |= 128;
            
            /* Cube is entirely in/out of the surface */
            if (edgeTable[cubeindex] == 0)
                return 0;
            
            /* Find the vertices where the surface intersects the cube */
            if (edgeTable[cubeindex] & 1)
                VertexInterp(vertlist[0], isolevel,grid.p[0],grid.p[1],grid.val[0],grid.val[1]);
            if (edgeTable[cubeindex] & 2)
                VertexInterp(vertlist[1], isolevel,grid.p[1],grid.p[2],grid.val[1],grid.val[2]);
            if (edgeTable[cubeindex] & 4)
                VertexInterp(vertlist[2], isolevel,grid.p[2],grid.p[3],grid.val[2],grid.val[3]);
            if (edgeTable[cubeindex] & 8)
                VertexInterp(vertlist[3], isolevel,grid.p[3],grid.p[0],grid.val[3],grid.val[0]);
            if (edgeTable[cubeindex] & 16)
                VertexInterp(vertlist[4], isolevel,grid.p[4],grid.p[5],grid.val[4],grid.val[5]);
            if (edgeTable[cubeindex] & 32)
                VertexInterp(vertlist[5], isolevel,grid.p[5],grid.p[6],grid.val[5],grid.val[6]);
            if (edgeTable[cubeindex] & 64)
                VertexInterp(vertlist[6], isolevel,grid.p[6],grid.p[7],grid.val[6],grid.val[7]);
            if (edgeTable[cubeindex] & 128)
                VertexInterp(vertlist[7], isolevel,grid.p[7],grid.p[4],grid.val[7],grid.val[4]);
            if (edgeTable[cubeindex] & 256)
                VertexInterp(vertlist[8], isolevel,grid.p[0],grid.p[4],grid.val[0],grid.val[4]);
            if (edgeTable[cubeindex] & 512)
                VertexInterp(vertlist[9],isolevel,grid.p[1],grid.p[5],grid.val[1],grid.val[5]);
            if (edgeTable[cubeindex] & 1024)
                VertexInterp(vertlist[10],isolevel,grid.p[2],grid.p[6],grid.val[2],grid.val[6]);
            if (edgeTable[cubeindex] & 2048)
                VertexInterp(vertlist[11],isolevel,grid.p[3],grid.p[7],grid.val[3],grid.val[7]);

            edgeInfo = edgeTable[cubeindex];
            memcpy(indexList, triTable[cubeindex], 16*sizeof(int));
            
            return 1;
        }
        
//        /*
//         Given a grid cell and an isolevel, calculate the triangular
//         facets required to represent the isosurface through the cell.
//         Return the number of triangular facets, the array "triangles"
//         will be loaded up with the vertices at most 5 triangular facets.
//         0 will be returned if the grid cell is either totally above
//         of totally below the isolevel.
//         */
//        static int Polygonise(GRIDCELL grid, float isolevel, TRIANGLE *triangles)
//        {
//            int i,ntriang;
//            int cubeindex;
//            Wm5::Vector3f vertlist[12];
//            
//            /*
//             Determine the index into the edge table which
//             tells us which vertices are inside of the surface
//             */
//            cubeindex = 0;
//            if (grid.val[0] < isolevel) cubeindex |= 1;
//            if (grid.val[1] < isolevel) cubeindex |= 2;
//            if (grid.val[2] < isolevel) cubeindex |= 4;
//            if (grid.val[3] < isolevel) cubeindex |= 8;
//            if (grid.val[4] < isolevel) cubeindex |= 16;
//            if (grid.val[5] < isolevel) cubeindex |= 32;
//            if (grid.val[6] < isolevel) cubeindex |= 64;
//            if (grid.val[7] < isolevel) cubeindex |= 128;
//            
//            /* Cube is entirely in/out of the surface */
//            if (edgeTable[cubeindex] == 0)
//                return(0);
//            
//            /* Find the vertices where the surface intersects the cube */
//            if (edgeTable[cubeindex] & 1)
//                vertlist[0] =
//                VertexInterp(isolevel,grid.p[0],grid.p[1],grid.val[0],grid.val[1]);
//            if (edgeTable[cubeindex] & 2)
//                vertlist[1] =
//                VertexInterp(isolevel,grid.p[1],grid.p[2],grid.val[1],grid.val[2]);
//            if (edgeTable[cubeindex] & 4)
//                vertlist[2] =
//                VertexInterp(isolevel,grid.p[2],grid.p[3],grid.val[2],grid.val[3]);
//            if (edgeTable[cubeindex] & 8)
//                vertlist[3] =
//                VertexInterp(isolevel,grid.p[3],grid.p[0],grid.val[3],grid.val[0]);
//            if (edgeTable[cubeindex] & 16)
//                vertlist[4] =
//                VertexInterp(isolevel,grid.p[4],grid.p[5],grid.val[4],grid.val[5]);
//            if (edgeTable[cubeindex] & 32)
//                vertlist[5] =
//                VertexInterp(isolevel,grid.p[5],grid.p[6],grid.val[5],grid.val[6]);
//            if (edgeTable[cubeindex] & 64)
//                vertlist[6] =
//                VertexInterp(isolevel,grid.p[6],grid.p[7],grid.val[6],grid.val[7]);
//            if (edgeTable[cubeindex] & 128)
//                vertlist[7] =
//                VertexInterp(isolevel,grid.p[7],grid.p[4],grid.val[7],grid.val[4]);
//            if (edgeTable[cubeindex] & 256)
//                vertlist[8] =
//                VertexInterp(isolevel,grid.p[0],grid.p[4],grid.val[0],grid.val[4]);
//            if (edgeTable[cubeindex] & 512)
//                vertlist[9] =
//                VertexInterp(isolevel,grid.p[1],grid.p[5],grid.val[1],grid.val[5]);
//            if (edgeTable[cubeindex] & 1024)
//                vertlist[10] =
//                VertexInterp(isolevel,grid.p[2],grid.p[6],grid.val[2],grid.val[6]);
//            if (edgeTable[cubeindex] & 2048)
//                vertlist[11] =
//                VertexInterp(isolevel,grid.p[3],grid.p[7],grid.val[3],grid.val[7]);
//            
//            /* Create the triangle */
//            ntriang = 0;
//            for (i = 0; triTable[cubeindex][i] != -1; i += 3)
//            {
//                triangles[ntriang].p[0] = vertlist[triTable[cubeindex][i  ]];
//                triangles[ntriang].p[1] = vertlist[triTable[cubeindex][i+1]];
//                triangles[ntriang].p[2] = vertlist[triTable[cubeindex][i+2]];
//
//                ntriang++;
//            }
//            
//            return(ntriang);
//        }
        
//        /*
//         Linearly interpolate the position where an isosurface cuts
//         an edge between two vertices, each with their own scalar value
//         */
//        static Wm5::Vector3f VertexInterp(float isolevel, const Wm5::Vector3f& p1, const Wm5::Vector3f& p2, float valp1, float valp2)
//        {
//            Wm5::Vector3f p;
//            
//            if (std::abs(isolevel-valp1) < 0.00001)
//                return(p1);
//            if (std::abs(isolevel-valp2) < 0.00001)
//                return(p2);
//            if (std::abs(valp1-valp2) < 0.00001)
//                return(p1);
//            float mu = (isolevel - valp1) / (valp2 - valp1);
//            p.X() = p1.X() + mu * (p2.X() - p1.X());
//            p.Y() = p1.Y() + mu * (p2.Y() - p1.Y());
//            p.Z() = p1.Z() + mu * (p2.Z() - p1.Z());
//            
//            return p;
//        }
        
        /*
         Linearly interpolate the position where an isosurface cuts
         an edge between two vertices, each with their own scalar value
         */
        inline static void VertexInterp(Wm5::Vector3f& p, float isolevel, const Wm5::Vector3f& p1, const Wm5::Vector3f& p2, float valp1, float valp2)
        {
            if (std::abs(isolevel-valp1) < 0.00001)
                p = p1;
            if (std::abs(isolevel-valp2) < 0.00001)
                p = p2;
            if (std::abs(valp1-valp2) < 0.00001)
                p = p1;
            
            float mu = (isolevel - valp1) / (valp2 - valp1);
            p.X() = p1.X() + mu * (p2.X() - p1.X());
            p.Y() = p1.Y() + mu * (p2.Y() - p1.Y());
            p.Z() = p1.Z() + mu * (p2.Z() - p1.Z());
        }
        
//        mpVector LinearInterp(mp4Vector p1, mp4Vector p2, float value)
//        {
//            if (p2 < p1)
//            {
//                mp4Vector temp;
//                temp = p1;
//                p1 = p2;
//                p2 = temp;
//            }
//            
//            mpVector p;
//            if(fabs(p1.val - p2.val) > 0.00001)
//                p = (mpVector)p1 + ((mpVector)p2 - (mpVector)p1)/(p2.val - p1.val)*(value - p1.val);
//            else
//                p = (mpVector)p1;
//            return p;
//        }
//        
//        bool operator<(const mp4Vector &right) const
//        {
//            if (x < right.x)
//                return true;
//            else if (x > right.x)
//                return false;
//            
//            if (y < right.y)
//                return true;
//            else if (y > right.y)
//                return false;
//            
//            if (z < right.z)
//                return true;
//            else if (z > right.z)
//                return false;
//            
//            return false;
//        }


    };
        
}




