//
//  RASTLExporter.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-11.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2TriMesh.h"
//#include "OssaES2PlateManager.h"
//#include "OssaScrewNode.h"
//#include "OssaPrerequisites.h"

namespace Ossa
{
    class STLExporter
    {
    public:
//        static int ExportToBinarySTL(const RAEngine::ES2TriMesh& mesh, const std::string& path);
//        static int ExportToBinarySTL(const RAEngine::ES2TriMesh& mesh, const std::string& path, Wm5::Transform transform);
        
        static int ExportToBinarySTL(RAEngine::ES2TriMesh* mesh, const std::string& path);
        
        //static int ExportToBinarySTL(RAEngine::ES2TriMesh* node, const std::string& path);

//        int ExportToASCIISTL(const RAEngine::ES2TriMesh& plate, const std::string& path);
    };
}
