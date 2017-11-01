// Arthur, Emanoel, Gabriell

/*
 *  PlayState.h
 *  Normal "play" state
 *
 *  Created by Marcelo Cohen on 08/13.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#ifndef PLAY_STATE_H_
#define PLAY_STATE_H_

#include "GameState.h"
#include "Sprite.h"
#include "InputManager.h"
#include <tmx/MapLoader.h>
#include <string>

class PlayState : public cgf::GameState
{
    public:

    void init();
    void cleanup();

    void pause();
    void resume();

    void handleEvents(cgf::Game* game);
    void update(cgf::Game* game);
    void draw(cgf::Game* game);

    // Implement Singleton Pattern
    static PlayState* instance()
    {
        return &m_PlayState;
    }

    protected:

    PlayState() {}

    private:

    static PlayState m_PlayState;

    enum { RIGHT=0, LEFT, UP, DOWN };
    std::string walkStates[4];
    int currentDir;

    int dirx, diry, money;

    cgf::Sprite player;
    cgf::Sprite player2;
    cgf::Sprite enemy;

    sf::RenderWindow* screen;
    cgf::InputManager* im;

    tmx::MapLoader* map;

    sf::Font font;
    sf::Text text;

    // Centers the camera on the player position
    void centerMapOnPlayer();

    // Checks collision between a sprite and a map layer
    bool checkCollision(uint8_t layer, cgf::Game* game, cgf::Sprite* obj);

    // get a cell GID from the map (x and y in world coords)
    sf::Uint16 getCellFromMap(uint8_t layernum, float x, float y);

    // Audio
    sf::SoundBuffer sheepSoundBuffer;
    sf::Sound sheepSound;
    sf::SoundBuffer hornSoundBuffer;
    sf::Sound hornSound;
    sf::Music music;
    sf::Music musicRadio;
};

#endif
