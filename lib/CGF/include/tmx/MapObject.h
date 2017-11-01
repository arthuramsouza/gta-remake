/*********************************************************************
Matt Marchant 2013
SFML Tiled Map Loader - https://github.com/bjorn/tiled/wiki/TMX-Map-Format

The zlib license has been used to make this software fully compatible
with SFML. See http://www.sfml-dev.org/license.php

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
   you must not claim that you wrote the original software.
   If you use this software in a product, an acknowledgment
   in the product documentation would be appreciated but
   is not required.

2. Altered source versions must be plainly marked as such,
   and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
   source distribution.
*********************************************************************/

#ifndef MAP_OBJECT_H
#define MAP_OBJECT_H

#include <string>
#include <vector>
#include <map>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <tmx/Helpers.h>
#include "Debug.h"

namespace tmx
{
	enum MapObjectShape
	{
		Rectangle,
		Ellipse,
		Polygon,
		Polyline,
		Tile
	};

	//map object class.
	class MapObject
	{
	public:
		MapObject() : m_visible(true), m_rotation(0.f), m_shape(Rectangle), m_furthestPoint(0.f)
		{
			m_debugShape.setPrimitiveType(sf::LinesStrip);

			//this loads a font for text output during debug drawing
			//you need to select your own font here as SFML no longer
			//supports a default font. If you do not plan to use this
			//during debugging it can be ignored.
            if(!m_debugFont.loadFromFile("data/fonts/arial.ttf"))
			{
				//feel free to supress these messages
				DEBUG_MSG("If you wish to output text during debugging please specify a font file in the map object class");
				DEBUG_MSG("If you do not wish to use debug output this can be safely ignored.");
			}
		};

		//**accessors**//
		//returns empty string if property not found
		const std::string GetPropertyString(std::string name)
		{
			if(m_properties.find(name) != m_properties.end())
				return m_properties[name];
			else
				return std::string();
		}
		//returns top left corner of bounding rectangle
		const sf::Vector2f GetPosition(void) const {return m_position;}
		//returns precomputed centre of mass, or zero for polylines
        const sf::Vector2f GetCentre(void) const {return m_centrePoint;}
		//returns the type of shape of the object
        const MapObjectShape GetShapeType(void) const {return m_shape;}
		//returns and object's name
        const std::string GetName(void) const {return m_name;}
		//returns the object's type
        const std::string GetType(void) const {return m_type;}
		//returns the name of the object's parent layer
        const std::string GetParent(void) const {return m_parent;}
		//returns the object's rotation in degrees
        const float GetRotation(void) const {return m_rotation;}
		//returns the objects AABB in world coordinates
        const sf::FloatRect GetAABB(void) const {return m_AABB;}
		//returns visibility
		const bool Visible(void) const {return m_visible;}
		//sets a property value, or adds it if property doesn't exist
		void SetProperty(const std::string name, const std::string value)
		{
			m_properties[name] = value;
		}
		//sets the object position in world coords
		void SetPosition(const sf::Vector2f& position)
		{
			sf::Vector2f distance = position - m_position;
			Move(distance);
		};
		//moves the object by given amount
		void Move(const sf::Vector2f& distance)
		{
			//update properties by movement amount
			m_centrePoint += distance;
            for(auto p : m_polypoints)
				p += distance;
			
			for(auto i = 0u; i < m_debugShape.getVertexCount(); ++i)
				m_debugShape[i].position += distance;

			m_AABB.left += distance.x;
			m_AABB.top += distance.y;

			//set new position
			m_position += distance;			
		}
		//sets the width and height of the object
        void SetSize(const sf::Vector2f& size){m_size = size;}
		//sets the object's name
		void SetName(const std::string name){m_name = name;}
		//sets the object's type
        void SetType(const std::string type){m_type = type;}
		//sets the name of the object's parent layer
        void SetParent(const std::string parent){m_parent = parent;}
		//sets the rotation of the object in degrees
		void SetRotation(const float angle)
		{
			m_rotation = angle;
		}
		//sets the shape type
        void SetShapeType(MapObjectShape shape){m_shape = shape;}
        //sets visibility
        void SetVisible(bool visible){m_visible = visible;}
		//adds a point to the list of polygonal points. If calling this manually
		//call CreateDebugShape() afterwards to rebuild debug output
        void AddPoint(const sf::Vector2f point){m_polypoints.push_back(point);}

