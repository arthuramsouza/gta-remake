// Arthur, Emanoel, Gabriell

/*
 *  PlayState.cpp
 *  Normal "play" state
 *
 *  Created by Marcelo Cohen on 08/13.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#include <iostream>
#include <cmath>
#include "Game.h"
#include "PlayState_Solution.h"
#include "InputManager.h"

PlayState PlayState::m_PlayState;

using namespace std;
double accel;
int mode, radio; // MODE (1- humano; 2- veiculo) & RADIO (1- OFF; 2- Flash; 3- Emotion; 4- Park)

void PlayState::init()
{
    mode = 2;
    radio = 1;

    if (!font.loadFromFile("data/fonts/dollar.ttf")) {
        cout << "Cannot load arial.ttf font!" << endl;
        exit(1);
    }
    text.setFont(font);
    text.setString(L"Testing text output in SFML");
    text.setCharacterSize(24); // in pixels
    text.setFillColor(sf::Color::Green);
    //text.setPosition(screen->getView().getViewport().left + 5, screen->getView().getViewport().top + 5);
    //text.setStyle(sf::Text::Bold);
    map = new tmx::MapLoader("data/maps");       // all maps/tiles will be read from data/maps
    // map->AddSearchPath("data/maps/tilesets"); // e.g.: adding more search paths for tilesets
    map->Load("dungeon-tilesets2.tmx");

    walkStates[0] = "walk-right";
    walkStates[1] = "walk-left";
    walkStates[2] = "walk-up";
    walkStates[3] = "walk-down";
    currentDir = RIGHT;
    player.load("data/img/car-beast.png");
    player2.load("data/img/warrior.png",64,64,0,0,0,0,13,21,273);
    player.setPosition(200,100);
    player.setOrigin(player.getGlobalBounds().width/2, player.getGlobalBounds().height/2);
    player.setScale(0.5,0.5);
    player2.setPosition(200,100);
    player2.loadAnimation("data/img/warrioranim.xml");
    player2.setAnimation(walkStates[currentDir]);
    player2.setAnimRate(15);
    player2.setScale(1,1);
    player2.play();
    player2.setVisible(false);

    enemy.load("data/img/money.png");
    enemy.setPosition(310,310);
    //enemy.setXspeed(50);
    enemy.setScale(0.2,0.2);
//    edirx = 1; // right

    dirx = 0; // sprite dir: right (1), left (-1)
    diry = 0; // down (1), up (-1)

    im = cgf::InputManager::instance();

    im->addKeyInput("left", sf::Keyboard::Left);
    im->addKeyInput("right", sf::Keyboard::Right);
    im->addKeyInput("up", sf::Keyboard::Up);
    im->addKeyInput("down", sf::Keyboard::Down);
    im->addKeyInput("quit", sf::Keyboard::Escape);
    im->addKeyInput("stats", sf::Keyboard::S);
    im->addMouseInput("rightclick", sf::Mouse::Right);
    im->addKeyInput("horn", sf::Keyboard::H);
    im->addKeyInput("radio1", sf::Keyboard::Num1); // Flash
    im->addKeyInput("radio2", sf::Keyboard::Num2); // Emotion
    im->addKeyInput("radio3", sf::Keyboard::Num3); // Park
    // Camera control
    im->addKeyInput("zoomin", sf::Keyboard::Z);
    im->addKeyInput("zoomout", sf::Keyboard::X);

    // Sheep sound
    sheepSoundBuffer.loadFromFile("data/audio/sports-car.wav");
    sheepSound.setBuffer(sheepSoundBuffer);
    sheepSound.setVolume(50);
    // Horn sound
    hornSoundBuffer.loadFromFile("data/audio/horn.wav");
    hornSound.setBuffer(hornSoundBuffer);
    hornSound.setVolume(20);
    // City music
    music.openFromFile("data/audio/city.ogg");
    music.setVolume(10000);  // 30% do volume máximo
    music.setLoop(true);  // modo de loop: repete continuamente.
    music.play();
    // Radio music
    musicRadio.setVolume(50);
    musicRadio.setLoop(true);
    musicRadio.stop();

    cout << "PlayState: Init" << endl;
}

void PlayState::cleanup()
{
    delete map;
    cout << "PlayState: Clean" << endl;
}

void PlayState::pause()
{
    cout << "PlayState: Paused" << endl;
}

void PlayState::resume()
{
    cout << "PlayState: Resumed" << endl;
}

void PlayState::handleEvents(cgf::Game* game)
{
    screen = game->getScreen();
    sf::View view = screen->getView(); // gets the view
    sf::Event event;

    while (screen->pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
            game->quit();
    }

    dirx = diry = 0;
    int newDir = currentDir;
/*
     if(newDir == UP && !im->testEvent("down")) {
        if(accel > 0) {
            diry = accel;
            accel = accel-0.0008;
        }
    }

    if(newDir == DOWN && !im->testEvent("up")) {
        if(accel < 0) {
            diry = accel;
            accel = accel+0.0008;
        }
    }
*/
    if(im->testEvent("left")) {

        if(mode == 1) {
            dirx = -1;
            newDir = LEFT;
        }

        if(mode == 2)
            player.setRotation(player.getRotation()-0.25); // -0.25 no LAPRO, -0.05 no LABCG
    }

    if(im->testEvent("right")) {

        if(mode == 1) {
            dirx = 1;
            newDir = RIGHT;
        }

        if(mode == 2)
            player.setRotation(player.getRotation()+0.25); // +0.25 no LAPRO, +0.05 no LABCG
    }

    if(im->testEvent("down")) {
    /*
        diry = accel;
        if(accel < 8000)
            accel = accel + 0.0015;
        cout << accel;
        newDir = UP;
        */

         if(mode == 1) {
            diry = 1;
            newDir = DOWN;
         }

         if(mode == 2) {
            diry = cos(player.getRotation()*3.14159265/180)*4;
            dirx = sin(player.getRotation()*3.14159265/180)*-4;
         }
    }

    if(im->testEvent("up")) {
    /*
        if(accel > 0)
            accel = accel - 0.001;
        else {
            accel = accel - 0.001;
            diry = diry + accel;
        }
        newDir = UP;
        */

        if(mode == 1) {
            diry = -1;
            newDir = UP;
        }

        if(mode == 2) {
            diry = cos(player.getRotation()*3.14159265/180)*-3;
            dirx = sin(player.getRotation()*3.14159265/180)*3;
        }

    }

    if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == sf::Keyboard::Return)
        {

            if(mode == 1) {
                player2.setVisible(false);
                mode = 2;
            }

            else if(mode == 2) {
                if(player.getRotation() < 45)
                    player2.setPosition(player.getPosition().x - 75, player.getPosition().y - 50);
                else if(player.getRotation() < 120)
                    player2.setPosition(player.getPosition().x - 25, player.getPosition().y - 90);
                else if(player.getRotation() < 180)
                    player2.setPosition(player.getPosition().x + 15, player.getPosition().y - 65);
                else if(player.getRotation() < 270)
                    player2.setPosition(player.getPosition().x + 15, player.getPosition().y - 15);
                else
                    player2.setPosition(player.getPosition().x - 20, player.getPosition().y + 20);
                player2.setVisible(true);
                mode = 1;
            }
        }

    }

    if(im->testEvent("quit") || im->testEvent("rightclick"))
        game->quit();

    if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == sf::Keyboard::Num1)
        {
            musicRadio.stop();
        }
    }

    if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == sf::Keyboard::Num2)
        {
            musicRadio.openFromFile("data/audio/music.wav");
            musicRadio.play();
        }
    }

    if(im->testEvent("stats"))
        game->toggleStats();

    if(im->testEvent("zoomin")) {
        view.zoom(1.01);
        screen->setView(view);
    }
    else if(im->testEvent("zoomout")) {
        view.zoom(0.99);
        screen->setView(view);
    }

    if(dirx == 0 && diry == 0) {
        player.pause();
    }
    else {
        if(currentDir != newDir) {
            player2.setAnimation(walkStates[newDir]);
            currentDir = newDir;
        }
        player.play();
    }

    if(mode == 2) {
        player.setXspeed(100*dirx);
        player.setYspeed(100*diry);
    }
    else if(mode == 1) {
        player2.setXspeed(100*dirx);
        player2.setYspeed(100*diry);
    }
}

