// ----------------------------------------------------------------------- //
//
// MODULE  : World.h
//
// PURPOSE : Центр управления игровым миром
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2017)
//
// ----------------------------------------------------------------------- //

#ifndef _WORLD_H
#define _WORLD_H

#include "stdafx.h"
#include "link_shader.h"
#include "link_audio.h"
#include "link_network.h"
#include "Player.h"
#include "UI.h"
#include "GameMap.h"

using namespace SAVFGAME;

namespace SAVFGAME
{	
	enum eInitWorldSettings
	{
		SET_NONE,						// write value settings below

		SET_RESOLUTION,
		SET_WINDOW_POS,
		SET_FULLSCREEN,
		SET_UI_SCREEN,
		SET_BACKBUFFER,
		SET_SHOW_FPS,
		SET_CAM_FOV,
		SET_CAM_PLANE,
		SET_CAM_ASPECT,
		SET_CAM_MOUSESP,
		SET_CAM_MOVESP,
		SET_CAM_ROLLSP,
		SET_CAM_RADSP,
		SET_CAM_RADIUS,
		SET_NAME_HEIGHT,
		SET_SV_MAX_PL,

		SET_WSTRING_SETTINGS_SEPARATOR,	// write wstring settings below

		SET_MAP_DEFAULT,
		SET_FONT_DEFAULT,
		SET_FONT_LOADING_SCREEN,
		SET_FONT_ESCAPE_MENU,
		SET_FONT_FPS,
		SET_FONT_CHAT,
		SET_FONT_PLNAME_SCENE,
		SET_SOUND_LOADING_SCREEN,	
		SET_PLAYER_NAME,

		SET_STRING_SETTINGS_SEPARATOR,	// write string settings below

		SET_TARGET_IP,
		SET_TARGET_PORT,
		SET_SERVER_IP,
		SET_SERVER_PORT,
		SET_CLIENT_IP,

		SET_ENUM_MAX
	};
	struct InitWorldSettings
	{
		InitWorldSettings()
		{
			for (uint32 i = 0; i < eUIFontID::FONT_ENUM_MAX; i++) font.push_back(L"");
			font[eUIFontID::FONT_DEFAULT].assign(L"MISSING_DEFAULT_FONT");
			font[eUIFontID::FONT_LOADING_SCREEN].assign(L"MISSING_LOADING_SCREEN_FONT");
			font[eUIFontID::FONT_ESCAPE_MENU].assign(L"MISSING_MENU_FONT");
			font[eUIFontID::FONT_FPS].assign(L"MISSING_FPS_FONT");
			font[eUIFontID::FONT_CHAT].assign(L"MISSING_CHAT_FONT");
			font[eUIFontID::FONT_PLNAME_SCENE].assign(L"MISSING_PLAYER_NAME_SCENE_FONT");
			map_default          = L"MISSING_DEFAULT_MAP";
			loading_screen_sound = L"MISSING_LOADING_SOUND";
			window_width  = 800; window_posx = 200;
			window_height = 600; window_posy = 300;
			custom_backbuffer_width = 0;	// auto
			custom_backbuffer_height = 0;	// auto
			mode = WM_WINDOWED;
			show_fps = true;
			camera_fov = 45.f;
			camera_near = 1.f;
			camera_far = 100.f;
			camera_aspect = 0.f;			// auto
			camera_mousesp = 0.5f;
			camera_rollsp = 0.5f;
			camera_movesp = 0.2f;
			camera_radiussp = 1.0f;
			camera_minR = 10;
			camera_maxR = 100;
			server_max_players = MAX_PLAYERS;
			target_ip = "127.0.0.1";
			target_port = DEFAULT_PORT;
			server_ip = "";
			server_port = DEFAULT_PORT;
			client_ip = "";
			player_name = L"Player name";
			name_height = 1.f;
		}
		vector<wstring> font;
		wstring map_default;
		wstring loading_screen_sound;
		    int window_width, window_posx;
			int window_height, window_posy;
			int custom_backbuffer_width;
			int custom_backbuffer_height;
			int ui_base_width;
			int ui_base_height;
		eWindowMode mode;
		   bool show_fps;
		  float camera_fov;
		  float camera_near;
		  float camera_far;
		  float camera_aspect;
		  float camera_mousesp;
		  float camera_movesp;
		  float camera_rollsp;
		  float camera_radiussp;
		  float camera_minR;
		  float camera_maxR;
		 uint32 server_max_players;
	    std::string target_ip;
		std::string target_port;
		std::string server_ip;
		std::string server_port;
		std::string client_ip;
		wstring player_name;
		  float name_height;
	};

	class CWorld final
	{
	private:
		struct WorldToRender
		{
			friend class CWorld;
		protected:
			CStatusWindow *	window;					// текущее состояние окна
			CStatusWindow	window_memory;			// предыдущее состояние окна
			CStatusWindow	custom_backbuffer;
			bool			use_custom_backbuffer { 0 };
			bool			move_cursor_call      { 0 };
			bool			reset_call            { 0 };
			bool			exit_call             { 0 };
		public:
			int             x, y;	// move cursor to x, y [client window rectangle]
			bool			show_cursor           { 1 };
			bool			centered_cursor       { 1 };
		public:
			WorldToRender(){};
			void GetBackbuffer(int & out_width, int & out_height)
			{
				if (use_custom_backbuffer)
				{
					out_width  = custom_backbuffer.client_width;
					out_height = custom_backbuffer.client_height;
				}
			}
			bool CursorMoveCheck()    { _RETBOOL(move_cursor_call); }
			bool WindowResetCheck()   { _RETBOOL(reset_call); }
			bool ExitGameCheck()	  { _RETBOOL(exit_call);  }
		protected:
			void MemoryWindowCondition()
			{
				window_memory.sys_width  = window->sys_width;
				window_memory.sys_height = window->sys_height;
				window_memory.position_x = window->position_x;
				window_memory.position_y = window->position_y;
			}
			void RestoreWindowCondition()
			{
				window->sys_width  = window_memory.sys_width;
				window->sys_height = window_memory.sys_height;
				window->position_x = window_memory.position_x;
				window->position_y = window_memory.position_y;
			}
		};
		CShaderT _;
	public:
		WorldToRender				to_render;	// 
	protected:
		DEV3DDAT *					dev      { nullptr }; // from render
		CShaderT * const			shader   { & _ };
		CPlayerF *					self     { nullptr }; // self player data
		CStatusIO *					IO       { nullptr }; // from self player
		CKeyEvent *					keyev    { nullptr }; // from self player
		CPlayerCameraF *			camera   { nullptr }; // from self player
		CBaseInteract *				interact { nullptr }; // from self player
		CGameMapF *					map      { nullptr }; // current map (from <gamemap> manager)
	protected:
		InitWorldSettings			settings;	// game settings
		CGameMapManager				gamemap;	// manage game maps
		CPlayerManager				player;		// manage players
		CNetworkT					network;	// manage net
		CBaseInterface				UI;			// manage user interface		
		bool						skipframe;
	public:
		CWorld(const CWorld& src)				= delete;
		CWorld(CWorld&& src)					= delete;
		CWorld& operator=(CWorld&& src)			= delete;
		CWorld& operator=(const CWorld& src)	= delete;
	public:
		CWorld()  { Init();  };
		~CWorld() { Close(); };
	
	public:
		void CallSkipFrame()     { skipframe = true; }
	protected:
		bool CheckSkipFrame()    { _RETBOOL(skipframe); }
		
	protected:
		void Init()
		{
			network.Init();

			skipframe = false;
		}
	public:
		void Close()
		{
			network.Close();
			player.Close();
			gamemap.Close();
			UI.Close();	

			dev      = nullptr;
			self     = nullptr;
			IO       = nullptr;
			keyev    = nullptr;
			camera   = nullptr;
			interact = nullptr;
			map      = nullptr;
		}

