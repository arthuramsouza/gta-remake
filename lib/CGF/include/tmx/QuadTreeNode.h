///Creates a node used to build quad trees for spatial partitioning of MapObjects///
//Example usage: create a root node the size of the viewable area, and insert each
//available map object. Then test the root node by calling retrieve passing for example
//the AABB of a sprite. The resulting vector will contain pointers to any objects contained
//in quads which are them selves contained, or intersected, by the sprites AABB. These can
//then be collision tested.

#ifndef QUADTREE_NODE_H_
#define QUADTREE_NODE_H_

//#include <Game/Common.h>
#include <tmx/MapObject.h>
#include <memory>

namespace tmx
{
	class QuadTreeNode
	{
	public:
		QuadTreeNode(sf::Uint16 level = 0, const sf::FloatRect& bounds = sf::FloatRect(0.f, 0.f, 1.f, 1.f))
			: MAX_OBJECTS(5u), MAX_LEVELS(5u), m_level(level),
			m_bounds(bounds)
		{ 
			m_children.reserve(4); 
			m_debugShape = sf::RectangleShape(sf::Vector2f(bounds.width, bounds.height));
			m_debugShape.setPosition(bounds.left, bounds.top);
			m_debugShape.setFillColor(sf::Color::Transparent);
			m_debugShape.setOutlineColor(sf::Color::Green);
			m_debugShape.setOutlineThickness(-2.f);

		};

		virtual ~QuadTreeNode(){};

		//fills vector with references to all objects which
		//appear in quads which are contained or intersect bounds.
		std::vector<MapObject*> Retrieve(const sf::FloatRect& bounds, sf::Uint16& currentDepth);
		//inserts a reference to the object into the node's object list
		void Insert(MapObject& object);
		//draws the node and any child nodes to given target
		void DebugDraw(sf::RenderTarget& rt);
	protected:
		//maximum objects per node before splitting
		const sf::Uint16 MAX_OBJECTS;
		//maximum number of levels to split
		const sf::Uint16 MAX_LEVELS;

		sf::Uint16 m_level;
		sf::FloatRect m_bounds;
		std::vector<MapObject*> m_objects; //objects contained in current node
		std::vector< std::shared_ptr<QuadTreeNode> > m_children; //vector of child nodes
		sf::RectangleShape m_debugShape;

		//returns the index of the child node into which the givens bounds fits.
		//returns -1 if doesn't completely fit a child. Numbered anti-clockwise
		//from top right node.
		sf::Int16 m_GetIndex(const sf::FloatRect& bounds);

		//divides node by creating 4 children
		void m_Split(void);

	};

	//specialisation of QuadTreeNode for counting tree depth
	class QuadTreeRoot : public QuadTreeNode
	{
	public:
		QuadTreeRoot(sf::Uint16 level = 0, const sf::FloatRect& bounds = sf::FloatRect(0.f, 0.f, 1.f, 1.f))
			: QuadTreeNode(level, bounds), m_depth(0u), m_searchDepth(0u){};

		//clears node and all children
		void Clear(const sf::FloatRect& newBounds);
		//retrieves all objects in quads which contains or intersect test area
		std::vector<MapObject*> Retrieve(const sf::FloatRect& bounds)
		{
			return QuadTreeNode::Retrieve(bounds, m_searchDepth);
		}

	private:
		//total depth of tree, and depth reached when querying
		sf::Uint16 m_depth, m_searchDepth;
	};
};


#endif //QUADTREE_NODE_H_