		//checks if an object contains given point in world coords.
		//Always returns false for polylines.
		const bool Contains(sf::Vector2f point)
		{
			if(m_shape == Polyline) return false;

			//convert point to local coords
			point-= m_position;

			//rotate point relative to object rotation
			if(m_rotation)
			{
				sf::Transform tf;
				tf.rotate(-m_rotation);
				point = tf.transformPoint(point);
			}

			//check if enough poly points
			if(m_polypoints.size() < 3) return false;

			//else raycast through points
			unsigned int i, j;
			bool result = false;
			for (i = 0, j = m_polypoints.size() - 1; i < m_polypoints.size(); j = i++)
			{
				if (((m_polypoints[i].y > point.y) != (m_polypoints[j].y > point.y)) &&
				(point.x < (m_polypoints[j].x - m_polypoints[i].x) * (point.y - m_polypoints[i].y)
					/ (m_polypoints[j].y - m_polypoints[i].y) + m_polypoints[i].x))
						result = !result;
			}
			return result;
		}
		//checks if two objects intersect, including polylines
		const bool Intersects(MapObject& object)
		{
			//check if distance between objects is less than sum of furthest points
			float distance = Helpers::Vectors::GetLength(m_centrePoint + object.m_centrePoint);
			if(distance > (m_furthestPoint + object.m_furthestPoint)) return false;

			//if shapes are close enough to intersect check each point
			for(auto i = object.m_polypoints.cbegin(); i != m_polypoints.cend(); ++i)
				if(Contains(*i + object.GetPosition())) return true;

			for(auto i = m_polypoints.cbegin(); i != m_polypoints.cend(); ++i)
				if(object.Contains(*i + GetPosition())) return true;

			return false;
		}

		//creates a shape used for debug drawing - points are in world space
		void CreateDebugShape(const sf::Color colour)
		{
			//reset any existing shapes incase new points have been added
			m_debugShape.clear();
			
			//draw poly points
			for(auto i = m_polypoints.cbegin(); i != m_polypoints.cend(); ++i)
				m_debugShape.append(sf::Vertex(*i + m_position, colour));

			if(m_shape != Polyline)
			{
				//close shape by copying first point to end
				m_debugShape.append(m_debugShape[0]);
			}

			//precompute shape values for intersection testing
			m_CalcTestValues();

			//create the AABB for quad tree testing
			m_CreateAABB();
		}
		//draws debug shape to given target
		void DrawDebugShape(sf::RenderTarget& rt)
		{
			rt.draw(m_debugShape);
			sf::Text text(m_name, m_debugFont, 14u);
			text.setPosition(m_position);
			text.setFillColor(sf::Color::Black);
			rt.draw(text);
		}
	
private:
		//object properties, reflects those which are part of the tmx format
		std::string m_name, m_type, m_parent; //parent is name of layer to which object belongs
		//sf::FloatRect m_rect; //width / height property of object plus position in world space
		sf::Vector2f m_position, m_size;
		std::map <std::string, std::string> m_properties;//map of custom name/value properties
		float m_rotation; //not yet implemented in official TILED release
		bool m_visible;
		std::vector<sf::Vector2f> m_polypoints; //list of points defining any polygonal shape
		MapObjectShape m_shape;
		sf::VertexArray m_debugShape;
		sf::Font m_debugFont;
		sf::Vector2f m_centrePoint;
		float m_furthestPoint; //furthest distance from centre of object to vertex - used for intersection testing
		//AABB created from polygonal shapes, used for adding MapObjects to a QuadTreeNode.
		//Note that the position of this box many not necessarily match the MapObject's position, as polygonal
		//points may have negative values relative to the object's world position.
		sf::FloatRect m_AABB; 