	public:	//////////////////////////////////////////////////////// BASE BLOCK
	
		//>> Обновление клавиш
		void UpdateKey(uint32 key, wchar_t wchar, bool pressed)
		{
			//wprintf(L"\nkey: %c %4x (wchar) %2x (key) pressed = %i", wchar, wchar, key, pressed);

			if (UI.LoadingScreenCheck()) return; // не фиксируем ничего во время загрузки

			if (pressed && IO->keyboard.key[key].pressed)
				 IO->keyboard.key[key].stuck = true;
			else IO->keyboard.key[key].stuck = false;

			IO->keyboard.key[key].wchar   = wchar;
			IO->keyboard.key[key].pressed = pressed;
			IO->keyboard.key[key].timeout = false;
		}
		//>> Обновление мыши
		void UpdateMouse(int32 X, int32 Y)
		{
			IO->mouse.sys_x = max(0,X);
			IO->mouse.sys_y = max(0,Y);
			IO->mouse.client_x = IO->mouse.sys_x - IO->window.border_left;
			IO->mouse.client_y = IO->mouse.sys_y - IO->window.border_top;
		}
		//>> Обновление мыши
		void UpdateMouse(eMouseButton button, bool pressed) //bool lb, bool mb, bool rb)
		{
			switch (button)
			{
			case M_LEFT:
				IO->mouse.lb = pressed;
				IO->keyboard.key[eKeyBoard::K_LBUTTON].pressed = pressed;
				IO->keyboard.key[eKeyBoard::K_LBUTTON].timeout = false;
				break;
			case M_MIDDLE:
				IO->mouse.mb = pressed;
				IO->keyboard.key[eKeyBoard::K_MBUTTON].pressed = pressed;
				IO->keyboard.key[eKeyBoard::K_MBUTTON].timeout = false;
				break;
			case M_RIGHT:
				IO->mouse.rb = pressed;
				IO->keyboard.key[eKeyBoard::K_RBUTTON].pressed = pressed;
				IO->keyboard.key[eKeyBoard::K_RBUTTON].timeout = false;
				break;
			}
		}
		//>> Обновление мыши
		void UpdateMouse(char wheel)
		{
			IO->mouse.wheel = wheel;
			if (wheel > 0)
			{
				IO->keyboard.key[eKeyBoard::K_WHEEL_UP].pressed = true;
				IO->keyboard.key[eKeyBoard::K_WHEEL_UP].timeout = false;
			}
			else if (wheel < 0)
			{
				IO->keyboard.key[eKeyBoard::K_WHEEL_DOWN].pressed = true;
				IO->keyboard.key[eKeyBoard::K_WHEEL_DOWN].timeout = false;
			}
		}
		//>> Обновление окна
		void UpdateWindow(const CStatusWindow * pState)
		{
			if (pState == nullptr) { _MBM(ERROR_PointerNone); return; }

			to_render.window          = const_cast<CStatusWindow*>(pState);
			to_render.window->Updated = false;
			IO->window                = *pState;
			IO->window.Updated        = true;

			if (settings.camera_aspect == 0)
				camera->SetAspectRatio((float)IO->window.client_width / IO->window.client_height);
		}
		//>> Обновление окна
		void UpdateWindow()
		{
			UpdateWindow(to_render.window);
		}
	
	protected:
		//>> Сброс состояний в конце кадра
		void ResetAtFrameEnd()
		{
			IO->keyboard.ResetTimeout();
			IO->keyboard.key[eKeyBoard::K_WHEEL_UP].Reset();
			IO->keyboard.key[eKeyBoard::K_WHEEL_DOWN].Reset();
			IO->window.Updated = false;
			IO->mouse.wheel    = 0;
			IO->mouse.center   = to_render.centered_cursor;	// нужно ?
			IO->mouse.show     = to_render.show_cursor;		// нужно ?
			interact->TipNextFrame();
		}

	public: //////////////////////////////////////////////////////// LOADING BLOCK
	
		//>> Запуск первичной загрузки
		void RunInitLoading(DEV3DDAT /*DEV3DBASE*/ * pDevice, const CStatusWindow * pState)
		{
			if (pDevice == nullptr) { _MBM(ERROR_PointerNone); return; }
			if (pState  == nullptr) { _MBM(ERROR_PointerNone); return; }

			dev = pDevice;

			Init();
			RunInitLoading_LoadBase();
			UpdateWindow(pState);
			RunInitLoading_UpdateScreenFromSettings();
			CallSkipFrame();

			UI.LoadingCall(settings.map_default.c_str(), DEFAULT_MAP_ID);
		}

	protected:
		//>> Запуск первичной загрузки - подготовка базовых объектов
		void RunInitLoading_LoadBase()
		{
			OpenSettings(DIRECTORY_GAME, BASE_SETTINGS_FILENAME);

			self     = player.self();
			camera   = self->camera;
			IO       = self->IO;
			keyev    = self->keyev;
			interact = self->interact;

			player.SetStatusNetwork(&network.status);

			shader->SetDevice(dev);
			shader->SetReferences(camera);
			shader->Load(DIRECTORY_GAME);		

			camera->SetDevice(dev);
			camera->SetFOV(settings.camera_fov);
			camera->SetNearPlane(settings.camera_near);
			camera->SetFarPlane(settings.camera_far);
			if (settings.camera_aspect) camera->SetAspectRatio(settings.camera_aspect);
			else                        camera->SetAspectRatio((float)IO->window.client_width / IO->window.client_height);
			camera->SetMouseSpeed(settings.camera_mousesp);
			camera->SetMoveSpeed(settings.camera_movesp);
			camera->SetRollSpeed(settings.camera_rollsp);
			camera->SetRadiusSpeed(settings.camera_radiussp);
			camera->SetMinCameraRadius(settings.camera_minR);
			camera->SetMaxCameraRadius(settings.camera_maxR);
			camera->Update(0);

			gamemap.Init(dev, shader, camera, IO);

			VECDATAP<CFontR> font;
			font.Create(eUIFontID::FONT_ENUM_MAX);
			for (uint32 i=0; i<eUIFontID::FONT_ENUM_MAX; i++)								{
				font[i]->SetDevice(dev);
				font[i]->SetShader(shader);
				font[i]->SetIO(IO);
				if (!font[i]->Load(DIRECTORY_GAME, settings.font[i].c_str()))
					 font[i]->Load(DIRECTORY_GAME, settings.font[FONT_DEFAULT].c_str());	}

			shared_ptr<CTexProfileR> base = make_shared<CTexProfileR>();
			base->Load(DIRECTORY_GAME, L"ui/menu.ini");
			base->SetShader(shader);
			base->SetDevice(dev);
			base->SetIO(IO);
			base->Prepare();

			// Звуковая база пока в рамках тестирования, т.к. не определена система менеджмента

			shared_ptr<CSoundT> sound = make_shared<CSoundT>(); // TODO: непоследовательная загрузка
			sound->Init();
			sound->SetMasterVolume(0.25f);
			for (int i=0; i<UISOUND_ENUM_MAX; i++)
			switch (i)
			{
			case UISOUND_LOADING:  sound->Load(DIRECTORY_GAME, settings.loading_screen_sound.c_str()); break;
			case UISOUND_TARGETED: sound->Load(DIRECTORY_GAME, L"kclick.wav");                         break;
			case UISOUND_CLICKED:  sound->Load(DIRECTORY_GAME, L"knock.wav");                          break;
			case UISOUND_WAKEDREAM_TEST: sound->Load(DIRECTORY_GAME, L"WakeDream.wav");                break;
			case UISOUND_CHAT_INCOMING: sound->Load(DIRECTORY_GAME, L"chat_incoming.wav");             break;
			};

			UI.InitUI(shader, IO, &network.status, font, base, sound);
			UI.SetBaseScreen(settings.ui_base_width, settings.ui_base_height);
			UI.ChatSetPlayerName(self->status.name.text);

			player.SetFont(font[FONT_PLNAME_SCENE]);
			player.self()->status.SetName(settings.player_name.c_str(), settings.name_height);

			network.client_ip = settings.client_ip;

			if (!settings.target_ip.size()) settings.target_ip = "127.0.0.1";

			network.server.SetPlayer(&player);
			network.server.SetUI(&UI);
			network.server.SetPlayersLimit(settings.server_max_players);

			network.client.SetPlayer(&player);
			network.client.SetUI(&UI);
		}
		//>> Запуск первичной загрузки - обновление параметров окна из файла настройки
		void RunInitLoading_UpdateScreenFromSettings()
		{
			to_render.window->mode = settings.mode;

			if (settings.window_width > 0 && settings.window_height > 0)
			{
				to_render.window->sys_width  = to_render.window_memory.sys_width  = settings.window_width;
				to_render.window->sys_height = to_render.window_memory.sys_height = settings.window_height;
			}

			if (settings.custom_backbuffer_width > 0 && settings.custom_backbuffer_height > 0)
			{
				to_render.use_custom_backbuffer = true;
				to_render.custom_backbuffer.client_width  = settings.custom_backbuffer_width;
				to_render.custom_backbuffer.client_height = settings.custom_backbuffer_height;
			}

			if (settings.window_posx >= 0 && settings.window_posy >= 0)
			{
				to_render.window->position_x = to_render.window_memory.position_x = settings.window_posx;
				to_render.window->position_y = to_render.window_memory.position_y = settings.window_posy;
			}
			
			to_render.reset_call = true;
		}
		
