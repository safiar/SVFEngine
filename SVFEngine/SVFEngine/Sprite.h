// ----------------------------------------------------------------------- //
//
// MODULE  : Sprite.h
//
// PURPOSE : Отрисовка плоских объектов
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _SPRITE_H
#define _SPRITE_H

#include "TexProfileRe.h"
#include "Camera.h"

#define  SPRITE_UI_WARNING  L"UI sprite size/position is managed via UID class member"

namespace SAVFGAME
{
	enum eBillboardType // turn sprite to camera
	{
		EBT_NONE = 0,	// skip
		EBT_Y    = 1,	// rotate around Y     axis : yaw Y
		EBT_XY   = 2,	// rotate around X/Y   axis : yaw Y + pitch X
		EBT_XYZ  = 3,	// rotate around X/Y/Z axis : yaw Y + pitch X + roll Z
		EBT_Yp   = 4,	// rotate around Y     axis : yaw Y                     with position-looking fix
	};

	class CSprite : public CObject // UI sprites status managed via <UID> member
	{
	protected:
		shared_ptr<CTexProfileR>	profile_shr { nullptr };
		CTexProfileR *				profile     { nullptr };
		CBaseCamera *				camera      { nullptr };
		std::string					name;		// base element name
		vector <int32>				id;			// IDs of TexProfile elements
	protected:
		COLORVEC	color;			// current sprite color
		float		W, H, sx, sy;	// current scene (not UI) sprite data :: (UI sprite data managed at UID)
		float		initial[2];		// initial W / H
		float		center_x;		// relative polygon center x : left [0.f ... 1.f] right
		float		center_y;		// relative polygon center y :   up [0.f ... 1.f] down
		bool		centered;		// status of polygon if it have relative center		
	protected:
		uint16		timems;	// animation time, milliseconds
		uint16		timer;	// animation delta time to change frame, milliseconds :: // timer * num_frames == full animation time //
		uint16		frame;	// current frame
		uint16		frameA;	// first frame
		uint16		frameZ; // last frame
		uint16		count;  // total count of frames
	public:
		TEXUIDATA	UID; // UI sprite manager
		WPOS* const	UIP; // wpos of an UI sprite
		WPOS* const pos; // wpos of an scene sprite
	protected:	
		bool		UI;	 // is this an UI sprite ?
	protected:
		eBillboardType				billboard;    // how to turn sprite to player (not UI option)
		const MATH3DQUATERNION *	camera_RY;	  // ptr to camera rotation status Y
		const MATH3DQUATERNION *	camera_RXY;	  // ptr to camera rotation status XY
		const MATH3DQUATERNION *	camera_RZZXY; // ptr to camera rotation status ZZXY
		const MATH3DVEC *			camera_pos;
	private:
		timepoint					time;		  // program mono timepoint, milliseconds
		bool						isInit;
	public:
		CSprite(const CSprite& src)				= delete;
		CSprite(CSprite&& src)					= delete;
		CSprite& operator=(CSprite&& src)		= delete;
		CSprite& operator=(const CSprite& src)	= delete;
	public:
		CSprite() : CObject(), UIP(UID.pos), pos(CObject::pos)
		{
			Close();
		};
		~CSprite() override final { };
		void Close() override final
		{
			profile      = nullptr;
			profile_shr  = nullptr;
			camera       = nullptr;
			camera_RY    = nullptr;
			camera_RXY   = nullptr;
			camera_RZZXY = nullptr;
			camera_pos   = nullptr;
			name         = "";
			timems		 = 1000;
			timer		 = 1000;
			time         = _TIME;
			frame        = 0;
			frameA       = 0;
			frameZ       = 0;
			count        = 0;
			UI           = false;
			billboard    = eBillboardType::EBT_NONE;
			isInit       = false;
			center_x     = 0.5f;
			center_y     = 0.5f;
			centered     = false;
			pos->_default();
			id.erase(id.begin(), id.end());
			for (auto i : initial) i = 0;
			Reset();
		}
		bool IsInit()
		{
			return isInit;
		}

		shared_ptr<CTexProfileR> GetProfile()         { return profile_shr; }
		bool                     GetTypeUI()          { return UI;          }
		eBillboardType           GetTypeBillboard()   { return billboard;   }
		uint16                   GetAnimationTime()   { return timems;      }
		float                    GetWidthOrigin()     { return initial[0];  }
		float                    GetHeightOrigin()    { return initial[1];  }
		float                    GetWidth()           { return W;           }
		float                    GetHeight()          { return H;           }
		float                    GetScaleX()          { return sx;          }
		float                    GetScaleY()          { return sy;          }
		const COLORVEC &         GetColor()           { return color;       }

