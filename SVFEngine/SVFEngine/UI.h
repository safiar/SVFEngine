// ----------------------------------------------------------------------- //
//
// MODULE  : UI.h
//
// PURPOSE : Управление элементами интерфейса
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _UI_H
#define _UI_H

#include "stdafx.h"
#include "title.h"
#include "version_s.h"

#include "Shader.h"
#include "FontRe.h"
#include "TexProfileRe.h"
#include "Sprite.h"
#include "Sound.h"
#include "Network.h"

#define  color_default		CLR_white
#define  color_inactive     CLR_grey				// interactive text coloring
#define  color_active       CLR_white				// interactive text coloring
#define  color_targeted     CLR_lightsteelblue		// interactive text coloring
#define  color_clicked      CLR_gold				// interactive text coloring
#define  color_chat_hst     CLR_indigo				// chat history text coloring
#define  color_chat_msg     CLR_black				// chat message text coloring
#define  color_cursor_game  CLR_olive
#define  color_cursor_menu  CLR_white

#define  CHATMSGSZ			128				// wchar_t, chat message size
#define  CHATHSTSZ			CHATMSGSZ * 64	// wchar_t, chat history size
#define  CHATHSTSZ_CLEAR	CHATMSGSZ * 16	// wchar_t, chat history size for clearing

#define  SVFENGINELOGO1		"SVF_LOGO_1"
#define  BACKGROUNDSHADOW   "SCREEN_SHADOW"  // затенение
#define  CHATWINDOWNAME     "CONSOLE_WINDOW" // name of texture's element
#define  LOADSCREEN0NM0		"LOADING_ARROW"  // name of texture's element
#define  LOADSCREEN0NM1		"FIRE_"          // name of texture's element
#define  LOADSCREEN0TX0		L"Загрузка"
#define  LOADSCREEN0TX1		L"Загрузка."
#define  LOADSCREEN0TX2		L"Загрузка.."
#define  LOADSCREEN0TX3		L"Загрузка..."
#define  TIPPICKROTATE		"ROTATING_ARROW"
#define  TIPPICKMOVE		"MOVING_ARROW"
#define  TIPPICKROTATETX	L"ЛКМ вращать"
#define  TIPPICKMOVETX		L"ПКМ перемещать"
#define  CURSORGAME1		"CURSOR_GAME_1"
#define  CURSORMENU1		"CURSOR_MENU_1"

#define  BASE_SCREEN_WIDTH  1024	// default
#define  BASE_SCREEN_HEIGHT 768		// default

#define  BASE_UI_SOUND_GROUP 1

namespace SAVFGAME
{
	enum eUIFontID
	{
		// ACTUALLY UI FONTS //

		FONT_DEFAULT,
		FONT_LOADING_SCREEN,
		FONT_ESCAPE_MENU,
		FONT_FPS,
		FONT_CHAT,

		// NOT ACTUALLY UI FONTS //

		FONT_PLNAME_SCENE,

		FONT_ENUM_MAX
	};
	enum eUISoundID
	{
		UISOUND_LOADING,			// loading screen (loop)
		UISOUND_TARGETED,			// if targeted on screen
		UISOUND_CLICKED,			// if clicked on screen
		UISOUND_CHAT_INCOMING,		// incoming chat message
	//	UISOUND_WAKEDREAM_TEST,		// test

		UISOUND_ENUM_MAX
	};
	
	enum eClickEvent
	{
		CLICK_EVENT_NONE,
		CLICK_EVENT_EXIT_GAME,		// exit from game
		CLICK_EVENT_FULLSCREEN,		// turn into fullscreen mode
		CLICK_EVENT_FULLWINDOW,		// turn into fullwindow mode
		CLICK_EVENT_WINDOWED,		// turn into windowed mode
		CLICK_EVENT_SV_CONNECT,		// connect to / disconnect from _ server
		CLICK_EVENT_SV_WAKEUP,		// wakeup / shutdown _ server

		ENUM_CLICK_EVENT_MAX
	};
	
	class CUIBase
	{
	private:
		CText *	custom_text;
	public:
		byte						mark { 0 };	// для разного
	protected:
		CShader *					shader { nullptr };
		CStatusIO *					IO     { nullptr };
		bool						isInit { false };
	protected:
		uint32						base_screen_W { BASE_SCREEN_WIDTH  }; // виртуальный проторазмер окна
		uint32						base_screen_H { BASE_SCREEN_HEIGHT }; // виртуальный проторазмер окна
	protected:
		VECDATAP   <CFontR>			font;				  // шрифты
		shared_ptr <CTexProfileR>	base_shr;			  // графические элементы
		shared_ptr <CSound>			sound_shr;			  // звуковые эффекты
		CTexProfileR *				base     { nullptr }; // графические элементы
		CSound *					sound    { nullptr }; // звуковые эффекты
		vector <uint64_gid>         sound_id;             // звуковые эффекты (eUISoundID IDs)
		CStatusNetwork *			net      { nullptr }; // статус сети
		CFontR *					font_default { nullptr };
	public:
		CUIBase(const CUIBase& src)				= delete;
		CUIBase(CUIBase&& src)					= delete;
		CUIBase& operator=(CUIBase&& src)		= delete;
		CUIBase& operator=(const CUIBase& src)	= delete;
	protected:
		CUIBase()
		{
			custom_text = new CText;
		}
		virtual ~CUIBase()
		{
			Close();
			_DELETE(custom_text);
		}
		virtual void Close()
		{
			mark = 0;
			base_screen_W = BASE_SCREEN_WIDTH;
			base_screen_H = BASE_SCREEN_HEIGHT;
			font.Delete(1);
			base_shr  = nullptr; //_CLOSESHARED(base);
			sound_shr = nullptr; //_CLOSESHARED(sound);
			base      = nullptr;
			sound     = nullptr;
			shader    = nullptr;
			isInit    = false;
		}
	public:
		//>> Will not affected to already loaded interface
		void SetBaseScreen(uint32 width, uint32 height)
		{
			if (isInit) return;

			base_screen_W = width;
			base_screen_H = height;
		};
		//>> Will not affected to already loaded interface
		void InitUI(const CShader * pShader, const CStatusIO * io, const CStatusNetwork * pNetStatus,
			VECDATAP<CFontR> & vecFont, shared_ptr<CTexProfileR> pTexbase, const CSound * pSound)
		{
			if (isInit) return;

			if (!SetShader(pShader)) return;
			if (!SetIO(io)) return;
			if (!SetFonts(vecFont)) return;
			if (!SetTexBase(pTexbase)) return;
			if (!SetSound(pSound)) return;
			if (!SetNetwork(pNetStatus)) return;

			SetSoundIDs();

			isInit = true;
		}

