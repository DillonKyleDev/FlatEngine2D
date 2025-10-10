#pragma once
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>


namespace FlatEngine
{
	enum Pivot {
		PivotCenter,
		PivotLeft,
		PivotRight,
		PivotTop,
		PivotBottom,
		PivotTopLeft,
		PivotTopRight,
		PivotBottomRight,
		PivotBottomLeft
	};
	const std::string F_PivotStrings[9] = {
		"PivotCenter",
		"PivotLeft",
		"PivotRight",
		"PivotTop",
		"PivotBottom",
		"PivotTopLeft",
		"PivotTopRight",
		"PivotBottomRight",
		"PivotBottomLeft"
	};

	class Texture
	{
	public:
		Texture(std::string path = "");
		~Texture();
		bool LoadFromFile(std::string path);
		bool LoadSurface(std::string path, SDL_Surface* screenSurface);
		bool LoadFromRenderedText(std::string textureText, SDL_Color textColor, TTF_Font* font);
		void FreeTexture();
		SDL_Texture* GetTexture();
		void FreeSurface();
		int GetWidth();
		int GetHeight();
		void SetDimensions(int width, int height);

	private:
		SDL_Texture* m_texture;
		SDL_Surface* m_surface;
		std::string m_path;
		int m_textureWidth;
		int m_textureHeight;
	};
}