void PlayState::update(cgf::Game* game)
{
    screen = game->getScreen();
    checkCollision(2, game, &player);
    checkCollision(2, game, &player2);
    if(checkCollision(2, game, &enemy))
        enemy.setXspeed(-enemy.getXspeed());
    //enemy.update(game->getUpdateInterval());
    //player2.update(game->getUpdateInterval());
    if(player2.bboxCollision(enemy)) {
        money = money + 250;
        enemy.setVisible(false);
        enemy.setPosition(-10,-10);
    }
/*
    if(player.isVisible()) {
        hornSound.setPosition(player.getPosition().x, player.getPosition().y, 0);
        if(hornSound.getStatus() == sf::Sound::Stopped && rand()%1000 > 950)
            hornSound.play();
    }
*/

    if(im->testEvent("horn")) {
        //sheepSound.setPosition(player.getPosition().x, player.getPosition().y, 0);
        if(mode == 2 && hornSound.getStatus() == sf::Sound::Stopped)
            hornSound.play();
    }
    else {
        if(mode == 2 && hornSound.getStatus() == sf::Sound::Playing)
            hornSound.stop();
    }

    if(im->testEvent("up") || im->testEvent("down")) {
        //sheepSound.setPosition(player.getPosition().x, player.getPosition().y, 0);
        if(mode == 2 && sheepSound.getStatus() == sf::Sound::Stopped)
            sheepSound.play();
    }

    if(im->testEvent("up") || im->testEvent("down")) {
        //sheepSound.setPosition(player.getPosition().x, player.getPosition().y, 0);
        if(mode == 2 && sheepSound.getStatus() == sf::Sound::Stopped)
            sheepSound.play();
    }
    else {
        if(mode == 2 && sheepSound.getStatus() == sf::Sound::Playing)
            sheepSound.stop();
    }

    centerMapOnPlayer();
}

