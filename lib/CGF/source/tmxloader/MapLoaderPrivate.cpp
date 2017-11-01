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
#include <sstream>
#include <zlib.h>
#include <cstring>
#include "Debug.h"

using namespace tmx;

void MapLoader::m_Unload()
{
	m_layers.clear();
	m_tileTextures.clear();
	m_imageLayerTextures.clear();
	m_mapLoaded = false;
	m_quadTreeAvailable = false;
}

void MapLoader::m_SetDrawingBounds(const sf::View& view)
{
	sf::FloatRect bounds;
	bounds.left = view.getCenter().x - (view.getSize().x / 2.f);
	bounds.top = view.getCenter().y - (view.getSize().y / 2.f);
	bounds.width = view.getSize().x;
	bounds.height = view.getSize().y;

	//add a tile border to prevent gaps appearing
	bounds.left -= static_cast<float>(m_tileWidth);
	bounds.top -= static_cast<float>(m_tileHeight);
	bounds.width += static_cast<float>(m_tileWidth * 2);
	bounds.height += static_cast<float>(m_tileHeight * 2);

	m_bounds = bounds;
}

const bool MapLoader::m_ParseMapNode(const pugi::xml_node& mapNode)
{
	//parse tile properties
	if(!(m_width = mapNode.attribute("width").as_int()) ||
		!(m_height = mapNode.attribute("height").as_int()) ||
		!(m_tileWidth = mapNode.attribute("tilewidth").as_int()) ||
		!(m_tileHeight = mapNode.attribute("tileheight").as_int()))
	{
		DEBUG_MSG("Invalid tile size found, check map data. Map not loaded.");
		return false;
	}

	//parse orientation property
	std::string orientation = mapNode.attribute("orientation").as_string();

	if(orientation == "orthogonal")
	{
		m_orientation = Orthogonal;
	}
	else if(orientation == "isometric")
	{
		m_orientation = Isometric;
		m_tileRatio = static_cast<float>(m_tileWidth) / static_cast<float>(m_tileHeight);
	}
	else
	{
		DEBUG_MSG("Map orientation " << orientation << " not currently supported. Map not loaded.");
		return false;
	}

	//parse any map properties
	if(pugi::xml_node propertiesNode = mapNode.child("properties"))
	{
		pugi::xml_node propertyNode = propertiesNode.child("property");
		while(propertyNode)
		{
			std::string name = propertyNode.attribute("name").as_string();
			std::string value = propertyNode.attribute("value").as_string();
			m_properties[name] = value;
			propertyNode = propertyNode.next_sibling("property");
			DEBUG_MSG("Added map property " << name << " with value " << value);
		}
	}

	return true;
}

const bool MapLoader::m_ParseTileSets(const pugi::xml_node& mapNode)
{
	pugi::xml_node tileset;
	if(!(tileset = mapNode.child("tileset")))
	{
		DEBUG_MSG("No tile sets found.");
		return false;
	}
	DEBUG_MSG("Caching image files, please wait...");

	//first tile should always be transparent / empty as GIDs start at 1
	if(!m_tileTextures.empty()) m_tileTextures.clear();
	sf::Image temp;
	temp.create(m_tileWidth, m_tileHeight, sf::Color::Transparent);
	m_tileTextures.push_back(sf::Texture());
	m_tileTextures.back().loadFromImage(temp);

	//empty vertex tile
	m_tileInfo.push_back(TileInfo());

	//parse tile sets in order so GUIDs match index
	while(tileset)
	{
		//if source attrib parse external tsx
		if(tileset.attribute("source"))
		{
			//try loading tsx
			std::string path = m_mapDirectory + tileset.attribute("source").as_string();
			pugi::xml_document tsxDoc;
			pugi::xml_parse_result result = tsxDoc.load_file(path.c_str());
			if(!result)
			{
				DEBUG_MSG("Failed to open external tsx document: " << path);
				DEBUG_MSG("Reason: " << result.description());
				m_Unload(); //purge any partially loaded data
				return false;
			}
			//try parsing tileset node
			pugi::xml_node ts = tsxDoc.child("tileset");

			if(!m_ProcessTiles(ts)) return false;
		}
		else //try for tmx map file data
		{
			if(!m_ProcessTiles(tileset)) return false;
		}

		//move on to next tileset node
		tileset = tileset.next_sibling("tileset");
	}

	DEBUG_MSG("Cached " << m_tileTextures.size() << " tiles.");
	return true;
}

