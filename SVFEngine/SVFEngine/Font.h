// ----------------------------------------------------------------------- //
//
// MODULE  : Font.h
//
// PURPOSE : Загрузка и подготовка растровых шрифтов
//           Для генерация текстуры и таблицы символов использую BMFont
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _FONT_H
#define _FONT_H

#include "stdafx.h"
#include "FontText.h"
#include "Shader.h"

#define  BMFh_SZ  3 + 1			// header size == magic (3) + version (1)
#define  BMFi_SZ  1 + 4			// every preblock info size == id (1) + size (4)
#define  BMF1_SZ  14			// block_01 pre-string size (14)
#define  BMF4_SZ  sizeof(BMF4)	// each block_04 size (20)
#define  BMF5_SZ  sizeof(BMF5)	// each block_05 size (10)

#define UIVERTEX FONTVERTEX

namespace SAVFGAME
{
	struct FONTVERTEX
	{
		MATH3DVEC3 P;	// x, y, z     Координаты 3D
		MATH3DVEC4 C;	// r, g, b, a  Цвет вершины
		MATH3DVEC2 T;	// u, v        Координаты текстуры
	};


	class CBaseFont
	{
	#pragma pack(push,1)		// http://www.angelcode.com/products/bmfont/documentation.html
		struct BMF1 {
			BMF1()  { Close(); }
			~BMF1() { Close(); }
			void Close()
			{
				fontSize = 0;
				bitField = 0;
				charSet = 0;
				stretchH = 0;
				aa = 0;
				paddingUp = 0;
				paddingRight = 0;
				paddingDown = 0;
				paddingLeft = 0;
				spacingHoriz = 0;
				spacingVert = 0;
				outline = 0;
				fontName.erase(fontName.begin(), fontName.end());
			}
			void _printf()
			{
				printf(	"\n---BMFont block 01---"
						"\n- fontName %s"
						"\n- fontSize %i"
						"\n- bitField %x_h"
						"\n- charSet  %x_h"
						"\n- stretchH %i"
						"\n- aa       %i"
						"\n- paddingUp    %i"
						"\n- paddingRight %i"
						"\n- paddingDown  %i"
						"\n- paddingLeft  %i"
						"\n- spacingHoriz %i"
						"\n- spacingVert  %i"
						"\n- outline      %i",
							fontName.c_str(),
							fontSize,
							bitField,
							charSet,
							stretchH,
							aa,
							paddingUp,
							paddingRight,
							paddingDown,
							paddingLeft,
							spacingHoriz,
							spacingVert,
							outline
					);
			}
			uint16 fontSize;		// .
			uint8  bitField;		// bit 0: smooth, bit 1: unicode, bit 2: italic, bit 3: bold, bit 4: fixedHeigth, bits 5-7: reserved
			uint8  charSet;			// The name of the OEM charset used (when not unicode).
			uint16 stretchH;		// The font height stretch in percentage. 100% means no stretch.
			uint8  aa;				// The supersampling level used. 1 means no supersampling was used.
			uint8  paddingUp;		// The padding for each character (up, right, down, left).
			uint8  paddingRight;	// .
			uint8  paddingDown;		// .
			uint8  paddingLeft;		// .
			uint8  spacingHoriz;	// The spacing for each character (horizontal, vertical).
			uint8  spacingVert;		// .
			uint8  outline;			// The outline thickness for the characters.
			std::string fontName;	// char  *fontName;
		};
		struct BMF2 {
			BMF2()  { Close(); }
			~BMF2() { Close(); }
			void Close()
			{
				lineHeight = 0;
				base = 0;
				scaleW = 0;
				scaleH = 0;
				pages = 0;
				bitField = 0;
				alphaChnl = 0;
				redChnl = 0;
				greenChnl = 0;
				blueChnl = 0;
			}
			void _printf()
			{
				printf(	"\n---BMFont block 02---"
						"\n- lineHeight %i"
						"\n- base       %i"
						"\n- scaleW     %i"
						"\n- scaleH     %i"
						"\n- pages      %i"
						"\n- bitField   %x_h"
						"\n- alphaChnl  %x_h"
						"\n- redChnl    %x_h"
						"\n- greenChnl  %x_h"
						"\n- blueChnl   %x_h",
							lineHeight,
							base,
							scaleW,
							scaleH,
							pages,
							bitField,
							alphaChnl,
							redChnl,
							greenChnl,
							blueChnl
					);
			}
			uint16 lineHeight;	// This is the distance in pixels between each line of text.
			uint16 base;		// The number of pixels from the absolute top of the line to the base of the characters.
			uint16 scaleW;		// The width of the texture, normally used to scale the x pos of the character image.
			uint16 scaleH;		// The height of the texture, normally used to scale the y pos of the character image.
			uint16 pages;		// The number of texture pages included in the font.
			uint8  bitField;	// bits 0-6: reserved, bit 7: packed
								// Set to 1 if the monochrome characters have been packed into each of the texture channels.
								// In this case alphaChnl describes what is stored in each channel.
			uint8  alphaChnl;	// Set to 0 if the channel holds the glyph data, 1 if it holds the outline, 2 if it holds the glyph
								// and the outline, 3 if its set to zero, and 4 if its set to one.
			uint8  redChnl;		// . same
			uint8  greenChnl;	// . same
			uint8  blueChnl;	// . same
		};
		struct BMF3 {
			BMF3() {}
			~BMF3() { Close(); }
			void FreeRAM()
			{
				pageFile.Close(0,0);
			}
			void Close()
			{
				pageFile.Close(1,1);
				pageName.erase(pageName.begin(), pageName.end());
			}
			void _printf()
			{
				wprintf(L"\n---BMFont block 03---");
				for (auto & cur : pageName)
					wprintf(L"\n- %s", cur.c_str());
			}
			VECPDATA<TBUFFER<byte,int32>>	pageFile; // файлы страниц
			vector<wstring>					pageName; // имена страниц
		};
		struct BMF4 {			// repeated block! -> numChars = charsBlock.blockSize / 20
			BMF4()  { Close(); }
			~BMF4() { Close(); }
			void Close()
			{
				id = 0;
				x = 0;
				y = 0;
				width = 0;
				height = 0;
				xoffset = 0;
				yoffset = 0;
				xadvance = 0;
				page = 0;
				chnl = 0;
			}
			void _printf()
			{
				wprintf(L"\n---BMFont CHAR %04X (%c)---"
						L"\n- pos x    %i"
						L"\n- pos y    %i"
						L"\n- width    %i"
						L"\n- height   %i"
						L"\n- xoffset  %i"
						L"\n- yoffset  %i"
						L"\n- xadvance %i"
						L"\n- page     %i"
						L"\n- channel  %x_h",
						id, ((uint32)id > 0x20) ? (wchar_t)id : (wchar_t)0x20,
						x,
						y,
						width,
						height,
						xoffset,
						yoffset,
						xadvance,
						page,
						chnl
					);
			}
			 int32 id;			// .	
			uint16 x;			// .
			uint16 y;			// .
			uint16 width;		// .
			uint16 height;		// .
			 int16 xoffset;		// How much the current position should be offset when copying the image from the texture to the screen.
			 int16 yoffset;		// How much the current position should be offset when copying the image from the texture to the screen.
			 int16 xadvance;	// How much the current position should be advanced after drawing the character.
			uint8  page;		// .
			uint8  chnl;		// The texture channel where the character image is found (1 = blue, 2 = green, 4 = red, 8 alpha, 15 = all channels).
		}; // Размер блока 20
		struct BMF5 {			// These fields are repeated until all kerning pairs have been described
			BMF5()  { Close(); }
			~BMF5() { Close(); }
			void Close()
			{
				first = 0;
				second = 0;
				amount = 0;
			}
			void _printf()
			{
				wprintf(L"\n- kerning %04X and %04X (%c and %c) : amount %i",
					first,
					second,
					((uint32)first  > 0x20) ? (wchar_t)first  : (wchar_t)0x20,
					((uint32)second > 0x20) ? (wchar_t)second : (wchar_t)0x20,
					amount
					);
			}
			uint32 first;		// The first character id.
			uint32 second;		// The second character id.
			 int16 amount;		// How much the x position should be adjusted when drawing the second character immediately following the first.
		}; // Размер блока 10	// This block is only in the file if there are any kerning pairs with amount differing from 0.
		struct BMFont {
			BMFont()  { Close(); }
			~BMFont() { Close(); }
			void Close()
			{
				ZeroMemory(magic, 3);
				version   = 0;
				blockT1   = 0;
				blockT1sz = 0;
				blockT2   = 0;
				blockT2sz = 0;
				blockT3   = 0;
				blockT3sz = 0;
				blockT4   = 0;
				blockT4sz = 0;
				blockT5   = 0;
				blockT5sz = 0;
				block01.Close();
				block02.Close();
				block03.Close();
				block04.Close();
				block05.Close();
				maxCharH = 0;
			}
			char   magic[3];	// magic[3]="BMF";
			char   version;		// current ver.03
			uint8  blockT1;		// block 01: id = 1
			 int32 blockT1sz;	// block 01: size
			  BMF1 block01;		// block 01: info block
			uint8  blockT2;		// block 02: id = 2
			 int32 blockT2sz;	// block 02: size
			  BMF2 block02;		// block 02: common block
			uint8  blockT3;		// block 03: id = 3
			 int32 blockT3sz;	// block 03: size
			  BMF3 block03;		// block 03: pages block
			uint8  blockT4;		// block 04: id = 4
			 int32 blockT4sz;	// block 04: size
			 TBUFFER <BMF4, int32> block04; // block 04: chars block
			uint8  blockT5;		// block 05: id = 5
			 int32 blockT5sz;	// block 05: size
			 TBUFFER <BMF5, int32> block05; // block 05: kerning block
			//////////////////
			uint16 maxCharH;	// высота самого высокого символа
		};
	#pragma pack(pop)