	private:
		//>> Доступ к шейдерам
		bool SetShader(const CShader * pShader)
		{
			if (pShader == nullptr) { _MBM(ERROR_PointerNone); return false; }
			shader = const_cast<CShader*>(pShader);
			return true;
		}
		//>> Доступ к состоянию устройств ввода-вывода
		bool SetIO(const CStatusIO * io)
		{
			if (io == nullptr) { _MBM(ERROR_PointerNone); return false; }
			IO = const_cast<CStatusIO*>(io);
			return true;
		}
		//>> Установка шрифтов
		bool SetFonts(VECDATAP<CFontR> & vecFont)
		{
			if (vecFont.size() < FONT_ENUM_MAX)
				{ _MBM(ERROR_BUFLIMIT); return false; }
			font.Set(FONT_ENUM_MAX);
			for (int i=0; i<FONT_ENUM_MAX; i++)
			{
				if (vecFont[i] == nullptr)
					{ _MBM(ERROR_PointerNone); return false; }
				font.Reset(vecFont[i], i);
			}
			font_default = font[FONT_DEFAULT].get();
			return true;
		}
		//>> Установка базовых текстурных элементов
		bool SetTexBase(shared_ptr<CTexProfileR> pTexbase)
		{
			if (pTexbase == nullptr) { _MBM(ERROR_PointerNone); return false; }
			base_shr = pTexbase;
			base = base_shr.get();
			return true;
		}
		//>> Установка звуков
		bool SetSound(const CSound * pSound)
		{
			if (pSound == nullptr) { _MBM(ERROR_PointerNone); return false; }
			sound = const_cast<CSound*>(pSound);
			return true;
		}
		//>> Для определения текущего статуса сети (клиент/сервер)
		bool SetNetwork(const CStatusNetwork * pNetStatus)
		{
			if (pNetStatus == nullptr) { _MBM(ERROR_PointerNone); return false; }
			net = const_cast<CStatusNetwork*>(pNetStatus);
			return true;
		}
		//>> Определение фактических GID звуков
		void SetSoundIDs()
		{
			error_t     hRes;
			wstring     sound_formal_name;
			uint64_gid  GID;

			sound_id.resize(eUISoundID::UISOUND_ENUM_MAX);

			for (uint32 i = 0; i < UISOUND_ENUM_MAX; i++)
			{
				sound_id[i].element = eSoundGroup_ELEMENT_MAX;
				sound_id[i].group   = eSoundGroup_USER_MAX;

				switch (i)
				{
					case UISOUND_LOADING:       sound_formal_name = L"UISOUND_LOADING";       break;
					case UISOUND_TARGETED:      sound_formal_name = L"UISOUND_TARGETED";      break;
					case UISOUND_CLICKED:       sound_formal_name = L"UISOUND_CLICKED";       break;
					case UISOUND_CHAT_INCOMING: sound_formal_name = L"UISOUND_CHAT_INCOMING"; break;
					default:
						continue;
				}

				hRes = sound->FormalNameFind(sound_id[i], sound_formal_name);

				if (hRes != eSoundError_TRUE)
				{
					wprintf(L"\nUI error : can't find sound with formal name %s", sound_formal_name.c_str());
				}
			}
		}
	public:
		//>> TEST: (Временно) Для внешнего пользования звуковой базой UI
		CSound * GetSound() { return sound; }
	protected:
		//>> Check interactive element's targeting condition
		bool IsTargeted(int32 x, int32 y, int32 dx, int32 dy)
		{
			int min_x = min(x, x + dx);
			int max_x = max(x, x + dx);
			int min_y = min(y, y + dy);
			int max_y = max(y, y + dy);
			//	printf("\nPOS: X %4i Y %4i [%4i %4i %4i %4i]", cur_x, cur_y, min_x, min_y, max_x, max_y);
			if (IO->mouse.client_x >= min_x && IO->mouse.client_x <= max_x &&
				IO->mouse.client_y >= min_y && IO->mouse.client_y <= max_y) return true;
			return false;
		}
	/*	//>> Proceed interactive text on screen
		eUIClickStatus DrawClickableText(CText * text, CFontR * font)
		{
			eUIClickStatus ret;
			bool ct = text->IsTextConstant();

			text->UpdateUI(font, IO->window.client_width, IO->window.client_height, false);

			auto mem_mark = text->mark;
			const WPOS * tpos = text->pos;
			bool not_active = (text->interactive == eUIClickStatus::CLICK_NONE);

			if (not_active)       { ret = CLICK_MISSED;   text->color1 = COLORVEC(color_inactive, 1); text->mark = CLICK_MISSED; }
			else
			{
				if (IsTargeted((int32)tpos->x, (int32)tpos->y, (int32)text->GetPolygonWidth(0), (int32)text->GetPolygonHeight(0)))
				{
					if (IO->mouse.lb) { ret = CLICK_CLICKED;  text->color1 = COLORVEC(color_clicked,  1); text->mark = CLICK_CLICKED;  }
					else              { ret = CLICK_TARGETED; text->color1 = COLORVEC(color_targeted, 1); text->mark = CLICK_TARGETED; }
				}   else              { ret = CLICK_MISSED;   text->color1 = COLORVEC(color_active,   1); text->mark = CLICK_MISSED;   }
			}

			if (_NOTEQUAL(mem_mark, text->mark) && ct) // Перезагрузка цвета текста
				text->CallReloadConstantText();

			font->ShowText(text);
			return ret;
		} //*/
		//>> Proceed interactive text on screen
		eClickStatus DrawClickableText(CText * text, CFontR * font)
		{
			eClickStatus ret = CLICK_NO_CHANGES;
			const WPOS * p = text->pos;
			bool ch = false;

			text->UpdateUI(font, IO->window.client_width, IO->window.client_height, false);

			if (!text->ClickIsOn())
			{ 
				if (text->ClickIsChanged() && text->IsTextConstant()) // Перезагрузка цвета текста
					text->CallReloadConstantText();

						 { ret = CLICK_MISSED;   text->color1 = COLORVEC(color_inactive, 1); ch = text->ClickChange(CLICK_OFF); }
			}
			else
			{
				if (IsTargeted((int32)p->x, (int32)p->y, (int32)text->GetPolygonWidth(0), (int32)text->GetPolygonHeight(0)))
				{
					if (IO->mouse.lb)
					{
						if (IO->keyboard.key[eKeyBoard::K_LBUTTON].is_active())
						{
							{ ret = CLICK_CLICKED;  text->color1 = COLORVEC(color_clicked, 1);  ch = text->ClickChange(CLICK_CLICKED);  }
						}
						else
						{
							if (text->ClickGet() != CLICK_CLICKED)
							{ ret = CLICK_TARGETED; text->color1 = COLORVEC(color_targeted, 1); ch = text->ClickChange(CLICK_TARGETED); }
						}
					}
					else { ret = CLICK_TARGETED; text->color1 = COLORVEC(color_targeted, 1); ch = text->ClickChange(CLICK_TARGETED); }
				}   else { ret = CLICK_MISSED;   text->color1 = COLORVEC(color_active,   1); ch = text->ClickChange(CLICK_MISSED);   }

				if (ch && text->IsTextConstant()) // Перезагрузка цвета текста
					text->CallReloadConstantText();
			}

			if (!ch) ret = CLICK_NO_CHANGES;

			font->ShowText(text);
			return ret;
		}
		//>> TODO: interactive resizable sprite
		eClickStatus DrawClickableElement()
		{
			//
		}
		//>> TODO: scene 3D text
		eClickStatus DrawClickable3DText()
		{

		}
		//>> TODO: scene 3D sprite
		eClickStatus DrawClickable3DElement()
		{

		}
	public:
		//>> Quick text :: (x,y) < 0 is valid
		void ShowInformation(wstring & text, float x, float y, float height, COLORVEC & color)
		{
			bool force_update = false;
			auto cur_text_len = custom_text->text.size();
			auto new_text_len = text.size();

			if (height < 0) height = abs(height);
			if (x < 0) x += base_screen_W;
			if (y < 0) y += base_screen_H;

			if ( (cur_text_len != new_text_len)                                       ||
			     (!Compare(custom_text->text.c_str(), text.c_str(), cur_text_len))    ||
			     (custom_text->GetInitPosX() != x || custom_text->GetInitPosY() != y) ||
			     (custom_text->GetInitTextHeight() != height)
			) force_update = true;

			if (force_update)
			{
				custom_text->text = text;
				custom_text->color1 = color;
				custom_text->InitUI(font_default, base_screen_W, base_screen_H, height,
					x, y, CALC_XT_FROM_LEFT, CALC_YT_FROM_TOP, SCALE_TALL, false, false);
				custom_text->UpdateUI(font_default, IO->window.client_width, IO->window.client_height, true);
			}
			else
				custom_text->UpdateUI(font_default, IO->window.client_width, IO->window.client_height, false);

			font_default->ShowText(custom_text);
		}
		//>> Quick text :: (x,y) < 0 is valid
		void ShowInformation(wstring & text, float x, float y, float height)
		{
			ShowInformation(text, x, y, height, COLORVEC(CLR_red, 1.f));
		}
	};
	class CUIEscapeMenu : public virtual CUIBase
	{
		struct EventText
		{
			EventText()
			{
				text    = new CText;
				command = CLICK_EVENT_NONE;
			}
			EventText(eClickEvent activated_event)
			{
				text    = new CText;
				command = activated_event;
			}
			~EventText()
			{
				_DELETE(text);
			}
			CText *		text;
			eClickEvent	command;
		};
	private:
		CFontR *	font   { nullptr };
		bool		isInit { false };
	private:
		CSprite						background;
		TBUFFER <EventText, byte>	text;
		byte						nNewGame;
		byte						nConnect;
		byte						nWakeup;
		byte						nFullscreen;
		byte						nFullwindow;
		byte						nWindowed;
		byte						nExit;
	private:
		bool		escape_menu_switch { false };	// хранит вызов на переключение
		bool		escape_menu        { false };	// статус включения
	protected:
		CUIEscapeMenu() : CUIBase() { }
		virtual ~CUIEscapeMenu()  override { Close(); }
	private:
		void Init()
		{
			if (isInit) return;
			if (!CUIBase::isInit) { _MBM(ERROR_InitNone); return; }

			font = CUIBase::font[FONT_ESCAPE_MENU].get();

			const float textH = 30.f;
			const float initX = 5.f;
			const float initY = textH + 5.f; // 1st top text position
			const eUITextCalcTypeX tX = CALC_XT_FROM_LEFT;
			const eUITextCalcTypeY tY = CALC_YT_FROM_TOP;
			const eUITextScaleType tS = SCALE_TALL;
			const bool fixedW = false;
			const bool fixedH = false;

			nNewGame    = 0;
			nConnect    = 1;
			nWakeup     = 2;
			nFullscreen = 3;
			nFullwindow = 4;
			nWindowed   = 5;
			nExit       = 6;

			text.Create(7);

			text[nNewGame].command    = eClickEvent::CLICK_EVENT_NONE;
			text[nConnect].command    = eClickEvent::CLICK_EVENT_SV_CONNECT;
			text[nWakeup].command     = eClickEvent::CLICK_EVENT_SV_WAKEUP;
			text[nFullscreen].command = eClickEvent::CLICK_EVENT_FULLSCREEN;
			text[nFullwindow].command = eClickEvent::CLICK_EVENT_FULLWINDOW;
			text[nWindowed].command   = eClickEvent::CLICK_EVENT_WINDOWED;
			text[nExit].command       = eClickEvent::CLICK_EVENT_EXIT_GAME;

			*text[nNewGame].text    = L"Новая игра";			text[nNewGame].text->ClickChange(CLICK_OFF);
			*text[nConnect].text    = L"Присоединиться";
			*text[nWakeup].text     = L"Включить сервер";
			*text[nFullscreen].text = L"Полный экран";
			*text[nFullwindow].text = L"Полное окно";
			*text[nWindowed].text   = L"В окне";
			*text[nExit].text       = L"Выход";

			for (byte i=0; i<text.count; i++)
				text[i].text->InitUI (font, base_screen_W, base_screen_H, textH, initX, initY * i + base_screen_H/2, tX, tY, tS, fixedW, fixedH);

			background.Init(base_shr, BACKGROUNDSHADOW);
			background.Reset(COLORVEC(CLR_white, 1.f));
			background.UID.InitUI(
				base_screen_W, base_screen_H,
				0, 0,
				background.GetWidthOrigin(),
				background.GetHeightOrigin(),
				(float)base_screen_W / background.GetWidthOrigin(),
				(float)base_screen_H / background.GetHeightOrigin(),
				CALC_X_FROM_LEFT, CALC_Y_FROM_TOP, SCALE_ALL);

			isInit = true;
		}
	protected:
		virtual void Close() override
		{
			if (!isInit) return;

			text.Close();

			font               = nullptr;
			escape_menu        = false;
			escape_menu_switch = false;

			isInit = false;
		}
	public:
		void EscapeMenuSwitch()				{ escape_menu_switch = true; }
		bool EscapeMenuSwitchCheck()		{ _RETBOOL(escape_menu_switch); }
		bool EscapeMenuGetStatus()			{ return escape_menu; }
		void EscapeMenuSetStatus(bool ON)	{ escape_menu = ON; escape_menu_switch = false; }
	public:
		//>> Show escape menu
		eClickEvent EscapeMenu()
		{
			if (!isInit) Init();

			eClickEvent	 user_command = CLICK_EVENT_NONE;
			eClickStatus status       = CLICK_NO_CHANGES;
			eClickStatus status_temp  = CLICK_NO_CHANGES;

			background.UID.UpdateUI ( IO->window.client_width, IO->window.client_height, false ); 
			background.Show();

			for (int i=0; i<text.count; i++)
			{
				status_temp = DrawClickableText(text[i].text, font);
				switch (status_temp)
				{
				case CLICK_TARGETED: status = CLICK_TARGETED;                                 break;
				case CLICK_CLICKED:  status = CLICK_CLICKED;  user_command = text[i].command; break;
				}
			}

			const uint32 eSRS = eSoundRunAsync;// | eSoundRunMix;
			switch (status)
			{
			case CLICK_TARGETED: sound->RunThreadPlay(sound_id[UISOUND_TARGETED], eSRS); return CLICK_EVENT_NONE;
			case CLICK_CLICKED:  sound->RunThreadPlay(sound_id[UISOUND_CLICKED],  eSRS); return user_command;
			}

			return CLICK_EVENT_NONE;
		}
		void EscapeMenuChangeConnectText(bool disconnect)
		{
			if (disconnect)
				 { *text[nConnect].text = L"Отсоединиться";  text[nWakeup].text->ClickChange(CLICK_OFF); }
			else { *text[nConnect].text = L"Присоединиться"; text[nWakeup].text->ClickChange(CLICK_ON); }
			
			text[nConnect].text->UpdateUI ( font, IO->window.client_width, IO->window.client_height, true );
		}
		void EscapeMenuChangeWakeupText(bool shutdown)
		{
			if (shutdown)
				 { *text[nWakeup].text = L"Выключить сервер"; text[nConnect].text->ClickChange(CLICK_OFF); }
			else { *text[nWakeup].text = L"Включить сервер";  text[nConnect].text->ClickChange(CLICK_ON);  }

			text[nWakeup].text->UpdateUI(font, IO->window.client_width, IO->window.client_height, true);
		}
	};
	class CUIMonitorFPS : public virtual CUIBase
	{
	private:
		CFontR *	font;
		bool		isInit;
	private:
		CText *		text;
		uint32		frames;
		clock_t		time;
		clock_t		ttime;
		float		fps;
		wchar_t		fpsstr[64];		
	protected:
		CUIMonitorFPS() : CUIBase(), font(nullptr), isInit(false)
		{
			text = new CText;
		}
		virtual ~CUIMonitorFPS() override
		{
			Close();
			_DELETE(text);
		}
	private:
		void Init()
		{
			if (isInit) return;
			if (!CUIBase::isInit) { _MBM(ERROR_InitNone); return; }

			font = CUIBase::font[FONT_FPS].get();

			const float textH = 30.f;
			const float initX = 5.f;
			const float initY = 5.f;
			const eUITextCalcTypeX tX = CALC_XT_FROM_LEFT;
			const eUITextCalcTypeY tY = CALC_YT_FROM_TOP;
			const eUITextScaleType tS = SCALE_TALL;
			const bool fixedW = false;
			const bool fixedH = false;

			*text = L"";
			text->InitUI(font, base_screen_W, base_screen_H, textH, initX, initY, tX, tY, tS, fixedW, fixedH);

			frames = -1;
			time = 1000 + 1;
			ttime = clock();
			fps = 0;

			isInit = true;
		}
	protected:
		virtual void Close() override
		{
			if (!isInit) return;
			font = nullptr;
			isInit = false;
		}
	public:
		//>> Show FPS info
		void IndicateFPS()
		{
			if (!isInit) Init();

			frames++;
			time += clock() - ttime;
			ttime = clock();

			bool reset = false;

			if (time > 1000)
			{
				fps = 1000 * static_cast<float>(frames) / static_cast<float>(time);
				time = 0;
				frames = 0;
				reset = true;
				swprintf_s(fpsstr, L"FPS: %6.2f", fps);
			}

			if (reset || text->UpdateUI (font, IO->window.client_width, IO->window.client_height, false))
			{
				text->text.assign(fpsstr);
				text->CallReloadConstantText();
				reset = false;
			}

			font->ShowText(text);
		}
	};
	class CUILoadingScreen : public virtual CUIBase
	{
	private:
		CFontR *	font { nullptr };
		bool		isInit;
	private: /////////// БЛОК УПРАВЛЕНИЯ ЗАГРУЗКАМИ
		THREADDATA	loading_thread;		// поток для загрузки
		bool		loading_call;		// вызов на загрузку
		bool		loading_screen;		// метка для показа загрузочного экрана
		wstring		loading_mapname;	// имя карты на загрузку
		uint32		loading_mapid;		// id карты на загрузку
	public:
		//>> Сообщает имя файла карты на загрузку
		const wchar_t* LoadingGetMapNameToLoad()
		{
			return loading_mapname.c_str();
		}
		//>> Сообщает в какой ID набора карт загружать
		uint32 LoadingGetMapIDToLoad()
		{
			return loading_mapid;
		}
		//>> Вызов загрузки, указываем имя файла и в какой ID общего набора карт будем загружать
		void LoadingCall(const wchar_t* mapname, const uint32 mapid)
		{
			loading_call = true;
			loading_mapname = mapname;
			loading_mapid = mapid;
		}
		//>> Проверка был ли вызов на запуск загрузочного потока
		bool LoadingCallCheck()
		{
			if (!loading_call) return false;
			else {	loading_call = false;
					return true;			}
		}
		//>> Проверка надо ли показывать загрузочный экран
		bool LoadingScreenCheck()
		{
			return loading_screen;
		}
		//>> Проверка окончания загрузочного потока
		bool LoadingThreadCheckIsFinished()
		{
			bool ret = loading_thread.IsFinished();
			if (ret)
			{
				loading_thread.Join(true);
				loading_call    = false;
				loading_screen  = false;
				loading_mapname = L"MISSING MAP NAME TO LOAD";	// сброс
				loading_mapid   = 0;							// сброс

				sound->ControlInterrupt(sound_id[UISOUND_LOADING], true);  // завершаем фоновый звук
			}
			return ret;
		}
		template<class TFUNC, class TCLASS, class ...TARGS>
		//>> Запуск загрузочной функции в поток
		bool LoadingRunThread(TFUNC&& ptr_function, TCLASS&& ptr_class, TARGS&&... args)
		{
			auto ret = loading_thread ( ptr_function, ptr_class, args... );
			if (ret == TC_SUCCESS)
			{
				loading_call = false;
				loading_screen = true;
				loading_mapname = L"MISSING MAP NAME TO LOAD";	// сброс
				loading_mapid = 0;								// сброс

				sound->ControlInterrupt(SOUND_ALL, true);                       // прерываем прочие
				sound->RunThreadPlay(sound_id[UISOUND_LOADING], eSoundRunLoop); // играем фоновый звук
			}
			else _MBM(ERROR_THREAD);
			return (ret == TC_SUCCESS);
		}
	private: /////////// БЛОК ЗАГРУЗОЧНОГО ЭКРАНА	
		VECPDATA<CText> text;
		CSprite			logo;
		CSprite			arrow;
		CSprite			fire;
		timepoint		time;
	private: /////////////////// Scheme 0
		float		fire_width;
		float		fire_height;
		float		arrow_cur_height;
		const float update_speed = 0.1f;
		const float scale_pos_fire_x = +0.5f;
		const float scale_pos_fire_y = -0.2f;
		const float arrow_scale = 0.4f;
		const float text_height = 30.f;
		const float rotate_center_fix_X = 12.f;
		const float rotate_center_fix_Y = 0.f;
		const byte  IDLOGO  = 0;
		const byte  IDARROW = 1;
		const byte  IDFIRE  = 2;
	protected:
		CUILoadingScreen() : CUIBase(), loading_thread(), isInit(false), loading_screen(false), loading_call(false), loading_mapid(0)
		{
			loading_mapname = L"MISSING MAP NAME TO LOAD";
			text.Create(4);
		}
		virtual ~CUILoadingScreen() override
		{
			Close();
			text.Delete(true);
		}
	private:
		void Init()
		{
			if (isInit) return;

			if (!CUIBase::isInit) { _MBM(ERROR_InitNone); return; }

			font = CUIBase::font[FONT_LOADING_SCREEN].get();

			logo.Init(base_shr, SVFENGINELOGO1);
			arrow.Init(base_shr, LOADSCREEN0NM0);
			fire.Init(base_shr, LOADSCREEN0NM1, 1300);

			fire_width  = fire.GetWidthOrigin();
			fire_height = fire.GetHeightOrigin();

			float arrow_width  = arrow.GetWidthOrigin();
			float arrow_height = arrow.GetHeightOrigin();
			float arrow_height_scaled = arrow_height * arrow_scale;					arrow_cur_height = arrow_height_scaled;

			float logo_width  = logo.GetWidthOrigin();
			float logo_height = logo.GetHeightOrigin();

			arrow.UID.InitUI(
				base_screen_W, base_screen_H,
				(float)base_screen_W, (float)base_screen_H,
				arrow_width, arrow_height,
				arrow_scale, arrow_scale,
				CALC_X_FROM_RIGHT, CALC_Y_FROM_BOTTOM, SCALE_MIN);
			arrow.UID.InitRotateCenter(
				rotate_center_fix_X + 0.5f * arrow_width,
				rotate_center_fix_Y + 0.5f * arrow_height);

			fire.UID.InitUI(base_screen_W, base_screen_H,
				(0.5f * base_screen_W) - (scale_pos_fire_x * fire_width),
				(0.5f * base_screen_H) - (scale_pos_fire_y * fire_height),
				fire_width, fire_height,
				1.f, 1.f,
				CALC_X_FROM_LEFT, CALC_Y_FROM_BOTTOM, SCALE_MAX);

			logo.UID.InitUI(base_screen_W, base_screen_H, (float)base_screen_W / 2, (float)base_screen_H / 2,
				logo_width, logo_height, 1, 1, CALC_X_FROM_CENTER, CALC_Y_FROM_CENTER, SCALE_MIN);

			text[0]->text = LOADSCREEN0TX0;
			text[1]->text = LOADSCREEN0TX1;
			text[2]->text = LOADSCREEN0TX2;
			text[3]->text = LOADSCREEN0TX3;
			text[3]->height = text_height;
			text[3]->UpdatePolygonWH(font);
			auto dx = text[3]->GetPolygonWidth(1);

			const float posX = base_screen_W - dx;
			const float posY = base_screen_H - arrow_height_scaled;
			const eUITextCalcTypeX tX = CALC_XT_FROM_LEFT;
			const eUITextCalcTypeY tY = CALC_YT_FROM_BOTTOM;
			const eUITextScaleType tS = SCALE_TMIN;
			const bool fixedW = true;
			const bool fixedH = false;

			text[0]->InitUI(font, base_screen_W, base_screen_H, text_height, posX, posY, tX, tY, tS, fixedW, fixedH);
			text[1]->InitUI(font, base_screen_W, base_screen_H, text_height, posX, posY, tX, tY, tS, fixedW, fixedH);
			text[2]->InitUI(font, base_screen_W, base_screen_H, text_height, posX, posY, tX, tY, tS, fixedW, fixedH);
			text[3]->InitUI(font, base_screen_W, base_screen_H, text_height, posX, posY, tX, tY, tS, fixedW, fixedH);

			time = _TIME;

			isInit = true;
		}
	protected:
		virtual void Close() override
		{
			if (!isInit) return;

			font = nullptr;
			loading_thread.Join(true);
			loading_screen = false;
			loading_call = false;
			loading_mapid = 0;
			loading_mapname = L"MISSING MAP NAME TO LOAD";

			logo.Close();
			fire.Close();
			arrow.Close();

			isInit = false;
		}
	public:
		//>> Show loading screen 
		void LoadingScreen()
		{
			if (!isInit) Init();

			uint32 W = IO->window.client_width;
			uint32 H = IO->window.client_height;

			logo.UID.UpdateUI(W, H, false);
			if (fire.UID.UpdateUI(W, H, false))
			{
				fire_width  = fire.UID.GetCurWidth();
				fire_height = fire.UID.GetCurHeight();
			}
			if (arrow.UID.UpdateUI(W, H, false))
				arrow_cur_height = arrow.UID.GetCurHeight();

			for (int i = 0; i < 4; i++)
				if (text[i]->UpdateUI(font, W, H, false))
					text[i]->SetCurPosY(H - arrow_cur_height, true); // fix линии привязки к стрелке

			fire.UID.SetCurPosX ( IO->mouse.client_x - scale_pos_fire_x * fire_width,  0 ); // привязка к курсору
			fire.UID.SetCurPosY ( IO->mouse.client_y - scale_pos_fire_y * fire_height, 0 ); // привязка к курсору

			arrow.UID.AddAngle(update_speed * _TIMER(time), true);		time = _TIME;
			float angle = arrow.UID.GetCurAngle();

			logo.Show();
			arrow.Show();

			int i = (int)(fmod(angle, 120.f)) / 30; // text [0..3]
			i = (i>3) ? 3 : i;	
			font->ShowText(text[i]);

			fire.Show();
		}
	};
	class CUIChatWindow : public virtual CUIBase
	{
		struct ChatNetMsg
		{
			friend class CNetClient;
			friend class CNetServer;
			ChatNetMsg()
			{
				message.reserve(CHATHSTSZ + 1);
				message.clear();
				Close();
			}
			void Close()
			{
				message   = L"";
				repeat    = 0;
				ID        = 0;
				NEXT_ID   = false;
			}
		private:
			wstring		message;	// text		
			uint16		repeat;		// count of sending repeats
			uint16		ID;			// unique message ID
			bool		NEXT_ID;	// (tip) new ID / new message
		public:
			void NetSetMessage(const wchar_t * msg) { message = msg; NetSetNextID(); }
			void NetSetMessage(const wstring & msg) { message = msg; NetSetNextID(); }
		private:
			void NetSetNextID()                     { ID++; NEXT_ID = true; }					
			void NetSetID(uint16 NEW_ID)            { if (ID != NEW_ID) { ID = NEW_ID; NEXT_ID = true; } }
			void NetRepeatSet(uint16 repeats)       { repeat = repeats; }
			void NetRepeatSubOne()                  { if (repeat) { repeat--; NEXT_ID = false; } }
		};
	private:
		CFontR *	font { nullptr };
		bool		isInit;
		bool		reset;
	private:
		float       chat_transparency;	 // прозрачнсть окна чата
		float		scale_chat_window_X; // ширина чата как % от base_screen_W
		float		scale_chat_window_Y; // высота чата как % от base_screen_H
		bool		chat_menu_switch;	 // хранит вызов на переключение
		bool		chat_menu;			 // статус показа
		bool		chat_menu_passive;	 // статус пассивного показа ( не дольше < transparency_timer > )
	public:	
		wstring		chat_history;		 // память всех принятых сообщений
		wstring		chat_message;		 // текущее неотправленное сообщение
		ChatNetMsg	chat_net;			 // память для рассылки в сети
	private:
		wstring		chat_history_actual; // фактически показываемая часть истории
		CText *		history;
		CText *		message;
		CSprite		console;
		uint32		screen_width  {0};		// memory from base.GetW()
		uint32		screen_height {0};		// memory from base.GetH()
		float		message_text_height;
		float		history_text_height;
		float		mem_chat_transparency;
		int32		max_polygon_width;		// max width of text string polygon inside chat
		int32		max_text_lines;			// max lines of history text
	private:	
		wchar_t *	msg;	  // == chat_message.c_str();
		int64		hst_sz;	  // == chat_history.size();
		int64		mem_msz;  // == chat_message.size();
	private:
		const wstring *		player_name     { nullptr }; // ptr to self name
		bool				reload_actual   { false };	 // reload call for < chat_history_actual >
		timepoint			transparency_time;			 // starting timepoint of timing transparency
		timepoint			transparency_time_delta;	 // delta timer to add/sub transparency chunk
	private:
		const float transparency_delta  = 0.03f;	// add/sub transparency chunk
		const float transparency_target = 0.75f;	// target timing transparency
		const int transparency_timer = 8000;		// ms, passive show time
		const int transparency_delta_timer = 50;	// ms, time to add/sub < transparency_delta >
		const int console_delta_x = 5; // Отступ чата от левой  границы окна
		const int console_delta_y = 5; // Отступ чата от нижней границы окна
		const float message_text_scale = 0.070f; // Размер шрифта сообщения в % от высоты окна чата
		const float history_text_scale = 0.060f; // Размер шрифта  истории  в % от высоты окна чата
		const float message_posx_scale = 0.017f; // Позиция X сообщения в % от ширины окна чата (от левой границы)
		const float message_posy_scale = 0.030f; // Позиция Y сообщения в % от высоты окна чата (от нижней границы)
		const float history_posy_scale = 0.120f; // Позиция Y  истории  в % от высоты окна чата (от нижней границы)
		const float max_polygon_width_scale = 0.960f; // Длина строки в % от ширины окна чата
	protected:
		CUIChatWindow() : CUIBase(), chat_menu(false), chat_menu_passive(false), chat_menu_switch(false),
			isInit(false), reset(true)
		{
			history = new CText;
			message = new CText;
		}
		virtual ~CUIChatWindow() override
		{
			Close();
			_DELETE(history);
			_DELETE(message);
		}
	private:
		void Init()
		{
			if (isInit) return;
			if (!CUIBase::isInit) { _MBM(ERROR_InitNone); return; }

			font = CUIBase::font[FONT_CHAT].get();

			chat_transparency = 1.f;
			scale_chat_window_X = 0.35f;
			scale_chat_window_Y = 0.35f;
			chat_history.reserve (CHATHSTSZ + 1);  chat_history.clear();
			chat_message.reserve (CHATMSGSZ + 1);  chat_message.clear();

			*history = L"";
			*message = L"";

			console.Init(base_shr, CHATWINDOWNAME);

			mem_chat_transparency = chat_transparency;
			history->color1 = COLORVEC(CLR_indigo, chat_transparency);
			message->color1 = COLORVEC(CLR_black,  chat_transparency);

			chat_history_actual.reserve(CHATHSTSZ * 2 + 1);
			chat_history_actual.clear();
			history->ExTextPointerSet(&chat_history_actual);
			message->ExTextPointerSet(&chat_message);

			// GetW*/H*() переменная зависима от scale_chat_window_X/Y, пока что я предполагаю, что scale не будет меняться
			// иначе надо настроить обновления части соотв. переменных под текущий размер чата; также обратить внимание на console_delta_x/y

			float console_width  = console.GetWidth();
			float console_height = console.GetHeight();

			message_text_height = message_text_scale * console_height;
			history_text_height = history_text_scale * console_height;

			max_text_lines = (int)(scale_chat_window_Y * base_screen_H / (1.1f * history_text_height) - 1);

			float message_pos_x = (float)console_delta_x + message_posx_scale * console_width;
			float message_pos_y = (float)base_screen_H - (console_delta_y + (message_posy_scale * console_height));
			float history_pos_y = (float)base_screen_H - (console_delta_y + (history_posy_scale * console_height));

			const eUITextCalcTypeX tX = CALC_XT_FROM_LEFT;
			const eUITextCalcTypeY tY = CALC_YT_FROM_BOTTOM;
			const eUITextScaleType tS = SCALE_TALL;
			const bool fW = false;
			const bool fH = false;

			history->InitUI(font, base_screen_W, base_screen_H, history_text_height, message_pos_x, history_pos_y, tX, tY, tS, fW, fH);
			message->InitUI(font, base_screen_W, base_screen_H, message_text_height, message_pos_x, message_pos_y, tX, tY, tS, fW, fH);

			console.UID.InitUI ( base_screen_W,          base_screen_H,
			                     (float)console_delta_x, (float)base_screen_H - console_delta_y,
								 console_width,          console_height,
								 scale_chat_window_X * base_screen_W / console_width,
								 scale_chat_window_Y * base_screen_H / console_height,
								 CALC_X_FROM_LEFT,       CALC_Y_FROM_BOTTOM,
								 SCALE_ALL );

		//	console_UID.InitUI ( base_screen_W, base_screen_H,
		//						(float)console_delta_x, (float)base_screen_H - console_delta_y,
		//						console_width, console_height,
		//						1.0f, 1.0f,
		//						CALC_X_FROM_LEFT, CALC_Y_FROM_BOTTOM, SCALE_ALL);

			msg = (wchar_t*)chat_message.c_str();
			hst_sz = chat_history.size();
			mem_msz = chat_message.size();

			isInit = true;
		}
	protected:
		virtual void Close() override
		{
			if (!isInit) return;

			font        = nullptr;
			player_name = nullptr;

			chat_menu        = false;
			chat_menu_switch = false;

			isInit = false;
			reset  = true;		
		}
	public:
		void ChatSetTransparency(const float & transparency)
		{
			chat_transparency = min(1, max(0, transparency));
		}
		void ChatSetWindowScale(const float & scale_x, const float & scale_y)
		{
			scale_chat_window_X = min(0.99f, max(0.01f, scale_x));
			scale_chat_window_Y = min(0.99f, max(0.01f, scale_y));
		}		
		void ChatSwitchMenu()       { chat_menu_switch = true; }
		bool ChatSwitchMenuCheck()  { _RETBOOL(chat_menu_switch); }
		bool ChatGetMenuStatus()	    { return chat_menu; }
		bool ChatGetMenuStatusPassive()	{ return chat_menu_passive; }
		void ChatSetMenuStatus(bool ON)
		{
			chat_menu = ON;
			chat_menu_switch  = false;
			chat_menu_passive = false;
			chat_transparency = 1.f;
		}
		void ChatSetPlayerName(const wstring * name) { player_name = name; }
		void ChatHistoryAppend(const wstring & text, bool show_passively, bool incoming_sound)
		{
			if (!isInit) Init();

			auto text_size = text.size();
			auto history_size = chat_history.size();

			if ((history_size + text_size) >= CHATHSTSZ) // история переполнена
			{
				chat_history.erase(chat_history.begin(),
					chat_history.begin() + CHATHSTSZ_CLEAR);
			}

			chat_history.append(text);			

			ChatHistoryUpdate();
			if (show_passively) ChatPassiveMode();
			if (incoming_sound) sound->RunThreadPlay(sound_id[UISOUND_CHAT_INCOMING], eSoundRunNONE);
		}
		void ChatHistoryAppend(const wchar_t * text, bool show_passively, bool incoming_sound)
		{
			if (!isInit) Init();

			auto text_size = wcslen(text);
			auto history_size = chat_history.size();

			if ((history_size + text_size) >= CHATHSTSZ) // история переполнена
			{
				chat_history.erase(chat_history.begin(),
					chat_history.begin() + CHATHSTSZ_CLEAR);
			}

			chat_history.append(text);

			ChatHistoryUpdate();
			if (show_passively) ChatPassiveMode();
			if (incoming_sound) sound->RunThreadPlay(sound_id[UISOUND_CHAT_INCOMING], eSoundRunNONE);
		}
		void ChatHistorySet(const wstring & text, bool show_passively, bool incoming_sound)
		{
			if (!isInit) Init();

			chat_history = text;

			auto history_size = chat_history.size();
			if (history_size >= CHATHSTSZ) // история переполнена
			{
				chat_history.erase(chat_history.begin(),
					chat_history.begin() + (history_size - CHATHSTSZ) + CHATHSTSZ_CLEAR);
			}
			
			ChatHistoryUpdate();
			if (show_passively) ChatPassiveMode();
			if (incoming_sound) sound->RunThreadPlay(sound_id[UISOUND_CHAT_INCOMING], eSoundRunNONE);
		}
		void ChatHistorySet(const wchar_t * text, bool show_passively, bool incoming_sound)
		{
			if (!isInit) Init();

			chat_history = text;

			auto history_size = chat_history.size();
			if (history_size >= CHATHSTSZ) // история переполнена
			{
				chat_history.erase(chat_history.begin(),
					chat_history.begin() + (history_size - CHATHSTSZ) + CHATHSTSZ_CLEAR);
			}

			ChatHistoryUpdate();
			if (show_passively) ChatPassiveMode();
			if (incoming_sound) sound->RunThreadPlay(sound_id[UISOUND_CHAT_INCOMING], eSoundRunNONE);
		}
	private:
		wstring * ChatGetHistoryPreparedText(wstring & message, const uint32 max_width, CBaseFont * font, CText * text) const
		{
			wstring * ret = new wstring;
			wstring * mem_text_pointer = text->ExTextPointerGet();
			text->ExTextPointerSet(&message);

			text->ClearStartSymbolPos();
			text->ClearEndSymbolPos();
			text->UpdatePolygonWH(font);
			uint32 message_width  = (uint32)text->GetPolygonWidth(1);
			uint32 message_height = (uint32)text->GetPolygonHeight(1);

			while (message_width > max_width)
			{
				const auto max_height = 999999.f;

				text->SetStartEndSymbolsPos((float)max_width, max_height, false, true, font);
				ret->append(message.substr(text->start_symbol_pos, text->end_symbol_pos));
				ret->push_back('\r');

				message.erase(text->start_symbol_pos, text->end_symbol_pos);

				text->ClearStartSymbolPos();
				text->ClearEndSymbolPos();
				text->UpdatePolygonWH(font);
				message_width = (uint32)text->GetPolygonWidth(1);
				message_height = (uint32)text->GetPolygonHeight(1);
			}

			ret->append(message);
			text->ExTextPointerSet(mem_text_pointer);
			return ret;
		}
		void ChatHistoryUpdate()
		{
			hst_sz = chat_history.size();
			history->CallReloadConstantText();
			reload_actual = true;
		}
		void ChatPassiveMode()
		{
			if (!chat_menu && !chat_menu_passive) // chat is inactive
			{
				chat_menu_passive       = true;
				transparency_time       = _TIME;
				transparency_time_delta = _TIME;
				chat_transparency       = 0.01f;
			}
			else if (chat_menu_passive) // already running in passive mode
			{
				transparency_time = _TIME; // добавим времени показа, перезапустив таймер
			}
		}
	public:
		//>> Show chat window
		void Chat()
		{
			if (!isInit) Init();

			bool prepare_chat_history_actual = false;

			if (reload_actual)							{
				prepare_chat_history_actual = true;
				reload_actual = false;					}

			if (chat_menu_passive) // passive showing (incoming net messages)
			{
				auto timer       = _TIMER(transparency_time);		// smoothly add/sub transparency
				auto timer_delta = _TIMER(transparency_time_delta);	// .
				if (timer < transparency_timer) // fade in
				{
					if (chat_transparency < transparency_target)
					{
						if (timer_delta > transparency_delta_timer)
						{
							chat_transparency += transparency_delta;
							transparency_time_delta = _TIME;
						}
					}
				}
				else // fade out
				{
					if (chat_transparency > 0.01f)
					{
						if (timer_delta > transparency_delta_timer)
						{
							chat_transparency -= transparency_delta;
							transparency_time_delta = _TIME;
						}
					}
					else // exit from passive mode
					{
						chat_transparency = 1.f;
						chat_menu_passive = false;
						return;
					}
				}
			}

			if (mem_chat_transparency != chat_transparency) // Перезагрузка в связи с изменением прозрачности
			{
				mem_chat_transparency = chat_transparency;
				history->color1 = COLORVEC(color_chat_hst, chat_transparency);
				message->color1 = COLORVEC(color_chat_msg, chat_transparency);
				console.Reset(COLORVEC(CLR_white, chat_transparency));
				history->CallReloadConstantText();
				message->CallReloadConstantText();
			}

			if (screen_width  != IO->window.client_width ||
				screen_height != IO->window.client_height ) reset = true;

			if (reset) // Обновление актуальным размерам окна
			{
				screen_width  = IO->window.client_width;
				screen_height = IO->window.client_height;
				history->UpdateUI(font, screen_width, screen_height, true);
				message->UpdateUI(font, screen_width, screen_height, true);
				console.UID.UpdateUI(screen_width, screen_height, true);
				max_polygon_width = (int)(console.UID.GetCurWidth() * max_polygon_width_scale);
				prepare_chat_history_actual = true;
			}

			auto msz = chat_message.size();

			////////////////// Обновление памяти истории (отправка сообщения)

			if (mem_msz != msz)
			if (msg[msz-1]=='\r' || msg[msz-1]=='\n') 
			{
				while (msg[msz-1]=='\r' || msg[msz-1]=='\n')	{
					chat_message.pop_back();
					msz--;										}

				//size_t hst_sz = chat_history.size();
				if (msz)																{
					if ((hst_sz + msz) >= CHATHSTSZ) // история переполнена
					{
						chat_history.erase(chat_history.begin(),
							chat_history.begin() + CHATHSTSZ_CLEAR);
					}
					if (!hst_sz)
						 chat_history.append(L" \r"); // dummy init
					else chat_history.append(L"\r");
					if (player_name != nullptr)				{
						chat_history.append(L"[");
						chat_history.append(*player_name);
						chat_history.append(L"] ");			}
					chat_net.NetSetMessage(msg); // запоминаем для рассылки
					chat_history.append(msg);											}

				chat_message.clear();
				mem_msz = msz = 0;
				hst_sz = chat_history.size();

				history->CallReloadConstantText();
				prepare_chat_history_actual = true;
			}

			////////////////// Подгон фактически показываемой истории под вывод в окно

			if (prepare_chat_history_actual && chat_history.size()) 
			{
				chat_history_actual.clear();

				wchar_t * history_text = (wchar_t*)chat_history.c_str();
				int32 i, x, curpos_in_history, cur_text_lines = 0;

				curpos_in_history = (int32)(hst_sz - 1);
				while (cur_text_lines < max_text_lines && curpos_in_history > 0)
				{
					///////// Берём сообщения с конца, готовим их к виду, при котором они влезут в окно (wtemp_text) /////////
					///////// и заполняем chat_history_actual, пока есть место (text_lines < max_text_lines)         /////////

					for (i=curpos_in_history; i>=0; i--)
						if (history_text[i] == '\r')
							break;

					int32 temp_pos = (i>0) ? i+1 : 0;
					int32 temp_size = (curpos_in_history + 1) - temp_pos;
					curpos_in_history = i-1;

					wstring * wtemp_text = ChatGetHistoryPreparedText(chat_history.substr(temp_pos, temp_size), max_polygon_width, font, history);
					const wchar_t * temp_text = wtemp_text->c_str();

					int32 temp_text_lines = 1;
					temp_size = (int32)wtemp_text->size();
					for (i=0; i<temp_size; i++)
						if (temp_text[i] == '\r')
							temp_text_lines++;

					///////// Теперь надо скопировать всё сообщение или его часть в фактически показываемую историю

					if (cur_text_lines + temp_text_lines > max_text_lines) // copy part of text in actual history
					{
						int32 available_lines = max_text_lines - cur_text_lines;
						for (i=temp_size-1, x=0; ;i--)
						{
							if (temp_text[i] == '\r') x++;
							if (x == available_lines) break;
						}
						size_t temp_copy_size = temp_size - i;
						chat_history_actual.insert(0, wtemp_text->substr(i, temp_copy_size));
						cur_text_lines += available_lines;
					}
					else // copy full text in actual history
					{
						chat_history_actual.insert(0, *wtemp_text);
						chat_history_actual.insert(0, L"\r");				
						cur_text_lines += temp_text_lines;
					}

					delete wtemp_text;
				}

				history->UpdateUI(font, screen_width, screen_height, true);
			}

			////////////////// Если набираемое (неотправленное) сообщение обновилось

			if (mem_msz != msz || reset)
			{
				message->ClearStartSymbolPos();
				message->ClearEndSymbolPos();
				message->UpdatePolygonWH(font);
				float message_width  = message->GetPolygonWidth(true);
				float message_height = message->GetPolygonHeight(true);
				mem_msz = msz;

				// Если длина сообщения больше окна -> показываем только часть с конца
				if ((int)message_width > max_polygon_width)
					message->SetStartEndSymbolsPos((float)max_polygon_width, message_height, true, false, font);

				message->UpdateUI(font, screen_width, screen_height, true);

				reset = false;
			}

			console.Show();

			if (hst_sz) font->ShowText(history);
			if (msz)    font->ShowText(message);
		}
	};
	class CUITipInteractPick : public virtual CUIBase
	{
	private:
		CFontR *	font  { nullptr };
		bool		isInit;
	private:
		CText *		rotate;
		CText *		move;
		CSprite		rotate_sprite;
		CSprite		move_sprite;
		float       move_sprite_cur_dx;
		char        move_sprite_cur_dx_direction;
		float		move_text_width;
		timepoint   time;
	private:
		const float update_speed = 0.1f;
	protected:
		CUITipInteractPick() : CUIBase(), isInit(false)
		{
			rotate = new CText;
			move   = new CText;
		}
		virtual ~CUITipInteractPick() override
		{
			Close();
			_DELETE(rotate);
			_DELETE(move);
		}
	private:
		void Init()
		{
			if (isInit) return;
			if (!CUIBase::isInit) { _MBM(ERROR_InitNone); return; }

			font = CUIBase::font[FONT_ESCAPE_MENU].get();

			const float rotate_pos_x = (float)base_screen_W * (0.5f - 0.1f);
			const float move_pos_x   = (float)base_screen_W * (0.5f + 0.1f);
			const float rotate_pos_y = (float)base_screen_H * 0.40f;
			const float move_pos_y   = (float)base_screen_H * 0.40f;

			const float text_height  = 30.f;

			const eUITextCalcTypeX tXr = CALC_XT_FROM_RIGHT;
			const eUITextCalcTypeX tXm = CALC_XT_FROM_LEFT;
			const eUITextCalcTypeY tY  = CALC_YT_FROM_TOP;
			const eUITextScaleType tS  = SCALE_TALL;
			const bool fixedW = true;
			const bool fixedH = false;

			*rotate = TIPPICKROTATETX;
			*move   = TIPPICKMOVETX;
			rotate->color1 = COLORVEC(CLR_white, 0.75f);
			move->color1   = COLORVEC(CLR_white, 0.75f);
			rotate->InitUI (font, base_screen_W, base_screen_H, text_height, rotate_pos_x, rotate_pos_y, tXr, tY, tS, fixedW, fixedH);
			move->InitUI   (font, base_screen_W, base_screen_H, text_height, move_pos_x,   move_pos_y,   tXm, tY, tS, fixedW, fixedH);

			float rotate_W = rotate->GetPolygonWidth(1);	float move_W = move->GetPolygonWidth(1);	// text's polygon width
			float rotate_H = rotate->GetPolygonHeight(1);	float move_H = move->GetPolygonHeight(1);	// text's polygon height

			rotate_sprite.Init(base_shr, TIPPICKROTATE);
			move_sprite.Init(base_shr, TIPPICKMOVE);

			rotate_sprite.Reset(COLORVEC(CLR_white, 0.75f));
			move_sprite.Reset(COLORVEC(CLR_white, 0.75f));

			const float rotate_arrow_pos_y = rotate_pos_y + rotate_H * 1.5f; // под текстом
			const float move_arrow_pos_y   = move_pos_y   + move_H   * 1.5f; // .
			const float rotate_arrow_pos_x = rotate_pos_x - rotate_W * 0.5f; // по середине текста
			const float move_arrow_pos_x   = move_pos_x   + move_W   * 0.5f; // .

			float rotate_arrow_width  = rotate_sprite.GetWidthOrigin();
			float rotate_arrow_height = rotate_sprite.GetHeightOrigin();
			float move_arrow_width    = move_sprite.GetWidthOrigin();
			float move_arrow_height   = move_sprite.GetHeightOrigin();

			const float rotate_center_fix_X = 12.f;
			const float rotate_center_fix_Y = 0.f;

			const float rotate_arrow_scale = 0.4f;
			const float move_arrow_scale_x = 0.5f;
			const float move_arrow_scale_y = 0.4f;

			rotate_sprite.UID.InitUI ( base_screen_W, base_screen_H,
									   rotate_arrow_pos_x, rotate_arrow_pos_y,
									   rotate_arrow_width,
									   rotate_arrow_height,
									   rotate_arrow_scale, rotate_arrow_scale,
									   CALC_X_FROM_CENTER,  CALC_Y_FROM_TOP,  SCALE_MIN);

			move_sprite.UID.InitUI   ( base_screen_W, base_screen_H,
									   move_arrow_pos_x, move_arrow_pos_y,
									   move_arrow_width,
									   move_arrow_height,
									   move_arrow_scale_x, move_arrow_scale_y,
									   CALC_X_FROM_CENTER,  CALC_Y_FROM_TOP,  SCALE_MIN);

			rotate_sprite.UID.InitRotateCenter (
				rotate_center_fix_X + 0.5f * rotate_arrow_width,
				rotate_center_fix_Y + 0.5f * rotate_arrow_height );

			move_sprite_cur_dx           = 0.f;
			move_sprite_cur_dx_direction = +1;
			move_text_width              = move_W;

			time = _TIME;

			isInit = true;
		}
	protected:
		virtual void Close() override
		{
			if (!isInit) return;

			font = nullptr;
			rotate_sprite.Close();
			move_sprite.Close();

			isInit = false;
		}
	public:
		void TipInteractPick()
		{
			if (!isInit) Init();

			auto W = IO->window.client_width;
			auto H = IO->window.client_height;

			    rotate->UpdateUI(font, W, H, false);
			if (  move->UpdateUI(font, W, H, false) ) move_text_width = move->GetPolygonWidth(1);
			rotate_sprite.UID.UpdateUI(W, H, false);
			  move_sprite.UID.UpdateUI(W, H, false);

			rotate_sprite.UID.AddAngle(update_speed * _TIMER(time), true);

			if (move_sprite_cur_dx_direction == +1) move_sprite_cur_dx += 0.01f * update_speed * _TIMER(time);
			else                                    move_sprite_cur_dx -= 0.01f * update_speed * _TIMER(time);

			time = _TIME;

			if (move_sprite_cur_dx >  0.25f) { move_sprite_cur_dx  = 0.25f; move_sprite_cur_dx_direction = -1; }
			if (move_sprite_cur_dx < -0.25f) { move_sprite_cur_dx = -0.25f; move_sprite_cur_dx_direction = +1; }

										move_sprite.UIP->x += move_text_width * move_sprite_cur_dx;
			move_sprite.Show();			move_sprite.UIP->x -= move_text_width * move_sprite_cur_dx;
			rotate_sprite.Show();

			font->ShowText(rotate);
			font->ShowText(move);
		}
	};
	class CUICursorsManager : public virtual CUIBase
	{
	private:
		bool		isInit;
//	private:
//		TEXUIDATA	cursor_game[1];
//		TEXUIDATA	cursor_menu[1];
//		int32		id_cursor_game[1];
//		int32		id_cursor_menu[1];
//		WPOS *      cursor_game_pos[1];
//		WPOS *      cursor_menu_pos[1];
	private:
		CSprite		game_cursor;
		CSprite		menu_cursor;
	private:
		const float game_cursor_scale_x = 0.4f;
		const float game_cursor_scale_y = 0.4f;
		const float menu_cursor_scale_x = 0.5f;
		const float menu_cursor_scale_y = 0.5f;
	protected:
		CUICursorsManager() : CUIBase(), isInit(false) {}
		virtual ~CUICursorsManager() override { Close(); }
	private:
		void Init()
		{
			if (isInit) return;
			if (!CUIBase::isInit) { _MBM(ERROR_InitNone); return; }

			game_cursor.Init(base_shr, CURSORGAME1);
			menu_cursor.Init(base_shr, CURSORMENU1);

			game_cursor.Reset(COLORVEC(color_cursor_game, 0.8f));
			menu_cursor.Reset(COLORVEC(color_cursor_menu, 1.0f));

			float fix_y = 35.f * game_cursor_scale_y;

			game_cursor.UID.InitUI ( base_screen_W,                 base_screen_H,
									 (float)base_screen_W / 2,      (float)base_screen_H / 2 - fix_y,
									 game_cursor.GetWidthOrigin(),  game_cursor.GetHeightOrigin(),
									 game_cursor_scale_x,           game_cursor_scale_y,
									 CALC_X_FROM_CENTER,            CALC_Y_FROM_CENTER,
									 SCALE_MIN_Y );

			menu_cursor.UID.InitUI ( base_screen_W,                  base_screen_H,
									 (float)base_screen_W / 2,       (float)base_screen_H / 2,
									 menu_cursor.GetWidthOrigin(),   menu_cursor.GetHeightOrigin(),
									 menu_cursor_scale_x,			 menu_cursor_scale_y,
									 CALC_X_FROM_LEFT,               CALC_Y_FROM_TOP,
									 SCALE_MAX );
			isInit = true;
		}
	protected:
		virtual void Close() override
		{
			if (!isInit) return;

			game_cursor.Close();
			menu_cursor.Close();

			isInit = false;
		}
	public:
		void ShowCursorGame()
		{
			if (!isInit) Init();

			game_cursor.UID.UpdateUI( IO->window.client_width, IO->window.client_height, 0 );
			game_cursor.Show();
		}		
		void ShowCursorMenu()
		{
			if (!isInit) Init();

			menu_cursor.UID.UpdateUI( IO->window.client_width, IO->window.client_height, 0 );
			menu_cursor.UID.SetCurPosX( (float)IO->mouse.client_x, true );
			menu_cursor.UID.SetCurPosY( (float)IO->mouse.client_y, true );
			menu_cursor.Show();
		}
	};
	class CUICustomVarious : public virtual CUIBase
	{
	private:
		CFontR *	font { nullptr };
		bool		isInit;
		bool        reset;
		bool		help_status;
	private:
		VECPDATA <CText>	text;
		CText *				text_help;
		int32				size;
		float				mem;
	private:
		const float textH = 23.f;
	protected:
		CUICustomVarious() : CUIBase(), isInit(false), mem(MISSING), reset(true), help_status(false)
		{
			text_help = new CText;
		}
		virtual ~CUICustomVarious() override
		{
			Close();
			text.Delete(1);
			_DELETE(text_help);
		}
	private:
		void Init()
		{
			if (isInit) return;
			if (!CUIBase::isInit) { _MBM(ERROR_InitNone); return; }

			font = CUIBase::font[FONT_ESCAPE_MENU].get();

			wchar_t GAMEVER[128];
			wsprintf(GAMEVER, L"engine version %i.%i.%i", PRODUCT_VERSION_S);

			text.AddCreate(1);	*text.last() = L"Parallax scale [Z/z]: -0.020";
			text.AddCreate(1);	*text.last() = L"ESC/TAB: escape/chat menu";
			text.AddCreate(1);	*text.last() = L"W/A/S/D, Q/E: movements, roll";
			text.AddCreate(1);	*text.last() = L"SPACE/CTRL: get up/down";
			text.AddCreate(1);	*text.last() = L"1/3: camera view mode";
			text.AddCreate(1);	*text.last() = L"F: switch walking / fly mode";
			text.AddCreate(1);	*text.last() = L"G: switch walking X/Z speed fix";
			text.AddCreate(1);	*text.last() = GAMEVER;
			
			*text_help = L"Press H to show help";
			
			const float posX  = 5.f;
			const float posY  = 17.f; // 1st text Y pos
			const eUITextCalcTypeX tX = CALC_XT_FROM_LEFT;
			const eUITextCalcTypeY tY = CALC_YT_FROM_TOP;
			const eUITextScaleType tS = SCALE_TALL;
			const bool fixedW = false;
			const bool fixedH = false;

			size = (int32) text.size() - 1;
			for (int i=0; i<size; i++)
				text[i]->InitUI(font, base_screen_W, base_screen_H, textH, posX, posY + textH * (i + 1) + 5.f, tX, tY, tS, fixedW, fixedH);

			text_help->InitUI(font, base_screen_W, base_screen_H, 30.f, posX, 35.f, tX, tY, tS, fixedW, fixedH);

			text[size]->InitUI ( font, base_screen_W,      base_screen_H, 15,
				                 (float)base_screen_W - 3, (float)base_screen_H - 3,
							     CALC_XT_FROM_RIGHT,       CALC_YT_FROM_BOTTOM, tS,
							     false,                    false);

			size++;
			isInit = true;
		}
	protected:
		virtual void Close() override
		{
			if (!isInit) return;
			font = nullptr;
			isInit = false;
			reset = true;
			mem = MISSING;
		}
	public:
		//>> Show various
		void Interface(const float prlx_scale)
		{
			if (!isInit) Init();

			wchar_t wtext[256];
			uint32 W = IO->window.client_width;
			uint32 H = IO->window.client_height;

			if (mem != prlx_scale)
			{
				mem = prlx_scale;
				swprintf_s(wtext, L"Parallax scale [Z/z]: %6.3f", prlx_scale);
				reset = true;
			}
			
			if (text_help->UpdateUI(font, W, H, false))
				for (int i=0; i<size; i++) text[i]->UpdateUI(font, W, H, false);

			if (reset)
			{
				text[0]->text.assign(wtext);
				text[0]->CallReloadConstantText();
				reset = false;
			}

			if (help_status)
				for (int i=0; i<size; i++)
					font->ShowText(text[i]);
			else
			{
				font->ShowText(text_help);
				font->ShowText(text[size-1]);
			}
		}
		//>> Show help: yes/no
		void HelpStatus(bool status)
		{
			help_status = status;
		}
	};
	class CUINetworkPing : public virtual CUIBase
	{
	private:
		CFontR *	font   { nullptr };
		bool		isInit { false };
	private:
		const uint16  lost = 100; // ms, lost message update time 
		const uint16  show = 500; // ms, ping message update time
	private:
		CText *		ping;
		timepoint	time;
		uint32		time_lost   { lost + 1 };
		uint32		total_ms    { 0 };
		uint32		total_times { 0 };
	protected:
		CUINetworkPing() : CUIBase()
		{
			ping = new CText;
		}
		virtual ~CUINetworkPing() override
		{
			Close();
			_DELETE(ping);
		}
	private:
		void Init()
		{
			if (isInit) return;
			if (!CUIBase::isInit) { _MBM(ERROR_InitNone); return; }

			font = CUIBase::font[FONT_FPS].get();

			const float text_height = 35;
			const float pos_x = (float)base_screen_W - 5;
			const float pos_y = 5.f;

			ping->text   = L"Ping: 0 ms";
			ping->color1 = COLORVEC(CLR_red, 1);

			ping->InitUI ( font,
			               base_screen_W,       base_screen_H,
						   text_height,
						   pos_x,               pos_y,
						   CALC_XT_FROM_RIGHT,  CALC_YT_FROM_TOP,
						   SCALE_TALL,
						   false, false );

			isInit = true;
		}
	protected:
		virtual void Close() override
		{
			if (!isInit) return;
			
			font        = nullptr;
			total_ms    = 0;
			total_times = 0;

			isInit = false;
		}
	public:
		void NetPingResetTimer()
		{
			if (!isInit) Init();

			time      = _TIME;
			time_lost = lost + 1;

			ping->text = L"Ping: 0 ms";
			ping->UpdateUI(font, IO->window.client_width, IO->window.client_height, true);
		}
		void NetPingUpdate(uint32 ms)
		{
			if (!isInit) Init();

			total_times++;
			total_ms += ms;
		}
		void NetPingShow()
		{
			if (!isInit) Init();

			if (_TIMER(time) > show)
			{
				total_ms /= (total_times) ? total_times : 1;

				wchar_t text[128];
				swprintf_s(text, L"Ping: %i ms", total_ms);
				ping->text = text;
				ping->UpdateUI(font, IO->window.client_width, IO->window.client_height, true);

				time        = _TIME;
				time_lost   = 101;
				total_ms    = 0;
				total_times = 0;
			}

			font->ShowText(ping);
		}
		void NetPingLostConnection(uint32 ms)
		{
			if (!isInit) Init();

			if (time_lost > lost)
			{
				uint32 time_s   = ms / 1000;
				uint32 time_ms  = ms - 1000 * time_s;
					   time_ms /= lost;

				wchar_t disconnect[128];
				swprintf_s(disconnect, L"Lost connection to server: %i,%i s", time_s, time_ms);
				ping->text = disconnect;
				ping->UpdateUI(font, IO->window.client_width, IO->window.client_height, true);

				time        = _TIME;
				total_ms    = 0;
				total_times = 0;
			}

			time_lost = (uint32)_TIMER(time);

			font->ShowText(ping);
		}
	};