		void GetSize(float & width, float & height)
		{
			width  = W;
			height = H;
		}
		void GetScale(float & x_scale, float & y_scale)
		{
			x_scale = sx;
			y_scale = sy;
		}
		
	protected:
		//>> Инициализация (с анимацией) :: сборка всех ID от элементов, имеющих указанный <prefix>
		uint16 _InitAnimated(const shared_ptr<CTexProfileR> & pTexProfile, const char * element_name_prefix)
		{
			uint16 ret = 0;
			id.erase(id.begin(), id.end());

			auto prefix_size = strlen(element_name_prefix);		if (!prefix_size) return ret;

			auto element = const_cast<CTexProfile::TEXELEMENT*>(pTexProfile->ElementBuffer());

			int32 count = pTexProfile->ElementCount();

			for (int32 i=0; i<count; i++) // пройдёмся по всем элементам
			{
				auto & name = element[i].GetName();
				auto name_size = name.size();
				if (name_size <= prefix_size) continue;
				if (Compare(element_name_prefix, name.c_str(), prefix_size))
				{
					id.emplace_back(i);
					ret++;
				}
			}

			return ret;
		}
		//>> Инициализация (без анимации)
		bool _Init(const shared_ptr<CTexProfileR> & pTexProfile, const CBaseCamera * pCamera, const char * element_name, bool UI)
		{
			if (isInit)                    { _MBM(ERROR_InitAlready); return false; }
			if (pTexProfile    == nullptr) { _MBM(ERROR_PointerNone); return false; }
			if (!UI && pCamera == nullptr) { _MBM(ERROR_PointerNone); return false; }

			profile_shr  = pTexProfile;
			profile      = pTexProfile.get();
			name         = element_name;
			camera       = const_cast<CBaseCamera*>(pCamera);
			if (camera != nullptr)
			{
				camera_RY    = camera->GetCameraRotateY();
				camera_RXY   = camera->GetCameraRotateXY();
				camera_RZZXY = camera->GetCameraRotateZZXY();
				camera_pos   = camera->GetCameraPos();
			}
			else { camera_RY = camera_RXY = camera_RZZXY = nullptr; }
		
			id.erase(id.begin(), id.end());
			id.emplace_back( profile->GetElementID(element_name) );

			isInit = (__NOMISS(id[0]));

			if (isInit)
			{
				this->UI = UI;

				initial[0] = (float)profile->Element(id[0]).GetWidthOrigin();
				initial[1] = (float)profile->Element(id[0]).GetHeightOrigin();
				W = initial[0];
				H = initial[1];
				sx = sy = 1.f;
				frameA = frameZ = 1;

				if (!UI)
				{
					center_x = 0.5f;
					center_y = 0.5f;
					centered = true;
					profile->ResetProfileElement(0, 0, 0, 0, COLORVEC(CLR_white, 1), id[0]);
					profile->ResetProfileElement(centered, center_x, center_y, id[0]);
					billboard = eBillboardType::EBT_XY;
				}
				else
				{
					center_x = 0.f;
					center_y = 0.f;
					centered = false;
					profile->ResetProfileElement(0, 0, 0, 0, COLORVEC(CLR_white, 1), id[0]);
					profile->ResetProfileElement(centered, 0.f, 0.f, id[0]);
					billboard = eBillboardType::EBT_NONE;
				}
				count = 1;
			}
			else
			{
				count = 0;
				center_x = 0.f;
				center_y = 0.f;
				centered = false;
				W = H = 0;
				sx = sy = 1.f;
				billboard = eBillboardType::EBT_NONE;			
			}

			return isInit;
		}
		//>> Инициализация (с анимацией в окне времени для показа <time_ms>)
		bool _Init(const shared_ptr<CTexProfileR> & pTexProfile, const CBaseCamera * pCamera, const char * name_prefix,  bool UI, uint16 time_ms)
		{
			if (isInit)                    { _MBM(ERROR_InitAlready); return false; }
			if (pTexProfile    == nullptr) { _MBM(ERROR_PointerNone); return false; }
			if (!UI && pCamera == nullptr) { _MBM(ERROR_PointerNone); return false; }

			count = _InitAnimated(pTexProfile, name_prefix);
			if (count == 0) return isInit = false;

			profile_shr  = pTexProfile;
			profile      = pTexProfile.get();
			name         = name_prefix;
			camera       = const_cast<CBaseCamera*>(pCamera);
			if (camera != nullptr)
			{
				camera_RY    = camera->GetCameraRotateY();
				camera_RXY   = camera->GetCameraRotateXY();
				camera_RZZXY = camera->GetCameraRotateZZXY();
				camera_pos   = camera->GetCameraPos();
			}
			else { camera_RY = camera_RXY = camera_RZZXY = nullptr; }

			initial[0] = (float)profile->Element(id[0]).GetWidthOrigin();
			initial[1] = (float)profile->Element(id[0]).GetHeightOrigin();
			W = initial[0];
			H = initial[1];
			sx = sy = 1.f;
			frameA = 1;
			frameZ = count;

			this->UI = UI;

			if (!UI)
			{
				center_x = 0.5f;
				center_y = 0.5f;
				centered = true;
				profile->ResetProfileElement(0, 0, 0, 0, COLORVEC(CLR_white, 1), id[0]);
				Reset(centered, center_x, center_y, false);
				billboard = eBillboardType::EBT_XY;
			}
			else
			{
				center_x = 0.f;
				center_y = 0.f;
				centered = false;
				profile->ResetProfileElement(0, 0, 0, 0, COLORVEC(CLR_white, 1), id[0]);
				Reset(centered, 0.f, 0.f, false);			
				billboard = eBillboardType::EBT_NONE;
			}		

			ResetAnimationTime(time_ms);
			time  = _TIME;
			frame = 0;
			
			return isInit = true;
		}