	private:
		static uint32 unique_font_id_counter;	// счётчик загрузок ID :: значения MISSING (-1) и NULL (0) зарезервированы
	protected:
		CShader *				shader { nullptr };
		const CStatusIO *		IO     { nullptr };
		bool					isInit { false };
	protected:
		BMFont					bmfont;
		MATH3DMATRIX *			matWorld;		// screen UI
		MATH3DMATRIX *			matViewUI;		// screen UI
		MATH3DMATRIX *			matProjUI;		// screen UI
		MATH3DMATRIX *			matVPUI;		// screen UI
		uint32					window_W;		// дополнительная память размера окна
		uint32					window_H;		// дополнительная память размера окна
		uint32					ID;				// уникальный идентификатор загрузки		
	public:
		CBaseFont(const CBaseFont& src)				= delete;
		CBaseFont(CBaseFont&& src)					= delete;
		CBaseFont& operator=(const CBaseFont& src)	= delete;
		CBaseFont& operator=(CBaseFont&& src)		= delete;
	public:
		CBaseFont() : window_W(0), window_H(0), ID(0)
		{ 
			matWorld  = new MATH3DMATRIX;
			matViewUI = new MATH3DMATRIX;
			matProjUI = new MATH3DMATRIX;
			matVPUI   = new MATH3DMATRIX;
			MathLookAtLHMatrix(MATH3DVEC(0,0,0), MATH3DVEC(0,0,1), MATH3DVEC(0,1,0), *matViewUI);
		};
		~CBaseFont()
		{
			Close();
			_DELETE(matWorld);
			_DELETE(matViewUI);
			_DELETE(matProjUI);
			_DELETE(matVPUI);
		};
	protected:
		void DeleteTEX()
		{
			bmfont.block03.FreeRAM();
		};
	public:
		void Close()
		{
			if (isInit)
			{
				bmfont.Close();
				window_W = 0;
				window_H = 0;
				ID       = 0;
				shader = nullptr;
				IO     = nullptr;
				isInit = false;
			}
		}