	protected:
		//>> Загрузчик карты, подготовка объектов в RAM
		void RunMapLoading(uint32 map_id, wstring filename)
		{
			if (dev == nullptr) { _MBM(ERROR_PointerNone); return; }

			gamemap.DeleteMap();			// очистим память от старой карты
			gamemap.Load(map_id, filename); // загрузим новую
			gamemap.Switch(map_id);			// переключимся на новую

			network.server.SetMap(gamemap.GetMap());
			network.client.SetMap(gamemap.GetMap());

			map = gamemap.GetMap();

			auto default_model_id = map->GetDefaultModelID();
			auto default_model    = map->data.model[default_model_id];
			for (auto & p : player) // set players' defaults
			{
				p->status.SetModel(default_model_id, default_model->box.maxCoord);
			}
		}
		//>> 
	//	void _TestFunc()
	//	{
	//		printf("\nEnter test");
	//
	//		UI.LoadingCall(settings.map_default.c_str(), DEFAULT_MAP_ID);
	//	}

	protected: ///////////////////////////////////////////////////// DRAWING PROC BLOCK

		//>> Показ окружения
		void DrawSkybox()
		{
			map->ShowSkybox();
		}
		//>> Показ интерфейса
		void DrawUI()
		{
			UI.EnableUIShader();

			//////////// Интерфейс ////////////

			if (!UI.EscapeMenuGetStatus())
			{
				UI.Interface(map->data.light.GetParallaxScale());
				UI.ShowCursorGame();
			}

			if (!UI.EscapeMenuGetStatus() && interact->status == INTMODE_IN_VIEW)
				UI.TipInteractPick();

			////////////    Чат    ////////////

			if (UI.ChatSwitchMenuCheck()) // проверка вызова на переключение
			{
				if (UI.ChatGetMenuStatus() && !UI.EscapeMenuGetStatus()) // меню было активно -> выключаем
				{
					UI.ChatSetMenuStatus(false);
				//	to_render.show_cursor = false;
					to_render.centered_cursor = true;
					camera->RestoreCameraMode();
					camera->SkipUpdate();					// исправляет необоснованное вращение
					self->status.LockGameActionsSub();		// разблокируем взаимодействие со сценой		
				}
				else if (!UI.EscapeMenuGetStatus()) // меню было не активно -> включаем
				{
					UI.ChatSetMenuStatus(true);
				//	to_render.show_cursor = true;
					to_render.centered_cursor = false;
					camera->SetCameraMode(CM_NOCONTROL);
					self->status.LockGameActionsAdd();		// блокируем взаимодействие со сценой
				}
			}

			if (!UI.EscapeMenuGetStatus())
			if (UI.ChatGetMenuStatus() || UI.ChatGetMenuStatusPassive())
				UI.Chat(); // показ окна чата

			//////////// ESC вызов ////////////

			if (UI.EscapeMenuSwitchCheck()) // проверка вызова на переключение
			{
				if (UI.EscapeMenuGetStatus()) // меню было активно -> выключаем
				{
					UI.EscapeMenuSetStatus(false);
				//	to_render.show_cursor = false;
					to_render.centered_cursor = true;
					camera->RestoreCameraMode();
					camera->SkipUpdate();					// исправляет необоснованное вращение
					self->status.LockGameActionsSub();		// разблокируем взаимодействие со сценой

					if (UI.ChatGetMenuStatusPassive()) UI.ChatSetMenuStatus(0);
				}
				else // меню было не активно -> включаем
				{
					UI.EscapeMenuSetStatus(true);
				//	to_render.show_cursor = true;
					to_render.centered_cursor = false;
					camera->SetCameraMode(CM_NOCONTROL);
					self->status.LockGameActionsAdd();		// блокируем взаимодействие со сценой

					if (UI.ChatGetMenuStatusPassive()) UI.ChatSetMenuStatus(0);
					else if (UI.ChatGetMenuStatus())
					{
						self->status.LockGameActionsSub();
						UI.ChatSetMenuStatus(0);
					}
				}
			}

			if (UI.EscapeMenuGetStatus()) switch (UI.EscapeMenu())
			{
			case CLICK_EVENT_EXIT_GAME:  to_render.exit_call = true; break;
			case CLICK_EVENT_FULLSCREEN: if (to_render.window->mode != WM_FULLSCREEN)						{
					if (to_render.window->mode == WM_WINDOWED || to_render.window->mode == WM_NOBORDERS)		// Запоминаем состояние окна
						to_render.MemoryWindowCondition();														// .
					to_render.window->mode = WM_FULLSCREEN;
					to_render.reset_call = true;															}
				break;
			case CLICK_EVENT_FULLWINDOW: if (to_render.window->mode != WM_FULLWINDOW)						{
					if (to_render.window->mode == WM_WINDOWED || to_render.window->mode == WM_NOBORDERS)		// Запоминаем состояние окна
						to_render.MemoryWindowCondition();														// .
					to_render.window->mode = WM_FULLWINDOW;
					to_render.reset_call = true;															}
				break;
			case CLICK_EVENT_WINDOWED:
				if (to_render.window->mode == WM_FULLSCREEN || to_render.window->mode == WM_FULLWINDOW)			// Возвращаем состояние окна
					to_render.RestoreWindowCondition();
					 if (to_render.window->mode != WM_WINDOWED) { to_render.window->mode = WM_WINDOWED;  to_render.reset_call = true; }
				else if (to_render.window->mode == WM_WINDOWED) { to_render.window->mode = WM_NOBORDERS; to_render.reset_call = true; } break;
			case CLICK_EVENT_SV_CONNECT:
				if (!network.client.Status())														{
					const char * IP   = settings.target_ip.c_str();
					const char * PORT = settings.target_port.c_str();
					if (!settings.target_port.size()) PORT = nullptr;
					if (network.client.Connect(IP, PORT, CONNECTION_TIME))
						UI.EscapeMenuChangeConnectText(true);										}
				else																				{
					network.client.Disconnect();
					UI.EscapeMenuChangeConnectText(false);											} break;
			case CLICK_EVENT_SV_WAKEUP:
				if (!network.server.Status())											{
					const char * IP   = settings.server_ip.c_str();
					const char * PORT = settings.server_port.c_str();
					if (!settings.server_port.size()) PORT = nullptr;
					if (network.server.Wakeup(IP, PORT, false))
						UI.EscapeMenuChangeWakeupText(true);							}
				else																	{
					network.server.Shutdown();
					UI.EscapeMenuChangeWakeupText(false);								} break;
			default: ;
			}

			///////////////////////////////////
			//////////// Показ FPS ////////////
			///////////////////////////////////

			if (settings.show_fps) UI.IndicateFPS();

			/////////// MENU CURSOR ///////////

			if (UI.EscapeMenuGetStatus()) UI.ShowCursorMenu();

			///////////////////////////////////

			UI.DisableUIShader();
		}
		//>> Показ загрузочного экрана
		bool DrawLoadingScreen()
		{
			if (!UI.LoadingScreenCheck()) return false;

			to_render.show_cursor = false;
			to_render.centered_cursor = false;
			camera->SetCameraMode(CM_NOCONTROL);

			UI.EnableUIShader();
			UI.LoadingScreen();
			UI.DisableUIShader();

			network.client.SendAtLoading();
			network.server.SendAtLoading();

			if ( UI.LoadingThreadCheckIsFinished() ) // Следим за окончанием загрузки
			{
				gamemap.LoadVRAM();
				auto spawn = map->obj.player[DEFAULT_PLAYER_SPAWN];

				network.client.SendAfterLoading();
				network.server.SendAfterLoading();

				UI.ChatHistorySet(L"", false, false); // сброс истории
				UI.ChatSetMenuStatus(false);
				UI.EscapeMenuSetStatus(false);
			//	to_render.show_cursor = true;
				to_render.centered_cursor = true;
				UpdateMouse(IO->window.center_x, IO->window.center_y); // избавляет от лишнего сдвига камеры
				if (network.status.Client() == false)
				{
					camera->ResetCamera   ( spawn->pos->P, spawn->pos->A, true );	player.Update();
					camera->SetCameraMode ( spawn->camera_mode );					player.Update();
				}
				else
				{
					camera->RestoreCameraMode(); // возврат в режим, определённый сервером
				}
				camera->ResetCameraMovements();
				camera->AddCameraRadius(1);
				self->status.LockGameActionsReset();
				IO->keyboard.Reset();
				IO->mouse.Reset(MR_WHEEL | MR_BUTTONS);

				return false;
			}
			return true;
		}
		//>> Показ игроков
		void DrawPlayers()
		{
			uint32 count   = player.Count();
			uint32 self_id = player.GetSelfID();

			map->EnableLightShader();
			for (uint32 i=0; i<count; i++)
				if (i == self_id && camera->IsFirstPerson()) continue; // себя не отрисовываем
				else
				{
					auto p = player[i];
					if ((p->status.InGame() || p->status.bot) && p->status.model.show)
					{
						auto position = p->status.model.pos;
						if (!interact->CheckVisibility(position)) continue; // не отрисовываем игроков, которых не видим
						map->data.model[ p->status.GetModelID() ]->ShowModel(position);
					}
				}
			map->DisableLightShader();
		}
		//>> Показ спрайтов и сценовых текстов
		void DrawSprites()
		{
			uint32 self_id = player.GetSelfID();
			uint32 players = player.Count();
			uint32 sprites = (uint32)map->obj.sprite.size();
			uint32 total   = players + sprites;
			auto & cam_pos = *camera->GetCameraPos();
			
			vector<pair<float, pair<eObjectType, uint32>>> distance;
			pair<eObjectType, uint32> info;

			distance.reserve(total);

			for (uint32 i=0; i<players; i++)
			{
				if (i == self_id && camera->IsFirstPerson()) // для 1-го лица свой текст не показываем
					continue;
				
				if (player[i]->status.InGame() &&
					player[i]->status.name.show && player[i]->status.model.show)
				{
					float dist = MathDistance(cam_pos, player[i]->status.name.pos->P);
					info.first = eObjectType::OBJECT_TEXT;
					info.second = i;
					distance.emplace_back(pair<float, pair<eObjectType, uint32>>(dist, info));
				}		
			}

			for (uint32 i=0; i<sprites; i++)
			{
				if (map->obj.sprite[i]->enable)
				{
					float dist = MathDistance(cam_pos, map->obj.sprite[i]->pos->P);
					info.first = eObjectType::OBJECT_SPRITE;
					info.second = i;
					distance.emplace_back(pair<float, pair<eObjectType, uint32>>(dist, info));
				}
			}

			total = (uint32)distance.size();
			std::sort(distance.begin(), distance.end(), std::greater<>());

			UI.EnableUIShader();		
			for (uint32 i=0; i<total; i++)
			{
				switch (distance[i].second.first)
				{
				case eObjectType::OBJECT_TEXT:
					player[distance[i].second.second]->ShowPlayerName();
					break;
				case eObjectType::OBJECT_SPRITE:
					map->obj.sprite[distance[i].second.second]->Show();
					break;
				}			
			}
			UI.DisableUIShader();
		}
		//>> Показ объектов моделей
		void DrawModels()
		{
			map->EnableLightShader();
			for (auto object : map->obj.model)
			{
				auto o = object.get();

				if (!o->enable) continue;							// объект отключен
				if (!interact->CheckVisibility(o->pos)) continue;	// не рендерим модели, которых не видим

				o->Show();
			}
			map->DisableLightShader();
		}

