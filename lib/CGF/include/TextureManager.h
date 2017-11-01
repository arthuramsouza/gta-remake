/*
 *  TextureManager.h
 *  manages textures to avoid loading the same image twice
 *
 *  Created by Marcelo Cohen on 07/13.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#ifndef CGF_TEXTUREMANAGER_H
#define CGF_TEXTUREMANAGER_H

#include <map>
#include <string>
#include <SFML/Graphics.hpp>
#include <memory>

namespace cgf
{

class TextureManager
{
    using TexPtr = std::unique_ptr<sf::Texture>;
    private:

        static TextureManager m_TextureManager;
        std::map<std::string, TexPtr> imgs;
        std::string defaultImageDir;            // base dir to load images from

    public:

        sf::Texture* findTexture(const char *nomeArq);
        void setDefaultImageDir(const char *dir);
        void releaseTexture(const char *nomeArq);
        static TextureManager * getInstance ()
        {
            return & m_TextureManager;
        }

    protected:

        TextureManager ();
};

} // namespace cgf

#endif // CGF_TEXTUREMANAGER_H