		//>> Доступ к шейдерам
		void SetShader(const CShader * pShader)
		{
			if (pShader == nullptr) { _MBM(ERROR_PointerNone); return; }

			shader = const_cast<CShader*>(pShader);
		}
		//>> Доступ к состоянию устройств ввода-вывода
		void SetIO(const CStatusIO * io)
		{
			if (io == nullptr) { _MBM(ERROR_PointerNone); return; }

			IO = io;
		}
		//>> Обновление связанных с размером окна данных
		void Update()
		{
			window_W = IO->window.client_width;
			window_H = IO->window.client_height;

			MathOrthoOffCenterLHMatrix ( 0,
										 static_cast<float>(IO->window.client_width),
										 0,
										 static_cast<float>(IO->window.client_height),
										 0,
										 1,
										 *matProjUI );

			(*matVPUI)  = (*matViewUI);
			(*matVPUI) *= (*matProjUI);
		}

	public:
		//>> Загрузчик (текстуры необходимо разместить вместе c таблицей)
		bool Load(const wchar_t* gamePath, const wchar_t* fntName)
		{
			if (isInit) { _MBM(ERROR_InitAlready);  return false; }

			if (shader == nullptr)  { _MBM(ERROR_PointerNone);  return false; }
			if (IO == nullptr)      { _MBM(ERROR_PointerNone);  return false; }

			wchar_t syspath[256], error[256], p1[256], p2[256];
			GetCurrentDirectory(256, syspath);
			wsprintf(error, L"%s %s", ERROR_FntLoadFailed, fntName);
			wsprintf(p1, L"%s\\%s", gamePath, DIRECTORY_FONTS);
			wsprintf(p2, L"%s\\%s", syspath, DIRECTORY_FONTS);

			if (!LoadProc(p1, fntName))
				if (!LoadProc(p2, fntName)) { _MBM(error); return false; }

			ID = unique_font_id_counter++;
			if (_ISMISS(unique_font_id_counter)) unique_font_id_counter++;
			if (unique_font_id_counter == 0)     unique_font_id_counter++;	//wprintf(L"\nFont ID %X %s", ID, fntName);		

			return true;
		}

