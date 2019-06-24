// ----------------------------------------------------------------------- //
//
// MODULE  : Font.cpp
//
// PURPOSE : Шрифты / тексты
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#include "FontText.h"
#include "Font.h"

namespace SAVFGAME
{
	uint32 CBaseFont::unique_font_id_counter = 0xFA000;		// счётчик загрузок, первый шрифт получит ID == 0xFA000

	//>> Получение текущего размера полигона текста
	void CText::UpdatePolygonWH(CBaseFont * font)
	{
		if (ptext != nullptr)
			 font->GetGeneratedWH(*ptext, height, start_symbol_pos, end_symbol_pos, W, H);
		else font->GetGeneratedWH(  text, height, start_symbol_pos, end_symbol_pos, W, H);
	}

	//>> Ограничение выводимого текста под размер полигона
	void CText::SetStartEndSymbolsPos(float max_polygon_width, float max_polygon_height, bool from_end, bool space_divider, CBaseFont * font)
	{
		float W, H;
		uint32 wsize, cur, space_pos = -1;
		wstring & wstr = text;

		if (ptext != nullptr)
			wstr = *ptext;

		wsize = (uint32)wstr.size();
		if (from_end)
		{
			cur = wsize;
			while (cur > 0)
			{
				cur--;
				if (wstr[cur] == ' ') space_pos = cur;
				font->GetGeneratedWH(wstr, height, cur, MISSING, W, H);
				if (W > max_polygon_width ||
					H > max_polygon_height)
					{ cur++; break; }
			}

			if (space_divider && space_pos != -1 && wsize > 1)
				 start_symbol_pos = space_pos + 1;
			else start_symbol_pos = cur;

			end_symbol_pos = wsize;
		}
		else
		{
			cur = 0;
			while (cur <= wsize)
			{
				if (wstr[cur] == ' ') space_pos = cur;
				cur++;
				font->GetGeneratedWH(wstr, height, 0, cur, W, H);
				if (W > max_polygon_width ||
					H > max_polygon_height)
					{ cur--; break; }
			}
			start_symbol_pos = 0;
			
			if (space_divider && space_pos != -1 && wsize > 1)
				 end_symbol_pos = space_pos + 1;
			else end_symbol_pos = cur;
		}
	}
}