const bool MapLoader::m_ProcessTiles(const pugi::xml_node& tilesetNode)
{
	sf::Uint16 spacing, margin;
    auto tileWidth = tilesetNode.attribute("tilewidth").as_uint();
    auto tileHeight = tilesetNode.attribute("tileheight").as_uint();
	//try and parse tile sizes
	if(!(tileWidth) ||
		!(tileHeight))
	{
		DEBUG_MSG( "Invalid tileset data found. Map not loaded.");
		m_Unload();
		return false;
	}
	spacing = (tilesetNode.attribute("spacing")) ? tilesetNode.attribute("spacing").as_int() : 0u;
	margin = (tilesetNode.attribute("margin")) ? tilesetNode.attribute("margin").as_int() : 0u;

	//try parsing image node
	pugi::xml_node imageNode;
	if(!(imageNode = tilesetNode.child("image")) || !imageNode.attribute("source"))
	{
		DEBUG_MSG("Missing image data in tmx file. Map not loaded.");
		m_Unload();
		return false;
	}

	//process image from disk
	std::string imagePath;
	imagePath = m_mapDirectory + imageNode.attribute("source").as_string();

	std::shared_ptr<sf::Image> sourceImage = m_LoadImage(imagePath);

	//add transparency mask from colour if it exists
	if(imageNode.attribute("trans"))
		sourceImage->createMaskFromColor(m_ColourFromHex(imageNode.attribute("trans").as_string()));

	//store image as a texture for drawing with vertex array
	sf::Texture tileset;
	tileset.loadFromImage(*sourceImage);
	m_tilesetTextures.push_back(tileset);

	//parse offset node if it exists - TODO store somewhere tileset info can be referenced
	sf::Vector2u offset;
	if(pugi::xml_node offsetNode = tilesetNode.child("tileoffset"))
	{
		offset.x = (offsetNode.attribute("x")) ? offsetNode.attribute("x").as_uint() : 0u;
		offset.y = (offsetNode.attribute("y")) ? offsetNode.attribute("y").as_uint() : 0u;
	}
	//TODO parse any tile properties and store with offset above

	//slice into tiles
    int columns = (sourceImage->getSize().x-margin) / (tileWidth+spacing);
    int rows = (sourceImage->getSize().y-margin) / (tileHeight+spacing);

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < columns; x++)
		{
			sf::IntRect rect; //must account for any spacing or margin on the tileset
			rect.top = y * (tileHeight + spacing);
			rect.top += margin;
			rect.height = tileHeight;
			rect.left = x * (tileWidth + spacing);
			rect.left += spacing;
			rect.width = tileWidth;

			sf::Image temp;
			temp.create(tileWidth, tileHeight, sf::Color::Transparent);
			temp.copy(*sourceImage, 0u, 0u, rect, true);
			sf::Texture texture;
			texture.loadFromImage(temp);
			m_tileTextures.push_back(texture);

			//store texture coords and tileset index for vertex array
			m_tileInfo.push_back(TileInfo(rect,
				sf::Vector2f(static_cast<float>(rect.width), static_cast<float>(rect.height)),
				m_tilesetTextures.size() - 1u));
		}
	}

	DEBUG_MSG("Processed " << imagePath);
	return true;
}