	protected: ///////////////////////////////////////////////////// CHECKING PROC BLOCK

		//>> Проверка игровых действий
		void CheckGameActions()
		{
			auto camera_mode = camera->GetCameraMode();
			
			{
				auto & chat_key = IO->keyboard.key [ keyev->ev[EKE_CHAT_MENU].key             ];
				auto & esc_key  = IO->keyboard.key [ keyev->ev[EKE_ESCAPE_MENU].key           ];
				auto & del_key  = IO->keyboard.key [ keyev->ev[EKE_CHAT_MENU_DELETE_CHAR].key ];

				if (chat_key.is_active_s())			// вызов переключения
					UI.ChatSwitchMenu();			// .

				if (esc_key.is_active_s())			// вызов переключения
					UI.EscapeMenuSwitch();			// .

				if (UI.ChatGetMenuStatus())	// удаление/добавление символа к текущему сообщению
				{
					if (del_key.pressed && !del_key.timeout && UI.chat_message.size()) UI.chat_message.pop_back();

					for (uint32 i=0; i < eKeyBoard::K_ENUM_MAX; i++)
						if (i >= K_SPACE || i == K_ENTER || i == K_0x0A)
							if (IO->keyboard.key[i].pressed && !IO->keyboard.key[i].timeout)
								if (IO->keyboard.key[i].wchar && UI.chat_message.size() < CHATMSGSZ)
								{
									if (UI.chat_message.size() < CHATMSGSZ - 1)
										UI.chat_message.push_back( IO->keyboard.key[i].wchar );
									else
									if (UI.chat_message.size() < CHATMSGSZ && (i == K_ENTER || i == K_0x0A))
										UI.chat_message.push_back( IO->keyboard.key[i].wchar );
								}
				}
			}

			// Игровые действия, когда не висим в чате, меню и т.п. //

			if (!self->status.LockGameActions()) 
			{
				///////////////////////////////////// HELPER ////////////////////////////////////////
				{
					auto & help_key = IO->keyboard.key[ keyev->ev[EKE_HELP_SHOW_STATUS].key ];

					UI.HelpStatus(help_key.pressed);
				}

				///////////////////////////////// PARALLAX SCALE ////////////////////////////////////
				{
					auto & inc_prlx_key = IO->keyboard.key [ keyev->ev[EKE_INCREASE_PARALLAX_SCALE].key ];
					auto & dec_prlx_key = IO->keyboard.key [ keyev->ev[EKE_DECREASE_PARALLAX_SCALE].key ];	

					if (inc_prlx_key.is_active())
					{
						if (inc_prlx_key.wchar == keyev->ev[EKE_INCREASE_PARALLAX_SCALE].wchar ||
							inc_prlx_key.wchar == keyev->ev[EKE_INCREASE_PARALLAX_SCALE_RU].wchar)
								map->data.light.IncreaseParallaxScale();
					}

					if (dec_prlx_key.is_active())
					{
						if (dec_prlx_key.wchar == keyev->ev[EKE_DECREASE_PARALLAX_SCALE].wchar ||
							dec_prlx_key.wchar == keyev->ev[EKE_DECREASE_PARALLAX_SCALE_RU].wchar)
								map->data.light.DecreaseParallaxScale();
					}
				}

				//////////////////////////////////// MOVEMENTS ////////////////////////////////////
				{
					auto & mov_forward = IO->keyboard.key[ keyev->ev[EKE_MOVE_FORWARD].key ];
					auto & mov_back    = IO->keyboard.key[ keyev->ev[EKE_MOVE_BACK].key    ];
					auto & mov_left    = IO->keyboard.key[ keyev->ev[EKE_MOVE_LEFT].key    ];
					auto & mov_right   = IO->keyboard.key[ keyev->ev[EKE_MOVE_RIGHT].key   ];
					auto & mov_up      = IO->keyboard.key[ keyev->ev[EKE_MOVE_UP].key      ];
					auto & mov_down    = IO->keyboard.key[ keyev->ev[EKE_MOVE_DOWN].key    ];

					auto cam_forward = camera->GetMoveInputForward();
					auto cam_right   = camera->GetMoveInputRight();
					auto cam_up      = camera->GetMoveInputUp();


					if (mov_forward.pressed)									{ //////// вперёд
						if (cam_forward == eCameraSignal::ZERO)
							camera->SetMoveInputForward(eCameraSignal::PLUS);	}
					else														{
						if (cam_forward == eCameraSignal::PLUS)
							camera->SetMoveInputForward(eCameraSignal::ZERO);	} 

					if (mov_back.pressed)										{ //////// назад
						if (cam_forward == eCameraSignal::ZERO)
							camera->SetMoveInputForward(eCameraSignal::MINUS);	}
					else														{
						if (cam_forward == eCameraSignal::MINUS)
							camera->SetMoveInputForward(eCameraSignal::ZERO);	} 

					if (mov_left.pressed)										{ //////// влево
						if (cam_right == eCameraSignal::ZERO)
							camera->SetMoveInputRight(eCameraSignal::MINUS);	}
					else														{
						if (cam_right == eCameraSignal::MINUS)
							camera->SetMoveInputRight(eCameraSignal::ZERO);		} 

					if (mov_right.pressed)										{ //////// вправо
						if (cam_right == eCameraSignal::ZERO)
							camera->SetMoveInputRight(eCameraSignal::PLUS);		}
					else														{
						if (cam_right == eCameraSignal::PLUS)
							camera->SetMoveInputRight(eCameraSignal::ZERO);		} 

					if (mov_up.pressed)											{ //////// вверх
						if (cam_up == eCameraSignal::ZERO)
							camera->SetMoveInputUp(eCameraSignal::PLUS);		}
					else														{
						if (cam_up == eCameraSignal::PLUS)
							camera->SetMoveInputUp(eCameraSignal::ZERO);		} 

					if (mov_down.pressed)										{ //////// вниз
						if (cam_up == eCameraSignal::ZERO)
							camera->SetMoveInputUp(eCameraSignal::MINUS);		}
					else														{
						if (cam_up == eCameraSignal::MINUS)
							camera->SetMoveInputUp(eCameraSignal::ZERO);		} 
				}
				
				///////////////////////////////// ROLLING CAMERA /////////////////////////////////
				{
					auto & roll_left  = IO->keyboard.key[ keyev->ev[EKE_ROLL_LEFT].key  ];
					auto & roll_right = IO->keyboard.key[ keyev->ev[EKE_ROLL_RIGHT].key ];

					auto cam_roll = camera->GetRollInput();

					if (roll_right.pressed)									{ //////// крен вправо
						if (cam_roll == eCameraSignal::ZERO)
							camera->SetRollInput(eCameraSignal::PLUS);		}
					else													{
						if (cam_roll == eCameraSignal::PLUS)
							camera->SetRollInput(eCameraSignal::ZERO);		} 

					if (roll_left.pressed)									{ //////// крен влево
						if (cam_roll == eCameraSignal::ZERO)
							camera->SetRollInput(eCameraSignal::MINUS);		}
					else													{
						if (cam_roll == eCameraSignal::MINUS)
							camera->SetRollInput(eCameraSignal::ZERO);		}
				}

				/////////////////////////////// SWITCH CAMERA MODES ///////////////////////////////
				{
					if (!interact->IsCameraBlockingMode())
					{
						auto & _1st_person = IO->keyboard.key[ keyev->ev[EKE_CAMERA_1ST_PERSON].key ];
						auto & _3rd_person = IO->keyboard.key[ keyev->ev[EKE_CAMERA_3RD_PERSON].key ];
						auto & _no_control = IO->keyboard.key[ keyev->ev[EKE_CAMERA_NO_CONTROL].key ];

						if (_1st_person.is_active_s()) camera->SetCameraMode(CM_1STPERSON);
						if (_3rd_person.is_active_s()) camera->SetCameraMode(CM_3RDPERSON);
						if (_no_control.is_active_s()) camera->SetCameraMode(CM_NOCONTROL);
					}

					auto & _fly_mode   = IO->keyboard.key[ keyev->ev[EKE_CAMERA_FLY_MODE].key   ];
					auto & _XZ_mov_fix = IO->keyboard.key[ keyev->ev[EKE_CAMERA_XZ_FIX].key     ];

					if (_fly_mode.is_active_s())   camera->SwitchFlyMode();
					if (_XZ_mov_fix.is_active_s()) camera->SwitchXZMoveFix();			
				}

				//////////////////////////////// ADD CAMERA RADIUS ////////////////////////////////
				if (!camera->IsFirstPerson() && camera_mode != CM_NOCONTROL)
				{
					auto & _add_radius = IO->keyboard.key[ keyev->ev[EKE_CAMERA_ADD_RADIUS].key ];
					auto & _sub_radius = IO->keyboard.key[ keyev->ev[EKE_CAMERA_SUB_RADIUS].key ];

					if (_add_radius.is_active()) camera->AddCameraRadius(+1);
					if (_sub_radius.is_active()) camera->AddCameraRadius(-1);
				}

				//////////////////////////////////// INTERACT /////////////////////////////////////
				{
					auto & _rotate = IO->keyboard.key[ keyev->ev[EKE_INTERACT_ROTATE_MODEL].key ];
					auto & _move   = IO->keyboard.key[ keyev->ev[EKE_INTERACT_MOVE_MODEL].key   ];

					interact->SetRotate(_rotate.pressed);
					interact->SetMove(_move.pressed);
				}
			}
		}
		//>> Проверка взаимодействия с объектами
		void CheckPicking()
		{
			bool client = network.status.Client();
			bool server = network.status.Server();

			if (client)
			{
				float t = network.client.sv_tick_o->GetInterpolation();

				for (auto & cur : map->obj.model)
				{
					auto o = cur.get();
					if (o->enable && o->dynamic)
						o->InterpolateWPOS(t);
				}
			}

			interact->SetNetClient(client);
			interact->SetNetServer(server);
			interact->Pick(map, self->status.LockGameActions());
			switch (interact->status)
			{
			case INTMODE_NO_CHANGES: break;
			case INTMODE_ROTATE:    //UI.GetSound()->PlayThread(UISOUND_WAKEDREAM_TEST, true, false); break;
			case INTMODE_TRANSLATE: //UI.GetSound()->PlayThread(UISOUND_WAKEDREAM_TEST, true, false); break;
			default: break;
			}

			if (server) // TODO: как с камерой, следует оптимизировать расчёты для случая с сервером
			{
				uint32 players = player.Count();
				for (uint32 i=0; i<players; i++)
				if (i != SELF_PL_SLOT)
				{
					if (player[i]->status.InGame() // || player[i]->status.bot   // TODO   с ботами пока не определено
						)
					{
						auto interact = player[i]->interact;
						interact->TipNextFrame();
						interact->SetNetClient(client);
						interact->SetNetServer(server);
						interact->Pick(map, player[i]->status.LockGameActions());
					}
					else
					{
						auto interact = player[i]->interact;
						interact->SetMove(false);	// сброс
						interact->SetRotate(false);	// сброс
					}
				}
			}
		}
		//>> Проверка вызова и запуск загрузки другой карты
		bool CheckMapReload()
		{
			if (UI.LoadingCallCheck())
			{
				wstring map = UI.LoadingGetMapNameToLoad();
				uint32 mpid = UI.LoadingGetMapIDToLoad();

				// Запускаем функцию загрузки в загрузочный поток и следим за окончанием из DrawLoadingScreen(); //

				if ( ! UI.LoadingRunThread ( &CWorld::RunMapLoading, this, mpid, map ) 
				   ) return false;

				return true;
			}
			return false;
		}

