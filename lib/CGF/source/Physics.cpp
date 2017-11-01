/*
 *  Physics.h
 *  Physics wrapper using Box2D
 *
 *  Created by Marcelo Cohen on 09/13.
 *  Copyright 2013 PUCRS. All rights reserved.
 *
 */

#include "Physics.h"
#include "Debug.h"
#define RAD2DEG 57.2957795f

namespace cgf
{

using namespace std;

float Physics::CONV = 10;
Physics Physics::m_Physics;

void Physics::setConvFactor(float conv)
{
    CONV = conv;
    DEBUG_MSG("Physics::setConvFactor" << conv);
}

Physics::Physics(): userData{}
{
    // Init Box2D world
    b2Vec2 gravity(0,10.0f);
    world = std::unique_ptr<b2World>{new b2World(gravity)};
    offsetX = offsetY = 0;
}

void Physics::setRenderTarget(sf::RenderTarget& win)
{
    debugDraw.LinkTarget(win);
    world->SetDebugDraw(&debugDraw);
    debugDraw.SetFlags( b2Draw::e_shapeBit );
}

// Add a new rect and associate a sprite to it
b2Body* Physics::newRect(int id, Sprite* image, float density, float friction, float restitution, bool staticObj)
{
    b2BodyDef bd;
    if(!staticObj)
        bd.type = b2_dynamicBody;
    sf::Vector2f pos = image->getPosition();
    sf::Vector2f scale = image->getScale();
    sf::Vector2u size = image->getSize();
    float width = size.x*scale.x/CONV;
    float height = size.y*scale.y/CONV;
    image->setOrigin(sf::Vector2f(size.x/2,size.y/2));

    DEBUG_MSG("Physics::newBoxImage " << pos.x << "," << pos.y << " - " << size.x << " x " << size.y);

	b2PolygonShape box;
    box.SetAsBox(width/2,height/2);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &box;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    fixtureDef.restitution = restitution;

    b2Vec2 pos2;
    pos2.x = pos.x + width*CONV/2;
    pos2.y = pos.y + height*CONV/2;
    bd.position.Set(pos2.x/CONV, pos2.y/CONV);
	b2Body* body = world->CreateBody(&bd);

	body->CreateFixture(&fixtureDef);

    // Pelo que eu pude ver essa linha gera uma memory leak.
    // Lendo a documentção do Box2D a função SetUserData, não faz nenhuma cópia dos dados e
    // nem faz um delete do ponteiro passado para a função.
    // Assumindo que esta função possa ser chamada várias vezes, com dados diferentes
    // é necessário guardar os dados gerados em um array até o fim da classe
    BodyData* ptr = createBodyData(id, image, b2Color(0, 0, 0));
	body->SetUserData(ptr);

	return body;
}

// Add new rect to world
b2Body* Physics::newRect(int id, float x, float y, float width, float height, float density, float friction, float restitution, bool staticObj)
{
    b2BodyDef bd;
    if(!staticObj)
        bd.type = b2_dynamicBody;

    DEBUG_MSG("Physics::newBox " << x << "," << y << " - " << width << " x " << height);

	b2PolygonShape box;
    width = width/2;
    height = height/2;
    box.SetAsBox(width/CONV,height/CONV);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &box;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    fixtureDef.restitution = restitution;

    x += width;
    y += height;
    bd.position.Set(x/CONV,y/CONV);
	b2Body* body = world->CreateBody(&bd);

	body->CreateFixture(&fixtureDef);

    BodyData* ptr = createBodyData(id, nullptr, b2Color(0, 0, 0));
    body->SetUserData(ptr);
	return body;
}

// Add a new circle and associate a sprite to it
b2Body* Physics::newCircle(int id, Sprite* image, float density, float friction, float restitution, bool staticObj)
{
    b2BodyDef bd;
    if(!staticObj)
        bd.type = b2_dynamicBody;
    sf::Vector2f pos = image->getPosition();
    sf::Vector2f scale = image->getScale();
    sf::Vector2u size = image->getSize();
    float width = size.x*scale.x/CONV;
    float height = size.y*scale.y/CONV;
    image->setOrigin(sf::Vector2f(size.x/2,size.y/2));

    // Assume radius as the longest dimension
    float radius = max(width,height);
    radius/=2;

    DEBUG_MSG("Physics::newCircleImage " << pos.x << "," << pos.y << " - " << radius);

    b2CircleShape cs;
    cs.m_radius = radius;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &cs;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    fixtureDef.restitution = restitution;

    b2Vec2 pos2;
    pos2.x = pos.x + radius*CONV/2;
    pos2.y = pos.y + radius*CONV/2;
    bd.position.Set(pos2.x/CONV, pos2.y/CONV);
	b2Body* body = world->CreateBody(&bd);

	body->CreateFixture(&fixtureDef);

	BodyData* bodyData = createBodyData(id, image, b2Color(0, 0, 0));
	body->SetUserData(bodyData);

	return body;
}

// Add new circle to world
b2Body* Physics::newCircle(int id, float x, float y, float radius, float density, float friction, float restitution, bool staticObj)
{
    b2BodyDef bd;
    bd.type = b2_dynamicBody;

    b2CircleShape cs;
    cs.m_radius = radius/CONV;
    bd.position.Set(x/CONV,y/CONV);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &cs;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
	fixtureDef.restitution = restitution;

	b2Body* body = world->CreateBody(&bd);
	body->CreateFixture(&fixtureDef);

	BodyData* bodyData = createBodyData(id, nullptr, b2Color(0, 0, 0));
	body->SetUserData(bodyData);

    return body;
}

void Physics::setImage(b2Body* body, Sprite* Image)
{
    BodyData* bd = static_cast<BodyData*>(body->GetUserData());
    bd->image = Image;
}

Sprite* Physics::getImage(b2Body* body)
{
    BodyData* bd = static_cast<BodyData*>(body->GetUserData());
    return bd->image;
}

void Physics::setColor(b2Body* body, const b2Color& cor)
{
    BodyData* bd = static_cast<BodyData*>(body->GetUserData());
    bd->color = cor;
}

b2Color& Physics::getColor(b2Body* body)
{
    BodyData* bd = static_cast<BodyData*>(body->GetUserData());
    return bd->color;
}

void Physics::destroy(b2Body* bptr)
{
    world->DestroyBody(bptr);
}

void Physics::step()
{
    world->Step(timeStep, velocityIterations, positionIterations);
    world->ClearForces();
    for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
    {
        BodyData* ptr = static_cast<BodyData*>(b->GetUserData());
        if(ptr->image != nullptr) {
            const b2Vec2& pos = b->GetPosition();
            float rot = b->GetAngle();
            rot = rot * RAD2DEG;
            ptr->image->setPosition(pos.x*CONV, pos.y*CONV);
            ptr->image->setRotation(rot);
        }
    }

}

void Physics::drawDebugData()
{
    world->DrawDebugData();
}

// Process contact list for this time step, and return ptr of body
// with id2, if there was a contact between body id1 and id2
b2Body* Physics::haveContact(int id1, int id2)
{
    for (b2Contact* c = world->GetContactList(); c; c = c->GetNext())
    {
        // process c
        b2Fixture* a = c->GetFixtureA();
        b2Fixture* b = c->GetFixtureB();
        b2Body* ba = a->GetBody();
        b2Body* bb = b->GetBody();
        BodyData* bda = static_cast<BodyData*>(ba->GetUserData());
        BodyData* bdb = static_cast<BodyData*>(bb->GetUserData());

        if(bda->id==id1 && bdb->id==id2)
            return bb;
        else if(bda->id==id2 && bdb->id==id1)
            return ba;
    }
    return nullptr;
}

// Draw all physics objects with transparency
#ifdef DEBUG_DRAW_INLINE
void Physics::debugDraw(sf::RenderWindow& win)
{
    sf::CircleShape circle;
    sf::RectangleShape rectangle;

    for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
    {
        BodyData* ptr = static_cast<BodyData*>(b->GetUserData());
        if(ptr->image == nullptr || 1) {
            const b2Vec2& pos = b->GetPosition();
            float rot = b->GetAngle();
            rot = rot * 180/M_PI;
            //cout << "Box: " << pos.x << " " << pos.y << " - " << rot;
            //glColor4f(ptr->color.r, ptr->color.g, ptr->color.b, 0.2);
            // Iterate through all fixtures
            b2Fixture* f = b->GetFixtureList();
            float tx = pos.x * CONV;
            float ty = pos.y * CONV;
//            cout << " " << ptr->offset.x << " " << ptr->offset.y << endl;
            //tx += 64;//ptr->offset.x;
            //ty += 64;//ptr->offset.y;
            //tx += ptr->offset.x;
            //ty += ptr->offset.y;
            tx += offsetX;
            ty += offsetY;
            /*
            if(ptr->image != nullptr)
            {
                tx += ptr->image->getXOffset();
                ty += ptr->image->getYOffset();
            }
            else {
                tx += offsetX;
                ty += offsetY;
            }
            */
            //glPushMatrix();
            //glColor4f(1,1,1,0.4);
            //glTranslatef(tx, ty, 0);
            //glRotatef(rot, 0, 0, 1);
//#define NEWAPPROACH
#ifdef NEWAPPROACH
            glBegin(GL_LINE_LOOP);
            glVertex2f(-ptr->size.x/2,-ptr->size.y/2);
            glVertex2f(-ptr->size.x/2, ptr->size.y/2);
            glVertex2f( ptr->size.x/2, ptr->size.y/2);
            glVertex2f( ptr->size.x/2,-ptr->size.y/2);
            glEnd();
            glPopMatrix();
#else
            b2PolygonShape* pol;
            b2CircleShape* circ;
            b2Vec2 min, max, sizes;
            while(f != nullptr)
            {
                switch(f->GetType())
                {
                    case b2Shape::e_polygon:
                        pol = (b2PolygonShape*) f->GetShape();

                        DEBUG_MSG("b2Shape::poly");
                        min.x = min.y = 1E5;
                        max.x = max.y = -1E5;

                        /*glBegin(GL_POLYGON);*/
                          for(int i=0; i<pol->GetVertexCount(); i++)
                          {
                              const b2Vec2& v = pol->GetVertex(i);
//                              cout << v.x << "," << v.y << " ";
                              if(v.x < min.x) min.x = v.x;
                              if(v.x > max.x) max.x = v.x;
                              if(v.y < min.y) min.y = v.y;
                              if(v.y > max.y) max.y = v.y;
                              //glVertex2f(v.x*CONV,v.y*CONV);
                          }

                        sizes = max - min;
//                        cout << "Min: " << min.x << ", " << min.y << endl;
//                        cout << "Max: " << max.x << ", " << max.y << endl;
//                        cout << "Size: " << sizes.x << " x " << sizes.y << endl;
                        //glEnd();*/
                        rectangle.setSize(sf::Vector2f(sizes.x*CONV,sizes.y*CONV));
                        rectangle.setPosition(tx,ty);
                        rectangle.setFillColor(sf::Color(255,0,0,100));
                        win.draw(rectangle);
                        break;
                    case b2Shape::e_circle:
                        circ = (b2CircleShape*) f->GetShape();
                        float radius = circ->m_radius;
                        DEBUG_MSG("b2Shape::circle");
                        //cout << "drawing circle " << radius << endl;
                        /*glBegin(GL_TRIANGLE_FAN);
                          glVertex2f(0,0);
                          for(float a=0; a<=2*M_PI; a+=0.6)
                          {
                              float px = cos(a) * radius;
                              float py = sin(a) * radius;
                              //cout << px << " " << py << endl;
                              glVertex2f(px*CONV,py*CONV);
                          }
                        //cout << endl;
                        glEnd();*/
                }
//                cout << "******" << endl;
                f = f->GetNext();
                //glPopMatrix();
            }
#endif
        }
    }
    //glColor3f(1,1,1);
}
#endif

void Physics::setGravity(float grav)
{
    gravity = grav;
    world->SetGravity(b2Vec2(0,grav));
}

float Physics::getGravity()
{
    return gravity;
}

void Physics::setPosition(b2Body* body, const b2Vec2& pos)
{
    float angle = body->GetAngle();
    b2Vec2 p = pos;
    p.x = p.x/CONV;
    p.y = p.y/CONV;
    body->SetTransform(pos, angle);
}

b2Vec2 Physics::getPosition(b2Body* body)
{
   b2Vec2 pos=body->GetPosition();
   pos.x = pos.x*CONV;
   pos.y = pos.y*CONV;
   return pos;
}

void Physics::setAngle(b2Body* body, float angle)
{
    const b2Vec2& pos = body->GetPosition();
    body->SetTransform(pos, angle*PI/180.0f);
}

void Physics::setDrawOffset(float ox, float oy)
{
    offsetX = ox;
    offsetY = oy;
}


BodyData *Physics::createBodyData(int id, cgf::Sprite *image, b2Color color) {
    BDataPtr bodyData{new BodyData};
    bodyData->id = id;
    bodyData->image = image;
    bodyData->color = color;
    BodyData* ptr = bodyData.get();
    userData.push_back(std::move(bodyData));
    return ptr;
}

} // namespace cgf