		//returns centre of poly shape if available, else centre of
		//bounding rectangle in world space
		const sf::Vector2f m_CalcCentre(void) const
		{
			if(m_shape == Polyline) return sf::Vector2f();

			if(m_shape == Rectangle || m_polypoints.size() < 3)
			{
				//we don't have a triangle so use bounding box
				return sf::Vector2f(m_position.x + (m_size.x / 2.f), m_position.y + (m_size.y / 2.f));
			}
			//calc centroid of poly shape
			sf::Vector2f centroid;
			float signedArea = 0.f;
			float x0 = 0.f; // Current vertex X
			float y0 = 0.f; // Current vertex Y
			float x1 = 0.f; // Next vertex X
			float y1 = 0.f; // Next vertex Y
			float a = 0.f;  // Partial signed area

			// For all vertices except last
			unsigned i;
			for(i = 0; i < m_polypoints.size() - 1; ++i)
			{
				x0 = m_polypoints[i].x;
				y0 = m_polypoints[i].y;
				x1 = m_polypoints[i + 1].x;
				y1 = m_polypoints[i + 1].y;
				a = x0 * y1 - x1 * y0;
				signedArea += a;
				centroid.x += (x0 + x1) * a;
				centroid.y += (y0 + y1) * a;
			}

			// Do last vertex
			x0 = m_polypoints[i].x;
			y0 = m_polypoints[i].y;
			x1 = m_polypoints[0].x;
			y1 = m_polypoints[0].y;
			a = x0 * y1 - x1 * y0;
			signedArea += a;
			centroid.x += (x0 + x1) * a;
			centroid.y += (y0 + y1) * a;

			signedArea *= 0.5;
			centroid.x /= (6 * signedArea);
			centroid.y /= (6 * signedArea);

			//convert to world space
			centroid += m_position;
			return centroid;
		};
		//precomputes centre point and furthest point to be used in intersection testing
		void m_CalcTestValues(void)
		{
			m_centrePoint = m_CalcCentre();
			for(auto i = m_polypoints.cbegin(); i != m_polypoints.cend(); ++i)
			{
				float length = Helpers::Vectors::GetLength(*i - m_centrePoint);
				if(m_furthestPoint < length)
				{
					m_furthestPoint = length;
					if(m_shape == Polyline) m_centrePoint = *i / 2.f;
				}
			}
			//polyline centre ought to be half way between the start point and the furthest vertex
			if(m_shape == Polyline) m_furthestPoint /= 2.f;
		}
		//creates an AABB around the object based on its polygonal points, in world space
		void m_CreateAABB(void)
		{
			if(!m_polypoints.empty())
			{
				m_AABB = sf::FloatRect(m_polypoints[0], m_polypoints[0]);
				for(auto point = m_polypoints.cbegin(); point != m_polypoints.cend(); ++point)
				{
					if(point->x < m_AABB.left) m_AABB.left = point->x;
					if(point->x > m_AABB.width) m_AABB.width = point->x;
					if(point->y < m_AABB.top) m_AABB.top = point->y;
					if(point->y > m_AABB.height) m_AABB.height = point->y;
				}

				//calc true width and height by distance between points
				m_AABB.width -= m_AABB.left;
				m_AABB.height -= m_AABB.top;

				//offset into world position
				m_AABB.left += m_position.x;
				m_AABB.top += m_position.y;


				//debug draw AABB
				//m_debugShape.append(sf::Vector2f(m_AABB.left, m_AABB.top));
				//m_debugShape.append(sf::Vector2f(m_AABB.left + m_AABB.width, m_AABB.top));
				//m_debugShape.append(sf::Vector2f(m_AABB.left + m_AABB.width, m_AABB.top + m_AABB.height));
				//m_debugShape.append(sf::Vector2f(m_AABB.left, m_AABB.top + m_AABB.height));
			}
		};
	};

	//represents a single tile on a layer
	struct MapTile
	{
		//returns the base centre point of sprite / tile
		const sf::Vector2f GetBase(void) const
		{
			return sf::Vector2f(sprite.getPosition().x + (sprite.getLocalBounds().width / 2.f),
				sprite.getPosition().y + sprite.getLocalBounds().height);
		}
		sf::Sprite sprite;
		sf::Vector2i gridCoord;
        sf::Uint16 gid; // MC 09/13: need gid to check contents
		sf::RenderStates renderStates; //used to perform any rendering with custom shaders or blend mode
	};

	enum MapLayerType
	{
		Layer,
		ObjectGroup,
		ImageLayer
	};

	//represents a layer of tiles, corresponding to a tmx layer, object group or image layer
	struct MapLayer
	{
        MapLayer(MapLayerType layerType) : opacity(1.f), visible(true), dynamic(false), type(layerType){}
		std::string name;
		float opacity; //range 0 - 1
		bool visible, dynamic; //dynamic layers contain sprites which need their order sorting, such as players / NPCs
		std::vector <MapTile> tiles;
		std::vector<MapObject> objects; //vector of objects if layer is object group
		MapLayerType type;
		std::map <std::string, std::string> properties;
		std::vector<sf::VertexArray> vertexArrays;

		//used for drawing specific layers
		enum DrawType
		{
			Front,
			Back,
			Debug,
			All
		};
	};
};

#endif //MAP_OBJECT_H