	protected:
		//>> Загрузчик
		bool LoadProc(const wchar_t* fntPath, const wchar_t* fntName)
		{
			wchar_t filepath[256];		wsprintf(filepath, L"%s\\%s", fntPath, fntName);

			///////////////// Чтение бинарной таблицы //////////////////////

			TBUFFER <byte, int32> file;								//wprintf(L"\nReading %s", filepath);
			ReadFileInMemory(filepath, file, 0);					if (file.buf == nullptr) return false;
			byte* fpbuf = file.buf;
			byte* fpend = fpbuf + file.count;

			memcpy(&bmfont.magic,		fpbuf, BMFh_SZ);						fpbuf += BMFh_SZ;				// magic (3) + version (1)
			memcpy(&bmfont.blockT1,		fpbuf, BMFi_SZ);						fpbuf += BMFi_SZ;				// id (1) + size (4)
			memcpy(&bmfont.block01,		fpbuf, BMF1_SZ);						fpbuf += BMF1_SZ;				// size of block_01 - len of string
			while (*fpbuf) bmfont.block01.fontName.push_back(*fpbuf++);			fpbuf += 1;						// len of string
			memcpy(&bmfont.blockT2,		fpbuf, BMFi_SZ);						fpbuf += BMFi_SZ;				// id (1) + size (4)
			memcpy(&bmfont.block02,		fpbuf, bmfont.blockT2sz);				fpbuf += bmfont.blockT2sz;		// size of block_02
			memcpy(&bmfont.blockT3,		fpbuf, BMFi_SZ);						fpbuf += BMFi_SZ;				// id (1) + size (4)
			for (int32 i=0; i<bmfont.block02.pages; i++)											{			// len of string(s)
				bmfont.block03.pageName.push_back(wstring());													// .
				while (*fpbuf) bmfont.block03.pageName[i].push_back(*fpbuf++);	fpbuf += 1;			}			// .
			memcpy(&bmfont.blockT4,		fpbuf, BMFi_SZ);						fpbuf += BMFi_SZ;				// id (1) + size (4)	
			bmfont.block04.Create(bmfont.blockT4sz / BMF4_SZ);													// 
			memcpy(bmfont.block04.buf,	fpbuf, bmfont.blockT4sz);				fpbuf += bmfont.blockT4sz;		// size of all block_04s		
			if (fpbuf >= fpend)								/* Нет кернинга */								{
				bmfont.blockT5 = 5;
				bmfont.blockT5sz = 0;																		}
			else											/* Есть кернинг */								{
				memcpy(&bmfont.blockT5, fpbuf, BMFi_SZ);						fpbuf += BMFi_SZ;				// id (1) + size (4)
				bmfont.block05.Create(bmfont.blockT5sz / BMF5_SZ);												// 
				memcpy(bmfont.block05.buf, fpbuf, bmfont.blockT5sz);			fpbuf += bmfont.blockT5sz;	}	// size of all block_05s

		//	bmfont.block01._printf(); //getchar();
		//	bmfont.block02._printf(); //getchar();
		//	bmfont.block03._printf(); //getchar();
		//	for (int32 i=0; i<bmfont.block04.count; i++)
		//		{ bmfont.block04[i]._printf(); getchar(); }
		//	for (int32 i=0; i<bmfont.block05.count; i++)
		//		{ bmfont.block05[i]._printf(); getchar(); }

			///////////////// Загрузка текстур в память //////////////////////

			bmfont.block03.pageFile.Create(bmfont.block02.pages);
			for (uint16 i=0; i<bmfont.block02.pages; i++)
			{
				wsprintf(filepath, L"%s\\%s", fntPath, bmfont.block03.pageName[i].c_str());
				ReadFileInMemory(filepath, *bmfont.block03.pageFile[i], true);
			}

			///////////////// Определение максимальной высоты //////////////////////

			bmfont.maxCharH = 0;
			for (int32 i=0; i<bmfont.block04.count; i++)
				if (bmfont.maxCharH < bmfont.block04[i].height)
					bmfont.maxCharH = bmfont.block04[i].height;

			return isInit = true;
		}