const bool MapLoader::m_ParseLayer(const pugi::xml_node& layerNode)
{
	DEBUG_MSG("Found standard map layer " << layerNode.attribute("name").as_string());

	MapLayer layer(Layer);
	if(layerNode.attribute("name")) layer.name = layerNode.attribute("name").as_string();
	if(layerNode.attribute("opacity")) layer.opacity = layerNode.attribute("opacity").as_float();
	if(layerNode.attribute("visible")) layer.visible = layerNode.attribute("visible").as_bool();

	//make sure there are enough vertex arrays for tile sets
	for(auto arr = m_tilesetTextures.begin(); arr != m_tilesetTextures.end(); ++arr)
		layer.vertexArrays.push_back(sf::VertexArray(sf::Quads));

	pugi::xml_node dataNode;
	if(!(dataNode = layerNode.child("data")))
	{
		DEBUG_MSG("Layer data missing or corrupt. Map not loaded.");
		return false;
	}
	//decode and decompress data first if necessary. See https://github.com/bjorn/tiled/wiki/TMX-Map-Format#data
	//for explanation of bytestream retrieved when using compression
	if(dataNode.attribute("encoding"))
	{
		std::string encoding = dataNode.attribute("encoding").as_string();
		std::string data = dataNode.text().as_string();

		if(encoding == "base64")
		{
			DEBUG_MSG("Found Base64 encoded layer data, decoding...");
			//remove any newlines or white space created by tab spaces in document
			std::stringstream ss;
			ss << data;
			ss >> data;
			data = base64_decode(data);

			//calc the expected size of the uncompressed string
			int expectedSize = m_width * m_height * 4; //number of tiles * 4 bytes = 32bits / tile
			std::vector<unsigned char>byteArray; //to hold decompressed data as bytes
			byteArray.reserve(expectedSize);

			//check for compression (only used with base64 encoded data)
			if(dataNode.attribute("compression"))
			{
#ifndef DISABLE_LOGGING
				std::string compression	= dataNode.attribute("compression").as_string();
				DEBUG_MSG("Found " << compression << " compressed layer data, decompressing...");
#endif
				//decompress with zlib
				int dataSize = data.length() * sizeof(unsigned char);
				if(!m_Decompress(data.c_str(), byteArray, dataSize, expectedSize))
				{
					DEBUG_MSG("Failed to decompress map data. Map not loaded.");
					return false;
				}
			}
			else //uncompressed
			{
				for(auto i = data.cbegin(); i != data.cend(); ++i)
					byteArray.push_back(*i);
			}

			//extract tile GIDs using bitshift (See https://github.com/bjorn/tiled/wiki/TMX-Map-Format#data) and add the tiles to layer
			sf::Uint16 x, y;
			x = y = 0;
			for(int i = 0; i < expectedSize - 3; i +=4)
			{
				int tileGID = byteArray[i] | byteArray[i + 1] << 8 | byteArray[i + 2] << 16 | byteArray[i + 3] << 24;
				m_AddTileToLayer(layer, x, y, tileGID);

				x++;
				if(x == m_width)
				{
					x = 0;
					y++;
				}
			}
		}
		else if(encoding == "csv")
		{
			DEBUG_MSG("CSV encoded layer data found.");

			std::vector<int> tileGIDs;
			std::stringstream datastream(data);

			//parse csv string into vector of IDs
			int i;
			while (datastream >> i)
			{
				tileGIDs.push_back(i);
				if(datastream.peek() == ',')
					datastream.ignore();
			}

			//create tiles from IDs
			sf::Uint16 x, y;
			x = y = 0;
			for(unsigned int i = 0; i < tileGIDs.size(); i++)
			{
				m_AddTileToLayer(layer, x, y, tileGIDs[i]);
				x++;
				if(x == m_width)
				{
					x = 0;
					y++;
				}
			}
		}
		else
		{
			DEBUG_MSG("Unsupported encoding of layer data found. Map not Loaded.");
			return false;
		}
	}
	else //unencoded
	{
		DEBUG_MSG("Found unencoded data.");
		pugi::xml_node tileNode;
		if(!(tileNode = dataNode.child("tile")))
		{
			DEBUG_MSG("No tile data found. Map not loaded.");
			return false;
		}

		sf::Uint16 x, y;
		x = y = 0;
		while(tileNode)
		{
			sf::Uint16 gid = tileNode.attribute("gid").as_int();
			m_AddTileToLayer(layer, x, y, gid);
			tileNode = tileNode.next_sibling("tile");
			x++;
			if(x == m_width)
			{
				x = 0;
				y++;
			}
		}
	}

	//parse any layer properties
	if(pugi::xml_node propertiesNode = layerNode.child("properties"))
		m_ParseLayerProperties(propertiesNode, layer);

	//convert layer tile coords to isometric if needed
	if(m_orientation == Isometric) m_SetIsometricCoords(layer);

	m_layers.push_back(layer);
	return true;
}

