/*
 *  Game.h
 *  Main game class
 *
 *  Created by Marcelo Cohen on 05/13.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#ifndef CGF_GAME_H
#define CGF_GAME_H

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <stack>
#include <memory>
#include "ClockHUD.h"

namespace cgf
{

class GameState;

class Game
{
    public:

        Game(int minFrameRate=15, int maxFrameRate=20);
        void init(const char* title, int width, int height, bool fullscreen);
        void handleEvents();
        void changeState(GameState* state);
        void pushState(GameState* state);
        void popState();
        void update();
        void draw();
        void clean();
        bool isRunning() { return running; }
        void quit()    { running = false; }
        sf::RenderWindow* getScreen() { return screen.get(); }
        static void printAttributes();
        float getUpdateInterval() { return updateInterval; }

        void toggleStats() { showStats = !showStats; }
        void enableStats() { showStats = true; }
        void disabeStats() { showStats = false; }

    private:

        // idealmente screen deveria ser inicializado no construtor da classe
        std::unique_ptr<sf::RenderWindow> screen{nullptr};
        sf::View originalView;
        bool running;
        bool fullscreen;
        bool showStats;

        std::stack<GameState*> states;
        // Fixed interval time-based animation
        sf::Clock gameClock;
        int minFrameRate;
        int maxFrameRate;
        float updateInterval;
        float maxCyclesPerFrame;
        float lastFrameTime;
        float cyclesLeftOver;

        sf::Font font;
        sfx::FrameClock clock;
        std::unique_ptr<ClockHUD> hud;
        // Audio engine
//        irrklang::ISoundEngine* audioEngine;
};

} // namespace cgf

#endif // CGF_GAME