		//>> Генерация полигонов текста
		void Generate(wstring text, float height, uint32 start_symbol_pos, uint32 end_symbol_pos,
			TBUFFER <FONTVERTEX, uint32> & vertices, TBUFFER <uint32, uint32> & indices)
		{
			if (!height) height = 1.0f;

			float  scaleFactor, scaleX, scaleY;
			uint32 text_length = (uint32)text.length();
			uint32 chrCount = text_length;

			if (!text_length) return;

			if (text_length > start_symbol_pos)		chrCount = text_length - start_symbol_pos;
			else start_symbol_pos = 0;

			if (_NOMISS(end_symbol_pos))
			if (text_length >= end_symbol_pos &&
				end_symbol_pos > start_symbol_pos)	chrCount = end_symbol_pos - start_symbol_pos;

			TBUFFER <uint32, uint32> ID;

			vertices.Create(chrCount * 4); // 4 точки на 1 прямоугольник
			indices.Create(chrCount * 6);  // 2 треугольника по 3 точки составляют 1 прямоугольник
			ID.Create(chrCount);           // Быстрый доступ к block04[ID[номер_символа_в_строке]]	
			ID.Clear();

			for (uint32 i=0; i<chrCount; i++)
				for (int32 x=0; x<bmfont.block04.count; x++)
					if (text[start_symbol_pos + i] == bmfont.block04[x].id)
						{ ID[i] = x; break; }

			scaleFactor = height / static_cast<float>(bmfont.maxCharH);	 // для расчёта величины полигона
			scaleX = 1.000f / static_cast<float>(bmfont.block02.scaleW); // для uv-координат
			scaleY = 1.000f / static_cast<float>(bmfont.block02.scaleH); // для uv-координат
			float lastX = 0;
			float lineY = 0;
			for (uint32 i=0; i<chrCount; i++)
			{
				uint32 id = ID[i];                             // cur id
				uint32 idn = (i == chrCount-1) ? -1 : ID[i+1]; // next id
				uint32 v[] = { i*4+0, i*4+1, i*4+2, i*4+3 };

				uint16 ct = text[start_symbol_pos + i];
				if (!(ct ^ 0x000D) || !(ct ^ 0x000A)) { lastX = 0; lineY += height; continue; }	// Переход на новую строку

				BMF4 data = bmfont.block04[id];
				float _x = static_cast<float>(data.x) * scaleX;
				float _y = static_cast<float>(data.y) * scaleY;
				float _w = static_cast<float>(data.width) * scaleX;
				float _h = static_cast<float>(data.height) * scaleY;
				float _ws = static_cast<float>(data.width) * scaleFactor;
				float _hs = static_cast<float>(data.height) * scaleFactor;
				float _dx = static_cast<float>(data.xoffset) * scaleFactor;	// смещение от lastX-ой линии вправо
				float _dy = static_cast<float>(data.yoffset) * scaleFactor;	// смещение от height-потолка вниз

				//	float _px = bmfont.block02.scaleW * bmfont.block04[id].page;	// TODO: фикс на многостраничность
				//	float _py = bmfont.block02.scaleH * bmfont.block04[id].page;	// TODO: фикс на многостраничность

				float _xadvance = static_cast<float>(bmfont.block04[id].xadvance) * scaleFactor;
				/*	float _kerning  = 0;

				if (bmfont.myChrKrngNum && idn!=-1) // Ищем для пары букв кернинг между ними
				for(uint32 x=0; x<bmfont.myChrKrngNum; x++)
				if (bmfont.block05[x].first  == bmfont.block04[id].id  &&
				bmfont.block05[x].second == bmfont.block04[idn].id )
				{ _kerning = static_cast<float>(bmfont.block05[x].amount) * scaleFactor;  break; }	//*/

				float x_calc = lastX + _dx;
				float y_calc = height - _dy - lineY;											// v1 + + v2
																								// +       +
				FONTVERTEX &v1 = vertices.buf[v[1]];	FONTVERTEX &v2 = vertices.buf[v[2]];	// +       +
				FONTVERTEX &v0 = vertices.buf[v[0]];	FONTVERTEX &v3 = vertices.buf[v[3]];	// v0 + + v3

				v1.P.x = x_calc;						v2.P.x = x_calc + _ws;
				v1.P.y = y_calc;						v2.P.y = y_calc;
				v1.P.z = NULL;							v2.P.z = NULL;
				v1.T.u = _x;							v2.T.u = _x + _w;
				v1.T.v = _y;							v2.T.v = _y;

				v0.P.x = x_calc;						v3.P.x = x_calc + _ws;
				v0.P.y = y_calc - _hs;					v3.P.y = y_calc - _hs;
				v0.P.z = NULL;							v3.P.z = NULL;
				v0.T.u = _x;							v3.T.u = _x + _w;				//lastX += _ws + _dx;
				v0.T.v = _y + _h;						v3.T.v = _y + _h;				//lastX += _xadvance;// + _kerning;

				lastX += ((_ws + _dx)>_xadvance) ? (_ws + _dx) : _xadvance;

				// v1									v1 + + v2
				// +  +									   +    +
				// +    +								     +  +
				// v0 + + v3							       v3

				int ind = i * 6;
				if (CULL_DEFCCW)
				{
					indices.buf[ind + 0] = 1 + v[0];				indices.buf[ind + 3] = 1 + v[0];
					indices.buf[ind + 1] = 3 + v[0];				indices.buf[ind + 4] = 2 + v[0];
					indices.buf[ind + 2] = 0 + v[0];				indices.buf[ind + 5] = 3 + v[0];
				}
				else
				{
					indices.buf[ind + 0] = 1 + v[0];				indices.buf[ind + 3] = 1 + v[0];
					indices.buf[ind + 1] = 0 + v[0];				indices.buf[ind + 4] = 3 + v[0];
					indices.buf[ind + 2] = 3 + v[0];				indices.buf[ind + 5] = 2 + v[0];
				}
			}
		}