void MapLoader::m_AddTileToLayer(MapLayer& layer, sf::Uint16 x, sf::Uint16 y, sf::Uint16 gid)
{
	sf::Uint8 opacity = static_cast<sf::Uint8>(255.f * layer.opacity);
	sf::Color colour = sf::Color(255u, 255u, 255u, opacity);
	MapTile tile;
	tile.gridCoord = sf::Vector2i(x, y);
	tile.sprite.setTexture(m_tileTextures[gid]);
	tile.sprite.setColor(colour);
    tile.gid = gid;
	//update origin of isometric tiles
	if(m_orientation == Isometric) tile.sprite.setOrigin(static_cast<float>(m_tileWidth / 2), static_cast<float>(m_tileHeight / 2));
	tile.sprite.setPosition(static_cast<float>(m_tileWidth * x), static_cast<float>(m_tileHeight * y));

	//tiles with a size other than the map grid need to be offset
	if(m_tileTextures[gid].getSize().y != m_tileHeight)
	{
		tile.sprite.move(0.f, static_cast<float>(m_tileHeight - m_tileTextures[gid].getSize().y));
	}

	layer.tiles.push_back(tile);

	//update the layer's vertex array(s)
	sf::Vertex v0, v1, v2, v3;

	//applying half pixel trick avoids artifacting when scrolling
	v0.texCoords = m_tileInfo[gid].Coords[0] + sf::Vector2f(0.5f, 0.5f);
	v1.texCoords = m_tileInfo[gid].Coords[1] + sf::Vector2f(-0.5f, 0.5f);
	v2.texCoords = m_tileInfo[gid].Coords[2] + sf::Vector2f(-0.5f, -0.5f);
	v3.texCoords = m_tileInfo[gid].Coords[3] + sf::Vector2f(0.5f, -0.5f);

	v0.position = sf::Vector2f(static_cast<float>(m_tileWidth * x), static_cast<float>(m_tileHeight * y));
	v1.position = sf::Vector2f(static_cast<float>(m_tileWidth * x) + m_tileInfo[gid].Size.x, static_cast<float>(m_tileHeight * y));
	v2.position = sf::Vector2f(static_cast<float>(m_tileWidth * x) + m_tileInfo[gid].Size.x, static_cast<float>(m_tileHeight * y) + m_tileInfo[gid].Size.y);
	v3.position = sf::Vector2f(static_cast<float>(m_tileWidth * x), static_cast<float>(m_tileHeight * y) + m_tileInfo[gid].Size.y);

	//offset tiles with size not equal to map grid size
	sf::Uint16 tileHeight = static_cast<sf::Uint16>(m_tileInfo[gid].Size.y);
	if(tileHeight != m_tileHeight)
	{
		float diff = static_cast<float>(m_tileHeight - tileHeight);
		v0.position.y += diff;
		v1.position.y += diff;
		v2.position.y += diff;
		v3.position.y += diff;
	}

	//adjust position for isometric maps
	if(m_orientation == Isometric)
	{
		sf::Vector2f offset(-static_cast<float>(x * (m_tileWidth / 2u)), static_cast<float>(x * (m_tileHeight / 2u)));
		offset.x -= static_cast<float>(y * (m_tileWidth / 2u));
		offset.y -= static_cast<float>(y * (m_tileHeight / 2u));
		offset.x -= static_cast<float>(m_tileWidth / 2u);
		offset.y += static_cast<float>(m_tileHeight / 2u);

		v0.position += offset;
		v1.position += offset;
		v2.position += offset;
		v3.position += offset;
	}

	v0.color = colour;
	v1.color = colour;
	v2.color = colour;
	v3.color = colour;

	layer.vertexArrays[m_tileInfo[gid].TileSetId].append(v0);
	layer.vertexArrays[m_tileInfo[gid].TileSetId].append(v1);
	layer.vertexArrays[m_tileInfo[gid].TileSetId].append(v2);
	layer.vertexArrays[m_tileInfo[gid].TileSetId].append(v3);

}