void PlayState::draw(cgf::Game* game)
{
    screen = game->getScreen();
    map->Draw(*screen);          // draw all layers
    screen->draw(player);
    screen->draw(player2);
    map->Draw(*screen, 1);     // draw only the second layer
    screen->draw(enemy);
    screen->draw(text);
    std::stringstream ss;
    ss << "$ " << money;
    text.setString(ss.str());
}

void PlayState::centerMapOnPlayer()
{
    sf::View view = screen->getView();
    sf::Vector2u mapsize = map->GetMapSize();
    sf::Vector2f viewsize = view.getSize();
    sf::Vector2f pos;
    viewsize.x /= 2;
    viewsize.y /= 2;
    if(mode == 2)
        pos = player.getPosition();
    else
        pos = player2.getPosition();
    float panX = viewsize.x; // minimum pan
    if(pos.x >= viewsize.x)
        panX = pos.x;

    if(panX >= mapsize.x - viewsize.x)
        panX = mapsize.x - viewsize.x;

    float panY = viewsize.y; // minimum pan
    if(pos.y >= viewsize.y)
        panY = pos.y;

    if(panY >= mapsize.y - viewsize.y)
        panY = mapsize.y - viewsize.y;

    text.setPosition(panX-300,panY-250);
    sf::Vector2f center(panX,panY);
    view.setCenter(center);
    screen->setView(view);
}