		//>> Настройка цвета букв (у всех букв один цвет)
		void GenerateColor(FONTVERTEX * vertices, uint32 count, COLORVEC color)
		{
			if (vertices == nullptr || !count) return;

			for (uint32 i=0; i<count; i++)
				vertices[i].C = color;
		}

		//>> Настройка цвета букв (градиент color1 -> color2)
		void GenerateColor(FONTVERTEX * vertices, uint32 count, COLORVEC color1, COLORVEC color2)
		{
			if (vertices == nullptr || !count) return;

			float numSteps = static_cast<float>(count / 4);
			COLORVEC delta = (color2 - color1) * (1.f / numSteps);

			for (uint32 i=0; i<count; i+=4)
			{
				if (i==0)															{
					vertices[i].C   = vertices[i+1].C = color1;
					vertices[i+2].C = vertices[i+3].C = color1 + delta;				}
				else																{
					vertices[i].C   = vertices[i+1].C = vertices[i-1].C;
					vertices[i+2].C = vertices[i+3].C = vertices[i-1].C + delta;	}
			};
		}

		//>> TODO: Ещё какие-нибудь эффекты с цветом текста (например, мультиградиент или сложение с дополнительной текстурой)
		//>> TODO: Генерация страниц шрифта в единый атлас. Пока что работаю без поддержки многостраничного шрифта
		void GenerateAtlas()
		{
			// Не забыть сделать фикс в CBaseFont::Generate(), см. комментарии там
		}