	public:
		//>> Инициализация сценового спрайта (без анимации)
		bool Init(const shared_ptr<CTexProfileR> & pTexProfile, const CBaseCamera * pCamera, const char * element_name)
		{
			return _Init(pTexProfile, pCamera, element_name, false);
		}
		//>> Инициализация сценового спрайта (с анимацией в окне времени для показа <time_ms>)
		bool Init(const shared_ptr<CTexProfileR> & pTexProfile, const CBaseCamera * pCamera, const char * name_prefix, uint16 time_ms)
		{
			return _Init(pTexProfile, pCamera, name_prefix, false, time_ms);
		}
		//>> Инициализация UI спрайта (без анимации)
		bool Init(const shared_ptr<CTexProfileR> & pTexProfile, const char * element_name)
		{
			return _Init(pTexProfile, nullptr, element_name, true);
		}
		//>> Инициализация UI спрайта (с анимацией в окне времени для показа <time_ms>)
		bool Init(const shared_ptr<CTexProfileR> & pTexProfile, const char * name_prefix, uint16 time_ms)
		{
			return _Init(pTexProfile, nullptr, name_prefix, true, time_ms);
		}

	public:
		//>> Сброс [W/H/sx/sy/color] по умолчанию
		void Reset()
		{
			color = COLORVEC(CLR_white, 1.f);
			W  = initial[0];
			H  = initial[1];
			sx = 1.f;
			sy = 1.f;

			if (profile != nullptr && id.size() > 0)
				for (auto cid : id)
					if (_NOMISS(cid))
						profile->ResetProfileElement(W, H, sx, sy, color, cid);
		}
		//>> Изменение полигона :: set [W/H] to 0 for no changes
		void Reset(float width, float height)
		{
			if (!isInit) return;

			if (!width)   width   = W;		else W  = width;
			if (!height)  height  = H;		else H  = height;

			for (auto cid : id)
				if (_NOMISS(cid))
					profile->ResetProfileElement(W, H, sx, sy, color, cid);
		}
		//>> Изменение полигона :: set [W/H] to 0 for no changes
		void Reset(float width, float height, const COLORVEC & sprite_color)
		{
			if (!isInit) return;

			if (!width)   width = W;		else W = width;
			if (!height)  height = H;		else H = height;

			color = sprite_color;

			for (auto cid : id)
				if (_NOMISS(cid))
					profile->ResetProfileElement(W, H, sx, sy, color, cid);
		}
		//>> Изменение полигона (not for UI) :: set [W/H/sx/sy] to 0 for no changes
		void Reset(float width, float height, float scale_x, float scale_y, const COLORVEC & sprite_color)
		{
			if (!isInit) return;

			if (UI) { _MBM(SPRITE_UI_WARNING); return; }

			if (!width)   width   = W;		else W  = width;
			if (!height)  height  = H;		else H  = height;
			if (!scale_x) scale_x = sx;		else sx = scale_x;
			if (!scale_y) scale_y = sy;		else sy = scale_y;

			color = sprite_color;

			for (auto cid : id)
				if (_NOMISS(cid))
					profile->ResetProfileElement(W, H, sx, sy, color, cid);
		}
		//>> Изменение полигона (not for UI) :: set [W/H/sx/sy] to 0 for no changes
		void Reset(float width, float height, float scale_x, float scale_y)
		{
			if (!isInit) return;

			if (UI) { _MBM(SPRITE_UI_WARNING); return; }

			if (!width)   width   = W;		else W  = width;
			if (!height)  height  = H;		else H  = height;
			if (!scale_x) scale_x = sx;		else sx = scale_x;
			if (!scale_y) scale_y = sy;		else sy = scale_y;

			for (auto cid : id)
				if (_NOMISS(cid))
					profile->ResetProfileElement(W, H, sx, sy, color, cid);
		}
		//>> Изменение цвета
		void Reset(const COLORVEC & sprite_color)
		{
			if (!isInit) return;

			color = sprite_color;

			for (auto cid : id)
				if (_NOMISS(cid))
					profile->ResetProfileElement(W, H, sx, sy, color, cid);
		}
		//>> Изменение центра (not for UI) :: [0.0f, 0.0f] left-up   [1.0f, 1.0f] right-down 
		void Reset(bool centered, float cx, float cy, bool absolute_center_dxdy)
		{
			if (!isInit) return;
			if (UI) { _MBM(SPRITE_UI_WARNING); return; }

			this->centered = centered;

			if (centered)
			{
				if (absolute_center_dxdy)
					for (auto cid : id)
						if (_NOMISS(cid))
						{
							int32 dx, dy;
							profile->Element(cid).GetElementDXDY(dx, dy);
							cx /= dx;
							cy /= dy;
							break;
						}

				center_x = cx;
				center_y = cy;

				for (auto cid : id)
					if (_NOMISS(cid))
						profile->ResetProfileElement(true, center_x, center_y, cid);
			}
			else
			{
				center_x = 0;
				center_y = 0;

				for (auto cid : id)
					if (_NOMISS(cid))
						profile->ResetProfileElement(false, 0, 0, cid);
			}
		}
		//>> Изменение типа щита (not for UI)
		void Reset(eBillboardType eBT)
		{
			billboard = eBT;
		}
		//>> Изменение времени анимации
		void ResetAnimationTime(uint16 time_ms)
		{
			timems = (time_ms == 0) ? 1000 : time_ms;
			uint16 frames = (uint16)((count == 0) ? 1 : count);
			timer = timems / frames;
			if (!timer) timer = 1;
		}
		//>> Изменение текущего кадра :: from 1 to COUNT frames
		void ResetAnimationFrame(uint16 current_frame)
		{
			if (current_frame > count)
				{ _MBM(ERROR_IDLIMIT); return; }

			frame = (current_frame == 0) ? 1 : current_frame;
		}
		//>> Изменение диапазона отрисовываемых кадров :: [first > last] is correct as well
		void ResetAnimationFrame(uint16 first_frame, uint16 last_frame)
		{
			if (first_frame > count || last_frame > count)
				{ _MBM(ERROR_IDLIMIT); return; }

			frameA = (first_frame == 0) ? 1 : first_frame;
			frameZ = (last_frame  == 0) ? 1 : last_frame;
		}

