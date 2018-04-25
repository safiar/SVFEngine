// ----------------------------------------------------------------------- //
//
// MODULE  : FontText.h
//
// PURPOSE : Вспомогательные класс текстов
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _FONTTEXT_H
#define _FONTTEXT_H

#include "stdafx.h"
#include "Object.h"

namespace SAVFGAME
{
	enum eUITextScaleType // Типы масштабирования для UI элементов
	{
		SCALE_TALL,		// normal scaling
		SCALE_TMIN,		// scale_X = scale_Y = min(scale_X, scale_Y)
		SCALE_TMAX,		// scale_X = scale_Y = max(scale_X, scale_Y)	
	};
	enum eUITextCalcTypeX
	{
		CALC_XT_FROM_RIGHT,		// точка привязки полигонов текста с правой стороны
		CALC_XT_FROM_LEFT		// точка привязки полигонов текста с левой стороны
	};
	enum eUITextCalcTypeY
	{
		CALC_YT_FROM_TOP,		// точка привязки полигонов текста с верхней стороны
		CALC_YT_FROM_BOTTOM		// точка привязки полигонов текста с нижней стороны
	};

	class CBaseFont;
	class CFontR;

	class CText : public CObject // Вспомогательный класс управления текстами
	{
		friend class CBaseFont;
		friend class CFontR;
	public:
		CText(const CText& src)				= delete;
		CText(CText&& src)					= delete;
		CText& operator=(CText&& src)		= delete;
		CText& operator=(const CText& src)	= delete;
	public:
		CText() : CObject(), text(L""), height(1), color1(1,1,1,1), color2(1,1,1,1), UI(false), pos(CObject::pos)	{ Init(); };
		CText( wstring  _text)
			: CObject(),     height(1), color1(1,1,1,1), color2(1,1,1,1), UI(true), text(_text), pos(CObject::pos)	{ Init(); };
		CText( wstring  _text, float _height, WPOS pos, COLORVEC RGBA, bool _UI)
			: CObject(pos),  height(_height), color1(RGBA), color2(RGBA),   UI(_UI), text(_text), pos(CObject::pos) { Init(); };
		CText( wstring  _text, float _height, WPOS pos, COLORVEC RGBA, COLORVEC RGBA_2, bool _UI)
			: CObject(pos),  height(_height), color1(RGBA), color2(RGBA_2), UI(_UI), text(_text), pos(CObject::pos) { Init(); gradient=1; };
		CText( wchar_t* _text)
			: CObject(),     height(1), color1(1,1,1,1), color2(1,1,1,1), UI(true), pos(CObject::pos)	{ text=_text; Init(); };
		CText( wchar_t* _text, float _height, WPOS pos, COLORVEC RGBA, bool _UI)
			: CObject(pos),  height(_height), color1(RGBA), color2(RGBA),   UI(_UI), pos(CObject::pos)	{ text=_text; Init(); };
		CText( wchar_t* _text, float _height, WPOS pos, COLORVEC RGBA, COLORVEC RGBA_2, bool _UI)
			: CObject(pos),  height(_height), color1(RGBA), color2(RGBA_2), UI(_UI), pos(CObject::pos)	{ text=_text; Init(); gradient=1; };
		///////////////////////// Operators /////////////////////////
		CText& operator=(const wchar_t * uitext)
		{
			text.assign(uitext);
			return *this;
		}
		CText& operator=(const wstring & uitext)
		{
			text = uitext;
			return *this;
		}
		/////////////////////////
		WPOS * const pos;
		///////////////////////// Base info block /////////////////////////
		wstring			text; // actual showing text if SetTextPointer() didn't presented
		float			height;
		COLORVEC		color1;
		COLORVEC		color2;
		uint32			start_symbol_pos; // show from this symbol | ClearStartSymbolPos()
		uint32			end_symbol_pos;	// show  to  this symbol | ClearEndSymbolPos()
		bool			UI;
		bool			gradient;
		///////////////////////// Interactive block /////////////////////////	
	protected:
		eClickStatus	click[2];
	public:
		bool ClickChange(eClickStatus status)
		{
			click[0] = click[1];
			click[1] = status;
			return (click[0] != click[1]);
		}
		bool ClickIsChanged()
		{
			return (click[0] != click[1]);
		}
		bool ClickIsOn()
		{
			return (click[1] != CLICK_OFF);
		}
		eClickStatus ClickGet()
		{
			return click[1];
		}
		///////////////////////// Font render manager block /////////////////////////
	protected:
		struct CTextCID /* Данный блок используется в рендер части класса шрифта для управления редко меняющимися текстами */
		{
			CTextCID() : constant_ID(MISSING), font_ID(NULL), font_iteration(MISSING) {}
			//////////////////
			 int64 constant_ID;		// mark from font
			uint32 font_ID;			// unique font ID which have marked text
			 int64 font_iteration;	// для случаев, когда шрифт был перезагружен и/или потерял буферы текстов
		};
		vector<CTextCID> CTCID;
		bool      constant_text;	// false == dynamic text (always update/reload)
		bool      reload_call;		// call if need to update constant text
	public:
		//>> Очищает память связи со шрифтами (CTCID)
		void ClearCTCIDmem()
		{
			CTCID.erase(CTCID.begin(), CTCID.end());
		}
		//>> Делает текст постоянным
		void SetTextConstant()
		{
			if (constant_text) return; // already constant

			for (auto & cur : CTCID)
				if ((_NOMISS(cur.constant_ID)) &&
					(cur.font_ID != NULL))
						{ reload_call = true; break; }
			constant_text = true;
		}
		//>> Делает текст динамическим
		void SetTextDynamic()
		{
			constant_text = false;
		}
		//>> Проверяет тип текста
		bool IsTextConstant() { return constant_text; }
		//>> Перезагружает текст, если тот был изменён (для редких изменений)
		void CallReloadConstantText() { reload_call = true; }
	protected:
		//>> Font class only :: Проверяет, был ли запрос на перезагрузку постоянного текста
		bool ConstantTextCheckReload() { _RETBOOL(reload_call); }
		//>> Font class only :: Новый уникальный шрифт и новая ячейка для хранение id
		void ConstantTextAddID(int64 id, uint32 font_id, int64 current_iteration)
		{
			CTextCID _new_CTCID;
			_new_CTCID.constant_ID    = id;
			_new_CTCID.font_ID        = font_id;
			_new_CTCID.font_iteration = current_iteration;
			CTCID.emplace_back(_new_CTCID);
		}
		//>> Font class only :: Старый шрифт, но буферы изменились (смена id)
		void ConstantTextSetIDInAlreadyExistPattern(int64 id, uint32 font_id, int64 current_iteration)
		{
			for (auto & cur : CTCID)
				if (_EQUAL(cur.font_ID, font_id))
				{
					cur.constant_ID = id;
					cur.font_iteration = current_iteration;
				}
		}
		//>> Font class only :: Узнаём, по какому id мы найдём нужный буфер модели
		int64 ConstantTextGetID(uint32 font_id)
		{
			for (auto & cur : CTCID)
				if (_EQUAL(cur.font_ID, font_id))
					return (int64)cur.constant_ID;
			return (int64)MISSING;
		}
		//>> Font class only :: <true> если существует ячейка с записью для данного шрифта
		bool ConstantTextIsHaveFontID(uint32 font_id)
		{
			for (auto & cur : CTCID)
				if (_EQUAL(cur.font_ID, font_id))
					return true;
			return false;
		}
		//>> Font class only :: Сверяет итератор шрифта
		int ConstantTextCheckFontIteration(uint32 font_id, int64 current_iteration)
		{
			for (auto & cur : CTCID)
				if (_EQUAL(cur.font_ID, font_id))
					return (current_iteration == cur.font_iteration);
			return MISSING;
		}
		//>> Font class only :: Узнаёт итерацию шрифта в записи текста
		int64 ConstantTextGetFontIteration(uint32 font_id)
		{
			for (auto & cur : CTCID)
				if (_EQUAL(cur.font_ID, font_id))
					return cur.font_iteration;
			return MISSING;
		}
		//>> Font class only :: Устанавливает итерацию шрифта
		void ConstantTextSetFontIteration(uint32 font_id, int64 current_iteration)
		{
			for (auto & cur : CTCID)
				if (_EQUAL(cur.font_ID, font_id))
					cur.font_iteration = current_iteration;
		}
		/////////////////////////  UI extra block  /////////////////////////
	public:
		void ClearStartSymbolPos() { start_symbol_pos = 0;       }
		void ClearEndSymbolPos()   { end_symbol_pos   = MISSING; }
		//>> return -width if (CALC_XT_FROM_RIGHT && !abs) :: make UpdateWH() before getting it
		float GetPolygonWidth(bool abs)
		{
			if (abs) return W;
			else
			{
				switch (calc_x_type)
				{
				case CALC_XT_FROM_RIGHT: return -W;
				default:                 return  W;
				}
			}
		}
		//>> return -height if (CALC_YT_FROM_TOP && !abs) :: make UpdateWH() before getting it
		float GetPolygonHeight(bool abs)
		{
			if (abs) return  H;
			else
			{
				switch (calc_y_type)
				{
				case CALC_YT_FROM_TOP: return -H;
				default:               return  H;
				}
			}	
		}
		//>> Initial UI text setup :: type your <text> before calling with <scale width == true> option
		void InitUI(CBaseFont * font,
					uint32 _initial_screen_width,
					uint32 _initial_screen_height,
					float  _initial_text_height,
			        float  _initial_pos_x,
					float  _initial_pos_y,
					eUITextCalcTypeX _calc_x_type,
					eUITextCalcTypeY _calc_y_type,
					eUITextScaleType  _scale_type,
					bool   _scale_text_width_to_initial_proportion,
					bool   _const_text_height)
		{
			initial_screen_width  = _initial_screen_width;
			initial_screen_height = _initial_screen_height;
			initial_text_height   = _initial_text_height;
			initial_X             = _initial_pos_x;
			initial_Y             = _initial_pos_y;
			calc_x_type           = _calc_x_type;
			calc_y_type           = _calc_y_type;
			scale_type            = _scale_type;
			text_fixed_h          = _const_text_height;
			text_fixed_w          = _scale_text_width_to_initial_proportion;
			pos->sx               = 1;
			UI                    = true;

			if (text_fixed_w)
			{
				height = initial_text_height;
				UpdatePolygonWH(font);			// text example should be presented, otherwise W will be NULL
				text_fixed_init_width = W;		//
				text_fixed_width      = W;		//
			}

			UpdateUI(font, initial_screen_width, initial_screen_height, true);
		};
		//>> Trying to update UI text variables :: return <false> if screen resolution hasn't changed from last call
		bool UpdateUI(CBaseFont * font, uint32 screen_width, uint32 screen_height, bool force_update)
		{
			//           abcdefghijklmnopq    abcdefghijklm----
			// pos->sx : 1.0 ==  [--------]   0.5 ==  [----]        X scaling to left    <
			// pos->sy :                                            Y scaling to bottom  v

			if (UI &&
			   (force_update || _NOTEQUAL(screen_height, current_screen_height) ||
								_NOTEQUAL(screen_width,  current_screen_width)  ) )
			{
				float screen_scale_x_mem = screen_scale_x = (float)screen_width  / initial_screen_width;
				float screen_scale_y_mem = screen_scale_y = (float)screen_height / initial_screen_height;
				switch (scale_type)
				{
				case SCALE_TALL: break;
				case SCALE_TMIN: screen_scale_x = screen_scale_y = min(screen_scale_x, screen_scale_y); break;
				case SCALE_TMAX: screen_scale_x = screen_scale_y = max(screen_scale_x, screen_scale_y); break;
				default: break;
				}
				if (text_fixed_h) height = initial_text_height;
				else              height = initial_text_height * screen_scale_y;
				current_screen_height = screen_height;
				current_screen_width  = screen_width;
				UpdatePolygonWH(font);
				if (text_fixed_w)
				{
					text_fixed_width = text_fixed_init_width  * screen_scale_x_mem;	// определяем новую ширину полигона исходя из исходно заданной
					pos->sx = text_fixed_width / W;									// масштабируем сгенерированный W под эту ширину
					W *= pos->sx;													// корректируем данные о реальной ширине полигона W
				}
				switch (calc_x_type)
				{
				case CALC_XT_FROM_RIGHT:  pos->x = initial_X * screen_scale_x_mem - W; break;
				case CALC_XT_FROM_LEFT:   pos->x = initial_X * screen_scale_x_mem;     break;
				}
				switch (calc_y_type)
				{
				case CALC_YT_FROM_TOP:    pos->y = initial_Y * screen_scale_y_mem + height;       break; // + 1 line           up
				case CALC_YT_FROM_BOTTOM: pos->y = initial_Y * screen_scale_y_mem - (H - height); break; // - (lines - 1 line) down
				}
				if (constant_text) CallReloadConstantText();
				return true;
			}
			return false;
		}
		//>> Обновление данных о текущих размерах полигона текста [ для взятие данных - GetPolygonWidth() / GetPolygonHeight() ]
		void UpdatePolygonWH(CBaseFont * font);
		//>> Ограничение выводимого текста под размер полигона 
		void SetStartEndSymbolsPos(float max_polygon_width, float max_polygon_height, bool from_end, bool space_divider, CBaseFont * font);
		//>> alt: set pointer to external wstring < using CText->*ptext instead of own CText->text >
		void ExTextPointerSet(wstring * ptr) { ptext = ptr; }
		//>> alt: get pointer to external wstring < using CText->*ptext instead of own CText->text >
		wstring * ExTextPointerGet() { return ptext; }
		//>> alt: clear pointer to external wstring < using own CText->text instead of extern CText->*ptext >
		void ExTextPointerClose() { ptext = nullptr; }
		float GetCurPosX()   { return pos->x; }
		float GetCurPosY()   { return pos->y; }
		void SetCurPosX(float x, bool use_calc_x_type)
		{
			if (!use_calc_x_type) pos->x = x;
			else
			{
				switch (calc_x_type)
				{
				case CALC_XT_FROM_RIGHT:  pos->x = x - W; break;
				case CALC_XT_FROM_LEFT:   pos->x = x;     break;
				}
			}
		}
		void SetCurPosY(float y, bool use_calc_y_type)
		{
			if (!use_calc_y_type) pos->y = y;
			else
			{
				switch (calc_y_type)
				{
				case CALC_YT_FROM_TOP:    pos->y = y + height;       break; // + 1 line           up
				case CALC_YT_FROM_BOTTOM: pos->y = y - (H - height); break; // - (lines - 1 line) down
				}
			}
		}
		uint32 GetInitScreenWidth() { return initial_screen_width; }
		uint32 GetInitScreenHeight() { return current_screen_height; }
		float GetInitTextHeight() { return initial_text_height; }
		float GetInitPosX() { return initial_X; }
		float GetInitPosY() { return initial_Y; }
	protected:
		void Init()
		{
			click[0] = click[1] = CLICK_ON;
			gradient = false;
			initial_text_height = height;
			initial_screen_height = 0;
			current_screen_height = 0;
			initial_X = pos->x;
			initial_Y = pos->y;
			calc_x_type = CALC_XT_FROM_LEFT;
			calc_y_type = CALC_YT_FROM_BOTTOM;
			scale_type  = SCALE_TALL;
			W = H = 0;
			ptext = nullptr;
			start_symbol_pos = 0;
			end_symbol_pos = MISSING;
			constant_text = true;
			reload_call = false;
		}
		wstring * ptext;					// ptr to external text 
		float     initial_text_height;
		uint32    initial_screen_height;
		uint32    initial_screen_width;
		uint32    current_screen_height;
		uint32    current_screen_width;
		float     W, H;						// polygon W / H  (from last used font)
		float     initial_X;				// initial pos.x
		float     initial_Y;				// initial pos.y
		float     screen_scale_x;
		float     screen_scale_y;
		eUITextCalcTypeX calc_x_type;		// re-calc pos.x from  < right / left >
		eUITextCalcTypeY calc_y_type;		// re-calc pos.y from  < top / bottom >
		eUITextScaleType  scale_type;
		float     text_fixed_init_width;	// исходно заданная ширина
		float     text_fixed_width;			// ширина под текущее окно
		bool      text_fixed_w;				// метка, что ширина полигона подгоняется под исходное соотношение < W / screen_width >
		bool      text_fixed_h;				// метка, что высота текста не меняется с изменением screen_height
	};

}

#endif // _FONTTEXT_H