	public:
		//>> Сообщает размер всего набора полигонов
		void GetGeneratedWH(wstring text, float height, uint32 start_symbol_pos, uint32 end_symbol_pos, float& W, float& H)
		{
			if (!height) height = 1.0f;

			float  scaleFactor, scaleX, scaleY;
			uint32 text_length = (uint32)text.length();
			uint32 chrCount = text_length;

			if (!text_length) { W = H = 0;  return; }

			if (text_length > start_symbol_pos) chrCount = text_length - start_symbol_pos;
			else start_symbol_pos = 0;

			if (_NOMISS(end_symbol_pos))
				if (text_length >= end_symbol_pos &&
					end_symbol_pos > start_symbol_pos)
					chrCount = end_symbol_pos - start_symbol_pos;

			TBUFFER <uint32, uint32> ID; // Быстрый доступ к block04[ID[номер_символа_в_строке]]	
			ID.Create(chrCount);           
			ID.Clear();
	
			for (uint32 i=0; i<chrCount; i++)
				for (int32 x=0; x<bmfont.block04.count; x++)
					if (text[start_symbol_pos + i] == bmfont.block04[x].id)
						{ ID[i] = x; break; }

			scaleFactor = height / static_cast<float>(bmfont.maxCharH);	 // для расчёта величины полигона
			scaleX = 1.000f / static_cast<float>(bmfont.block02.scaleW); // для uv-координат
			scaleY = 1.000f / static_cast<float>(bmfont.block02.scaleH); // для uv-координат
			float lastX = 0;
			float lineY = 0;

			FONTVERTEX vertices[4 + 2];
		
			FONTVERTEX & v1 = vertices[1];		FONTVERTEX & v2 = vertices[2];
			FONTVERTEX & v0 = vertices[0];		FONTVERTEX & v3 = vertices[3];

			for (uint32 i=0; i<chrCount; i++)
			{
				uint32 id = ID[i];	// cur id

				uint16 ct = text[start_symbol_pos + i];
				if (!(ct ^ 0x000D) || !(ct ^ 0x000A)) { lastX = 0; lineY += height; continue; }	// Переход на новую строку

				BMF4 data = bmfont.block04[id];
				float _x = static_cast<float>(data.x) * scaleX;
				float _y = static_cast<float>(data.y) * scaleY;
				float _w = static_cast<float>(data.width) * scaleX;
				float _h = static_cast<float>(data.height) * scaleY;
				float _ws = static_cast<float>(data.width) * scaleFactor;
				float _hs = static_cast<float>(data.height) * scaleFactor;
				float _dx = static_cast<float>(data.xoffset) * scaleFactor;	// смещение от lastX-ой линии вправо
				float _dy = static_cast<float>(data.yoffset) * scaleFactor;	// смещение от height-потолка вниз

				float _xadvance = static_cast<float>(data.xadvance) * scaleFactor;

				float x_calc = lastX + _dx;
				float y_calc = height - _dy - lineY;										// v1 + + v2
																							// +       +
				v1.P.x = x_calc;						v2.P.x = x_calc + _ws;				// +       +
				v1.P.y = y_calc;						v2.P.y = y_calc;					// v0 + + v3

				v0.P.x = x_calc;						v3.P.x = x_calc + _ws;
				v0.P.y = y_calc - _hs;					v3.P.y = y_calc - _hs;

				lastX += ((_ws + _dx)>_xadvance) ? (_ws + _dx) : _xadvance;

				if (i == 0) vertices[4] = v1; // top left
			}			    vertices[5] = v3; // bottom right

			W = vertices[5].P.x - vertices[4].P.x;
			H = lineY + height; //vertices[4].P.y - vertices[5].P.y;
		}
	};
}

#endif // _FONT_H