/*
 *  TextureManager.cpp
 *  framework
 *
 *  Created by Marcelo Cohen on 07/13.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#include "TextureManager.h"
#include "Debug.h"

namespace cgf
{

using namespace std;

TextureManager TextureManager::m_TextureManager;

TextureManager::TextureManager()
{
    defaultImageDir = "data";
}

sf::Texture* TextureManager::findTexture(const char *nomeArq)
{
    auto find = imgs.find(nomeArq);
    if(find == imgs.end()) {
        DEBUG_MSG_NN("New texture: " << nomeArq);
        TexPtr tex = TexPtr{new sf::Texture};
        if(!tex->loadFromFile(nomeArq)) {
            return nullptr;
        }
        DEBUG_MSG(" (" << tex->getSize().x << " x " << tex->getSize().y << ")");
        
        tex->setSmooth(true);
        sf::Texture* texptr = tex.get();
        imgs.insert(std::make_pair(nomeArq, std::move(tex)));
        
        return texptr;
    }

    DEBUG_MSG("Existing texture: " << nomeArq << " (" << find->second->getSize().x << " x " << find->second->getSize().y << ")");

    // Return pointer to texture
    return find->second.get();
}

void TextureManager::setDefaultImageDir(const char *dir)
{
    defaultImageDir = dir;
}

void TextureManager::releaseTexture(const char *nomeArq)
{
    imgs.erase(nomeArq);
}

} // namespace cgf
