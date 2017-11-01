/*
 *  Physics.h
 *  Physics manager using Box2D
 *
 *  Created by Marcelo Cohen on 05/11.
 *  Copyright 2011 PUCRS. All rights reserved.
 *
 */

#ifndef CGF_PHYSICS_H
#define CGF_PHYSICS_H

#include <Box2D/Box2D.h>
#include <memory>
#include <vector>
#include "Sprite.h"
#include "EEDebugDraw3.h"

#define _USE_MATH_DEFINES 1
#include <cmath>

namespace cgf
{

struct BodyData
{
    int id;         // id for the body (used in contacts)
    Sprite* image;  // image associated to the body (can be NULL)
    b2Color color;  // color for drawing collision object (when image == NULL)
};

class Physics
{
    using BDataPtr = std::unique_ptr<BodyData>;
    public:
        b2World* getWorld() { return world.get(); }
        b2Body* newRect(int id, Sprite* sprite, float density, float friction, float restitution, bool staticObj=false);
        b2Body* newRect(int id, float x, float y, float width, float height, float density, float friction, float restitution, bool staticObj=false);
        b2Body* newCircle(int id, Sprite* sprite, float density, float friction, float restitution, bool staticObj=false);
        b2Body* newCircle(int id, float x, float y, float radius, float density, float friction, float restitution, bool staticObj=false);
        void destroy(b2Body* bptr);

        void setImage(b2Body* body, Sprite* sprite);
        Sprite* getImage(b2Body* body);

        void setColor(b2Body* body, const b2Color& cor);
        b2Color& getColor(b2Body* body);

        void setPosition(b2Body* body, const b2Vec2& pos);
        void setAngle(b2Body* body, float angle);
        b2Vec2 getPosition(b2Body* body);

        void setGravity(float grav);
        float getGravity();
    
        void step();
        void drawDebugData();

        //void debugDraw(sf::RenderWindow &win);
        b2Body* haveContact(int id1, int id2);

        void setDrawOffset(float ox, float oy);

        static void setConvFactor(float conv);

        // Set where the drawing will take place (texture or screen)
        void setRenderTarget(sf::RenderTarget &win);

        // Implement Singleton Pattern
        static Physics* instance()
        {
            return &m_Physics;
        }
    protected:
        Physics();
    private:

        BodyData* createBodyData(int id, cgf::Sprite* image, b2Color color);
        static Physics m_Physics;
        static float CONV; // fator de conversão Box2D -> OpenGL

        //constantes
        static constexpr int velocityIterations{10};
        static constexpr int positionIterations{10};
        static constexpr float timeStep{1.0f / 30.0f};
        static constexpr float PI = 3.14159265358979323846f;



        float gravity;
        float offsetX, offsetY; // offset to apply to translation when drawing
        std::unique_ptr<b2World> world;
        std::vector<BDataPtr> userData;
        EEDebugDraw3 debugDraw;
};

} // namespace cgf
#endif // Physics_H
