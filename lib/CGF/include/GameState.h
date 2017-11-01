/*
 *  GameState.h
 *  Abstract class to represent game states
 *
 *  Created by Marcelo Cohen on 04/11.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#ifndef CGF_GAMESTATE_H
#define CGF_GAMESTATE_H

#include "Game.h"

namespace cgf
{

class GameState
{
    public:

    virtual void init() = 0;
    virtual void cleanup() = 0;

    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual void handleEvents(Game* game) = 0;
    virtual void update(Game* game) = 0;
    virtual void draw(Game* game) = 0;

    void changeState(Game* game, GameState* state) {
        game->changeState(state);
    }

    protected:

    GameState() { }
};

} // namespace cgf

#endif // CGF_GAMESTATE_H