	public: //////////////////////////////////////////////////////// RUN

		//>> Hello, world !  ＼(＾▽＾)／
		void Run()
		{
			// Общий алгоритм отрисовки
			// 1. Обновление позиции камеры с расчётом новых матриц
			// 2. Показ skybox, установка IDX_TEX_ENVCUBE по умолчанию
			// 3. TODO: Мировая геометрия
			// 4. Все непрозрачные объекты и свет
			// 5. Прозрачные объекты, включая текст на 3d-сцене (от дальнего к ближнему)
			// 6. Зеркала с отражениями объектов и света (от дальнего зеркала к ближнему)
			// 7. UI интерфейс

			if (CheckSkipFrame()) return;
			if (CheckMapReload()) ;
			if (DrawLoadingScreen()) return;

			network.ClientListen(CLIENT_LISTEN_TIME);
			network.ServerListen(SERVER_LISTEN_TIME);

			player.Update();
			DrawSkybox();
			
			CheckGameActions();
			CheckPicking();

			DrawModels();
			DrawPlayers();

		/*	light.EnableLightShader();
			DrawTestMirror();
			light.DisableLightShader(); //*/

		/*	UI.EnableUIShader();
			static CText test(L"ГРАДИЕНТ", 10, WPOS(0, 10, 0, 0, 0, 0), COLORVEC(1, 1, 1, 0.3f), COLORVEC(1, NULL, NULL, 0.8f), false);
			test.SetTextConstant();
			UI.CUIBase::font[FONT_DEFAULT].ShowText(test);
			UI.DisableUIShader(); //*/

			DrawSprites();
			DrawUI();

			network.client.SendPlayerStatus();
			network.server.SendWorldStatus();

			ResetAtFrameEnd();
		}

