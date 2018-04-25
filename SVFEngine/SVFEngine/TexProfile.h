// ----------------------------------------------------------------------- //
//
// MODULE  : TexProfile.h
//
// PURPOSE : Загрузчик профиля текстурных элементов
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _TEXPROFILE_H
#define _TEXPROFILE_H

#include "stdafx.h"
#include "Object.h"
#include "Font.h"
#include "Shader.h"

namespace SAVFGAME
{
	enum eUIScaleType // Методология перерасчёта размеров UI элементов под изменение соотношения сторон окна
	{
		SCALE_ALL,		// normal scaling
		SCALE_MIN,		// scale_X = scale_Y = min(scale_X, scale_Y)
		SCALE_MAX,		// scale_X = scale_Y = max(scale_X, scale_Y)
		SCALE_MAX_Y,	// scale_X = scale_Y if (scale_Y > scale_X)
		SCALE_MAX_X,	// scale_Y = scale_X if (scale_X > scale_Y)
		SCALE_MIN_Y,	// scale_X = scale_Y if (scale_Y < scale_X)
		SCALE_MIN_X,	// scale_Y = scale_X if (scale_X < scale_Y)
		SCALE_X_ONLY,	// scale_Y = 1.f
		SCALE_Y_ONLY,	// scale_X = 1.f
	};
	enum eUICalcTypeX // Положение координаты 0 по оси Х относительно границ полигона
	{
		CALC_X_FROM_RIGHT,		// scalepoint = 1.f
		CALC_X_FROM_LEFT,		// scalepoint = 0.f
		CALC_X_FROM_CENTER,		// scalepoint = 0.5f
		CALC_X_FROM_SCALEPOINT	// [0.f...1.f] = left, right :: use SetCalcXScalePoint()
	};
	enum eUICalcTypeY // Положение координаты 0 по оси Y относительно границ полигона
	{
		CALC_Y_FROM_TOP,		// scalepoint = 1.f
		CALC_Y_FROM_BOTTOM,		// scalepoint = 0.f
		CALC_Y_FROM_CENTER,		// scalepoint = 0.5f
		CALC_Y_FROM_SCALEPOINT	// [0.f...1.f] = bottom, top :: use SetCalcYScalePoint()
	};

