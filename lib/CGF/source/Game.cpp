/*
 *  Game.cpp
 *  Main game class
 *
 *  Created by Marcelo Cohen on 08/13.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#include "Game.h"
#include "GameState.h"
#include <iostream>
#include <cmath>
#include "Debug.h"

namespace cgf
{

using namespace std;

// Construtor
Game::Game(int minFrameRate, int maxFrameRate)
{
    this->minFrameRate = minFrameRate;
    this->maxFrameRate = maxFrameRate;
    updateInterval = 1.0f / maxFrameRate*1000;
    DEBUG_MSG("Update interval: " << updateInterval);
    maxCyclesPerFrame = maxFrameRate / minFrameRate;
    lastFrameTime = 0;
    cyclesLeftOver = 0;

    // Load the font; exit on error.
    if (!font.loadFromFile("data/fonts/arial.ttf"))
    {
        sf::err() << "Failed to load arial.ttf";
        exit(EXIT_FAILURE);
    }

    showStats = false;
    hud = std::unique_ptr<ClockHUD>{new ClockHUD{clock, font}};
    clock.setSampleDepth(100); // Sample 100 frames for averaging.
}

void Game::init(const char* title, int width, int height, bool fullscreen)
{
    screen = std::unique_ptr<sf::RenderWindow>{new sf::RenderWindow(sf::VideoMode(width, height), title)};
    // Enable transparency through blending
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    originalView = screen->getView();

	this->fullscreen = fullscreen;

	running = true;
    // Essa função não fazia nada, pois tinha um return logo no início dela.
    // achei que seria melhor deixá-la ativada caso a pessoa passe tenha habilitado os logs de debug
    // (se essa função sempre estiver habilitada e DISABLE_LOGGING estiver definido ela gera um warning de variável definida, porém não usada.
    //  Esse warning ocorre devido ao macro DEBUG_MSG que foi colocado no corpo da função)
#ifndef DISABLE_LOGGING
    printAttributes();
#endif

//	glewInit();

	DEBUG_MSG("Game Initialised Succesfully");
}


void Game::printAttributes ()
{
    std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
    for (std::size_t i = 0; i < modes.size(); ++i)
    {
        sf::VideoMode mode = modes[i];
        std::cout << "Mode #" << i << ": "
                  << mode.width << "x" << mode.height << " - "
                  << mode.bitsPerPixel << " bpp" << std::endl;
    }
}

void Game::handleEvents()
{
    states.top()->handleEvents(this);
}

void Game::changeState(GameState* state)
{
    // cleanup the current state
    if ( !states.empty() ) {
        states.top()->cleanup();
        states.pop();
    }

    // store and init the new state
    states.push(state);
    states.top()->init();
}

void Game::pushState(GameState* state)
{
	// pause current state
	if ( !states.empty() ) {
		states.top()->pause();
	}

	// store and init the new state
	states.push(state);
	states.top()->init();
}

void Game::popState()
{
	// cleanup the current state
	if ( !states.empty() ) {
		states.top()->cleanup();
		states.pop();
	}

	// resume previous state
	if ( !states.empty() ) {
		states.top()->resume();
	}
}

void Game::update()
{
    float currentTime, updateIterations;

    currentTime = gameClock.getElapsedTime().asMilliseconds();
    updateIterations = ((currentTime - lastFrameTime) + cyclesLeftOver);

    if (updateIterations > maxCyclesPerFrame * updateInterval) {
        updateIterations = maxCyclesPerFrame * updateInterval;
    }

    // int cont = 1;
    while(updateIterations > updateInterval) {
        updateIterations -= updateInterval;
        //cout << "updating " << cont++ << endl;
        if ( !states.empty() )
            states.top()->update(this);
    }
    cyclesLeftOver = updateIterations;
    lastFrameTime = currentTime;
}

void Game::draw()
{
    // let the state draw the screen
    screen->clear();
    clock.beginFrame();
    states.top()->draw(this);

    if(showStats)
    {
        // Draw the frame statistics.
        sf::View view = screen->getView();
        sf::Vector2f pos = originalView.getCenter() - originalView.getSize()/2.f;
        hud->setPosition(pos); //cout << "screen " << screen << endl;
        screen->setView(originalView);
        screen->draw(*hud);
        screen->setView(view);
    }
    screen->display();
    clock.endFrame();
}

void Game::clean()
{
    while ( !states.empty() ) {
		states.top()->cleanup();
		states.pop();
    }
}

} // namespace cgf