	class CBaseInterface final :
		public CUIEscapeMenu, public CUIMonitorFPS, public CUILoadingScreen, public CUIChatWindow, public CUICustomVarious,
		public CUITipInteractPick, public CUICursorsManager, public CUINetworkPing
	{
	public:
		CBaseInterface(const CBaseInterface& src)				= delete;
		CBaseInterface(CBaseInterface&& src)					= delete;
		CBaseInterface& operator=(CBaseInterface&& src)			= delete;
		CBaseInterface& operator=(const CBaseInterface& src)	= delete;
	public:
		CBaseInterface() : CUIEscapeMenu(), CUIMonitorFPS(), CUILoadingScreen(), CUIChatWindow(), CUICustomVarious(), CUITipInteractPick(),
			CUICursorsManager(), CUINetworkPing() {};
		~CBaseInterface() override final { Close(); };

		void Close() override final
		{
			CUINetworkPing::Close();
			CUIEscapeMenu::Close();
			CUIMonitorFPS::Close();
			CUILoadingScreen::Close();
			CUIChatWindow::Close();
			CUICustomVarious::Close();
			CUITipInteractPick::Close();
			CUICursorsManager::Close();
			CUIBase::Close();
		}

		//>> Включение UI шейдера
		void EnableUIShader()
		{
			if (shader == nullptr) { _MBM(ERROR_PointerNone); return; }

			shader->State(SHADERUIV, true);
			shader->State(SHADERUIP, true);
		}

		//>> Выключение UI шейдера
		void DisableUIShader()
		{
			if (shader == nullptr) { _MBM(ERROR_PointerNone); return; }

			shader->State(SHADERUIV, false);
			shader->State(SHADERUIP, false);
		}

	};
}

#endif // _UI_H