	struct TEXUIDATA : public CObject // Вспомогательная структура для работы с UI элементами
	{
	public:
		TEXUIDATA(const TEXUIDATA& src)				= delete;
		TEXUIDATA(TEXUIDATA&& src)					= delete;
		TEXUIDATA& operator=(TEXUIDATA&& src)		= delete;
		TEXUIDATA& operator=(const TEXUIDATA& src)	= delete;
	public:
		TEXUIDATA() : CObject(), pos(CObject::pos)
		{
			initial_screen_height = 0;
			initial_screen_width  = 0;
			current_screen_height = MISSING;
			current_screen_width  = MISSING;
			current_width         = 0;
			current_height        = 0;
			initial_width         = 0;
			initial_height        = 0;
			initial_pos_x         = 0;
			initial_pos_y         = 0;
			initial_scale_x       = 0;
			initial_scale_y       = 0;
			screen_scale_x        = 0;
			screen_scale_y        = 0;
			calc_x_type           = CALC_X_FROM_LEFT;
			calc_y_type           = CALC_Y_FROM_BOTTOM;
			calc_x_scale_point    = 0;
			calc_y_scale_point    = 0;
			rotate_center_dx      = 0;
			rotate_center_dy      = 0;
			cur_rotate_center_x   = 0;
			cur_rotate_center_y   = 0;
			current_pos_x_mem     = 0;
			current_pos_y_mem     = 0;
			scale_type            = SCALE_ALL;
		}
		~TEXUIDATA(){}
		void InitAngle(float angle)
		{
			pos->Q = MATH3DQUATERNION(0, 0, angle);
			pos->ax = 0;
			pos->ay = 0;
			pos->az = angle;
		}
		//>> If you are using :: UpdateUI() , SetCurPosX() , SetCurPosY() :: so use them before UpdateRotatedPos()
		void AddAngle(float delta_angle, bool update_rotated_pos)
		{
			pos->az += delta_angle;
			if (pos->az > 360 || pos->az < -360)
				pos->az -= 360 * static_cast<int>(pos->az / 360);
			if (pos->az < 0)
				pos->az += 360;
			pos->Q = MATH3DQUATERNION(MATH3DVEC(0, 0, pos->az));
			pos->Q._normalize_check();

			if (update_rotated_pos) UpdateRotatedPos();
		}
		//>> If you are using :: UpdateUI() , SetCurPosX() , SetCurPosY() :: so use them before UpdateRotatedPos()
		void UpdateRotatedPos()
		{
			pos->x = (current_pos_x_mem + cur_rotate_center_x)
				- cur_rotate_center_x * CTAB::cosA(pos->az)
				+ cur_rotate_center_y * CTAB::sinA(pos->az);
			pos->y = (current_pos_y_mem - cur_rotate_center_y)
				+ cur_rotate_center_x * CTAB::sinA(pos->az)
				+ cur_rotate_center_y * CTAB::cosA(pos->az);
		}
		//>> relative rotate center [left_polygon_border + (right) dx, bottom_polygon_border + (up) dy]
		void InitRotateCenter(float right_dx, float up_dy)
		{
			rotate_center_dx = right_dx;
			rotate_center_dy = up_dy;
		}
		void InitUI(
			uint32 _initial_screen_width,
			uint32 _initial_screen_height,
			float  _initial_position_x,
			float  _initial_position_y,
			float  _initial_polygon_width,
			float  _initial_polygon_height,
			float  _initial_scale_x,
			float  _initial_scale_y,
			eUICalcTypeX _calc_x_type,
			eUICalcTypeY _calc_y_type,
			eUIScaleType _scale_type)
		{
			initial_screen_height = _initial_screen_height;
			initial_screen_width  = _initial_screen_width;
			initial_pos_x         = _initial_position_x;
			initial_pos_y         = _initial_position_y;
			initial_width         = _initial_polygon_width;
			initial_height        = _initial_polygon_height;
			initial_scale_x       = _initial_scale_x;
			initial_scale_y       = _initial_scale_y;
			calc_x_type           = _calc_x_type;
			calc_y_type           = _calc_y_type;
			scale_type            = _scale_type;
		};
		bool UpdateUI(uint32 screen_width, uint32 screen_height, bool force_update)
		{
			if (force_update || _NOTEQUAL(screen_height, current_screen_height) ||
								_NOTEQUAL(screen_width, current_screen_width))
			{
				float screen_scale_x_mem = screen_scale_x = (float)screen_width / initial_screen_width;
				float screen_scale_y_mem = screen_scale_y = (float)screen_height / initial_screen_height;
				switch (scale_type)
				{
				case SCALE_ALL: break;
				case SCALE_MIN: screen_scale_x = screen_scale_y = min(screen_scale_x, screen_scale_y); break;
				case SCALE_MAX: screen_scale_x = screen_scale_y = max(screen_scale_x, screen_scale_y); break;
				case SCALE_MIN_Y: if (screen_scale_y < screen_scale_x) screen_scale_x = screen_scale_y; break;
				case SCALE_MAX_Y: if (screen_scale_y > screen_scale_x) screen_scale_x = screen_scale_y; break;
				case SCALE_MIN_X: if (screen_scale_x < screen_scale_y) screen_scale_y = screen_scale_x; break;
				case SCALE_MAX_X: if (screen_scale_x > screen_scale_y) screen_scale_y = screen_scale_x; break;
				case SCALE_X_ONLY: screen_scale_y = 1.f; break;
				case SCALE_Y_ONLY: screen_scale_x = 1.f; break;
				default: break;
				}
				pos->sx = initial_scale_x * screen_scale_x;
				pos->sy = initial_scale_y * screen_scale_y;
				current_screen_width  = screen_width;
				current_screen_height = screen_height;
				current_width  = initial_width  * pos->sx;
				current_height = initial_height * pos->sy;
				switch (calc_x_type)
				{
				case CALC_X_FROM_RIGHT:      pos->x = initial_pos_x * screen_scale_x_mem - current_width;                       break;
				case CALC_X_FROM_LEFT:       pos->x = initial_pos_x * screen_scale_x_mem;                                       break;
				case CALC_X_FROM_CENTER:     pos->x = initial_pos_x * screen_scale_x_mem - current_width * 0.5f;                break;
				case CALC_X_FROM_SCALEPOINT: pos->x = initial_pos_x * screen_scale_x_mem - current_width * calc_x_scale_point;  break;
				}
				switch (calc_y_type)
				{
				case CALC_Y_FROM_TOP:        pos->y = initial_pos_y * screen_scale_y_mem + current_height;                      break;
				case CALC_Y_FROM_BOTTOM:     pos->y = initial_pos_y * screen_scale_y_mem;                                       break;
				case CALC_Y_FROM_CENTER:     pos->y = initial_pos_y * screen_scale_y_mem + current_height * 0.5f;               break;
				case CALC_Y_FROM_SCALEPOINT: pos->y = initial_pos_y * screen_scale_y_mem + current_height * calc_y_scale_point; break;
				}
				current_pos_x_mem = pos->x;
				current_pos_y_mem = pos->y;
				cur_rotate_center_x = rotate_center_dx * pos->sx;
				cur_rotate_center_y = rotate_center_dy * pos->sy;
				return true;
			}
			return false;
		}
		void SetCurPosX(float x, bool use_calc_x_type)
		{
			if (!use_calc_x_type) pos->x = x;
			else
			{
				switch (calc_x_type)
				{
				case CALC_X_FROM_RIGHT:      pos->x = x - current_width;                       break;
				case CALC_X_FROM_LEFT:       pos->x = x;                                       break;
				case CALC_X_FROM_CENTER:     pos->x = x - current_width * 0.5f;                break;
				case CALC_X_FROM_SCALEPOINT: pos->x = x - current_width * calc_x_scale_point;  break;
				}
			}
			current_pos_x_mem = pos->x;
		}
		void SetCurPosY(float y, bool use_calc_y_type)
		{
			if (!use_calc_y_type) pos->y = y;
			else
			{
				switch (calc_y_type)
				{
				case CALC_Y_FROM_TOP:        pos->y = y + current_height;                      break;
				case CALC_Y_FROM_BOTTOM:     pos->y = y;                                       break;
				case CALC_Y_FROM_CENTER:     pos->y = y + current_height * 0.5f;               break;
				case CALC_Y_FROM_SCALEPOINT: pos->y = y + current_height * calc_y_scale_point; break;
				}
			}
			current_pos_y_mem = pos->y;
		}
		float GetCurPosX()   { return pos->x; }
		float GetCurPosY()   { return pos->y; }
		//>> return current polygon width
		float GetCurWidth()  { return current_width;  }
		//>> return current polygon height
		float GetCurHeight() { return current_height; }
		float GetCurScaleX() { return pos->sx; }
		float GetCurScaleY() { return pos->sy; }
		float GetCurAngle()  { return pos->az; }
		eUICalcTypeX GetCalcTypeX()			 { return calc_x_type; }
		eUICalcTypeY GetCalcTypeY()			 { return calc_y_type; }
		void SetCalcTypeX(eUICalcTypeX type) { calc_x_type = type; }
		void SetCalcTypeY(eUICalcTypeY type) { calc_y_type = type; }
		float GetCalcXScalePoint()			 { return calc_x_scale_point; }
		float GetCalcYScalePoint()			 { return calc_y_scale_point; }
		void  SetCalcXScalePoint(float sx)   { calc_x_scale_point = _CLAMP(sx, 0.f, 1.f); }
		void  SetCalcYScalePoint(float sy)   { calc_y_scale_point = _CLAMP(sy, 0.f, 1.f); }
		uint32 GetInitScreenHeight() { return initial_screen_height; }
		uint32 GetInitScreenWidth()  { return initial_screen_width;  }
		float  GetInitHeight()       { return initial_height;  }
		float  GetInitWidth()        { return initial_width;   }
		float  GetInitPosX()         { return initial_pos_x;   }
		float  GetInitPosY()         { return initial_pos_y;   }
		float  GetInitScaleX()       { return initial_scale_x; }
		float  GetInitScaleY()       { return initial_scale_y; }
	protected:
		uint32    initial_screen_height;
		uint32    initial_screen_width;
		uint32    current_screen_height;
		uint32    current_screen_width;
		float     current_width;
		float     current_height;
		float     initial_width;	// polygon W
		float     initial_height;	// polygon H
		float     initial_pos_x;
		float     initial_pos_y;
		float     initial_scale_x;
		float     initial_scale_y;
		float     screen_scale_x;
		float     screen_scale_y;
		eUICalcTypeX calc_x_type;		 // re-calc pos.x from  < right / left / center / scalepoint >
		eUICalcTypeY calc_y_type;		 // re-calc pos.y from  < top / bottom / center / scalepoint >
		float        calc_x_scale_point; // re-calc pos.x scale point
		float        calc_y_scale_point; // re-calc pos.y scale point
		float     rotate_center_dx;
		float     rotate_center_dy;
		float     cur_rotate_center_x;
		float     cur_rotate_center_y;
		float     current_pos_x_mem;
		float     current_pos_y_mem;
		eUIScaleType scale_type;
	public:
		WPOS * const pos;
	};
	