const bool MapLoader::m_ParseObjectgroup(const pugi::xml_node& groupNode)
{
	DEBUG_MSG("Found object layer " << groupNode.attribute("name").as_string());

	pugi::xml_node objectNode;
	if(!(objectNode = groupNode.child("object")))
	{
		DEBUG_MSG("Object group contains no objects");
		return true;
	}

	//add layer to map layers
	MapLayer layer(ObjectGroup);
	layer.name = groupNode.attribute("name").as_string();
	if(groupNode.attribute("opacity")) layer.opacity = groupNode.attribute("opacity").as_float();
	if(pugi::xml_node propertiesNode = groupNode.child("properties"))
		m_ParseLayerProperties(propertiesNode, layer);
	//NOTE we push the layer onto the vector at the end of the function in case we add any objects
	//with tile data to the layer's tiles property

	//parse all object nodes into MapObjects
	while(objectNode)
	{
		if(!objectNode.attribute("x") || !objectNode.attribute("y"))
		{
			DEBUG_MSG("Object missing position data. Map not loaded.");
			m_Unload();
			return false;
		}
		MapObject object;

		//set position
		sf::Vector2f position(objectNode.attribute("x").as_float(),
											objectNode.attribute("y").as_float());
		position = IsometricToOrthogonal(position);
		object.SetPosition(position);

		//set size if specified
		if(objectNode.attribute("width") && objectNode.attribute("height"))
		{
			sf::Vector2f size(objectNode.attribute("width").as_float(),
							objectNode.attribute("height").as_float());
			if(objectNode.child("ellipse"))
			{
				//add points to make ellipse
				const float x = size.x / 2.f;
				const float y = size.y / 2.f;
				const float tau = 6.283185f;
				const float step = tau / 16.f; //number of points to make up ellipse
				for(float angle = 0.f; angle < tau; angle += step)
				{
					sf::Vector2f point(x + x * cos(angle), y + y * sin(angle));
					object.AddPoint(IsometricToOrthogonal(point));
				}

				object.SetShapeType(Ellipse);
			}
			else //add points for rectangle to use in intersection testing
			{
				object.AddPoint(IsometricToOrthogonal(sf::Vector2f()));
				object.AddPoint(IsometricToOrthogonal(sf::Vector2f(size.x, 0.f)));
				object.AddPoint(IsometricToOrthogonal(sf::Vector2f(size.x, size.y)));
				object.AddPoint(IsometricToOrthogonal(sf::Vector2f(0.f, size.y)));
			}
			object.SetSize(size);
		}
		//else parse poly points
		else if(objectNode.child("polygon") || objectNode.child("polyline"))
		{
			if(objectNode.child("polygon"))
				object.SetShapeType(Polygon);
			else object.SetShapeType(Polyline);

			//split coords into pairs
			if(objectNode.first_child().attribute("points"))
			{
				DEBUG_MSG("Processing poly shape points...");
				std::string pointlist = objectNode.first_child().attribute("points").as_string();
				std::stringstream stream(pointlist);
				std::vector<std::string> points;
				std::string pointstring;
				while(std::getline(stream, pointstring, ' '))
					points.push_back(pointstring);

				//parse each pair into sf::vector2i
				for(unsigned int i = 0; i < points.size(); i++)
				{
					std::vector<float> coords;
					std::stringstream coordstream(points[i]);

					float j;
					while (coordstream >> j)
					{
						coords.push_back(j);
						if(coordstream.peek() == ',')
							coordstream.ignore();
					}
					object.AddPoint(IsometricToOrthogonal(sf::Vector2f(coords[0], coords[1])));
				}
			}
			else
			{
				DEBUG_MSG("Points for polygon or polyline object are missing");
			}
		}
		else if(!objectNode.attribute("gid")) //invalid  attributes
		{
			DEBUG_MSG("Objects with no parameters found, skipping..");
			objectNode = objectNode.next_sibling("object");
			continue;
		}

		//parse object node property values
		if(pugi::xml_node propertiesNode = objectNode.child("properties"))
		{
			pugi::xml_node propertyNode = propertiesNode.child("property");
			while(propertyNode)
			{
				std::string name = propertyNode.attribute("name").as_string();
				std::string value = propertyNode.attribute("value").as_string();
				object.SetProperty(name, value);

				DEBUG_MSG("Set object property " << name << " with value " << value);
				propertyNode = propertyNode.next_sibling("property");
			}
		}

		//set object properties
		if(objectNode.attribute("name")) object.SetName(objectNode.attribute("name").as_string());
		if(objectNode.attribute("type")) object.SetType(objectNode.attribute("type").as_string());
		if(objectNode.attribute("rotation")) object.SetRotation(objectNode.attribute("rotation").as_float());
		if(objectNode.attribute("visible")) object.SetVisible(objectNode.attribute("visible").as_bool());
		if(objectNode.attribute("gid"))
		{
			int gid = objectNode.attribute("gid").as_int();
			DEBUG_MSG("Found object with tile GID " << gid);
			MapTile tile;
			tile.sprite.setTexture(m_tileTextures[gid]);
			tile.sprite.setRotation(object.GetRotation());
			tile.sprite.setColor(sf::Color(255u, 255u, 255u, static_cast<sf::Uint8>(255.f * layer.opacity)));
			tile.sprite.setPosition(object.GetPosition());
			tile.gridCoord = sf::Vector2i(static_cast<int>(object.GetPosition().x / m_tileWidth), static_cast<int>(object.GetPosition().y / m_tileHeight));
			layer.tiles.push_back(tile);
			object.SetShapeType(Tile);

			//create bounding poly
			float width = static_cast<float>(m_tileTextures[gid].getSize().x);
			float height = static_cast<float>(m_tileTextures[gid].getSize().y);
			object.AddPoint(sf::Vector2f());
			object.AddPoint(sf::Vector2f(width, 0.f));
			object.AddPoint(sf::Vector2f(width, height));
			object.AddPoint(sf::Vector2f(0.f, height));
			object.SetSize(sf::Vector2f(width, height));
		}
		object.SetParent(layer.name);

		//call objects create debug shape function with colour / opacity
		sf::Color debugColour;
		if(groupNode.attribute("color"))
		{
			std::string colour = groupNode.attribute("color").as_string();
			//crop leading hash and pop the last (duplicated) char
			std::remove(colour.begin(), colour.end(), '#');
			colour.pop_back();
			debugColour = m_ColourFromHex(colour.c_str());
		}
		else debugColour = sf::Color(127u, 127u, 127u);
		debugColour.a = static_cast<sf::Uint8>(255.f * layer.opacity);
		object.CreateDebugShape(debugColour);

		//add objects to vector
		layer.objects.push_back(object);
		objectNode = objectNode.next_sibling("object");
	}

	m_layers.push_back(layer); //do this last
	DEBUG_MSG("Processed " << layer.objects.size() << " objects");
	return true;
}