	public:
		//>> Рендер спрайта
		void Show()
		{
			if (!isInit) { //_MBM(ERROR_InitNone);
			               return; }

			if (count == 0) return;

			if (!UI)
				switch (billboard)
				{
				case eBillboardType::EBT_NONE: break;
				case eBillboardType::EBT_Y:    pos->Q = *camera_RY;     break;
				case eBillboardType::EBT_XY:   pos->Q = *camera_RXY;    break;
				case eBillboardType::EBT_XYZ:  pos->Q = *camera_RZZXY;  break;
				case eBillboardType::EBT_Yp:   { MATH3DVEC to_camera(*camera_pos - pos->P);  // -x и -z поправка перевёрнутости
												 pos->Q = MATH3DQUATERNION(DF_VEC, MATH3DVEC(-to_camera.x, 0, -to_camera.z));
												 break; }
				}

			if (count == 1 && __NOMISS(id[0]))
			{
				if (!UI) profile->Show(id[0], pos, UI);
				else     profile->Show(id[0], UIP, UI);
				return;
			}

			// count > 1 : animated sprite //

			if (_TIMER(time) > timer || frame == 0)
			{
				time = _TIME;
				frame++;
				if (frame >= count) frame = 1;
				if (frameA < frameZ)										
					 { if (frame < frameA || frame > frameZ) frame = frameA; }
				else { if (frame > frameZ || frame < frameA) frame = frameA; }
			}

			auto CID = id[frame-1];
			if (_NOMISS(CID))
			{
				if (!UI) profile->Show(CID, pos, UI);
				else     profile->Show(CID, UIP, UI);
			}
		}

	};
};

#endif // _SPRITE_H