	protected: ///////////////////////////////////////////////////// LOAD SETTINGS BLOCK

		//>> Открытие файла первичных настроек
		void OpenSettings(const wchar_t* gamePath, const wchar_t* filename)
		{
			wchar_t syspath[256], error[256], p1[256], p2[256];
			GetCurrentDirectory(256, syspath);

			wchar_t *p = nullptr;

			wsprintf(error, L"%s %s", ERROR_OpenFile, filename);
			wsprintf(p1, L"%s\\%s\\%s", gamePath, DIRECTORY_DATAFOLDER, filename);
			wsprintf(p2, L"%s\\%s\\%s", syspath, DIRECTORY_DATAFOLDER, filename);

			if (p = LoadFileCheck64((int64)p1, (int64)p2)) LoadSettings(p);
			else _MBM(error);
		}
		//>> Считывание первичных настроек
		void LoadSettings(const wchar_t* filepath)
		{									
			TBUFFER <byte, int32> file;								//wprintf(L"\nReading %s", filepath);
			ReadFileInMemory(filepath, file, 0);					if (file.buf == nullptr) return;
			byte* fpbuf = file.buf;
			byte* fpend = fpbuf + file.count;

			for (; fpbuf != fpend; fpbuf++)
			{
				CODESKIPCOMMENTS

				CODECOMPARESETTING("map_default",           SET_MAP_DEFAULT)
				CODECOMPARESETTING("font_default",          SET_FONT_DEFAULT)
				CODECOMPARESETTING("font_chat",             SET_FONT_CHAT)
				CODECOMPARESETTING("font_loading_screen",   SET_FONT_LOADING_SCREEN)
				CODECOMPARESETTING("font_escape_menu",      SET_FONT_ESCAPE_MENU)
				CODECOMPARESETTING("font_fps",              SET_FONT_FPS)
				CODECOMPARESETTING("font_pl_name_onscene",  SET_FONT_PLNAME_SCENE)
				CODECOMPARESETTING("loading_screen_sound",  SET_SOUND_LOADING_SCREEN)
				CODECOMPARESETTING("player_name",           SET_PLAYER_NAME)

				CODECOMPARESETTING("resolution",            SET_RESOLUTION)
				CODECOMPARESETTING("fullscreen",            SET_FULLSCREEN)
				CODECOMPARESETTING("custom_backbuffer",     SET_BACKBUFFER)
				CODECOMPARESETTING("window_position",       SET_WINDOW_POS)
				CODECOMPARESETTING("ui_base_screen",        SET_UI_SCREEN)
				CODECOMPARESETTING("show_fps",				SET_SHOW_FPS)
				CODECOMPARESETTING("camera_FOV",            SET_CAM_FOV)
				CODECOMPARESETTING("camera_plane",          SET_CAM_PLANE)
				CODECOMPARESETTING("camera_aspect_ratio",   SET_CAM_ASPECT)
				CODECOMPARESETTING("camera_mouse_speed",    SET_CAM_MOUSESP)
				CODECOMPARESETTING("camera_move_speed",     SET_CAM_MOVESP)
				CODECOMPARESETTING("camera_roll_speed",     SET_CAM_ROLLSP)
				CODECOMPARESETTING("camera_radius_speed",   SET_CAM_RADSP)
				CODECOMPARESETTING("camera_radius",         SET_CAM_RADIUS)
				CODECOMPARESETTING("name_height",           SET_NAME_HEIGHT)

				CODECOMPARESETTING("server_max_players",    SET_SV_MAX_PL)
				CODECOMPARESETTING("target_ip",             SET_TARGET_IP)
				CODECOMPARESETTING("target_port",           SET_TARGET_PORT)
				CODECOMPARESETTING("server_ip",             SET_SERVER_IP)
				CODECOMPARESETTING("server_port",           SET_SERVER_PORT)
				CODECOMPARESETTING("client_ip",             SET_CLIENT_IP)
			}
		}
		//>> Считывание первичных настроек (sub)
		void LoadSettings(byte * fpbuf, eInitWorldSettings type)
		{
			char str[256];
			uint32 n, n2;
			float f, f2;

			CODESKIPSPACE
			ZeroMemory(str, 256);

			if (type < SET_WSTRING_SETTINGS_SEPARATOR)
			{
				CODEREADSTRINGVALUE

				switch (type)
				{
				case SET_RESOLUTION:  sscanf_s(str, "%i, %i", &n, &n2); settings.window_width = n;            settings.window_height = n2;            break;
				case SET_FULLSCREEN:  sscanf_s(str, "%i",     &n);      settings.mode = (eWindowMode)n;												  break;
				case SET_BACKBUFFER:  sscanf_s(str, "%i, %i", &n, &n2); settings.custom_backbuffer_width = n; settings.custom_backbuffer_height = n2; break;
				case SET_WINDOW_POS:  sscanf_s(str, "%i, %i", &n, &n2); settings.window_posx = n;             settings.window_posy = n2;              break;
				case SET_UI_SCREEN:   sscanf_s(str, "%i, %i", &n, &n2); settings.ui_base_width = n;           settings.ui_base_height = n2;           break;
				case SET_SHOW_FPS:    sscanf_s(str, "%i",     &n);      settings.show_fps = _BOOL(n);										          break;
				case SET_CAM_FOV:     sscanf_s(str, "%f",     &f);      settings.camera_fov = f;                                                      break;
				case SET_CAM_PLANE:   sscanf_s(str, "%f, %f", &f, &f2); settings.camera_near = f;             settings.camera_far = f2;               break;			
				case SET_CAM_ASPECT:  sscanf_s(str, "%f:%i",  &f, &n);  settings.camera_aspect = f/n;                                                 break;
				case SET_CAM_MOUSESP: sscanf_s(str, "%f",     &f);      settings.camera_mousesp = f;                                                  break;
				case SET_CAM_MOVESP:  sscanf_s(str, "%f",     &f);      settings.camera_movesp = f;                                                   break;
				case SET_CAM_ROLLSP:  sscanf_s(str, "%f",     &f);      settings.camera_rollsp = f;                                                   break;
				case SET_CAM_RADSP:   sscanf_s(str, "%f",     &f);      settings.camera_radiussp = f;                                                 break;
				case SET_CAM_RADIUS:  sscanf_s(str, "%f, %f", &f, &f2); settings.camera_minR = f;             settings.camera_maxR = f2;              break;
				case SET_NAME_HEIGHT: sscanf_s(str, "%f",     &f);      settings.name_height = f;                                                     break;
				case SET_SV_MAX_PL:   sscanf_s(str, "%i",     &n);      settings.server_max_players = n;                                              break;
				}
			/*	switch (type)
				{
				case SET_RESOLUTION:     printf("\n- resolution %ix%i", settings.window_width, settings.window_height); break;
				case SET_FULLSCREEN:     printf("\n- fullscreen %i", (int)settings.mode); break;
				case SET_BACKBUFFER:     printf("\n- backbuffer %ix%i", settings.custom_backbuffer_width, settings.custom_backbuffer_height); break;
				case SET_WINDOW_POS:     printf("\n- window_pos %i, %i", settings.window_posx, settings.window_posy); break;
				case SET_UI_SCREEN:      printf("\n- ui_base    %ix%i", settings.ui_base_width, settings.ui_base_height); break;
				case SET_SHOW_FPS:       printf("\n- show_fps   %i", settings.show_fps); break;
				case SET_CAM_FOV:        printf("\n- camera_fov     %f", settings.camera_fov); break;
				case SET_CAM_PLANE:      printf("\n- camera_plane   %f, %f", settings.camera_near, settings.camera_far); break;
				case SET_CAM_ASPECT:     printf("\n- camera_aspect  %f", settings.camera_aspect); break;
				case SET_CAM_MOUSESP:    printf("\n- camera_mousesp %f", settings.camera_mousesp); break;
				case SET_CAM_MOVESP:     printf("\n- camera_movesp  %f", settings.camera_movesp); break;
				case SET_CAM_ROLLSP:     printf("\n- camera_rollsp  %f", settings.camera_rollsp); break;
				case SET_CAM_RADSP:      printf("\n- camera_radsp   %f", settings.camera_radiussp); break;
				case SET_CAM_RADIUS:     printf("\n- camera_radius  %i, %i", settings.camera_minR, settings.camera_maxR); break;
				case SET_NAME_HEIGHT:    printf("\n- name_height    %f", settings.name_height); break;
				case SET_SV_MAX_PL:      printf("\n- server_max_pl  %i", settings.server_max_players); break;
				} //*/
			}
			else if (type < SET_STRING_SETTINGS_SEPARATOR) // SET_WSTRING_SETTINGS_SEPARATOR
			{			
				CODEREADSTRINGWTEXT

				switch (type)
				{
				case SET_MAP_DEFAULT:          settings.map_default.assign((wchar_t*)str);               break;
				case SET_FONT_DEFAULT:         settings.font[FONT_DEFAULT].assign((wchar_t*)str);        break;
				case SET_FONT_CHAT:            settings.font[FONT_CHAT].assign((wchar_t*)str);           break;
				case SET_FONT_LOADING_SCREEN:  settings.font[FONT_LOADING_SCREEN].assign((wchar_t*)str); break;
				case SET_FONT_ESCAPE_MENU:     settings.font[FONT_ESCAPE_MENU].assign((wchar_t*)str);    break;
				case SET_FONT_FPS:             settings.font[FONT_FPS].assign((wchar_t*)str);            break;
				case SET_FONT_PLNAME_SCENE:    settings.font[FONT_PLNAME_SCENE].assign((wchar_t*)str);	 break;
				case SET_SOUND_LOADING_SCREEN: settings.loading_screen_sound.assign((wchar_t*)str);      break;		
				case SET_PLAYER_NAME:          settings.player_name.assign((wchar_t*)str);               break;
				}
			/*	switch (type)
				{
				case SET_MAP_DEFAULT:          wprintf(L"\n- map_default         %s", settings.map_default.c_str());               break;
				case SET_FONT_DEFAULT:         wprintf(L"\n- font_default        %s", settings.font[FONT_DEFAULT].c_str());        break;
				case SET_FONT_CHAT:            wprintf(L"\n- font_chat           %s", settings.font[FONT_CHAT].c_str());           break;
				case SET_FONT_LOADING_SCREEN:  wprintf(L"\n- font_loading_screen %s", settings.font[FONT_LOADING_SCREEN].c_str()); break;
				case SET_FONT_ESCAPE_MENU:     wprintf(L"\n- font_escape_menu    %s", settings.font[FONT_ESCAPE_MENU].c_str());    break;
				case SET_FONT_FPS:             wprintf(L"\n- font_fps            %s", settings.font[FONT_FPS].c_str());            break;
				case SET_FONT_PLNAME_SCENE:    wprintf(L"\n- font_pl_name_scene  %s", settings.plname_scene_font.c_str());         break;
				case SET_SOUND_LOADING_SCREEN: wprintf(L"\n- loading_sound       %s", settings.loading_screen_sound.c_str());      break;
				case SET_PLAYER_NAME:          wprintf(L"\n- player_name         %s", settings.player_name.c_str());               break;
				} //*/
			}
			else // SET_STRING_SETTINGS_SEPARATOR
			{
				CODEREADSTRINGTEXT

				switch (type)
				{
				case SET_TARGET_IP:			settings.target_ip.assign(str);		break;
				case SET_TARGET_PORT:       settings.target_port.assign(str);	break;
				case SET_SERVER_IP:			settings.server_ip.assign(str);		break;
				case SET_SERVER_PORT:		settings.server_port.assign(str);	break;
				case SET_CLIENT_IP:			settings.client_ip.assign(str);		break;
				}
			/*	switch (type)
				{
				case SET_TARGET_IP:   printf("\n- target_ip           %s", settings.target_ip.c_str());   break;
				case SET_TARGET_PORT: printf("\n- target_port         %s", settings.target_port.c_str()); break;
				case SET_SERVER_IP:   printf("\n- server_ip           %s", settings.server_ip.c_str());   break;
				case SET_SERVER_PORT: printf("\n- server_port         %s", settings.server_port.c_str()); break;
				case SET_CLIENT_IP:   printf("\n- client_ip           %s", settings.client_ip.c_str());   break;
				} //*/
			}
		}