const bool MapLoader::m_ParseImageLayer(const pugi::xml_node& imageLayerNode)
{
	DEBUG_MSG("Found image layer " << imageLayerNode.attribute("name").as_string());

	pugi::xml_node imageNode;
	//load image
	if(!(imageNode = imageLayerNode.child("image")) || !imageNode.attribute("source"))
	{
		DEBUG_MSG("Image layer " << imageLayerNode.attribute("name").as_string() << " missing image source property. Map not loaded.");
		return false;
	}

	std::string imageName = m_mapDirectory + imageNode.attribute("source").as_string();
	std::shared_ptr<sf::Image> image = m_LoadImage(imageName);

	//set transparency if required
	if(imageNode.attribute("trans"))
	{
		image->createMaskFromColor(m_ColourFromHex(imageNode.attribute("trans").as_string()));
	}

	//load image to texture
	sf::Texture texture;
	texture.loadFromImage(*image);
	m_imageLayerTextures.push_back(texture);

	//add texture to layer as sprite, set layer properties
	MapTile tile;
	tile.sprite.setTexture(m_imageLayerTextures.back());
	MapLayer layer(ImageLayer);
	layer.name = imageLayerNode.attribute("name").as_string();
	if(imageLayerNode.attribute("opacity"))
	{
		layer.opacity = imageLayerNode.attribute("opacity").as_float();
		sf::Uint8 opacity = static_cast<sf::Uint8>(255.f * layer.opacity);
		tile.sprite.setColor(sf::Color(255u, 255u, 255u, opacity));
	}
	layer.tiles.push_back(tile);

	//parse layer properties
	if(pugi::xml_node propertiesNode = imageLayerNode.child("properties"))
		m_ParseLayerProperties(propertiesNode, layer);

	//push back layer
	m_layers.push_back(layer);

	return true;
}

