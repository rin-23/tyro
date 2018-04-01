//
//  RATexture.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-30.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RATexture.h"

namespace tyro
{

Texture::Texture(TextureFormat format, TextureType type)
:
mFormat(format),
mType(type)
{
}

Texture::~Texture()
{}

}