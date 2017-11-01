#ifndef CGF_SPRITE_H
#define CGF_SPRITE_H

/*
 *  Sprite.h
 *  Animated sprite class
 *
 *  Created by Marcelo Cohen on 08/13.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 *  Base on original code at https://github.com/LaurentGomila/SFML/wiki/Source%3A-AnimatedSprite
 *
 */

#include <vector>
#include <map>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>
#include "TextureManager.h"
#include "Anim.h"
#include "pugixml/pugixml.hpp"

namespace cgf
{

class Sprite : public sf::Drawable, public sf::Transformable
{
public:
    Sprite();
    virtual ~Sprite();

    bool load(const char *filename);
    bool load(const char *filename, int w, int h, int hSpace, int vSpace, int xIni, int yIni,
              int column, int row, int total);
    bool load(const char *filename, int w, int h, int hSpace, int vSpace, int xIni, int yIni,
              int column, int row);
    bool loadXML(const char *filename);
    bool loadAnimation(const char *filename);

    void setVisible(bool vis) { visible = vis; }
    bool isVisible() { return visible; }

    // Get sprite bounds in local coords
    sf::FloatRect getLocalBounds() {
        float width = static_cast<float>(spriteW);
        float height = static_cast<float>(spriteH);

        return sf::FloatRect(0.f, 0.f, width, height);
    }

    // Get sprite bounds in global coords
    sf::FloatRect getGlobalBounds() {
        return getTransform().transformRect(getLocalBounds());
    }

    // Mirroring (X-axis)
    void setMirror(bool mirror) { this->mirror = mirror; setCurrentFrame(curframe); }
    bool getMirror() { return mirror; }

    // Sprite speed
    void setXspeed(float xspeed);
    void setYspeed(float yspeed);
    float getXspeed() { return xspeed; }
    float getYspeed() { return yspeed; }

    // Animation control
    void setAnimation(std::string name);
    void setAnimRate(int fdelay);
    int getAnimRate() {return framedelay;}
    void play();
    void pause();
    void stop();
    void setLooped(bool looped) { looping = looped; }
    bool isLooped() { return looping; }
    bool isStopped() { return animState == STOPPED; }
    bool isPlaying() { return animState == PLAYING; }
    bool isPaused() { return animState == PAUSED; }
    enum AnimState { STOPPED, PLAYING, PAUSED };

    // Fine tuning animation controls
    bool setFrameRange(int first, int last);
    void setCurrentFrame(int c);
    void frameForward();
    void frameBack();

    int getCurrentFrame() { return curframe; }
    int getTotalFrames() { return totalFrames; }

    sf::Vector2u getSize() { return sf::Vector2u(spriteW, spriteH); }

    void update(float deltaTime, bool updatePos = true);

    // Basic collision checking
    bool bboxCollision(Sprite& other);
    bool circleCollision(Sprite& other);

    void setColor(const sf::Color& color) {
        vertices[0].color = color;
        vertices[1].color = color;
        vertices[2].color = color;
        vertices[3].color = color;
    }

    sf::Color getColor() { return vertices[0].color; }

//    sf::FloatRect getLocalBounds() const;
//    sf::FloatRect getGlobalBounds() const;
//    bool isPlaying() const;
//    sf::Time getFrameTime() const;
//    void setFrame(std::size_t newFrame, bool resetTime = true);

private:

    static TextureManager* tm;

    // Rendering
    const sf::Texture* tex;
    sf::Vertex vertices[4];
    int spriteW, spriteH;       // width and height of a single sprite frame
    bool visible;
    bool mirror;

    // Motion
    float xspeed,yspeed;       // speed in pixels/s

    // Animation
    std::map<std::string, cgf::Anim> anims;
    std::vector<sf::IntRect> frames;
    cgf::Anim* currentAnim;
    int firstFrame, lastFrame;
    bool looping;
    AnimState animState;
//    bool paused;
    int totalFrames;
    int curframe;		        // current frame
    double curFrameD;           // the current frame as double
    int framecount,framedelay;  // slow down the frame animation

    bool loadMultiImage(const char *nomeArq,
                        int w,
                        int h,
                        int hSpace,
                        int vSpace,
                        int xIni,
                        int yIni,
                        int column,
                        int row,
                        int total);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

} // namespace cgf

#endif // CGF_SPRITE_H
