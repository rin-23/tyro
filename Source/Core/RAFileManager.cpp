//
//  RAiOSBundleDocs.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-17.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAFileManager.h"
#include "RALogManager.h"

namespace tyro
{
    std::string GetFilePath(const std::string& fileName, const std::string& extension)
    {	
		//@TODO: Remove full path
		//std::string shaderFolder = "C:/Workspace/TyroWin/TyroWin/Source/Shaders/";
		std::string shaderFolder = "/home/rinat/Workspace/Tyro/TyroWin/Source/Shaders/";
		std::string fullPath = shaderFolder + fileName + std::string(".") + extension;
		RA_LOG_INFO(fullPath.data());
		return fullPath;
	}
}
