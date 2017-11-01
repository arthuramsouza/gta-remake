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

#include <tmx/MapLoader.h>
#include "Debug.h"

using namespace tmx;

//ctor
MapLoader::MapLoader(const std::string mapDirectory)
	: m_width			(1u),
	m_height			(1u),
	m_tileWidth			(1u),
	m_tileHeight		(1u),
	m_mapLoaded			(false),
    m_quadTreeAvailable	(false),
	m_mapDirectory		(mapDirectory),
	m_tileRatio			(1.f)
{
	//reserve some space to help reduce reallocations
	m_layers.reserve(10);
	m_tileTextures.reserve(80);

	//check map directory contains trailing slash
	if(!m_mapDirectory.empty() && *m_mapDirectory.rbegin() != '/')
		m_mapDirectory += '/';

}
//dtor
MapLoader::~MapLoader()
{

}

const bool MapLoader::Load(std::string map)
{
	map = m_mapDirectory + map;
	m_Unload(); //clear any old data first

	//parse map xml, return on error
	pugi::xml_document mapDoc;
	pugi::xml_parse_result result = mapDoc.load_file(map.c_str());
	if(!result)
	{
		DEBUG_MSG("Failed to open " << map);
		DEBUG_MSG("Reason: " << result.description());
		return m_mapLoaded = false;
	}

	//set map properties
	pugi::xml_node mapNode = mapDoc.child("map");
	if(!mapNode)
	{
		DEBUG_MSG("Map node not found. Map " << map << " not loaded.");
		return m_mapLoaded = false;
	}
	if(!(m_mapLoaded = m_ParseMapNode(mapNode))) return false;

	//load map textures / tilesets
	if(!(m_mapLoaded = m_ParseTileSets(mapNode))) return false;


	//actually we need to traverse map node children and parse each layer as found
	pugi::xml_node currentNode = mapNode.first_child();
	while(currentNode)
	{
		std::string name = currentNode.name();
		if(name == "layer")
		{
			if(!(m_mapLoaded = m_ParseLayer(currentNode)))
			{
				m_Unload(); //purge partially loaded data
				return false;
			}
		}
		else if(name == "imagelayer")
		{
			if(!(m_mapLoaded = m_ParseImageLayer(currentNode)))
			{
				m_Unload();
				return false;
			}
		}
		else if(name == "objectgroup")
		{
			if(!(m_mapLoaded = m_ParseObjectgroup(currentNode)))
			{
				m_Unload();
				return false;
			}
		}
		//std::cout << name << std::endl;
		currentNode = currentNode.next_sibling();
	}

	m_CreateDebugGrid();

	DEBUG_MSG("Parsed " << m_layers.size() << " layers.");
	DEBUG_MSG("Loaded " << map << " successfully.");

	return m_mapLoaded = true;
}

void MapLoader::UpdateQuadTree(const sf::FloatRect& rootArea)
{
	m_rootNode.Clear(rootArea);
	for(auto layer = m_layers.begin(); layer != m_layers.end(); ++layer)
	{
		for(auto object = layer->objects.begin(); object != layer->objects.end(); ++object)
		{
			m_rootNode.Insert(*object);
		}
	}
	m_quadTreeAvailable = true;
}

std::vector<MapObject*> MapLoader::QueryQuadTree(const sf::FloatRect& testArea)
{
	//quad tree must be updated at least once with UpdateQuadTree before we can call this
	if(!m_quadTreeAvailable) throw;
	return m_rootNode.Retrieve(testArea);
}

void MapLoader::Draw(sf::RenderTarget& rt)
{
	m_SetDrawingBounds(rt.getView());
	for(auto layer = m_layers.begin(); layer != m_layers.end(); ++layer)
	{
		if(!layer->visible) continue; //skip invisible layers
		for(unsigned i = 0; i < layer->vertexArrays.size(); i++)
		{
			rt.draw(layer->vertexArrays[i], &m_tilesetTextures[i]);
		}
		if(layer->type == ObjectGroup || layer->type == ImageLayer)
		{
			//draw tiles used on objects
			for(auto tile = layer->tiles.begin(); tile != layer->tiles.end(); ++tile)
			{
				//draw tile if in bounds and is not transparent
				if((m_bounds.contains(tile->sprite.getPosition()) && tile->sprite.getColor().a)
					|| layer->type == ImageLayer) //always draw image layer
				{
					rt.draw(tile->sprite, tile->renderStates);
				}
			}
		}
	}
}

void MapLoader::Draw(sf::RenderTarget& rt, MapLayer::DrawType type)
{
	switch(type)
	{
	default:
	case MapLayer::All:
		Draw(rt);
		break;
	case MapLayer::Back:
		{
		//remember front of vector actually draws furthest back
		MapLayer& layer = m_layers.front();
		m_DrawLayer(rt, layer);
		}
		break;
	case MapLayer::Front:
		{
		MapLayer& layer = m_layers.back();
		m_DrawLayer(rt, layer);
		}
		break;
	case MapLayer::Debug:
		m_SetDrawingBounds(rt.getView());
		for(auto layer : m_layers)
		{
			if(layer.type == ObjectGroup)
			{
			for(auto object : layer.objects)		
				object.DrawDebugShape(rt);
			}
		}
		rt.draw(m_gridVertices);
		m_rootNode.DebugDraw(rt);
		break;
	}
}

void MapLoader::Draw(sf::RenderTarget& rt, sf::Uint16 index)
{
	m_DrawLayer(rt, m_layers[index]);
}

//legacy draw function, avoid using this if possible
void MapLoader::Draw2(sf::RenderTarget& rt, bool debug)
{
	if(!m_mapLoaded) return; //no need to log this really
	//draw only visible tiles
	m_SetDrawingBounds(rt.getView());
	for(auto layer = m_layers.begin(); layer != m_layers.end(); ++layer)
	{
		if(!layer->visible) continue; //skip invisible layers
		for(auto tile = layer->tiles.begin(); tile != layer->tiles.end(); ++tile)
		{
			//draw tile if in bounds and is not transparent
			if((m_bounds.contains(tile->sprite.getPosition()) && tile->sprite.getColor().a)
				|| layer->type == ImageLayer) //always draw image layer
			{
				rt.draw(tile->sprite, tile->renderStates);
			}
		}
		if(debug && layer->type == ObjectGroup)
		{
			//draw debug shapes for each object
			for(auto object = layer->objects.begin(); object != layer->objects.end(); ++object)
				object->DrawDebugShape(rt);
		}
	}
	if(debug)
	{
		rt.draw(m_gridVertices);
		m_rootNode.DebugDraw(rt);
	}
}

const sf::Vector2f MapLoader::IsometricToOrthogonal(const sf::Vector2f& projectedCoords)
{
	//skip converting if we don't actually have an isometric map loaded
	if(m_orientation != Isometric) return projectedCoords;

	return sf::Vector2f(projectedCoords.x - projectedCoords.y, (projectedCoords.x / m_tileRatio) + (projectedCoords.y / m_tileRatio));
}

const sf::Vector2f MapLoader::OrthogonalToIsometric(const sf::Vector2f& worldCoords)
{
	if(m_orientation != Isometric) return worldCoords;

	return sf::Vector2f(((worldCoords.x / m_tileRatio) + worldCoords.y),
							(worldCoords.y - (worldCoords.x / m_tileRatio)));
}
