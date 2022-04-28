#pragma once

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite.lib")

#include <d2d1.h>
#include <dwrite.h>
#include <dxgi.h>
#include <d2d1_3.h>

struct UI_Rect_Element
{
	UI_Rect_Element()
	{
		UI_LayoutRect = D2D1::RectF(0.0f, 0.0f, 0.0f, 0.0f);
	}

	UI_Rect_Element(float x1, float y1, float x2, float y2)
	{
		UI_LayoutRect = D2D1::RectF(x1, y1, x2, y2);
	}

	void SetRectSize(float x1, float y1, float x2, float y2)
	{
		UI_LayoutRect = D2D1::RectF(x1, y1, x2, y2);
	}

	bool CheckIfPointIsInRect(float x, float y)
	{
		if (x >= UI_LayoutRect.left)
		{
			if (y >= UI_LayoutRect.top)
			{
				if (x <= UI_LayoutRect.right)
				{
					if (y <= UI_LayoutRect.bottom)
					{

					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	D2D1_RECT_F UI_LayoutRect;
};

struct UI_Line_Element
{
	UI_Line_Element()
	{
		point0.x = 0.0f;
		point0.y = 0.0f;

		point1.x = 0.0f;
		point1.y = 0.0f;

		ColorR = 0.0f;
		ColorG = 0.0f;
		ColorB = 0.0f;
	}

	UI_Line_Element(float x1, float y1, float x2, float y2, float r, float g, float b)
	{
		point0.x = x1;
		point0.y = y1;

		point1.x = x2;
		point1.y = y2;

		ColorR = r;
		ColorG = g;
		ColorB = b;
	}

	void SetRectSize(float x1, float y1, float x2, float y2)
	{
		point0.x = x1;
		point0.y = y1;

		point1.x = x2;
		point1.y = y2;
	}

	void SetColor(float r, float g, float b)
	{
		ColorR = r;
		ColorG = g;
		ColorB = b;
	}

	float ColorR, ColorG, ColorB;
	D2D1_POINT_2F point0;
	D2D1_POINT_2F point1;
};

struct UI_Text
{
	UI_Text()
	{
		Text = "";
		FontWeight = DWRITE_FONT_WEIGHT_NORMAL;
		FontStyle = DWRITE_FONT_STYLE_NORMAL;
		FontStretch = DWRITE_FONT_STRETCH_NORMAL;
		FontSize = 50.0f;
		ColorR = 1.0f;
		ColorG = 1.0f;
		ColorB = 1.0f;
	}
	UI_Text(std::string text, std::string fontName, float fontSize, float colorR, float colorG, float colorB, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch, float textBoxStartX, float textBoxStartY, float width, float height)
	{
		Rect.SetRectSize(textBoxStartX, textBoxStartY, textBoxStartX + width, textBoxStartY + height);
		Text = text;
		FontName = fontName;
		FontWeight = fontWeight;
		FontStyle = fontStyle;
		FontStretch = fontStretch;
		FontSize = fontSize;
		ColorR = colorR;
		ColorG = colorG;
		ColorB = colorB;
	}

	void SetTextBoxPosAndSize(float StartX, float StartY, float width, float height)
	{
		Rect.SetRectSize(StartX, StartY, StartX + width, StartY + height);
	}

	void SetColor(float r, float g, float b)
	{
		ColorR = r;
		ColorG = g;
		ColorB = b;
	}

	void SetFontSettings(std::string fontName, float fontSize, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch)
	{
		FontName = fontName;
		FontWeight = fontWeight;
		FontStyle = fontStyle;
		FontStretch = fontStretch;
		FontSize = fontSize;
	}

	void SetText(std::string text)
	{
		Text = text;
	}

	UI_Rect_Element Rect;
	std::string Text;
	std::string FontName;
	float FontSize;
	float ColorR, ColorG, ColorB;
	DWRITE_FONT_WEIGHT FontWeight;
	DWRITE_FONT_STYLE FontStyle;
	DWRITE_FONT_STRETCH FontStretch;
};