void MapLoader::m_ParseLayerProperties(const pugi::xml_node& propertiesNode, MapLayer& layer)
{
	pugi::xml_node propertyNode = propertiesNode.child("property");
	while(propertyNode)
	{
		std::string name = propertyNode.attribute("name").as_string();
		std::string value = propertyNode.attribute("value").as_string();
		layer.properties[name] = value;
		propertyNode = propertyNode.next_sibling("property");
		DEBUG_MSG("Added layer property " << name << " with value " << value);
	}
}

void MapLoader::m_CreateDebugGrid(void)
{
	sf::Color debugColour(0u, 0u, 0u, 120u);
	float mapHeight = static_cast<float>(m_tileHeight * m_height);
	for(int x = 0; x <= m_width; x += 2)
	{
		float posX = static_cast<float>(x * (m_tileWidth / m_tileRatio));
		m_gridVertices.append(sf::Vertex(IsometricToOrthogonal(sf::Vector2f(posX, 0.f)), debugColour));
		m_gridVertices.append(sf::Vertex(IsometricToOrthogonal(sf::Vector2f(posX, mapHeight)), debugColour));
		posX += static_cast<float>(m_tileWidth) / m_tileRatio;
		m_gridVertices.append(sf::Vertex(IsometricToOrthogonal(sf::Vector2f(posX, mapHeight)), debugColour));
		m_gridVertices.append(sf::Vertex(IsometricToOrthogonal(sf::Vector2f(posX, 0.f)), debugColour));
		posX += static_cast<float>(m_tileWidth) / m_tileRatio;
		m_gridVertices.append(sf::Vertex(IsometricToOrthogonal(sf::Vector2f(posX, 0.f)), debugColour));
	}
	float mapWidth = static_cast<float>(m_tileWidth * (m_width / m_tileRatio));
	for(int y = 0; y <= m_height; y += 2)
	{
		float posY = static_cast<float>(y *m_tileHeight);
		m_gridVertices.append(sf::Vertex(IsometricToOrthogonal(sf::Vector2f(0.f, posY)), debugColour));
		posY += static_cast<float>(m_tileHeight);
		m_gridVertices.append(sf::Vertex(IsometricToOrthogonal(sf::Vector2f(0.f, posY)), debugColour));
		m_gridVertices.append(sf::Vertex(IsometricToOrthogonal(sf::Vector2f(mapWidth, posY)), debugColour));
		posY += static_cast<float>(m_tileHeight);
		m_gridVertices.append(sf::Vertex(IsometricToOrthogonal(sf::Vector2f(mapWidth, posY)), debugColour));
	}

	m_gridVertices.setPrimitiveType(sf::LinesStrip);

}

void MapLoader::m_SetIsometricCoords(MapLayer& layer)
{
	//float offset  = static_cast<float>(m_width * m_tileWidth) / 2.f;
	for(auto tile = layer.tiles.begin(); tile != layer.tiles.end(); ++tile)
	{
		sf::Int16 posX = (tile->gridCoord.x - tile->gridCoord.y) * (m_tileWidth / 2);
		sf::Int16 posY = (tile->gridCoord.y + tile->gridCoord.x) * (m_tileHeight / 2);
		tile->sprite.setPosition(static_cast<float>(posX)/* + offset*/, static_cast<float>(posY));
	}
}

