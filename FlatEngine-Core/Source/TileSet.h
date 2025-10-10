#pragma once
#include "Vector2.h"
#include "Texture.h"

#include <string>
#include <vector>
#include <memory>
#include <map>


namespace FlatEngine
{
	class TileSet
	{
	public:
		TileSet();
		~TileSet();
		std::string GetData();

		void SetName(std::string name);
		std::string GetName();
		std::pair<Vector2, Vector2> GetIndexUVs(int index);
		void SetTexturePath(std::string texturePath);
		std::string GetTexturePath();
		void SetTileSetPath(std::string tileSetPath);
		std::string GetTileSetPath();
		std::shared_ptr<Texture> GetTexture();
		std::map<int, std::pair<Vector2, Vector2>> GetTileSet();
		void SetTileSetIndices(std::vector<int> indices);
		std::vector<int> GetTileSetIndices();
		void SetTileWidth(int width);
		void SetTileHeight(int height);
		int GetTileWidth();
		int GetTileHeight();		
		void InitializeUVs();
		void ToggleTile(int index);

	private:
		std::string m_name;
		std::string m_texturePath;
		std::string m_tileSetPath;
		std::shared_ptr<Texture> m_texture;
		std::map<int, std::pair<Vector2, Vector2>> m_allTileUVs;
		std::vector<int> m_tileSetIndices;
		int m_tileWidth;
		int m_tileHeight;
	};
}