	public: //////////////////////////////////////////////////////// SUPPORT BLOCK

		//>> test dx9 light  / deprecated /
		void InitTestDX9light()
		{
			D3DLIGHT9 light, point_light, spot_light;	// create the light struct

			ZeroMemory(&light, sizeof(light));
			ZeroMemory(&point_light, sizeof(point_light));
			ZeroMemory(&spot_light, sizeof(spot_light));

			light.Type = D3DLIGHT_DIRECTIONAL;    // make the light type 'directional light'
			light.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);    // set the light's color
			light.Direction = D3DXVECTOR3(6.0f, 0.0f, 0.0f);

			point_light.Type = D3DLIGHT_POINT;	// make the light type 'point light'
			point_light.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
			point_light.Position = D3DXVECTOR3(0.0f, 5.0f, 0.0f);
			point_light.Range = 100.0f;			// a range of 100								Attenuation = 1 / (att0 + att1 * range + att2 * range^2)
			point_light.Attenuation0 = 0.0f;	// no constant inverse attenuation
			point_light.Attenuation1 = 0.125f;	// only .125 inverse attenuation
			point_light.Attenuation2 = 0.0f;	// no square inverse attenuation

			spot_light.Type = D3DLIGHT_SPOT;	// make the light type 'spot light'
			spot_light.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
			spot_light.Position = D3DXVECTOR3(0.0f, 5.0f, 0.0f);
			spot_light.Direction = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
			spot_light.Range = 100.0f;					// a range of 100
			spot_light.Attenuation0 = 0.0f;				// no constant inverse attenuation
			spot_light.Attenuation1 = 0.125f;			// only .125 inverse attenuation
			spot_light.Attenuation2 = 0.0f;				// no square inverse attenuation
			spot_light.Phi = D3DXToRadian(40.0f);		// set the outer cone
			spot_light.Theta = D3DXToRadian(20.0f);		// set the inner cone
			spot_light.Falloff = 1.0f;					// use the typical falloff