void MapLoader::m_DrawLayer(sf::RenderTarget& rt, const MapLayer& layer)
{
	if(!layer.visible) return; //skip invisible layers
	m_SetDrawingBounds(rt.getView());
	for(unsigned i = 0; i < layer.vertexArrays.size(); i++)
    {
        // MC 11/2013: added a states variable to allow setting the transform to be applied
        // to the map (warning: will only work for tile layers)
        sf::RenderStates states;
        states.transform = transf;
        states.texture = &m_tilesetTextures[i];
        rt.draw(layer.vertexArrays[i], states); //&m_tilesetTextures[i]);
	}
	if(layer.type == ObjectGroup || layer.type == ImageLayer)
	{
		//draw tiles used on objects
		for(auto tile = layer.tiles.begin(); tile != layer.tiles.end(); ++tile)
		{
			//draw tile if in bounds and is not transparent
			if((m_bounds.contains(tile->sprite.getPosition()) && tile->sprite.getColor().a)
				|| layer.type == ImageLayer) //always draw image layer
			{
				rt.draw(tile->sprite, tile->renderStates);
			}
		}
	}
}

//decoding and utility functions
const sf::Color MapLoader::m_ColourFromHex(const char* hexStr) const
{
	//TODO proper checking valid string length
	unsigned int value, r, g, b;
	std::stringstream input(hexStr);
	input >> std::hex >> value;

	r = (value >> 16) & 0xff;
	g = (value >> 8) & 0xff;
	b = value & 0xff;

	return sf::Color(r, g, b);
}

const bool MapLoader::m_Decompress(const char* source, std::vector<unsigned char>& dest, int inSize, int expectedSize)
{
	if(!source)
	{
		DEBUG_MSG("Input string is empty, decompression failed.");
		return false;
	}

	int currentSize = expectedSize;
	unsigned char* byteArray = new unsigned char[expectedSize / sizeof(unsigned char)];
	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.next_in = (Bytef*)source;
	stream.avail_in = inSize;
	stream.next_out = (Bytef*)byteArray;
	stream.avail_out = expectedSize;

	if(inflateInit2(&stream, 15 + 32) != Z_OK)
	{
		DEBUG_MSG("inflate 2 failed");
		return false;//retVal;
	}

	int result;
	do
	{
		result = inflate(&stream, Z_SYNC_FLUSH);

		switch(result)
		{
		case Z_NEED_DICT:
		case Z_STREAM_ERROR:
			result = Z_DATA_ERROR;
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			inflateEnd(&stream);
			DEBUG_MSG(result);
			return false;
		}

		if(result != Z_STREAM_END)
		{
			int oldSize = currentSize;
			currentSize *= 2;
			unsigned char* newArray = new unsigned char[currentSize / sizeof(unsigned char)];
			std::memcpy(newArray, byteArray, currentSize / 2);
			delete[] byteArray;
			byteArray = newArray;

			stream.next_out = (Bytef*)(byteArray + oldSize);
			stream.avail_out = oldSize;

		}
	}
	while(result != Z_STREAM_END);

	if(stream.avail_in != 0)
	{
		DEBUG_MSG("stream.avail_in is 0");
		DEBUG_MSG("zlib decompression failed.");
		return false;
	}

	const int outSize = currentSize - stream.avail_out;
	inflateEnd(&stream);

	unsigned char* newArray = new unsigned char[outSize / sizeof(unsigned char)];
	std::memcpy(newArray, byteArray, outSize);
	delete[] byteArray;
	byteArray = newArray;

	//copy bytes to vector
	int length = currentSize / sizeof(unsigned char);
	for(int i = 0; i < length; i++)
		dest.push_back(byteArray[i]);
	delete[] byteArray;
	return true;
}

std::shared_ptr<sf::Image> MapLoader::m_LoadImage(std::string path)
{
	//first check if image is already loaded
	for(auto image : m_cachedImages)
	{
		if(image.first == path)
			return image.second;
	}
	//else attempt to load
	std::shared_ptr<sf::Image> newImage = std::shared_ptr<sf::Image>(new sf::Image());
	if(!newImage->loadFromFile(path))
	{
		newImage->create(20u, 20u, sf::Color::Yellow);
	}
	m_cachedImages[path] = newImage;
	return newImage;
}

//base64 decode function taken from:
/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 Ren� Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   Ren� Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

namespace tmx
{
static const std::string base64_chars =
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789+/";


static inline bool is_base64(unsigned char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string base64_decode(std::string const& encoded_string)
{
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
	{
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4)
		{
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
			ret += char_array_3[j];
	}

	return ret;
}
};