	class CTexProfile // Содержит текстурные элементы
	{
	public:
		struct TEXELEMENT // Элемент текстуры
		{
			friend class CTexProfile;
			friend class CTexProfileR;
			friend struct TBUFFER <TEXELEMENT, int32>;
		protected:
			int32		X;
			int32		Y;
			int32		dx;
			int32		dy;
			std::string	name;
			int32		TexID;
			FONTVERTEX	vbuf[9];	// 4 (not centered), 9 (centered)
			float		last[4];	// store last W/H/sx/sy at GenerateVerticies()
			float		cur_width;
			float		cur_height;
			float		center_x;	// default is 0 (polygon not centered) :: value [0.f ... 1.f]
			float		center_y;	// default is 0 (polygon not centered) :: value [0.f ... 1.f]
			bool		centered;
			bool		get_last;	// tip to GenerateVerticies() load last W/H/sx/sy
			HCRC32		crc32;		// crc32 from string name
			//>> Tip to GenerateVerticies() load last W/H/sx/sy
			void TipGetLast()
			{
				get_last = true;
			}
			//>> Drop get last tip
			void ResetGetLast()
			{
				get_last = false;
			}
			//>> Check get last tip
			bool CheckGetLast()
			{
				return get_last;
			}
		public:
			TEXELEMENT(const TEXELEMENT& src)				= delete;
			TEXELEMENT(TEXELEMENT&& src)					= delete;
			TEXELEMENT& operator=(TEXELEMENT&& src)			= delete;
			TEXELEMENT& operator=(const TEXELEMENT& src)	= delete;
		protected:
			TEXELEMENT()  { Close(); }
			~TEXELEMENT() { }
			void Close()
			{
				X = Y = dx = dy = TexID = 0;
				cur_width = cur_height = 0;
				crc32.m_crc32 = 0;
				name.clear();
				center_x = 0.f;
				center_y = 0.f;
				centered = false;
				get_last = false;
				ZeroMemory(vbuf, sizeof(FONTVERTEX) * 9);
			}
		public:
			float GetWidth()                { return cur_width;  }
			float GetHeight()               { return cur_height; }
			int32 GetWidthOrigin()          { return dx;         }
			int32 GetHeightOrigin()         { return dy;         }
			int32 GetTextureID()            { return TexID;      }
			const std::string & GetName()   { return name;       }
			bool IsElementCentered()        { return centered;   }
			void GetElementXY(int32 & out_x, int32 & out_y)
			{
				out_x = X;
				out_y = Y;
			}
			void GetElementDXDY(int32 & out_dx, int32 & out_dy)
			{
				out_dx = dx;
				out_dy = dy;
			}
			void GetElementUV(float & out_u, float & out_v)
			{
				if (!centered)
				{
					out_u = vbuf[0].T.u; // 0 is min
					out_v = vbuf[0].T.v; // 0 is min
				}
				else
				{
					out_u = vbuf[6].T.u; // 6 is min
					out_v = vbuf[6].T.v; // 6 is min
				}
			}
			void GetElementDUDV(float & out_du, float & out_dv)
			{
				if (!centered)
				{
					out_du = vbuf[2].T.u - vbuf[0].T.u; // 0 is min
					out_dv = vbuf[2].T.v - vbuf[0].T.v; // 0 is min
				}
				else
				{
					out_du = vbuf[2].T.u - vbuf[6].T.u; // 6 is min
					out_dv = vbuf[2].T.v - vbuf[6].T.v; // 6 is min
				}
			}
			void GetElementCenter(float & cx, float & cy)
			{
				cx = center_x;
				cy = center_y;
			}
		};
		struct TEXDATA // Текстура и её параметры
		{
			friend class CTexProfile;
			friend class CTexProfileR;
			friend struct TBUFFER <TEXDATA, int32>;
		protected:
			TBUFFER <byte, int32> file;
			uint32 width;
			uint32 height;
			float  scaleX; // = 1.f / width
			float  scaleY; // = 1.f / height
		public:
			TEXDATA(const TEXDATA& src)				= delete;
			TEXDATA(TEXDATA&& src)					= delete;
			TEXDATA& operator=(TEXDATA&& src)		= delete;
			TEXDATA& operator=(const TEXDATA& src)	= delete;
		protected:
			TEXDATA()  { Close(); }
			~TEXDATA() { }
			void Close()
			{
				file.Close();
				width = height = 0;
				scaleX = scaleY = 0.f;
			};
		public:
			uint32 GetTextureWidth()  { return width;  }
			uint32 GetTextureHeight() { return height; }
			float  GetTextureScaleX() { return scaleX; }
			float  GetTextureScaleY() { return scaleY; }
			bool   IsTexFileExist()   { return (file.buf != nullptr); }
			int32  GetTexFileSize()   { return file.count; }
			const void * GetTexFile() { return file.buf;   }
		};
	protected:
		CShader *			shader { nullptr };
		const CStatusIO *	IO     { nullptr };
		bool				isInit { false };
	protected:
		MATH3DMATRIX *		matWorld;		// screen UI
		MATH3DMATRIX *		matViewUI;		// screen UI
		MATH3DMATRIX *		matProjUI;		// screen UI
		MATH3DMATRIX *		matVPUI;		// screen UI
		uint32				window_W;		// дополнительная память размера окна
		uint32				window_H;		// дополнительная память размера окна	
	protected:
		TBUFFER <TEXELEMENT, int32>  element;
		TBUFFER <TEXDATA,    int32>  texture;
	public:
		CTexProfile(const CTexProfile& src)				= delete;
		CTexProfile(CTexProfile&& src)					= delete;
		CTexProfile& operator=(CTexProfile&& src)		= delete;
		CTexProfile& operator=(const CTexProfile& src)	= delete;
	public:
		CTexProfile() : window_W(0), window_H(0)
		{
			matWorld  = new MATH3DMATRIX;
			matViewUI = new MATH3DMATRIX;
			matProjUI = new MATH3DMATRIX;
			matVPUI   = new MATH3DMATRIX;
			MathLookAtLHMatrix(MATH3DVEC(0,0,0), MATH3DVEC(0,0,1), MATH3DVEC(0,1,0), *matViewUI);
		};
		~CTexProfile()
		{
			Close();
			_DELETE(matWorld);
			_DELETE(matViewUI);
			_DELETE(matProjUI);
			_DELETE(matVPUI);
		}