			dev->d3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
			dev->d3ddev->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(60, 60, 60));
			dev->d3ddev->SetRenderState(D3DRS_SPECULARENABLE, TRUE);

			dev->d3ddev->SetLight(0, &light);
			dev->d3ddev->SetLight(1, &point_light);
			dev->d3ddev->SetLight(2, &spot_light);
			//	dev->d3ddev->LightEnable(0, TRUE);
			//	dev->d3ddev->LightEnable(1, TRUE);
			//	dev->d3ddev->LightEnable(2, TRUE);

			dev->d3ddev->LightEnable(0, FALSE);	// test DX9 light
			dev->d3ddev->LightEnable(1, TRUE);
			dev->d3ddev->LightEnable(2, FALSE);
		} //*/
		//>>
		void OnLostDevice()
		{
			shader->OnLostDevice();
		}
		//>>
		void OnResetDevice()
		{
			IO->mouse.Reset();
			shader->OnResetDevice();

			InitTestDX9light();
		}


	/*	void DrawTestMirror()
		{
			CModelT & model_box = model[1];
			CModelT & model_mirror = model[2];

			MATH3DVEC p(0, 0, 14);   // позиция ящиков
			MATH3DVEC m(0, 0, 7);    // позиция зеркала
			MATH3DVEC a(30, 30, 0);  // поворот зеркала

			//	model_box.SetModelStatic(false);
			model_box.SetPosInstant(WPOS(p.x, p.y, p.z, 0, 0, 0, 1, 1, 1));
			model_box.ShowModel();
			model_box.SetPosInstant(WPOS(p.x, p.y + 2, -7, 0, 0, 0, 1, 1, 1));
			model_box.ShowModel();

			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			model_mirror.SetPosInstant(WPOS(m.x, m.y, m.z, a.x, a.y, a.z, 1, 1, 1));
			model_mirror.ShowModel();
			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

			model_box.SetPosInstant(WPOS(p.x + 6, p.y - 1, -7, 0, 0, 0, 1, 1, 1));	// Ещё ящики
			model_box.ShowModel();
			model_box.SetPosInstant(WPOS(p.x + 6, p.y - 6, p.z, 0, 0, 0, 1, 1, 1));	// у этого отражение тоже
			model_box.ShowModel();

			////////////////// Рисуем зеркало только в трафарет, буфер без изменений //////////////////////

			d3ddev->SetRenderState(D3DRS_STENCILENABLE, TRUE);
			d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);			// always pass
			d3ddev->SetRenderState(D3DRS_STENCILREF, 0x1);					// эталонное значение
			d3ddev->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
			d3ddev->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
			d3ddev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			d3ddev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);	// записывается эталонное значение

			d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);		// просчёт глубины убран, т.к. сейчас мы заняты трафаретом
			d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);	// кадр не меняю, только трафарет  Result = Src*(0,0,0,0) + Dest*(1,1,1,1) = Dest

			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);			// рисую в трафарет отражающую сторону зеркала
			model_mirror.SetPosInstant(WPOS(m.x, m.y, m.z, a.x, a.y, a.z, 1, 1, 1));
			model_mirror.ShowModel();
			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

			d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);		// просчёт глубины включен

			///////////////////////////////////////////////////////////////////////////////////////////////

			///////////////////////////////// Рисуем отражение коробки ////////////////////////////////////

			d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);			// (ref & mask) == (value & mask)
			d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);		// (0x1 & 0xffffffff) == (value & 0xffffffff)
			// (0x1) == (value & 0xffffffff)
			MATH3DMATRIX  mirror_plane_rotation_matrix = MathRotateMatrix(a);
			MATH3DVEC     mirror_plane_normal = MathVecTransformNormal(MATH3DVEC(0, 0, 1), mirror_plane_rotation_matrix);
			MATH3DPLANE   mirror_plane = MATH3DPLANE(m, MathNormalizeVec(mirror_plane_normal));

			MATH3DMATRIX  matReflect = MathReflectMatrix(mirror_plane);

			// Очистим z-буфер, чтобы зеркало не загораживало отражение
			// Если очищать здесь Z-буфер, все последующие объекты, нарисованные на сцене,
			// будут видны сквозь ранее нарисованные объекты, поэтому следует рисовать
			// отражения последними

			d3ddev->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

			// Смешиваем зеркало и отражение

			//d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			//d3ddev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_DESTCOLOR);	// Result = Src * Dest + Dest * (0,0,0,0) = Src * Dest
			//d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);		// Result = Src * 1 + Dest * 1 = Src + Dest
			d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			//d3ddev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);		// Result = Src
			//d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

			//d3ddev->SetClipPlane(0, FLOATCAST(&mirror_plane));
			//d3ddev->SetRenderState(D3DRS_CLIPPLANEENABLE, TRUE);

			eShaderID cur_vs = shader.GetCurVS();
			eShaderID cur_ps = shader.GetCurPS();

			if (cur_vs == SHADER_NONE)
			{
				d3ddev->SetTransform(D3DTS_VIEW, matReflect * (*camera.GetViewMatrixPtr()));
			}
			else
			{
				SHADER_HANDLE mat_viewproj = shader.exdata[cur_vs].handle[CONST_MAT_VIEWPROJ];

				MATH3DMATRIX matViewProj = matReflect * (*camera.GetViewProjMatrixPtr());
				//MATH3DMATRIX matViewProj = matReflect * camera.GetViewMatrix() *  camera.GetProjMatrix();
				shader.SetMatrix(cur_vs, mat_viewproj, matViewProj);
			}

			if (cur_ps != SHADER_NONE)
			{
				SHADER_HANDLE vec_camera = shader.exdata[cur_ps].handle[CONST_VEC_CAMERA];
				MATH3DVEC vecCamera = (*camera.GetCameraPosPtr()) * matReflect;
				shader.SetFloatArray(cur_ps, vec_camera, vecCamera, 3);
			}

			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			model_box.SetPosInstant(WPOS(p.x, p.y, p.z, 0, 0, 0, 1, 1, 1));
			model_box.ShowModel();
			model_box.SetPosInstant(WPOS(p.x + 6, p.y - 6, p.z, 0, 0, 0, 1, 1, 1));
			model_box.ShowModel();
			d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

			// Возвращаем нормальные настройки
			d3ddev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
			d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			d3ddev->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
		} //*/
};

}

#endif // _WORLD_H