#ifndef EEDEBUG_DRAW_H
#define EEDEBUG_DRAW_H

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

class EEDebugDraw3 : public b2Draw
{
private:
    sf::RenderTarget * m_target;
    //inliners for colour and point conversions
    inline sf::Color EEColor(const b2Color& gCol)
    {
        return sf::Color(static_cast<sf::Uint8>(255*gCol.r),
                        static_cast<sf::Uint8>(255*gCol.g),
                        static_cast<sf::Uint8>(255*gCol.b), 100);
    }
    inline sf::Vector2f EEVector(const b2Vec2& gVec){return sf::Vector2f(gVec.x*pixmeters,gVec.y*pixmeters);}
    const float pixmeters;//constants for point and degree conversions
public:
    EEDebugDraw3(void);
    virtual ~EEDebugDraw3(void) {}
    void LinkTarget(sf::RenderTarget& gtarget);
    virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
    virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
    virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
    virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
    virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
    virtual void DrawTransform(const b2Transform &xf) {}
    virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) {}
};

#endif