		const TEXELEMENT * ElementBuffer()
		{
			return element.buf;
		}
		const TEXDATA    * TextureBuffer()
		{
			return texture.buf;
		}
		TEXELEMENT & Element(int32 id)
		{
			if (id >= element.count) _MBM(ERROR_IDLIMIT);
			return element[id];
		}
		TEXDATA    & Texture(int32 id)
		{
			if (id >= texture.count) _MBM(ERROR_IDLIMIT);
			return texture[id];
		}
		int32 ElementCount()
		{
			return element.count;
		}
		int32 TextureCount()
		{
			return texture.count;
		}

	protected:
		void DeleteTEX()
		{
			for (int32 i=0; i<texture.count; i++)
				texture[i].file.Close();
		}
	public:
		void Close()
		{
			if (isInit)
			{
				element.Close();
				texture.Close();
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
		};
		//>> Доступ к состоянию устройств ввода-вывода
		void SetIO(const CStatusIO * pIO)
		{
			if (pIO == nullptr) { _MBM(ERROR_PointerNone);  return; }

			IO = pIO;
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
		//>> Взятие номера элемента по имени :: возвращает MISSING (-1), если не найдено
		int32 GetElementID(const char* element_name)
		{
			for (int32 i=0; i<element.count; i++)
				if (!element[i].name.compare(element_name))
					return i;
			return MISSING;
		}
		//>> Взятие номера элемента по crc32 от имени :: возвращает MISSING (-1), если не найдено
		int32 GetElementID(const uint32 element_name_crc32)
		{
			for (int32 i = 0; i<element.count; i++)
				if (element[i].crc32.m_crc32 == element_name_crc32)
					return i;
			return MISSING;
		}
		//>> Взятие хеша по номеру элемента
		uint32 GetElementCRC32(int32 id)
		{
			if (id >= element.count) { _MBM(ERROR_IDLIMIT); return 0; }

			return element[id].crc32.m_crc32;
		}
	
	protected:
		void _GenerateVerticies(float width, float height, float scale_x, float scale_y, int32 ID)
		{
			TEXELEMENT & e = element[ID];
			float dx = static_cast<float>(e.dx);
			float dy = static_cast<float>(e.dy);

			if (e.CheckGetLast())
			{
				if (!width)   width   = e.last[0];
				if (!height)  height  = e.last[1];
				if (!scale_x) scale_x = e.last[2];
				if (!scale_y) scale_y = e.last[3];
			}
			else
			{
				if (!width)  width  = dx;		if (!scale_x) scale_x = 1.f;
				if (!height) height = dy;		if (!scale_y) scale_y = 1.f;
			}

			if (!e.centered)
			{
				FONTVERTEX & v1 = e.vbuf[1];	FONTVERTEX & v2 = e.vbuf[2];
				FONTVERTEX & v0 = e.vbuf[0];	FONTVERTEX & v3 = e.vbuf[3];

				//      1     2
				//    +-----+
				//    | 0   | 3
				//    +-----+
				
				v1.P.x = 0;						v2.P.x = width  * scale_x;
				v1.P.y = 0;						v2.P.y = 0;
				v1.P.z = NULL;					v2.P.z = NULL;
			
				v0.P.x = 0;						v3.P.x = width  * scale_x;
				v0.P.y = height * scale_y;		v3.P.y = height * scale_y;
				v0.P.z = NULL;					v3.P.z = NULL;
			}
			else
			{
				FONTVERTEX & v8 = e.vbuf[8];	FONTVERTEX & v1 = e.vbuf[1];	FONTVERTEX & v2 = e.vbuf[2];
				FONTVERTEX & v7 = e.vbuf[7];	FONTVERTEX & v0 = e.vbuf[0];	FONTVERTEX & v3 = e.vbuf[3];
				FONTVERTEX & v6 = e.vbuf[6];	FONTVERTEX & v5 = e.vbuf[5];	FONTVERTEX & v4 = e.vbuf[4];

				//      8     1     2           -H
				//    +-----+-----+              |
				//    | 7   | 0   | 3    -W      |      +W
				//    +-----+-----+        ------+------
				//    | 6   | 5   | 4            |
				//    +-----+-----+              | +H

				float width_cx  = width  * e.center_x;
				float height_cy = height * e.center_y;

				float width_L = -(width_cx)            * scale_x;
				float width_R = (width - width_cx)     * scale_x;

				float height_U = -(height - height_cy) * scale_y;
				float height_D = height_cy             * scale_y;

				v8.P.x = width_L;				v1.P.x = 0;						v2.P.x = width_R;
				v8.P.y = height_U;				v1.P.y = height_U;				v2.P.y = height_U;
				v8.P.z = NULL;					v1.P.z = NULL;					v2.P.z = NULL;

				v7.P.x = width_L;				v0.P.x = 0;						v3.P.x = width_R;
				v7.P.y = 0;						v0.P.y = 0;						v3.P.y = 0;
				v7.P.z = NULL;					v0.P.z = NULL;					v3.P.z = NULL;

				v6.P.x = width_L;				v5.P.x = 0;						v4.P.x = width_R;
				v6.P.y = height_D;				v5.P.y = height_D;				v4.P.y = height_D;
				v6.P.z = NULL;					v5.P.z = NULL;					v4.P.z = NULL;
			}

			e.cur_width  = width  * scale_x;
			e.cur_height = height * scale_y;

			e.last[0] = width;
			e.last[1] = height;
			e.last[2] = scale_x;
			e.last[3] = scale_y;

			e.ResetGetLast();
		}
		void  GenerateVerticies(float W, float H, float sx, float sy) // для первичного вызова
		{
			for (int32 i=0; i<element.count; i++)
				_GenerateVerticies(W, H, sx, sy, i);
		}
		void  GenerateVerticies(float W, float H, float sx, float sy, int32 ID)
		{
			if (ID < 0 || ID >= element.count) { _MBM(ERROR_IDLIMIT); return; }
			_GenerateVerticies(W, H, sx, sy, ID);
		}	
		void  GenerateVerticies(float W, float H, float sx, float sy, int32 from_ID, int32 to_ID)
		{
			if (from_ID > to_ID || from_ID < 0 || to_ID < 0 || from_ID >= element.count || to_ID > element.count)
				{ _MBM(ERROR_IDRANGE); return; }
			for (int32 i = from_ID; i<to_ID; i++)
				_GenerateVerticies(W, H, sx, sy, i);
		}
		void  SetVerticiesColor(const COLORVEC & color) // для первичного вызова
		{
			for (int32 i=0; i<element.count; i++)
			for (int32 n=0; n<9; n++)
				element[i].vbuf[n].C = color;
		}
		void  SetVerticiesColor(const COLORVEC & color, int32 ID)
		{
			if (ID < 0 || ID >= element.count) { _MBM(ERROR_IDLIMIT); return; }
			for (int i=0; i<9; i++)
				element[ID].vbuf[i].C = color;
		}
		void  SetVerticiesColor(const COLORVEC & color, int32 from_ID, int32 to_ID)
		{
			if (from_ID > to_ID || from_ID < 0 || to_ID < 0 || from_ID >= element.count || to_ID > element.count)
				{ _MBM(ERROR_IDRANGE); return; }
			for (int32 i=from_ID; i<to_ID; i++)
				for (int32 n=0; n<9; n++)
					element[i].vbuf[n].C = color;
		}

	protected:
		//>> Перезагрузка UV-координат в вершинном буфере :: для более сложного варианта с центрированием
		bool ReloadElementUV(int32 ID, bool is_centered, float cx, float cy)
		{
			if (ID < 0 || ID >= element.count) { _MBM(ERROR_IDLIMIT); return false; }

			TEXELEMENT & e = element[ID];

			if (e.centered == false && is_centered == false) return false;	// Лишние действия не нужны
			if (e.centered == true  && is_centered == true &&				// .
				e.center_x == cx    && e.center_y == cy) return false;		// .

			float sx = texture[e.TexID].scaleX;
			float sy = texture[e.TexID].scaleY;		e.TipGetLast();

			if (!is_centered)
			{                     
				FONTVERTEX & v0 = e.vbuf[0];	float ux_ = static_cast<float>(e.X);
				FONTVERTEX & v1 = e.vbuf[1];	float uy_ = static_cast<float>(e.Y);
				FONTVERTEX & v2 = e.vbuf[2];	float ux_dx = static_cast<float>(e.X + e.dx);
				FONTVERTEX & v3 = e.vbuf[3];	float uy_dy = static_cast<float>(e.Y + e.dy);

				//      1     2
				//    +-----+
				//    | 0   | 3
				//    +-----+

				v1.T.u = ux_   * sx;			v2.T.u = ux_dx * sx;
				v1.T.v = uy_dy * sy;			v2.T.v = uy_dy * sy;

				v0.T.u = ux_   * sx;			v3.T.u = ux_dx * sx;
				v0.T.v = uy_   * sy;			v3.T.v = uy_   * sy;

				e.centered = false;
				e.center_x = 0.f;
				e.center_y = 0.f;
			}
			else
			{
				cx = _CLAMP(cx, 0.f, 1.f);
				cy = _CLAMP(cy, 0.f, 1.f);

				//      8     1     2
				//    +-----+-----+
				//    | 7   | 0   | 3
				//    +-----+-----+
				//    | 6   | 5   | 4
				//    +-----+-----+

				FONTVERTEX & v8 = e.vbuf[8];	FONTVERTEX & v1 = e.vbuf[1];	FONTVERTEX & v2 = e.vbuf[2];
				FONTVERTEX & v7 = e.vbuf[7];	FONTVERTEX & v0 = e.vbuf[0];	FONTVERTEX & v3 = e.vbuf[3];
				FONTVERTEX & v6 = e.vbuf[6];	FONTVERTEX & v5 = e.vbuf[5];	FONTVERTEX & v4 = e.vbuf[4];

				float ux_   = static_cast<float>(e.X);			float ux_cdx = ux_ + cx * e.dx; // дельта до центра
				float uy_   = static_cast<float>(e.Y);			float uy_cdy = uy_ + cy * e.dy; // дельта до центра
				float ux_dx = static_cast<float>(e.X + e.dx);
				float uy_dy = static_cast<float>(e.Y + e.dy);

				v8.T.u = ux_    * sx;		v1.T.u = ux_cdx * sx;		v2.T.u = ux_dx  * sx;
				v8.T.v = uy_dy  * sy;		v1.T.v = uy_dy  * sy;		v2.T.v = uy_dy  * sy;

				v7.T.u = ux_    * sx;		v0.T.u = ux_cdx * sx;		v3.T.u = ux_dx  * sx;
				v7.T.v = uy_cdy * sy;		v0.T.v = uy_cdy * sy;		v3.T.v = uy_cdy * sy;

				v6.T.u = ux_    * sx;		v5.T.u = ux_cdx * sx;		v4.T.u = ux_dx  * sx;
				v6.T.v = uy_    * sy;		v5.T.v = uy_    * sy;		v4.T.v = uy_    * sy;

				e.centered = true;
				e.center_x = cx;
				e.center_y = cy;
			}

			return true;
		}
		//>> Задание UV-координат в вершинном буфере
		void SetElementsUV()
		{
			for (int32 i=0; i<element.count; i++)
			{
				TEXELEMENT & e = element[i];	float ux_ = static_cast<float>(e.X);			float sx = texture[e.TexID].scaleX;
				FONTVERTEX & v0 = e.vbuf[0];	float uy_ = static_cast<float>(e.Y);			float sy = texture[e.TexID].scaleY;
				FONTVERTEX & v1 = e.vbuf[1];	float ux_dx = static_cast<float>(e.X + e.dx);
				FONTVERTEX & v2 = e.vbuf[2];	float uy_dy = static_cast<float>(e.Y + e.dy);
				FONTVERTEX & v3 = e.vbuf[3];

				v1.T.u = ux_   * sx;			v2.T.u = ux_dx * sx;
				v1.T.v = uy_dy * sy;			v2.T.v = uy_dy * sy;

				v0.T.u = ux_   * sx;			v3.T.u = ux_dx * sx;
				v0.T.v = uy_   * sy;			v3.T.v = uy_   * sy;
			}
		}
		//>> Определение параметров текстуры
		void GetTextureProperties(TEXDATA & tex)
		{
			uint64 header = *(uint64*)tex.file.buf;

			switch (header)
			{
			case 0x0000007c20534444: break; // DDS |
			default: _MBM(ERROR_FormatNoSupport);
			}

			if (header == 0x0000007c20534444) // DDS |
			{
				tex.height = *(uint32*)&tex.file.buf[0x0C];
				tex.width = *(uint32*)&tex.file.buf[0x10];
				tex.scaleX = 1.f / static_cast<float>(tex.width);
				tex.scaleY = 1.f / static_cast<float>(tex.height);
			}
		}
		//>> Подсчёт элементов
		void CountTexProfile(byte* fpbuf, uint32 size, int32& elements, int32& textures)
		{
			elements = 0;
			textures = 0;

			byte* fpend = fpbuf + size;
			for (; fpbuf != fpend; fpbuf++)
			{
				CODESKIPCOMMENTS
				else if (_EQUAL(*fpbuf, ':'))
				{
					elements++;
					CODESKIPSTRING
				}
				else if (Compare(fpbuf, _SLEN("texture ")))
				{
					textures++;
					CODESKIPSTRING
				}
			}
		}
		//>> Загрузчик
		bool LoadProc(wchar_t* filepath)
		{
			TBUFFER <byte, int32> file;								//wprintf(L"\nReading profile %s", filepath);
			ReadFileInMemory(filepath, file, 0);					if (file.buf == nullptr) return false;
			byte* fpbuf = file.buf;
			byte* fpend = fpbuf + file.count;

			int32 curelm = -1;
			int32 curtex = -1;

			CountTexProfile(fpbuf, file.count, element.count, texture.count);
			wstring path = GetPrenamePath(filepath);

			//printf("\nprofile elements %i", element.count);
			//printf("\nprofile textures %i", texture.count);

			element.Create(element.count);
			texture.Create(texture.count);

			uint32 n;
			wchar_t wstr[256];
			char str[256];

			for (; fpbuf != fpend; fpbuf++)
			{
				CODESKIPCOMMENTS
				else if (_EQUAL(*fpbuf, ':')) // читаем параметры элемента
				{
					fpbuf++;
					CODESKIPSPACE
					for (n=0;; n++, fpbuf++) // custom CODEREADSTRINGVALUE
					{
							 if (_EQUAL(*fpbuf, 0x0A))           { break; } //0A     \n
						else if (_EQUAL(*fpbuf, 0x0D))  { fpbuf++; break; } //0D0A \r\n
						else if (_EQUAL(*fpbuf, '#'))            { break; } //
						else if (Compare(fpbuf, _SLEN("name "))) { break; } //
						str[n] = *fpbuf;
					}	str[n] = NULL;

					TEXELEMENT & elm = element[++curelm];
					sscanf_s(str, "X %i; Y %i; dx %i; dy %i; ", &elm.X, &elm.Y, &elm.dx, &elm.dy);

					fpbuf += strlen("name ");
					CODESKIPSPACE
					CODEREADSTRINGTEXT

					elm.name.assign(str);
					elm.crc32.ProcessCRC((void*)elm.name.c_str(), (int32)elm.name.size());
					elm.TexID = curtex;

					if (elm.X<0 || elm.Y<0 || elm.dx<0 || elm.dy<0) _MBM(ERROR_VARIABLE);
					//printf("\nelement %i = XY[%5i %5i] dXY[%5i %5i] %s", curelm, elm.X, elm.Y, elm.dx, elm.dy, elm.name.c_str());
				}
				else if (Compare(fpbuf, _SLEN("texture "))) // читаем имя текстуры и загружаем
				{
					fpbuf += strlen("texture ");
					CODESKIPSPACE
					CODEREADSTRINGWTEXT

					TEXDATA & tex = texture[++curtex];
					wsprintf(wstr, L"%s/%s", path.c_str(), (wchar_t*)str);
					ReadFileInMemory(wstr, tex.file, 1);
					GetTextureProperties(tex);
					//wprintf(L"\ntexture %s [W=%5i H=%5i]", str, tex.width, tex.height);
					//Makedump(tex.buf, tex.size, "C:\\TheGame\\data\\textures\\ui\\dump");
				}
			}
			SetElementsUV();
			return true;
		}

	public:	
		//>> Загрузчик
		bool Load(const wchar_t* gamePath, const wchar_t* directSubPath, const wchar_t* filename)
		{
			if (isInit) { _MBM(ERROR_InitAlready); return false; }

			wchar_t syspath[256], error[256], p1[256], p2[256];
			GetCurrentDirectory(256, syspath);

			wchar_t *p = nullptr;

			wsprintf(error, L"%s %s", ERROR_OpenFile, filename);
			wsprintf(p1, L"%s\\%s\\%s", gamePath, directSubPath, filename);
			wsprintf(p2, L"%s\\%s\\%s", syspath, directSubPath, filename);

			if (p = LoadFileCheck(2, p1, p2)) return isInit = LoadProc(p);
			else _MBM(error);

			return false;
		};
		//>> Загрузчик
		bool Load(const wchar_t* gamePath, const wchar_t* filename)
		{
			return Load(gamePath, DIRECTORY_TEXTURES, filename);
		};

	};

}

#endif // _TEXPROFILE_H