bool PlayState::checkCollision(uint8_t layer, cgf::Game* game, cgf::Sprite* obj)
{
    int i, x1, x2, y1, y2;
    bool bump = false;

    // Get the limits of the map
    sf::Vector2u mapsize = map->GetMapSize();
    // Get the width and height of a single tile
    sf::Vector2u tilesize = map->GetMapTileSize();

    mapsize.x /= tilesize.x;
    mapsize.y /= tilesize.y;
    mapsize.x--;
    mapsize.y--;

    // Get the height and width of the object (in this case, 100% of a tile)
    sf::Vector2u objsize = obj->getSize();
    objsize.x *= obj->getScale().x;
    objsize.y *= obj->getScale().y;

    float px = obj->getPosition().x;
    float py = obj->getPosition().y;

    double deltaTime = game->getUpdateInterval();

    sf::Vector2f offset(obj->getXspeed()/1000 * deltaTime, obj->getYspeed()/1000 * deltaTime);

    float vx = offset.x;
    float vy = offset.y;

    //cout << "px,py: " << px << " " << py << endl;

    //cout << "tilesize " << tilesize.x << " " << tilesize.y << endl;
    //cout << "mapsize " << mapsize.x << " " << mapsize.y << endl;

    // Test the horizontal movement first
    i = objsize.y > tilesize.y ? tilesize.y : objsize.y;

    for (;;)
    {
        x1 = (px + vx) / tilesize.x;
        x2 = (px + vx + objsize.x - 1) / tilesize.x;

        y1 = (py) / tilesize.y;
        y2 = (py + i - 1) / tilesize.y;

        if (x1 >= 0 && x2 < mapsize.x && y1 >= 0 && y2 < mapsize.y)
        {
            if (vx > 0)
            {
                // Trying to move right

                int upRight   = getCellFromMap(layer, x2*tilesize.x, y1*tilesize.y);
                int downRight = getCellFromMap(layer, x2*tilesize.x, y2*tilesize.y);
                if (upRight || downRight)
                {
                    // Place the player as close to the solid tile as possible
                    px = x2 * tilesize.x;
                    px -= objsize.x;// + 1;
                    vx = 0;
                    bump = true;
                }
            }

            else if (vx < 0)
            {
                // Trying to move left

                int upLeft   = getCellFromMap(layer, x1*tilesize.x, y1*tilesize.y);
                int downLeft = getCellFromMap(layer, x1*tilesize.x, y2*tilesize.y);
                if (upLeft || downLeft)
                {
                    // Place the player as close to the solid tile as possible
                    px = (x1+1) * tilesize.x;
                    vx = 0;
                    bump = true;
                }
            }
        }

        if (i == objsize.y) // Checked player height with all tiles ?
        {
            break;
        }

        i += tilesize.y; // done, check next tile upwards

        if (i > objsize.y)
        {
            i = objsize.y;
        }
    }

    // Now test the vertical movement

    i = objsize.x > tilesize.x ? tilesize.x : objsize.x;

    for (;;)
    {
        x1 = (px / tilesize.x);
        x2 = ((px + i-1) / tilesize.x);

        y1 = ((py + vy) / tilesize.y);
        y2 = ((py + vy + objsize.y-1) / tilesize.y);

        if (x1 >= 0 && x2 < mapsize.x && y1 >= 0 && y2 < mapsize.y)
        {
            if (vy > 0)
            {
                // Trying to move down
                int downLeft  = getCellFromMap(layer, x1*tilesize.x, y2*tilesize.y);
                int downRight = getCellFromMap(layer, x2*tilesize.x, y2*tilesize.y);
                if (downLeft || downRight)
                {
                    // Place the player as close to the solid tile as possible
                    py = y2 * tilesize.y;
                    py -= objsize.y;
                    vy = 0;
                    bump = true;
                }
            }

            else if (vy < 0)
            {
                // Trying to move up

                int upLeft  = getCellFromMap(layer, x1*tilesize.x, y1*tilesize.y);
                int upRight = getCellFromMap(layer, x2*tilesize.x, y1*tilesize.y);
                if (upLeft || upRight)
                {
                    // Place the player as close to the solid tile as possible
                    py = (y1 + 1) * tilesize.y;
                    vy = 0;
                    bump = true;
                }
            }
        }

        if (i == objsize.x)
        {
            break;
        }

        i += tilesize.x; // done, check next tile to the right

        if (i > objsize.x)
        {
            i = objsize.x;
        }
    }

    // Now apply the movement and animation

    obj->setPosition(px+vx,py+vy);
    px = obj->getPosition().x;
    py = obj->getPosition().y;

    obj->update(deltaTime, false); // only update animation

    // Check collision with edges of map
    if (px < 0)
        obj->setPosition(px,py);
    else if (px + objsize.x >= mapsize.x * tilesize.x)
        obj->setPosition(mapsize.x*tilesize.x - objsize.x - 1,py);

    if(py < 0)
        obj->setPosition(px,0);
    else if(py + objsize.y >= mapsize.y * tilesize.y)
        obj->setPosition(px, mapsize.y*tilesize.y - objsize.y - 1);

    return bump;
}

// Get a cell GID from the map (x and y in global coords)
sf::Uint16 PlayState::getCellFromMap(uint8_t layernum, float x, float y)
{
    auto layers = map->GetLayers();
    tmx::MapLayer& layer = layers[layernum];
    sf::Vector2u mapsize = map->GetMapSize();
    sf::Vector2u tilesize = map->GetMapTileSize();
    mapsize.x /= tilesize.x;
    mapsize.y /= tilesize.y;
    int col = floor(x / tilesize.x);
    int row = floor(y / tilesize.y);
    return layer.tiles[row*mapsize.x + col].gid;
}
