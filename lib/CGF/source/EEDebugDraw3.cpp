#include "EEDebugDraw3.h"

EEDebugDraw3::EEDebugDraw3(void):
pixmeters(30.f),//arbitrary value dependant on the program needs
m_target(0x0)
{
    AppendFlags(static_cast<uint32>(~0));//set all drawing bits to 1(not all 32 are relevant but it's ok)
}

void EEDebugDraw3::LinkTarget(sf::RenderTarget& gtarget)
{
    m_target=&gtarget;
}

void EEDebugDraw3::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    sf::ConvexShape shape;
    shape.setOutlineColor(EEColor(color));
    shape.setOutlineThickness(1);
    shape.setFillColor(sf::Color::Transparent);
    shape.setPointCount(vertexCount);
    for(int i=0;i<vertexCount;++i)
    {shape.setPoint(i,EEVector(vertices[i]));}
    m_target->draw(shape);
}

void EEDebugDraw3::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    sf::ConvexShape shape;
    shape.setFillColor(EEColor(color));
    shape.setPointCount(vertexCount);
    for(int i=0;i<vertexCount;++i)
    {shape.setPoint(i,EEVector(vertices[i]));}
    m_target->draw(shape);
}

void EEDebugDraw3::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
    sf::CircleShape shape;
    shape.setOutlineColor(EEColor(color));
    shape.setOutlineThickness(1);
    shape.setFillColor(sf::Color::Transparent);
    shape.setRadius(radius*pixmeters);
    shape.setOrigin(sf::Vector2f(radius*pixmeters,radius*pixmeters));//set origin to middle or position setter below would not work correctly
    shape.setPosition(EEVector(center));
    m_target->draw(shape);
}

void EEDebugDraw3::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
    sf::CircleShape shape;
    shape.setFillColor(EEColor(color));
    shape.setRadius(radius*pixmeters);
    shape.setOrigin(sf::Vector2f(radius*pixmeters,radius*pixmeters));
    shape.setPosition(EEVector(center));
    m_target->draw(shape);
}

void EEDebugDraw3::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    sf::Vertex line[2];//const sized c styled array, safe enough in here
    line[0].color=EEColor(color);
    line[0].position=EEVector(p1);
    line[1].color=EEColor(color);
    line[1].position=EEVector(p2);
    m_target->draw(line,2,sf::Lines);
}

